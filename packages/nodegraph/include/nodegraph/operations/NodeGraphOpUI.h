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

        bool& GetInputState();
    protected:
        bool mInputState = false;
    };

    /*********************************************************************/
    class GraphUISlider : public NodeGraphOp {
    public:
        GraphUISlider(NodeGraphBase* node) :
            NodeGraphOp(node, "UI Slider")
        {
            AddInput2("In", 1, InputFlags(false, false, false, false));
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
        float& GetInputValue();
    protected:
        bool mExternallyChanged = false;
        float mInputValue = 0.0f;
        float mMin = 0.0f;
        float mMax = 1.0f;
    };

    /*********************************************************************/
    class GraphUIText : public NodeGraphOp {
    public:
        GraphUIText(NodeGraphBase* node) :
            NodeGraphOp(node, "UI Text")
        {
            AddInput2("In", 1, InputFlags(false, true, true, true));
        }

        virtual ~GraphUIText() = default;
        virtual void Process(int32_t numSamples, int32_t numCacheSamples, std::vector<NodeGraphInput>& inputs, std::vector<NodeGraphOutput>& outputs) override;
        virtual void Tick(int32_t, float) override;

        std::string_view GetOutputText();
    protected:
        l::string::string_buffer<64> mOutputText;
    };
    /*********************************************************************/
    class GraphUIChartLine : public NodeGraphOpCached {
    public:
        GraphUIChartLine(NodeGraphBase* node) :
            NodeGraphOpCached(node, "Chart Line")
        {

            AddInput2("X", 1, InputFlags(false, false, false, false));
            AddInput2("Y", 1, InputFlags(false, false, false, false));
            AddInput2("Name", 1, InputFlags(false, true, true, true));
            AddOutput("Data");
        }
        virtual ~GraphUIChartLine() = default;
        virtual void DefaultDataInit() override {
            mNode->SetInput(2, "Chart Line");
        }
        virtual void ProcessWriteCached(int32_t writtenSamples, int32_t numSamples, int32_t numCacheSamples, std::vector<NodeGraphInput>& inputs, std::vector<NodeGraphOutput>& outputs) override;
    protected:
        int32_t mLatestUnixtime = 0;
    };

    /*********************************************************************/
    class GraphUICandleSticks : public NodeGraphOpCached {
    public:
        GraphUICandleSticks(NodeGraphBase* node) :
            NodeGraphOpCached(node, "Candle Sticks")
        {
            AddInput2("Time", 1, InputFlags(false, false, false, false));
            AddInput2("Open", 1, InputFlags(false, false, false, false));
            AddInput2("Close", 1, InputFlags(false, false, false, false));
            AddInput2("High", 1, InputFlags(false, false, false, false));
            AddInput2("Low", 1, InputFlags(false, false, false, false));
            AddInput2("Volume", 1, InputFlags(false, false, false, false));
            AddInput2("Name", 1, InputFlags(false, true, true, true));
            AddOutput("Data");
        }
        virtual ~GraphUICandleSticks() = default;
        virtual void DefaultDataInit() override {
            mNode->SetInput(6, "Candle Sticks");
        }
        void ProcessWriteCached(int32_t writtenSamples, int32_t numSamples, int32_t numCacheSamples, std::vector<NodeGraphInput>& inputs, std::vector<NodeGraphOutput>& outputs) override;
    protected:
        int32_t mLatestUnixtime = 0;
    };

    /*********************************************************************/
    class GraphUIChartMarkers : public NodeGraphOp {
    public:
        GraphUIChartMarkers(NodeGraphBase* node) :
            NodeGraphOp(node, "Chart Markers")
        {
            AddInput2("Time");
            AddInput2("y");
            AddInput2("Marker");
            AddInput2("Name", 1, InputFlags(false, true, true, true));
        }
        virtual ~GraphUIChartMarkers() = default;
        virtual void DefaultDataInit() override {
            mNode->SetInput(3, "Chart Markers");
        }
        virtual void Process(int32_t numSamples, int32_t numCacheSamples, std::vector<NodeGraphInput>& inputs, std::vector<NodeGraphOutput>& outputs) override;
        const std::vector<std::tuple<int32_t, float, float, float>>& GetMarkers() {
            return mMarkers;
        }
    protected:
        int32_t mReadSamples = 0;
        float mPrevValue = 0.0f;
        float mPrevY = 0.0f;
        float mYTotalChange = 1.0f;
        int32_t mLastBeep = l::string::get_unix_epoch();
        std::vector<std::tuple<int32_t, float, float, float>> mMarkers;
    };
}

