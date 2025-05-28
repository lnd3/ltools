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
    class DataIOOCHLVDataIn : public NodeGraphOp {
    public:
        static const int32_t kIntervalCount = 10;
        const int32_t kIntervals[kIntervalCount] = { 1, 5, 15, 30, 60, 120, 240, 720, 1440, 10080 };

        DataIOOCHLVDataIn(NodeGraphBase* node) :
            NodeGraphOp(node, "OCHLV Data In")
        {
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
        virtual ~DataIOOCHLVDataIn() = default;

        virtual void InputHasChanged(int32_t numSamplesWritten) override;
        virtual void Reset() override;
        virtual void Process(int32_t numSamples, int32_t numCacheSamples, std::vector<NodeGraphInput>& inputs, std::vector<NodeGraphOutput>& outputs) override;

        int32_t GetNumSamplesLeft();
    protected:
        int32_t mUnixtimePrev = 0;
        int32_t mReadSamples = 0;
        int32_t mWrittenSamples = 0;
    };

    /*********************************************************************/
    class DataIOPlaceTrade : public NodeGraphOp {
    public:
        DataIOPlaceTrade(NodeGraphBase* node) :
            NodeGraphOp(node, "Place Trade")
        {
            AddInput2("Symbol", 16, InputFlags(false, true, false, true));
            AddInput2("Base", 16, InputFlags(false, true, false, true));
            AddInput("Interval Min", 1.0f, 1, 0.0f, 10.0f);
            AddInput("Unixtime", 0.0f, 2, -l::math::constants::FLTMAX, l::math::constants::FLTMAX, false, false);
            AddInput("Decision", 0.0f, 2, 1.0f, -1.0f, false, false);
            AddInput("Conviction", 0.0f, 2, 0.0f, 1.0f, false, false);
            AddInput("Indecision Level", 0.05f, 1, 0.0f, 0.5f);

            AddOutput2("Symbol", 16, OutputFlags(true, true));
            AddOutput2("Base", 16, OutputFlags(true, true));
            AddOutput("Interval Min", 1.0f);
            AddOutput("Unixtime", 0.0f, 2, false);
            AddOutput("Decision", 0.0f, 2, false);
            AddOutput("Conviction", 0.0f, 2, false);
            AddOutput("Candle Progress");
            AddOutput("Indecision Level", 0.25f);
        }
        virtual ~DataIOPlaceTrade() = default;

        virtual void Process(int32_t numSamples, int32_t numCacheSamples, std::vector<NodeGraphInput>& inputs, std::vector<NodeGraphOutput>& outputs) override;
    protected:
        int32_t mUnixtimePrev = 0;
        int32_t mWrittenSamples = 0;
    };

}