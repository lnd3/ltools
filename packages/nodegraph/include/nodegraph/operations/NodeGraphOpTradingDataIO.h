#pragma once
#include "nodegraph/core/NodeGraphBase.h"

#include "logging/LoggingAll.h"

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
    class TradingDataIOOCHLVDataIn : public NodeGraphOpCached {
    public:
        static const int32_t kIntervalCount = 10;
        const int32_t kIntervals[kIntervalCount] = { 1, 5, 15, 30, 60, 120, 240, 720, 1440, 10080 };

        TradingDataIOOCHLVDataIn(NodeGraphBase* node, int32_t mode = 0) :
            NodeGraphOpCached(node, "OCHLV Data In"),
			mMode(mode)
        {
            if (mMode == 1) {
                mName = "OCHLV Heikin-Ashi In";
            }

            AddInput2("In", 16, InputFlags(false, false, false, false));
            AddInput2("Symbol", 16, InputFlags(false, true, false, true));
            AddInput2("Base", 16, InputFlags(false, true, false, true));
            AddInput("Index", 2.0f, 1, 0.0f, 10.0f);
            AddInput("Timeframe", 1.0f, 1, 1.0f, 1440.0f);
            AddInput("Friction", 0.0f, 1, 0.0f, 1.0f);


            AddOutput2("Symbol", 16, OutputFlags(false, true));
            AddOutput2("Base", 16, OutputFlags(false, true));
            AddOutput("Min", 1.0f);

            AddOutput("Time", 0.0f, 2);
            AddOutput("Open", 0.0f, 2);
            AddOutput("Close", 0.0f, 2);
            AddOutput("High", 0.0f, 2);
            AddOutput("Low", 0.0f, 2);
            AddOutput("Volume", 0.0f, 2);
            AddOutput("Quantity", 0.0f, 2);
            AddOutput("Buy Volume", 0.0f, 2);
            AddOutput("Sell Volume", 0.0f, 2);
            AddOutput("Buy Quantity", 0.0f, 2);
            AddOutput("Sell Quantity", 0.0f, 2);
        }
        virtual ~TradingDataIOOCHLVDataIn() = default;

        virtual void ProcessWriteCached(int32_t writtenSamples, int32_t numSamples, int32_t numCacheSamples, std::vector<NodeGraphInput>& inputs, std::vector<NodeGraphOutput>& outputs) override;
        virtual void ProcessReadCached(int32_t readSamples, int32_t numSamples, int32_t numCacheSamples, std::vector<NodeGraphInput>& inputs, std::vector<NodeGraphOutput>& outputs) override;
    protected:
		int32_t mMode = 0; // 0 - ochlv, 1 - heikin-ashi

        int32_t mUnixtimePrev = 0;

        // Heikin ashi vars
		float mOpenPrev = 0.0f;
		float mClosePrev = 0.0f;

        // Time frame vars
        float mOpenMa = 0.0f;
        float mCloseMa = 0.0f;
        float mHighMa = 0.0f;
        float mLowMa = 0.0f;
        float mVolMa = 0.0f;
        float mQuantMa = 0.0f;
        float mBuyVolMa = 0.0f;
        float mBuyQuantMa = 0.0f;
    };

    /*********************************************************************/

    class TradingDataIOChartInfo : public NodeGraphOp {
    public:
        TradingDataIOChartInfo(NodeGraphBase* node) :
            NodeGraphOp(node, "Chart Info")
        {
            AddInput2("Symbol", 16, InputFlags(false, true, false, true));
            AddInput2("Base", 16, InputFlags(false, true, false, true));
            AddInput("Index", 2.0f, 1, 0.0f, 10.0f);
            AddInput2("Now");
            AddInput2("PTick");
            AddInput2("QStep");

            AddOutput2("Symbol", 16, OutputFlags(false, true));
            AddOutput2("Base", 16, OutputFlags(false, true));
            AddOutput("Index#0", 0.0f);
            AddOutput("Index#1", 1.0f);
            AddOutput("Index#2", 2.0f);
            AddOutput("Index#3", 3.0f);
            AddOutput("Now");
            AddOutput("Reset");
            AddOutput("PTick");
            AddOutput("QStep");
        }

        virtual ~TradingDataIOChartInfo() = default;

        virtual void Process(int32_t numSamples, int32_t numCacheSamples, std::vector<NodeGraphInput>& inputs, std::vector<NodeGraphOutput>& outputs) override;

    protected:
        int32_t mReadSamples = 0;
    };

}