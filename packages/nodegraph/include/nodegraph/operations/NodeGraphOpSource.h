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
        GraphSourceConstants(NodeGraphBase* node, int32_t mode) :
            NodeGraphOp(node, 0, 4, 4),
            mMode(mode)
        {}

        virtual ~GraphSourceConstants() = default;
        virtual void Reset();
        virtual void Process(int32_t numSamples, std::vector<NodeGraphInput>& inputs, std::vector<NodeGraphOutput>& outputs) override;
        virtual void Tick(int32_t, float) override;
        virtual std::string_view GetName() override {
            switch (mMode) {
            case 0:
                return "Constant [0,1]";
            case 1:
                return "Constant [-1,1]";
            case 2:
                return "Constant [0,100]";
            case 3:
                return "Constant [-inf,inf]";
            };
            return "";
        }
        virtual bool IsDataVisible(int8_t) override {return true;}
        virtual bool IsDataEditable(int8_t) override {return true;}
    protected:
        int32_t mMode;
        float mMax = 1.0f;
        float mMin = 0.0f;
    };

    /*********************************************************************/
    class GraphSourceTime : public NodeGraphOp {
    public:
        GraphSourceTime(NodeGraphBase* node) :
            NodeGraphOp(node, 0, 2, 0)
        {}

        std::string defaultOutStrings[2] = { "Audio Time", "Frame Time"};

        virtual ~GraphSourceTime() = default;
        virtual void Reset() override;
        virtual void Process(int32_t numSamples, std::vector<NodeGraphInput>& inputs, std::vector<NodeGraphOutput>& outputs) override;
        virtual void Tick(int32_t, float) override;
        virtual std::string_view GetOutputName(int8_t outputChannel) override {
            return defaultOutStrings[outputChannel];
        }
        virtual std::string_view GetName() override {
            return "Time";
        }
        virtual bool IsDataVisible(int8_t) override { return true; }
        virtual bool IsDataEditable(int8_t) override { return true; }
    protected:
        float mAudioTime = 0.0f;
        float mFrameTime = 0.0f;
    };

}

