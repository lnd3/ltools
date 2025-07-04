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

            AddInput("In", 0.0f, 2, -l::math::constants::FLTMAX, l::math::constants::FLTMAX, false, false);
            AddInput2("Symbol", 16, InputFlags(false, true, false, true));
            AddInput2("Base", 16, InputFlags(false, true, false, true));
            AddInput("Index", 2.0f, 1, 0.0f, 10.0f);


            AddOutput2("Symbol", 16, OutputFlags(true, true));
            AddOutput2("Base", 16, OutputFlags(true, true));
            AddOutput("Interval Min", 1.0f);

            AddOutput("Unixtime", 0.0f, 2);
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

		float mOpenPrev = 0.0f;
		float mClosePrev = 0.0f;
	};

    /*********************************************************************/



}