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

        static const int8_t mNumDefaultInputs = 5;
        static const int8_t mNumDefaultOutputs = 1;

        GraphSignalBase(NodeGraphBase* node, std::string_view name) :
            NodeGraphOp(node, name),
            mInputManager(*this)
        {
            mInputManager.AddInput(InputIterationType::SAMPLED, AddInput("Sync", 0.0f, 1, 0.0f, 1.0f));
            mInputManager.AddInput(InputIterationType::SAMPLED, AddInput("Rate", 256.0f, 1, 1.0f, 2048.0f));
            mInputManager.AddInput(InputIterationType::SAMPLED_RWA, AddInput("Freq", 0.0f, 1, 0.0f, 22050.0f));
            mInputManager.AddInput(InputIterationType::SAMPLED_RWA, AddInput("Volume", 0.0f, 1, 0.0f, 5.0f));
            mInputManager.AddInput(InputIterationType::SAMPLED, AddInput("Smooth", 1.0f, 1, 0.0f, 1.0f));

            AddOutput("Out", 0.0f, 2);

            mFreq = 0.0f;
            mVolumeTarget = 0.0f;
            mSmooth = 0.5f;
            mDeltaTime = 0.0f;
            mSamplesUntilUpdate = 0.0f;

            ResetInput();
        }

        virtual ~GraphSignalBase() = default;
        virtual void Process(int32_t numSamples, std::vector<NodeGraphInput>& inputs, std::vector<NodeGraphOutput>& outputs) override;

        virtual void ResetInput() {};
        virtual void UpdateSignal(std::vector<NodeGraphInput>&, std::vector<NodeGraphOutput>&) {};
        virtual float ProcessSignal(float deltaTime, float freq) = 0;
    protected:
        InputManager mInputManager;

        float mReset = 0.0f;
        float mFreq = 0.0f;
        float mSmooth = 0.5f;
        float mDeltaTime = 0.0f;
        float mVolumeTarget = 0.0f;
        float mSamplesUntilUpdate = 0.0f;
        float mUpdateRate = 16.0f;

        l::audio::FilterRWA<float> mRWAFreq;
    };

    /*********************************************************************/
    class GraphSignalSine2 : public GraphSignalBase {
    public:
        GraphSignalSine2(NodeGraphBase* node) :
            GraphSignalBase(node, "Sine 2")
        {
            mInputManager.AddInput(InputIterationType::SAMPLED_RWA, AddInput("Fmod", 0.0f, 1, 0.0f, 1.0f));
            mInputManager.AddInput(InputIterationType::SAMPLED_RWA, AddInput("Phase", 0.0f, 1, 0.0f, 1.0f));
        }

        virtual ~GraphSignalSine2() = default;
        void DefaultDataInit() override;
        void Reset() override;
        void UpdateSignal(std::vector<NodeGraphInput>& inputs, std::vector<NodeGraphOutput>& outputs) override;
        float ProcessSignal(float deltaTime, float freq) override;
    protected:
        float mFmod = 0.0f;
        float mPmod = 0.0f;
        float mPhase = 0.0f;
        float mPhaseFmod = 0.0f;

        l::audio::FilterRWA<float> mFilterFmod;
        l::audio::FilterRWA<float> mFilterPmod;
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
            GraphSignalBase(node, "Saw 2")
        {
            mInputManager.AddInput(InputIterationType::SAMPLED_RWA, AddInput("Attenuation", 0.0f, 1, 0.0f, 1.0f));
            mInputManager.AddInput(InputIterationType::SAMPLED_RWA, AddInput("Cutoff", 0.0f, 1, 0.0f, 1.0f));
        }

        virtual ~GraphSignalSaw2() = default;
        void ResetInput() override;
        void Reset() override;
        void UpdateSignal(std::vector<NodeGraphInput>& inputs, std::vector<NodeGraphOutput>& outputs) override;
        float ProcessSignal(float deltaTime, float freq) override;

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
                double cpsClamped = l::math::clamp(cps, 2.0 / 44100, 0.5);
                b->curcps = cpsClamped;        /* this cycle\'s frequency */
                b->curper = 1.0 / cpsClamped;  /* this cycle\'s period */

                P2 = b->curper / 2.0;
                b->N = 1.0 + l::math::floor(P2); /* # of partials incl. dc */

                /* find the roll-off parameter which gives
                 * the desired attenuation at nyquist
                 */

                b->a = l::math::pow(b->aNQ, 1.0 / P2);
                b->aN = l::math::pow(b->a, b->N);
            }

            beta = 2.0 * l::math::constants::PI * b->phase;

            Nbeta = b->N * beta;
            cosbeta = l::math::cos(beta);

            /* The dsf blit is scaled by 1 / period to give approximately the same
             * peak-to-peak over a wide range of frequencies.
             */

            n = 1.0 -
                b->aN * l::math::cos(Nbeta) -
                b->a * (cosbeta - b->aN * l::math::cos(Nbeta - beta));
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
            double Omega = l::math::atan(l::math::constants::PI * cutoff);
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
            NodeGraphOp(node, "Sine")
        {
            AddInput("Freq", 0.0f, 1, 0.0f, 22050.0f);
            AddInput("Volume", 0.5f, 1, 0.0f, 5.0f);
            AddInput("Fmod", 1.0f, 1, 0.0f, 2.0f);
            AddInput("Phase", 0.0f, 1, 0.0f, 1.0f);
            AddInput("Smooth", 1.0f, 1, 0.0f, 1.0f);
            AddInput("Reset", 0.0f, 1, 0.0f, 1.0f);
            AddOutput("Out");

            mPhase = 0.0f;
            mPhaseFmod = 0.0f;
            mWave = 0.0f;
            mVol = 0.0f;
        }

        virtual ~GraphSignalSine() = default;
        virtual void Process(int32_t numSamples, std::vector<NodeGraphInput>& inputs, std::vector<NodeGraphOutput>& outputs) override;
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
            NodeGraphOp(node, "Sine FM 1")
        {
            AddInput("Freq", 0.0f, 1, 0.0f, 22050.0f);
            AddInput("Volume", 0.5f, 1, 0.0f, 5.0f);
            AddInput("Fmod", 0.0f, 1, 0.0f, 100.0f);
            AddInput("FmodFreq", 0.0f, 1, 0.0f, 100.0f);
            AddInput("FmodVol", 0.0f, 1, 0.0f, 100.0f);
            AddInput("FmodOfs", 0.0f, 1, 0.0f, 100.0f);
            AddInput("FmodGain", 0.0f, 1, 0.0f, 100.0f);
            AddInput("Smooth", 1.0f, 1, 0.0f, 1.0f);
            AddInput("Reset", 0.0f, 1, 0.0f, 1.0f);
            AddOutput("Out", 0.0f);

            mPhase = 0.0f;
        }

        virtual ~GraphSignalSineFM() = default;
        virtual void Process(int32_t numSamples, std::vector<NodeGraphInput>& inputs, std::vector<NodeGraphOutput>& outputs) override;
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
            NodeGraphOp(node, "Sine FM 2")
        {
            AddInput("Freq", 0.0f, 1, 0.0f, 22050.0f);
            AddInput("Volume", 0.5f, 1, 0.0f, 5.0f);
            AddInput("FmodVol", 0.0f, 1, 0.0f, 1.0f);
            AddInput("FmodFreq", 0.0f, 1, 0.0f, 100.0f);
            AddInput("Smooth", 1.0f, 1, 0.0f, 1.0f);
            AddInput("Reset", 0.0f, 1, 0.0f, 1.0f);
            AddOutput("Out", 0.0f);

            mPhase = 0.0f;
        }

        virtual ~GraphSignalSineFM2() = default;
        virtual void Process(int32_t numSamples, std::vector<NodeGraphInput>& inputs, std::vector<NodeGraphOutput>& outputs) override;
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
            NodeGraphOp(node, "Sine FM 3")
        {
            AddInput("Freq", 0.0f, 1, 0.0f, 22050.0f);
            AddInput("Volume", 0.5f, 1, 0.0f, 5.0f);
            AddInput("Fmod", 0.5f, 1, 0.0f, 1.0f);
            AddInput("Smooth", 1.0f, 1, 0.0f, 1.0f);
            AddInput("Reset", 0.0f, 1, 0.0f, 1.0f);
            AddOutput("Out", 0.0f);

            mPhase = 0.0f;
        }

        virtual ~GraphSignalSineFM3() = default;
        virtual void Process(int32_t numSamples, std::vector<NodeGraphInput>& inputs, std::vector<NodeGraphOutput>& outputs) override;
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
            NodeGraphOp(node, "Saw")
        {
            AddInput("Freq", 0.0f, 1, 0.0f, 22050.0f);
            AddInput("Volume", 0.5f, 1, 0.0f, 5.0f);
            AddInput("Fmod", 1.0f, 1, 0.0f, 2.0f);
            AddInput("Phase", 0.0f, 1, 0.0f, 1.0f);
            AddInput("Smooth", 1.0f, 1, 0.0f, 1.0f);
            AddInput("Reset", 0.0f, 1, 0.0f, 1.0f);
            AddOutput("Out", 0.0f);

            mPhase = 0.0f;
        }

        virtual ~GraphSignalSaw() = default;
        virtual void Process(int32_t numSamples, std::vector<NodeGraphInput>& inputs, std::vector<NodeGraphOutput>& outputs) override;
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

