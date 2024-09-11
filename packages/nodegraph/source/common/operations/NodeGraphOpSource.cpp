#include "nodegraph/operations/NodeGraphOpSource.h"

#include "logging/Log.h"
#include "audio/AudioUtils.h"

#include "math/MathFunc.h"

#include <math.h>

namespace l::nodegraph {

    /* Mathematical operations */

    /*********************************************************************/
    void GraphSourceConstants::Reset() {
        switch (mMode) {
        case 0:
            for (int8_t i = 0; i < 4; i++) {
                mNode->SetInputBound(i, InputBound::INPUT_0_TO_1);
            }
            break;
        case 1:
            for (int8_t i = 0; i < 4; i++) {
                mNode->SetInputBound(i, InputBound::INPUT_NEG_1_POS_1);
            }
            break;
        case 2:
            for (int8_t i = 0; i < 4; i++) {
                mNode->SetInputBound(i, InputBound::INPUT_0_100);
            }
            break;
        default:
            for (int8_t i = 0; i < 4; i++) {
                mNode->SetInputBound(i, InputBound::INPUT_UNBOUNDED);
            }
            break;
        }
    }

    void GraphSourceConstants::Process(int32_t, std::vector<NodeGraphInput>& inputs, std::vector<NodeGraphOutput>& outputs) {
        for (int8_t i = 0; i < mNumOutputs; i++) {
            outputs.at(i).mOutput = inputs.at(i).Get();
        }
    }

    void GraphSourceConstants::Tick(int32_t, float) {
        mNode->ProcessSubGraph(1);
    }

    /*********************************************************************/
    void GraphSourceTime::Process(int32_t, std::vector<NodeGraphInput>&, std::vector<NodeGraphOutput>& outputs) {
        float rate = 44100.0f;
        float phaseChange = 1.0f / rate;
        mAudioTime += phaseChange;

        outputs.at(0).mOutput = mAudioTime;
        outputs.at(1).mOutput = mFrameTime;
    }

    void GraphSourceTime::Tick(int32_t, float deltaTime) {
        mFrameTime += deltaTime;
    }

    void GraphSourceTime::Reset() {
        mAudioTime = 0.0f;
        mFrameTime = 0.0f;
    }

}
