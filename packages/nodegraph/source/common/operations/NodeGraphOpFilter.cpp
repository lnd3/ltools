#include "nodegraph/operations/NodeGraphOpFilter.h"

#include "logging/Log.h"
#include "audio/AudioUtils.h"

#include "math/MathFunc.h"

#include <math.h>

namespace l::nodegraph {

    /* Stateful filtering operations */

    /*********************************************************************/
    void GraphFilterLowpass::Reset() {
        mState0 = 0.0f;
        mState1 = 0.0f;
        mNode->SetInput(1, 0.99f);
        mNode->SetInput(2, 0.01f);
        mNode->SetInputBound(1, InputBound::INPUT_0_TO_1);
        mNode->SetInputBound(2, InputBound::INPUT_0_TO_1);
    }

    void GraphFilterLowpass::Process(int32_t, std::vector<NodeGraphInput>& inputs, std::vector<NodeGraphOutput>& outputs) {
        float inputValue = inputs.at(0).Get();
        float cutoff = inputs.at(1).Get();
        float resonance = 1.0f - inputs.at(2).Get();

        cutoff *= cutoff;
        float rc = 1.0f - resonance * cutoff;

        mState0 = rc * mState0 - cutoff * (mState1 + inputValue);
        mState1 = rc * mState1 + cutoff * mState0;

        outputs.at(0).mOutput = -mState1;
    }

    /*********************************************************************/
    void GraphFilterHighpass::Reset() {
        mState0 = 0.0f;
        mState1 = 0.0f;
        mNode->SetInput(1, 0.99f);
        mNode->SetInput(2, 0.01f);
        mNode->SetInputBound(1, InputBound::INPUT_0_TO_1);
        mNode->SetInputBound(2, InputBound::INPUT_0_TO_1);
    }

    void GraphFilterHighpass::Process(int32_t, std::vector<NodeGraphInput>& inputs, std::vector<NodeGraphOutput>& outputs) {
        float inputValue = inputs.at(0).Get();
        float cutoff = inputs.at(1).Get();
        float resonance = 1.0f - inputs.at(2).Get();

        cutoff *= cutoff;
        float rc = 1.0f - resonance * cutoff;

        float v01 = mState0 - mState1;
        mState0 += cutoff * (inputValue - mState0 + rc * v01);
        mState1 += cutoff * v01;

        outputs.at(0).mOutput = inputValue - mState1;
    }
}
