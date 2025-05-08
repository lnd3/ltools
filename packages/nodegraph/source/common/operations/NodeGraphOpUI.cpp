#include "nodegraph/operations/NodeGraphOpUI.h"

#include "logging/Log.h"
#include "audio/AudioUtils.h"

#include "math/MathFunc.h"

#include <math.h>

namespace l::nodegraph {

    /* UI elements */

        /*********************************************************************/
    void GraphUICheckbox::Process(int32_t, std::vector<NodeGraphInput>& inputs, std::vector<NodeGraphOutput>& outputs) {
        auto input = inputs.at(0).GetIterator(1);
        auto output = outputs.at(0).GetIterator(1);

        if (mExternallyChanged) {
            *input = mState ? 1.0f : 0.0f;
            mExternallyChanged = false;
        }
        else {
            mState = *input;
        }
       
        *output = *input != 0.0f ? 1.0f : 0.0f;
    }

    bool& GraphUICheckbox::GetStatePtr() {
        return mState;
    }

    void GraphUICheckbox::ExternallyChanged() {
        mExternallyChanged = true;
    }

    void GraphUICheckbox::Tick(int32_t, float) {
        mNode->ProcessSubGraph(1);
    }

    /*********************************************************************/
    void GraphUISlider::Process(int32_t, std::vector<NodeGraphInput>& inputs, std::vector<NodeGraphOutput>& outputs) {
        auto input = inputs.at(0).GetIterator(1);
        auto min = inputs.at(1).GetIterator(1);
        auto max = inputs.at(2).GetIterator(1);
        auto power = inputs.at(3).GetIterator(1);
        auto output = outputs.at(0).GetIterator(1);

        mMin = *min;
        mMax = *max;

        if (mExternallyChanged) {
            *input = mValue;
            mExternallyChanged = false;
        }
        else {
            mValue = *input;
        }

        float length = mMax - mMin;
        if (length <= 0.0f) {
            *output = mMin;
        }
        float value = (mValue - mMin) / length;
        value = mMin + length * l::math::pow(value, *power);
        *output = l::math::clamp(value, mMin, mMax);
    }

    void GraphUISlider::Tick(int32_t, float) {
        mNode->ProcessSubGraph(1);
    }

    float& GraphUISlider::GetMin() {
        return mMin;
    }

    float& GraphUISlider::GetMax() {
        return mMax;
    }

    float& GraphUISlider::GetValue() {
        return mValue;
    }

    void GraphUISlider::ExternallyChanged() {
        mExternallyChanged = true;
    }


}
