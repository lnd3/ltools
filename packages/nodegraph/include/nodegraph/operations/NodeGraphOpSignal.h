#pragma once
#include "nodegraph/core/NodeGraphBase.h"

#include "logging/LoggingAll.h"

#include "hid/KeyboardPiano.h"
#include "hid/Midi.h"

#include "audio/PortAudio.h"
#include "audio/AudioUtils.h"

#include "math/MathFunc.h"

#include <string>
#include <vector>
#include <map>
#include <typeinfo>
#include <type_traits>
#include <math.h>
#include <random>
#include <unordered_set>

namespace l::nodegraph {

    class GraphSignalBase : public NodeGraphOp {
    public:

        static const int8_t mNumDefaultInputs = 4;
        static const int8_t mNumDefaultOutputs = 1;

        GraphSignalBase(NodeGraphBase* node, std::string_view name, int32_t numInputs = 0, int32_t numOutputs = 0, int32_t numConstants = 0) :
            NodeGraphOp(node, mNumDefaultInputs + numInputs, mNumDefaultOutputs + numOutputs, numConstants),
            mName(name)
        {}

        std::string defaultInStrings[mNumDefaultInputs] = { "Reset", "Freq", "Volume", "Smooth"};
        std::string defaultOutStrings[mNumDefaultOutputs] = { "Out" };

        virtual ~GraphSignalBase() = default;
        virtual void Reset() override final;
        virtual void Process(int32_t numSamples, std::vector<NodeGraphInput>& inputs, std::vector<NodeGraphOutput>& outputs) override final;
        virtual bool IsDataVisible(int8_t) override { return true; }
        virtual bool IsDataEditable(int8_t) override { return true; }
        virtual std::string_view GetInputName(int8_t inputChannel) override final {
            if (inputChannel >= mNumDefaultInputs) return GetInputNameExtra(inputChannel - mNumDefaultInputs);
            if (inputChannel >= 0) return defaultInStrings[static_cast<uint8_t>(inputChannel)];
            return "";
        }
        virtual std::string_view GetOutputName(int8_t outputChannel) override final {
            if (outputChannel >= mNumDefaultOutputs) return GetOutputNameExtra(outputChannel - mNumDefaultOutputs);
            return defaultOutStrings[outputChannel];
        }
        virtual std::string_view GetName() override {
            return mName;
        }

        virtual std::string_view GetInputNameExtra(int8_t) { return ""; };
        virtual std::string_view GetOutputNameExtra(int8_t) { return ""; };
        virtual void ResetSignal() {};
        virtual void UpdateSignal(std::vector<NodeGraphInput>&, std::vector<NodeGraphOutput>&) {};
        virtual float GenerateSignal(float deltaTime, float freq, float deltaPhase) = 0;
    protected:
        std::string mName;

        float mReset = 0.0f;
        float mFreq = 0.0f;
        float mVolume = 0.0f;
        float mSmooth = 0.5f;
        float mSignal = 0.0f;
        float mWave = 0.0f;
        float mDeltaPhase = 0.0f;
        float mDeltaTime = 0.0f;
        float mVolumeTarget = 0.0f;
        float mSamplesUntilUpdate = 0.0f;
        float mUpdateSamples = 256.0f;
    };

    /*********************************************************************/
    class GraphSignalSine2 : public GraphSignalBase {
    public:
        GraphSignalSine2(NodeGraphBase* node) :
            GraphSignalBase(node, "Sine 2", 2)
        {}
        std::string extraString[2] = { "Fmod", "Phase" };

        virtual ~GraphSignalSine2() = default;
        virtual std::string_view GetInputNameExtra(int8_t extraInputChannel) override {
            if(extraInputChannel < 2) return extraString[static_cast<uint8_t>(extraInputChannel)];
            return "";
        }
        void ResetSignal() override;
        void UpdateSignal(std::vector<NodeGraphInput>& inputs, std::vector<NodeGraphOutput>& outputs) override;
        float GenerateSignal(float deltaTime, float freq, float deltaPhase) override;
    protected:
        float mFmod = 0.0f;
        float mPmod = 0.0f;
        float mPhase = 0.0f;
        float mPhaseFmod = 0.0f;
    };

    /*********************************************************************/
    struct LowpassType {
        double x1 = 0.0;
        double y1 = 0.0;
        double a = 0.0;
        double b = 0.0;
    };

    struct WaveformBlit {
        double phase = 0.0;    /* phase accumulator */
        double aNQ = 0.0;      /* attenuation at nyquist */
        double curcps = 0.0;   /* current frequency, updated once per cycle */
        double curper = 0.0;   /* current period, updated once per cycle */
        LowpassType leaky; /* leaky integrator */
        double N = 0.0;        /* # partials */
        double a = 0.0;        /* dsf parameter which controls roll-off */
        double aN = 0.0;       /* former to the N */
    };

    class GraphSignalSaw2 : public GraphSignalBase {
    public:
        GraphSignalSaw2(NodeGraphBase* node) :
            GraphSignalBase(node, "Saw 2", 2)
        {}
        std::string extraString[2] = { "Attenuation", "Cutoff" };

        virtual ~GraphSignalSaw2() = default;
        virtual std::string_view GetInputNameExtra(int8_t extraInputChannel) override {
            if (extraInputChannel < 2) return extraString[static_cast<uint8_t>(extraInputChannel)];
            return "";
        }
        void ResetSignal() override;
        void UpdateSignal(std::vector<NodeGraphInput>& inputs, std::vector<NodeGraphOutput>& outputs) override;
        float GenerateSignal(float deltaTime, float freq, float deltaPhase) override;

        void InitSaw(WaveformBlit* b, double aNQ, double cutoff)
        {
            b->phase = 0.0;
            b->aNQ = aNQ;
            b->curcps = 0.0;
            b->curper = 0.0;
            InitLowpass(&b->leaky, cutoff);
        }

        void UpdateSaw(WaveformBlit* b, double aNQ, double cutoff) {
            b->aNQ = aNQ;
            UpdateLowpass(&b->leaky, cutoff + 0.00001);
        }

        /* Returns a sawtooth computed from a leaky integration
         * of a DSF bandlimited impulse train.
         *
         * cps (cycles per sample) is the fundamental
         * frequency: 0 -> 0.5 == 0 -> nyquist
         */

        double ProcessSaw(WaveformBlit* b, double cps) {
            double P2, beta, Nbeta, cosbeta, n, d, blit, saw;

            if (b->phase >= 1.0 || b->curcps == 0.0)
            {
                /* New cycle, update frequency and everything
                 * that depends on it
                 */

                if (b->phase >= 1.0)
                    b->phase -= 1.0;
                double cpsClamped = l::math::functions::clamp(cps, 2.0 / 44100, 0.5);
                b->curcps = cpsClamped;        /* this cycle\'s frequency */
                b->curper = 1.0 / cpsClamped;  /* this cycle\'s period */

                P2 = b->curper / 2.0;
                b->N = 1.0 + l::math::functions::floor(P2); /* # of partials incl. dc */

                /* find the roll-off parameter which gives
                 * the desired attenuation at nyquist
                 */

                b->a = l::math::functions::pow(b->aNQ, 1.0 / P2);
                b->aN = l::math::functions::pow(b->a, b->N);
            }

            beta = 2.0 * l::math::constants::PI * b->phase;

            Nbeta = b->N * beta;
            cosbeta = l::math::functions::cos(beta);

            /* The dsf blit is scaled by 1 / period to give approximately the same
             * peak-to-peak over a wide range of frequencies.
             */

            n = 1.0 -
                b->aN * l::math::functions::cos(Nbeta) -
                b->a * (cosbeta - b->aN * l::math::functions::cos(Nbeta - beta));
            d = b->curper * (1.0 + b->a * (-2.0 * cosbeta + b->a));

            b->phase += b->curcps; /* update phase */

            blit = n / d - b->curcps; /* This division can only fail if |a| == 1.0
               * Subtracting the fundamental frq rids of DC
               */

            saw = ProcessLowpass(&b->leaky, blit); /* shape blit spectrum into a saw */

            return 4.0 * saw;
        }

        void InitLowpass(LowpassType* lp, double cutoff) {
            lp->x1 = lp->y1 = 0.0;
            UpdateLowpass(lp, cutoff);
        }

        void UpdateLowpass(LowpassType* lp, double cutoff) {
            double Omega = l::math::functions::atan(l::math::constants::PI * cutoff);
            lp->a = -(1.0 - Omega) / (1.0 + Omega);
            lp->b = (1.0 - lp->b) / 2.0;
        }

        double ProcessLowpass(LowpassType* lp, double x) {
            double y;
            y = lp->b * (x + lp->x1) - lp->a * lp->y1;
            lp->x1 = x;
            lp->y1 = y;
            return y;
        }

    protected:
        double mAttenuation = 0.0f;
        double mCutoff = 0.0f;

        WaveformBlit mSaw;
    };

    /*********************************************************************/
    class GraphSignalSine : public NodeGraphOp {
    public:
        GraphSignalSine(NodeGraphBase* node) :
            NodeGraphOp(node, 6, 1)
        {}

        std::string defaultInStrings[6] = { "Freq", "Volume", "Fmod", "Phase", "Smooth", "Reset"};
        std::string defaultOutStrings[1] = { "Out"};

        virtual ~GraphSignalSine() = default;
        virtual void Reset() override;
        virtual void Process(int32_t numSamples, std::vector<NodeGraphInput>& inputs, std::vector<NodeGraphOutput>& outputs) override;
        virtual bool IsDataVisible(int8_t) override { return true; }
        virtual bool IsDataEditable(int8_t) override { return true; }
        virtual std::string_view GetInputName(int8_t inputChannel) override {
            return defaultInStrings[inputChannel];
        }
        virtual std::string_view GetOutputName(int8_t outputChannel) override {
            return defaultOutStrings[outputChannel];
        }
        virtual std::string_view GetName() override {
            return "Sine";
        }
    protected:
        double mFreq = 0.0;
        float mVolume = 0.0f;
        double mFmod = 0.0;
        double mPmod = 0.0;
        float mReset = 0.0f;

        double mWave = 0.0;
        double mDeltaTime = 0.0;
        float mVol = 0.0f;

        double mPhase = 0.0;
        double mPhaseFmod = 0.0;

        float mSamplesUntilUpdate = 0.0f;
    };

    /*********************************************************************/
    class GraphSignalSineFM : public NodeGraphOp {
    public:
        GraphSignalSineFM(NodeGraphBase* node) :
            NodeGraphOp(node, 9, 1)
        {}

        std::string defaultInStrings[9] = { "Freq", "Volume", "Fmod", "FmodFreq", "FmodVol", "FmodOfs", "FmodGain", "Smooth", "Reset"};
        std::string defaultOutStrings[1] = { "Out" };

        virtual ~GraphSignalSineFM() = default;
        virtual void Reset() override;
        virtual void Process(int32_t numSamples, std::vector<NodeGraphInput>& inputs, std::vector<NodeGraphOutput>& outputs) override;
        virtual bool IsDataVisible(int8_t) override { return true; }
        virtual bool IsDataEditable(int8_t) override { return true; }
        virtual std::string_view GetInputName(int8_t inputChannel) override {
            return defaultInStrings[inputChannel];
        }
        virtual std::string_view GetOutputName(int8_t outputChannel) override {
            return defaultOutStrings[outputChannel];
        }
        virtual std::string_view GetName() override {
            return "Sine FM 1";
        }
    protected:
        double mFreq = 0.0;
        float mVolume = 0.0f;
        double mFmod = 0.0;
        double mPmod = 0.0;
        float mReset = 0.0f;

        double mWave = 0.0;
        double mDeltaTime = 0.0;
        float mVol = 0.0f;

        double mPhase = 0.0;


        double mPhaseFmod = 0.0;
        double mFmodFrq = 0.0;
        double mFmodVol = 0.0;
        double mFmodOfs = 0.0;

        float mSamplesUntilUpdate = 0.0f;
    };

    /*********************************************************************/
    class GraphSignalSineFM2 : public NodeGraphOp {
    public:
        GraphSignalSineFM2(NodeGraphBase* node) :
            NodeGraphOp(node, 6, 1)
        {}

        std::string defaultInStrings[6] = { "Freq", "Volume", "FmodVol", "FmodFreq", "Smooth", "Reset" };
        std::string defaultOutStrings[1] = { "Out" };

        virtual ~GraphSignalSineFM2() = default;
        virtual void Reset() override;
        virtual void Process(int32_t numSamples, std::vector<NodeGraphInput>& inputs, std::vector<NodeGraphOutput>& outputs) override;
        virtual bool IsDataVisible(int8_t) override { return true; }
        virtual bool IsDataEditable(int8_t) override { return true; }
        virtual std::string_view GetInputName(int8_t inputChannel) override {
            return defaultInStrings[inputChannel];
        }
        virtual std::string_view GetOutputName(int8_t outputChannel) override {
            return defaultOutStrings[outputChannel];
        }
        virtual std::string_view GetName() override {
            return "Sine FM 2";
        }
    protected:
        double mFreq = 0.0;
        float mVolume = 0.0f;
        double mFmod = 0.0;
        double mPmod = 0.0;
        float mReset = 0.0f;

        double mWave = 0.0;
        double mDeltaTime = 0.0;
        double mDeltaLimit = 0.0;
        float mVol = 0.0f;

        double mPhase = 0.0;
        double mPhaseFmod = 0.0;
        float mSamplesUntilUpdate = 0.0f;
    };

    /*********************************************************************/
    class GraphSignalSineFM3 : public NodeGraphOp {
    public:
        GraphSignalSineFM3(NodeGraphBase* node) :
            NodeGraphOp(node, 5, 1)
        {}

        std::string defaultInStrings[5] = { "Freq", "Volume", "Fmod", "Smooth", "Reset" };
        std::string defaultOutStrings[1] = { "Out" };

        virtual ~GraphSignalSineFM3() = default;
        virtual void Reset() override;
        virtual void Process(int32_t numSamples, std::vector<NodeGraphInput>& inputs, std::vector<NodeGraphOutput>& outputs) override;
        virtual bool IsDataVisible(int8_t) override { return true; }
        virtual bool IsDataEditable(int8_t) override { return true; }
        virtual std::string_view GetInputName(int8_t inputChannel) override {
            return defaultInStrings[inputChannel];
        }
        virtual std::string_view GetOutputName(int8_t outputChannel) override {
            return defaultOutStrings[outputChannel];
        }
        virtual std::string_view GetName() override {
            return "Sine FM 3";
        }
    protected:
        double mFreq = 0.0;
        float mVolume = 0.0f;
        double mPhaseFmod = 0.0;
        float mReset = 0.0f;

        double mWave = 0.0;
        double mDeltaTime = 0.0;
        double mDeltaLimit = 0.0;
        float mVol = 0.0f;
        double mPhase = 0.0;
        float mSamplesUntilUpdate = 0.0f;
    };

    /*********************************************************************/
    class GraphSignalSaw : public NodeGraphOp {
    public:
        GraphSignalSaw(NodeGraphBase* node) :
            NodeGraphOp(node, 6, 1)
        {}

        std::string defaultInStrings[6] = { "Freq", "Volume", "Fmod", "Phase", "Smooth", "Reset" };
        std::string defaultOutStrings[1] = { "Out" };

        virtual ~GraphSignalSaw() = default;
        virtual void Reset() override;
        virtual void Process(int32_t numSamples, std::vector<NodeGraphInput>& inputs, std::vector<NodeGraphOutput>& outputs) override;
        virtual bool IsDataVisible(int8_t) override { return true; }
        virtual bool IsDataEditable(int8_t) override { return true; }
        virtual std::string_view GetInputName(int8_t inputChannel) override {
            return defaultInStrings[inputChannel];
        }
        virtual std::string_view GetOutputName(int8_t outputChannel) override {
            return defaultOutStrings[outputChannel];
        }
        virtual std::string_view GetName() override {
            return "Saw";
        }
    protected:
        double mFreq = 0.0;
        float mVolume = 0.0f;
        double mFmod = 0.0;
        double mPmod = 0.0;
        float mReset = 0.0f;

        double mDeltaTime = 0.0;
        float mVol = 0.0f;

        double mPhase = 0.0;
        double mPhaseFmod = 0.0;
        double mWave = 0.0;
        float mSamplesUntilUpdate = 0.0f;
    };
}

