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

    /* Stateful filtering operations */
    class SignalFilterBase : public NodeGraphOp {
    public:

        static const int8_t mNumDefaultInputs = 4;
        static const int8_t mNumDefaultOutputs = 1;

        SignalFilterBase(NodeGraphBase* node, std::string_view name) :
            NodeGraphOp(node, name),
            mInputManager(*this)
        {
            mDefaultInStrings.clear();
            mDefaultOutStrings.clear();
            mDefaultInData.clear();

            mInputManager.AddInput(InputIterationType::SAMPLED, AddInput("Sync", 0.0f));
            mInputManager.AddInput(InputIterationType::SAMPLED, AddInput("In"));
            mInputManager.AddInput(InputIterationType::SAMPLED_RWA, AddInput("Cutoff", 1.0f, 1, 0.0f, 1.0f));
            mInputManager.AddInput(InputIterationType::SAMPLED_RWA, AddInput("Resonance", 0.0f, 1, 0.0f, 1.0f));

            AddOutput("Out", 0.0f);
        }

        virtual ~SignalFilterBase() = default;
        virtual void DefaultDataInit() override;
        virtual void Process(int32_t numSamples, int32_t numCacheSamples, std::vector<NodeGraphInput>& inputs, std::vector<NodeGraphOutput>& outputs) override final;

        virtual void UpdateSignal(std::vector<NodeGraphInput>&, std::vector<NodeGraphOutput>&) {};
        virtual float ProcessSignal(float input, float cutoff, float resonance) = 0;
    protected:
        InputManager mInputManager;

        float mSync = 0.0f;
        float mSamplesUntilUpdate = 0.0f;
        float mUpdateRate = 128.0f;
    };

    /*********************************************************************/
    class SignalFilterLowpass : public SignalFilterBase {
    public:
        SignalFilterLowpass(NodeGraphBase* node) :
            SignalFilterBase(node, "Lowpass")
        {}
        virtual ~SignalFilterLowpass() = default;
        virtual void Reset() override;

        virtual float ProcessSignal(float input, float cutoff, float resonance) override;
    protected:
        float mInputValuePrev = 0.0f;
        float mState0 = 0.0f;
        float mState1 = 0.0f;
    };

    /*********************************************************************/
    class SignalFilterHighpass : public SignalFilterBase {
    public:
        SignalFilterHighpass(NodeGraphBase* node) :
            SignalFilterBase(node, "Highpass")
        {}
        virtual ~SignalFilterHighpass() = default;

        virtual void Reset() override;

        virtual float ProcessSignal(float input, float cutoff, float resonance) override;
    protected:
        float mInputValuePrev = 0.0f;
        float mState0 = 0.0f;
        float mState1 = 0.0f;
    };

    /*********************************************************************/
    // source: https://www.musicdsp.org/en/latest/Filters/23-state-variable.html
    class SignalFilterChamberlain2pole : public SignalFilterBase {
    public:
        SignalFilterChamberlain2pole(NodeGraphBase* node) :
            SignalFilterBase(node, "Chamberlin two-pole")
        {
            mInputManager.AddInput(InputIterationType::SAMPLED, AddInput("Mode"));
            mState.resize(4);
        }
        virtual ~SignalFilterChamberlain2pole() = default;

        virtual void DefaultDataInit() override;
        virtual void Reset() override;

        virtual void UpdateSignal(std::vector<NodeGraphInput>&, std::vector<NodeGraphOutput>&) override;
        virtual float ProcessSignal(float input, float cutoff, float resonance) override;
    protected:
        float mInputValuePrev = 0.0f;
        float mScale = 0.0f;
        int32_t mMode = 0;

        std::vector<float> mState;
        l::audio::FilterRWA<float> mScaleFilter;
    };


    /*********************************************************************/
    class SignalFilterMovingAverage : public NodeGraphOp {
    public:
        SignalFilterMovingAverage(NodeGraphBase* node, float undefinedValue = 0.0f) :
            NodeGraphOp(node, "Moving Average"),
            mInputManager(*this),
            mUndefinedValue(undefinedValue)
        {
            mDefaultInStrings.clear();
            mDefaultOutStrings.clear();
            mDefaultInData.clear();

            mInputManager.AddInput(InputIterationType::SAMPLED, AddInput("Sync", 0.0f));
            mInputManager.AddInput(InputIterationType::SAMPLED, AddInput("In"));
            mInputManager.AddInput(InputIterationType::SAMPLED, AddInput("Kernel Size", 1.0f, 1, 1.0f, 5000.0f));
            mInputManager.AddInput(InputIterationType::SAMPLED, AddInput("Kernel Balance", 0.0f, 1, 0.0f, 10.0f));

            AddOutput("Out", 0.0f);
        }

        virtual ~SignalFilterMovingAverage() = default;
        virtual void Reset() override;

        virtual void DefaultDataInit() override;
        virtual void Process(int32_t numSamples, int32_t numCacheSamples, std::vector<NodeGraphInput>& inputs, std::vector<NodeGraphOutput>& outputs) override final;
    protected:
        InputManager mInputManager;

        float mSync = 0.0f;
        float mSamplesUntilUpdate = 0.0f;
        float mUpdateRate = 128.0f;

        int32_t mDefaultKernelSize = 50;
        int32_t mWidth = mDefaultKernelSize;
        int32_t mFilterStateIndex = -1;
        float mUndefinedValue = 0.0f;
        bool mFilterInit = false;
        std::vector<float> mFilterState;
    };
}

