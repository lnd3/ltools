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
    class TradingDataIOOCHLVDataIn : public NodeGraphOp {
    public:
        static const int32_t kIntervalCount = 10;
        const int32_t kIntervals[kIntervalCount] = { 1, 5, 15, 30, 60, 120, 240, 720, 1440, 10080 };

        TradingDataIOOCHLVDataIn(NodeGraphBase* node, int32_t mode = 0) :
            NodeGraphOp(node, "OCHLV Data In"),
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

        virtual void InputHasChanged(int32_t numSamplesWritten) override;
        virtual void Reset() override;
        virtual void Process(int32_t numSamples, int32_t numCacheSamples, std::vector<NodeGraphInput>& inputs, std::vector<NodeGraphOutput>& outputs) override;

        int32_t GetNumSamplesLeft();
    protected:
		int32_t mMode = 0; // 0 - ochlv, 1 - heikin-ashi

        int32_t mUnixtimePrev = 0;
        int32_t mReadSamples = 0;
        int32_t mWrittenSamples = 0;

		float mOpenPrev = 0.0f;
		float mClosePrev = 0.0f;
	};

    /*********************************************************************/

	class TradeOpportunity {
	public:
		TradeOpportunity() = default;
		~TradeOpportunity() = default;

		void clear() {
			mIndex = -1;
			mTrendLevelPrev1 = 0.0f;
			mLongLevelPrev1 = 0.0f;
			mShortLevelPrev1 = 0.0f;
		}

		bool step(int32_t index) {
			if (mIndex == -1 || mIndex != index) {
				mIndex = index;
				mLongLevelPrev1 = mLongLevelNow;
				mShortLevelPrev1 = mShortLevelNow;
				mTrendLevelPrev1 = trend();
				return true;
			}
			return false;
		}

		void setIndecisionLevel(float indecisionLevel) {
			mIndecisionLevel = indecisionLevel;
		}

		void setLongLevel(float longLevel) {
			mLongLevelNow = l::math::clamp(longLevel, 0.0f, 1.0f);
		}

		void setShortLevel(float shortLevel) {
			mShortLevelNow = l::math::clamp(shortLevel, -1.0f, 0.0f);
		}

		bool isTrendReversal() {
			auto trendLevel = trend();
			if (trendLevel > 0.0f && mTrendLevelPrev1 < 0.0f) {
				return true;
			}
			if (trendLevel < 0.0f && mTrendLevelPrev1 > 0.0f) {
				return true;
			}
			return false;
		}

		float trend() {
			auto longTrend = isLong() ? 1.0f : 0.0f;
			return isShort() ? -1.0f : longTrend;
		}

		bool trendPrev() {
			auto longTrend = isLongPrev() ? 1.0f : 0.0f;
			return isShortPrev() ? -1.0f : longTrend;
		}

		float trendConviction() {
			auto t = trend();
			if (t > 0.0f) {
				auto longFactor = mLongLevelNow / (1.0f - mIndecisionLevel);
				return longFactor;
			}
			else if (t < 0.0f) {
				auto shortFactor = mShortLevelNow / (1.0f - mIndecisionLevel);
				return shortFactor;
			}
			else {
				return 0.0f;
			}
		}

		bool isSide() {
			return mLongLevelNow < mIndecisionLevel && mShortLevelNow > -mIndecisionLevel;
		}

		bool isLong() {
			return mLongLevelNow > mIndecisionLevel && mShortLevelNow >= -mIndecisionLevel;
		}

		bool isShort() {
			return mShortLevelNow < -mIndecisionLevel && mLongLevelNow <= mIndecisionLevel;
		}

		bool isSidePrev() {
			return mLongLevelPrev1 < mIndecisionLevel && mShortLevelPrev1 > -mIndecisionLevel;
		}

		bool isLongPrev() {
			return mLongLevelPrev1 > mIndecisionLevel && mShortLevelPrev1 >= -mIndecisionLevel;
		}

		bool isShortPrev() {
			return mLongLevelPrev1 <= mIndecisionLevel && mShortLevelPrev1 < -mIndecisionLevel;
		}

	protected:
		int32_t mIndex = -1;
		float mLongLevelNow = 0.0f;
		float mShortLevelNow = 0.0f;
		float mIndecisionLevel = 0.0f;

		float mTrendLevelPrev1 = 0.0f;
		float mLongLevelPrev1 = 0.0f;
		float mShortLevelPrev1 = 0.0f;
	};

    class TradingDataIOPlaceTrade : public NodeGraphOp {
    public:
        TradingDataIOPlaceTrade(NodeGraphBase* node) :
            NodeGraphOp(node, "Place Trade Out")
        {
            AddInput2("Symbol", 16, InputFlags(false, true, false, true));
            AddInput2("Base", 16, InputFlags(false, true, false, true));
            AddInput("Interval Min", 1.0f, 1, 0.0f, 10.0f);
            AddInput("Unixtime", 0.0f, 2, -l::math::constants::FLTMAX, l::math::constants::FLTMAX, false, false);
            AddInput("Long Level", 0.0f, 2, 1.0f, -1.0f, false, false);
            AddInput("Short Level", 0.0f, 2, 1.0f, -1.0f, false, false);
            AddInput("Indecision Level", 0.05f, 1, 0.0f, 0.5f);

            AddOutput2("Symbol", 16, OutputFlags(true, true));
            AddOutput2("Base", 16, OutputFlags(true, true));
            AddOutput("Interval Min", 1.0f);
            AddOutput("Unixtime", 0.0f, 2, false);
            AddOutput("Trend", 0.0f, 2, false);
            AddOutput("Conviction", 0.0f, 2, false);
			AddOutput("Reverse", 0.0f, 2, false);
			AddOutput("Long Level", 0.0f, 2, false);
			AddOutput("Short Level", 0.0f, 2, false);
		}
        virtual ~TradingDataIOPlaceTrade() = default;

        virtual void Process(int32_t numSamples, int32_t numCacheSamples, std::vector<NodeGraphInput>& inputs, std::vector<NodeGraphOutput>& outputs) override;
    protected:
		TradeOpportunity trade;
        int32_t mUnixtimePrev = 0;
        int32_t mWrittenSamples = 0;
    };

}