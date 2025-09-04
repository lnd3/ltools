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
#include <tuple>

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
            NodeGraphOp::DefaultDataInit();
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
            NodeGraphOp::DefaultDataInit();
            mNode->SetInput(6, "Candle Sticks");
        }
        void ProcessWriteCached(int32_t writtenSamples, int32_t numSamples, int32_t numCacheSamples, std::vector<NodeGraphInput>& inputs, std::vector<NodeGraphOutput>& outputs) override;
    protected:
        int32_t mLatestUnixtime = 0;
    };

    /*********************************************************************/
    struct TradePosition {
        float mEntry = 0.0f;
        float mExit = 0.0f;
        float mLotShare = 1.0f;
        int32_t mEntryTime = 0;
        int32_t mExitTime = 0;

        void Reset(float lotShare = 1.0f) {
            mLotShare = lotShare;
            mEntry = 0.0f;
            mExit = 0.0f;
            mEntryTime = 0;
            mExitTime = 0;
        }

        bool IsReady() {
            return mEntryTime == 0 && mExitTime == 0 && mEntry == 0.0f && mExit == 0.0f;
        }

        bool HasPosition() {
            return mEntryTime > 0 && mExitTime == 0 && mEntry > 0.0f && mExit == 0.0f;
        }

        bool HasEntry() {
            return mEntryTime > 0 && mEntry > 0.0f;
        }

        bool HasExit() {
            return mExitTime > 0 && mExit > 0.0f;
        }

        bool HasCompleted() {
            return mEntryTime > 0 && mExitTime > 0 && mEntry > 0.0f && mExit > 0.0f;
        }

        bool TradeEntered(int32_t time) {
            return time > 0 && time == mEntryTime && mEntry > 0.0f;
        }

        bool TradeExited(int32_t time) {
            return time > 0 && time == mExitTime && mExit > 0.0f;
        }

        float GetProfit(float slip) {
            if (mEntry > 0.0f && mExit > 0.0f) {
                auto entry = mEntry * (1.0f + slip); // entry commission
                auto change = mExit / entry;
                change = change * (1.0f - slip); // exit commission
                change = 1.0f + (change - 1.0f) * mLotShare;
                return change;
            }
            return 1.0f;
        }

        void Update(float state, float price, int32_t time) {
            if (time > 0 && mEntryTime == 0 && state > 0.0f) {
                mEntry = price;
                mEntryTime = time;
            }
            if (time > 0 && mEntryTime > 0 && mEntryTime < time && mExitTime == 0 && state < 0.0f) {
                mExit = price;
                mExitTime = time;
            }
        }
    };

    class GraphUIChartMarkers : public NodeGraphOp {
    public:
        GraphUIChartMarkers(NodeGraphBase* node) :
            NodeGraphOp(node, "Chart Markers")
        {
            AddInput2("Time");
            AddInput2("Open");
            AddInput2("Close");
            AddInput2("Entry 1");
            AddInput2("Entry 2");
            AddInput("Slip", 0.0002f, 1, 0.0f, 1.0f);
            AddInput2("Name", 1, InputFlags(false, true, true, true));
            AddInput("Pin Length", 30.0f, 1, 1.0f, 200.0f);
            AddInput("Pin Size", 5.0f, 1, 1.0f, 40.0f);
            AddInput("Font Size", 10.8f, 1, 3.0f, 20.0f);
            AddInput("Main Size", 0.5f, 1, 0.0f, 1.0f);
            AddInput2("Entry 3");
        }
        virtual ~GraphUIChartMarkers() = default;
        virtual void DefaultDataInit() override {
            NodeGraphOp::DefaultDataInit();
            mNode->SetInput(6, "Chart Markers");
        }
        virtual void Process(int32_t numSamples, int32_t numCacheSamples, std::vector<NodeGraphInput>& inputs, std::vector<NodeGraphOutput>& outputs) override;
        const std::vector<std::tuple<int32_t, float, float, float>>& GetMarkers() {
            return mMarkers;
        }
    protected:
        int32_t mReadSamples = 0;
        float mTotalProfit = 1.0f;

        TradePosition mEntry1;
        TradePosition mEntry2;
        TradePosition mEntry3;

        std::vector<std::tuple<int32_t, float, float, float>> mMarkers;
    };
}

