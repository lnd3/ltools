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

    /*********************************************************************/
    class GraphSourceConstants : public NodeGraphOp {
    public:

        const InputBound mModeToBoundList[4] = {InputBound::INPUT_0_TO_1, InputBound::INPUT_NEG_1_POS_1, InputBound::INPUT_0_100, InputBound::INPUT_UNBOUNDED};
        const std::string mNameList[4] = {"Constant [0,1]" , "Constant [-1,1]" , "Constant [0,100]", "Constant [-inf,inf]"};

        GraphSourceConstants(NodeGraphBase* node, int32_t mode) :
            NodeGraphOp(node, ""),
            mMode(mode)
        {
            mName = mNameList[mode];

            AddOutput("Out 1");
            AddOutput("Out 2");
            AddOutput("Out 3");
            AddOutput("Out 3");

            auto bound = GetInputBounds(mModeToBoundList[mMode]);
            AddConstant("Out 1", 0.0f, 1, bound.first, bound.second);
            AddConstant("Out 2", 0.0f, 1, bound.first, bound.second);
            AddConstant("Out 3", 0.0f, 1, bound.first, bound.second);
            AddConstant("Out 4", 0.0f, 1, bound.first, bound.second);
        }

        virtual ~GraphSourceConstants() {
        }
        virtual void Process(int32_t numSamples, int32_t numCacheSamples, std::vector<NodeGraphInput>& inputs, std::vector<NodeGraphOutput>& outputs) override;
        virtual void Tick(int32_t, float) override;
    protected:
        int32_t mMode;
        float mMax = 1.0f;
        float mMin = 0.0f;
    };

    /*********************************************************************/
    class GraphSourceTime : public NodeGraphOp {
    public:
        GraphSourceTime(NodeGraphBase* node, int32_t audioRate, int32_t frameRate) :
            NodeGraphOp(node, "Time"),
            mAudioRate(audioRate),
            mFrameRate(frameRate)
        {
            AddOutput("Audio Tick", 0.0f, 2);
            AddOutput("Frame Tick", 0.0f, 2);
            AddOutput("Audio Time", 0.0f, 2);
            AddOutput("Frame Time", 0.0f, 2);

            AddConstant("Audio Tick", 0.0f, 2);
            AddConstant("Frame Tick", 0.0f, 2);
            AddConstant("Audio Time", 0.0f, 2);
            AddConstant("Frame Time", 0.0f, 2);

            AddInput("Reset", 0.0f, 1, 0.0f, 1.0f);

            mAudioTime = 0.0f;
            mFrameTime = 0.0f;
            mFrameTick = 0;
            mAudioTick = 0;
        }

        virtual ~GraphSourceTime() {
        }

        virtual void Process(int32_t numSamples, int32_t numCacheSamples, std::vector<NodeGraphInput>& inputs, std::vector<NodeGraphOutput>& outputs) override;
        virtual void Tick(int32_t, float) override;
    protected:
        int32_t mAudioRate = 44100;
        int32_t mFrameRate = 60;

        int32_t mFrameTick = 0;
        int32_t mAudioTick = 0;
        float mAudioTime = 0.0f;
        float mFrameTime = 0.0f;
    };

    /*********************************************************************/
    class GraphSourceText : public NodeGraphOp {
    public:
        GraphSourceText(NodeGraphBase* node) :
            NodeGraphOp(node, "Text")
        {
            AddInput2("Text:", 16, InputFlags(false, true, true, true));
            AddOutput("Out");
        }

        virtual ~GraphSourceText() {
        }
        virtual void Process(int32_t numSamples, int32_t numCacheSamples, std::vector<NodeGraphInput>& inputs, std::vector<NodeGraphOutput>& outputs) override;
    };
}

