#include "nodegraph/operations/NodeGraphOpSource.h"

#include "logging/Log.h"
#include "audio/AudioUtils.h"

#include "math/MathFunc.h"

#include <math.h>

namespace l::nodegraph {

    /* Mathematical operations */

    /*********************************************************************/
    void GraphSourceConstants::Process(int32_t, std::vector<NodeGraphInput>& inputs, std::vector<NodeGraphOutput>& outputs) {
        for (int8_t i = 0; i < mNumOutputs; i++) {
            outputs.at(i).mOutput = inputs.at(i).Get();
        }
    }

    void GraphSourceConstants::Tick(int32_t, float) {
        mNode->ProcessSubGraph(1);
    }

    /*********************************************************************/
    void GraphSourceTime::Process(int32_t, std::vector<NodeGraphInput>& inputs, std::vector<NodeGraphOutput>& outputs) {
        mAudioTick++;

        if (inputs.at(4).Get() > 0.5f) {
            mAudioTick = 0;
            mFrameTick = 0;
        }

        inputs.at(0).Get(1) = static_cast<float>(mAudioTick);
        inputs.at(1).Get(1) = static_cast<float>(mFrameTick);
        inputs.at(2).Get(1) = mAudioTick / static_cast<float>(mAudioRate);
        inputs.at(3).Get(1) = mFrameTick / static_cast<float>(mFrameRate);

        for (int32_t i = 0; i < 4; i++) {
            outputs.at(i).Get() = inputs.at(i).Get();
        }
    }

    void GraphSourceTime::Tick(int32_t, float deltaTime) {
        mFrameTick++;
        mFrameTime += deltaTime;
    }

}
