#include "nodegraph/NodeGraphOperations.h"

#include "logging/Log.h"

#include <math.h>

namespace l::nodegraph {

    /* Mathematical operations */

    void GraphSourceConstants::Process(std::vector<NodeGraphInput>& inputs, std::vector<NodeGraphOutput>& outputs) {
        ASSERT(inputs.size() == static_cast<size_t>(mNumConstants));
        ASSERT(outputs.size() == static_cast<size_t>(mNumConstants));
        for (int8_t i = 0; i < mNumConstants; i++) {
            float val = inputs.at(i).Get();
            val = val > mMax ? mMax : val < mMin ? mMin : val;
            inputs.at(i).mInput.mInputFloatConstant = val;
            outputs.at(i).mOutput = val;
        }
    }

    void GraphSourceSine::Process(std::vector<NodeGraphInput>& inputs, std::vector<NodeGraphOutput>& outputs) {
        ASSERT(inputs.size() == static_cast<size_t>(mNumInputs));
        ASSERT(outputs.size() == static_cast<size_t>(mNumOutputs));

        float time = inputs.at(0).Get();
        float freq = inputs.at(1).Get();
        float freqMod = inputs.at(2).Get();
        float phaseMod = inputs.at(3).Get();

        if (mPrevTime == 0.0f) {
            mPrevTime = time;
        }

        float deltaTime = time - mPrevTime;
        mPrevTime = time;
        
        float phaseDelta = deltaTime * freq;

        mPhase += phaseDelta * freqMod;
        mPhase -= floorf(mPhase);

        float phase = (mPhase + phaseMod) * 0.5f;
        phase -= floorf(phase);

        outputs.at(0).mOutput = sinf(2.0f * 3.141529f * phase);
        outputs.at(1).mOutput = phase;
    }

    void GraphNumericAdd::Process(std::vector<NodeGraphInput>& inputs, std::vector<NodeGraphOutput>& outputs) {
        ASSERT(inputs.size() == static_cast<size_t>(mNumInputs));
        ASSERT(outputs.size() == static_cast<size_t>(mNumOutputs));
        outputs.at(0).mOutput = inputs.at(0).Get() + inputs.at(1).Get();
    }

    void GraphNumericMultiply::Process(std::vector<NodeGraphInput>& inputs, std::vector<NodeGraphOutput>& outputs) {
        ASSERT(inputs.size() == static_cast<size_t>(mNumInputs));
        ASSERT(outputs.size() == static_cast<size_t>(mNumOutputs));
        outputs.at(0).mOutput = inputs.at(0).Get() * inputs.at(1).Get();
    }

    void GraphNumericSubtract::Process(std::vector<NodeGraphInput>& inputs, std::vector<NodeGraphOutput>& outputs) {
        ASSERT(inputs.size() == static_cast<size_t>(mNumInputs));
        ASSERT(outputs.size() == static_cast<size_t>(mNumOutputs));
        outputs.at(0).mOutput = inputs.at(0).Get() - inputs.at(1).Get();
    }

    void GraphNumericNegate::Process(std::vector<NodeGraphInput>& inputs, std::vector<NodeGraphOutput>& outputs) {
        ASSERT(inputs.size() == static_cast<size_t>(mNumInputs));
        ASSERT(outputs.size() == static_cast<size_t>(mNumOutputs));
        outputs.at(0).mOutput = -inputs.at(0).Get();
    }

    void GraphNumericIntegral::Reset() {
        mOutput = 0.0f;
    }

    void GraphNumericIntegral::Process(std::vector<NodeGraphInput>& inputs, std::vector<NodeGraphOutput>& outputs) {
        ASSERT(inputs.size() == static_cast<size_t>(mNumInputs));
        ASSERT(outputs.size() == static_cast<size_t>(mNumOutputs));
        mOutput += inputs.at(0).Get();
        outputs.at(0).mOutput = mOutput;
    }

    /* Logical operations */

    void GraphLogicalAnd::Process(std::vector<NodeGraphInput>& inputs, std::vector<NodeGraphOutput>& outputs) {
        ASSERT(inputs.size() == static_cast<size_t>(mNumInputs));
        ASSERT(outputs.size() == static_cast<size_t>(mNumOutputs));
        bool input1 = inputs.at(0).Get() != 0.0f;
        bool input2 = inputs.at(1).Get() != 0.0f;
        outputs.at(0).mOutput = (input1 && input2) ? 1.0f : 0.0f;
    }

    void GraphLogicalOr::Process(std::vector<NodeGraphInput>& inputs, std::vector<NodeGraphOutput>& outputs) {
        ASSERT(inputs.size() == static_cast<size_t>(mNumInputs));
        ASSERT(outputs.size() == static_cast<size_t>(mNumOutputs));
        bool input1 = inputs.at(0).Get() != 0.0f;
        bool input2 = inputs.at(1).Get() != 0.0f;
        outputs.at(0).mOutput = (input1 || input2) ? 1.0f : 0.0f;
    }

    void GraphLogicalXor::Process(std::vector<NodeGraphInput>& inputs, std::vector<NodeGraphOutput>& outputs) {
        ASSERT(inputs.size() == static_cast<size_t>(mNumInputs));
        ASSERT(outputs.size() == static_cast<size_t>(mNumOutputs));
        bool input1 = inputs.at(0).Get() != 0.0f;
        bool input2 = inputs.at(1).Get() != 0.0f;
        outputs.at(0).mOutput = (input1 ^ input2) ? 1.0f : 0.0f;
    }

    /* Stateful filtering operations */

    void GraphFilterLowpass::Reset() {
        mState0 = 0.0f;
        mState1 = 0.0f;
    }

    void GraphFilterLowpass::Process(std::vector<NodeGraphInput>& inputs, std::vector<NodeGraphOutput>& outputs) {
        ASSERT(inputs.size() == static_cast<size_t>(mNumInputs));
        ASSERT(outputs.size() == static_cast<size_t>(mNumOutputs));
        float cutoff = inputs.at(0).Get();
        float resonance = inputs.at(1).Get();
        float inputValue = inputs.at(2).Get();

        cutoff *= cutoff;
        float rc = 1 - resonance * cutoff;

        mState0 = rc * mState0 - cutoff * (mState1 + inputValue);
        mState1 = rc * mState1 + cutoff * mState0;

        outputs.at(0).mOutput = -mState1;
    }

}
