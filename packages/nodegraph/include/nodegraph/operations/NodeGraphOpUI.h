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
    class GraphUICheckbox : public NodeGraphOp {
    public:
        GraphUICheckbox(NodeGraphBase* node) :
            NodeGraphOp(node, "UI Checkbox")
        {
            AddInput("In", 0.0f, 1, 0.0f, 1.0f);
            AddOutput("Out");
        }

        virtual ~GraphUICheckbox() = default;
        virtual void Process(int32_t numSamples, int32_t numCacheSamples, std::vector<NodeGraphInput>& inputs, std::vector<NodeGraphOutput>& outputs) override;
        virtual void Tick(int32_t, float) override;

        bool& GetStatePtr();
    protected:
        bool mState = false;
    };

    /*********************************************************************/
    class GraphUISlider : public NodeGraphOp {
    public:
        GraphUISlider(NodeGraphBase* node) :
            NodeGraphOp(node, "UI Slider")
        {
            AddInput("In", 0.0f);
            AddInput("Min", 0.0f);
            AddInput("Max", 1.0f);
            AddInput("Power", 1.0f);
            AddOutput("Out");
        }

        virtual ~GraphUISlider() = default;
        virtual void Process(int32_t numSamples, int32_t numCacheSamples, std::vector<NodeGraphInput>& inputs, std::vector<NodeGraphOutput>& outputs) override;
        virtual void Tick(int32_t, float) override;

        float& GetMin();
        float& GetMax();
        float& GetValue();
    protected:
        bool mExternallyChanged = false;
        float mValue = 0.0f;
        float mMin = 0.0f;
        float mMax = 1.0f;
    };

    /*********************************************************************/
    class GraphUIChartLine : public NodeGraphOp {
    public:
        GraphUIChartLine(NodeGraphBase* node) :
            NodeGraphOp(node, "Chart Lines")
        {
            AddInput2("x", 1, InputFlags(false, false, false, false));
            AddInput2("y", 1, InputFlags(false, false, false, false));
            AddInput2("name", 1, InputFlags(false, false, false, true));
            AddOutput("Interleaved Data");
        }
        virtual ~GraphUIChartLine() {

        }
        virtual void DefaultDataInit() override {
            mNode->SetInput(2, "Chart Lines");
        }
        virtual void Reset() override;
        virtual void Process(int32_t numSamples, int32_t numCacheSamples, std::vector<NodeGraphInput>& inputs, std::vector<NodeGraphOutput>& outputs) override;

        int32_t GetNumSamplesLeft();
    protected:
        int32_t mWrittenSamples = 0;
        int32_t mLatestUnixtime = 0;
    };

    /*********************************************************************/
    class GraphUICandleSticks : public NodeGraphOp {
    public:
        GraphUICandleSticks(NodeGraphBase* node) :
            NodeGraphOp(node, "Candle Sticks")
        {
            AddInput2("unixtime", 1, InputFlags(false, false, false, false));
            AddInput2("open", 1, InputFlags(false, false, false, false));
            AddInput2("close", 1, InputFlags(false, false, false, false));
            AddInput2("high", 1, InputFlags(false, false, false, false));
            AddInput2("low", 1, InputFlags(false, false, false, false));
            AddInput2("volume", 1, InputFlags(false, false, false, false));
            AddInput2("name", 1, InputFlags(false, false, false, true));
            AddOutput("Interleaved Data");
        }
        virtual ~GraphUICandleSticks() = default;
        virtual void DefaultDataInit() override {
            mNode->SetInput(6, "Candle Sticks");
        }
        virtual void Process(int32_t numSamples, int32_t numCacheSamples, std::vector<NodeGraphInput>& inputs, std::vector<NodeGraphOutput>& outputs) override;
    protected:
        int32_t mWrittenSamples = 0;
        int32_t mLatestUnixtime = 0;
    };
}

