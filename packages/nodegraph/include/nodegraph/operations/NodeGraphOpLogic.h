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

    /* Logical operations */

    /*********************************************************************/
    class GraphLogicalAnd : public NodeGraphOp {
    public:
        GraphLogicalAnd(NodeGraphBase* node) :
            NodeGraphOp(node, "And")
        {
            AddInput("In 1", 0.0f);
            AddInput("In 2", 0.0f);
            AddOutput("Out", 0.0f);
        }

        virtual ~GraphLogicalAnd() = default;
        virtual void Process(int32_t numSamples, int32_t, std::vector<NodeGraphInput>& inputs, std::vector<NodeGraphOutput>& outputs) override {
            auto input1 = inputs.at(0).GetIterator(numSamples);
            auto input2 = inputs.at(1).GetIterator(numSamples);
            auto output = outputs.at(0).GetIterator(numSamples);
            for (int32_t i = 0; i < numSamples; i++) {
                bool bool1 = (*input1++) != 0.0f;
                bool bool2 = (*input2++) != 0.0f;
                *output++ = (bool1 && bool2) ? 1.0f : 0.0f;
            }
        }
    };

    /*********************************************************************/
    class GraphLogicalOr : public NodeGraphOp {
    public:
        GraphLogicalOr(NodeGraphBase* node) :
            NodeGraphOp(node, "Or")
        {
            AddInput("In 1", 0.0f);
            AddInput("In 2", 0.0f);
            AddOutput("Out", 0.0f);
        }

        virtual ~GraphLogicalOr() = default;
        virtual void Process(int32_t numSamples, int32_t, std::vector<NodeGraphInput>& inputs, std::vector<NodeGraphOutput>& outputs) override {
            auto input1 = inputs.at(0).GetIterator(numSamples);
            auto input2 = inputs.at(1).GetIterator(numSamples);
            auto output = outputs.at(0).GetIterator(numSamples);
            for (int32_t i = 0; i < numSamples; i++) {
                bool bool1 = (*input1++) != 0.0f;
                bool bool2 = (*input2++) != 0.0f;
                *output++ = (bool1 || bool2) ? 1.0f : 0.0f;
            }
        }
    };

    /*********************************************************************/
    class GraphLogicalXor : public NodeGraphOp {
    public:
        GraphLogicalXor(NodeGraphBase* node) :
            NodeGraphOp(node, "Xor")
        {
            AddInput("In 1", 0.0f);
            AddInput("In 2", 0.0f);
            AddOutput("Out", 0.0f);
        }

        virtual ~GraphLogicalXor() = default;
        virtual void Process(int32_t numSamples, int32_t, std::vector<NodeGraphInput>& inputs, std::vector<NodeGraphOutput>& outputs) override {
            auto input1 = inputs.at(0).GetIterator(numSamples);
            auto input2 = inputs.at(1).GetIterator(numSamples);
            auto output = outputs.at(0).GetIterator(numSamples);
            for (int32_t i = 0; i < numSamples; i++) {
                bool bool1 = (*input1++) != 0.0f;
                bool bool2 = (*input2++) != 0.0f;
                *output++ = (bool1 ^ bool2) ? 1.0f : 0.0f;
            }
        }
    };

    /*********************************************************************/
    class GraphLogicalFlipGate : public NodeGraphOp {
    public:
        GraphLogicalFlipGate(NodeGraphBase* node) :
            NodeGraphOp(node, "Flip Gate")
        {
            AddInput("In", 0.0f);
            AddInput("Max input", 1.0f, 1, 0.00001f, l::math::constants::FLTMAX);
            AddOutput("Gate", 0.0f);
            AddOutput("Strength", 0.0f);
        }

        virtual ~GraphLogicalFlipGate() = default;
        virtual void Process(int32_t numSamples, int32_t, std::vector<NodeGraphInput>& inputs, std::vector<NodeGraphOutput>& outputs) override {
            auto input = &inputs.at(0).Get(numSamples);
            auto maxInput = &inputs.at(1).Get(numSamples);
            auto gate = &outputs.at(0).Get(numSamples);
            auto strength = &outputs.at(1).Get(numSamples);

            for (int32_t i = 0; i < numSamples; i++) {
                auto in = (*input++);
                bool pos = in > 0.01f;
                bool neg = in < -0.01f;

                if (mGate && neg) {
                    mGate = false;
                }
                if (!mGate && pos) {
                    mGate = true;
                }

                *gate++ = mGate ? 1.0f : -1.0f;
                *strength++ = in / (*maxInput);
            }
        }
    protected:
        bool mGate = false;
        float mStrength = 0.0f;
    };

    /*********************************************************************/
    class GraphLogicalFlipInfo: public NodeGraphOp {
    public:
        GraphLogicalFlipInfo(NodeGraphBase* node) :
            NodeGraphOp(node, "Flip Info")
        {
            AddInput("In", 0.0f);
            AddInput("Max Flips in info", 5.0f, 1, 1.0f, 100000.0f);

            AddOutput("mean+");
            AddOutput("mean-");
            AddOutput("max+");
            AddOutput("max-");
        }

        virtual ~GraphLogicalFlipInfo() = default;
        virtual void Process(int32_t numSamples, int32_t, std::vector<NodeGraphInput>& inputs, std::vector<NodeGraphOutput>& outputs) override {
            auto input = &inputs.at(0).Get(numSamples);
            auto maxFlips = l::math::max2(static_cast<int32_t>(inputs.at(1).Get() + 0.5f), 1);

            auto meanPosOut = &outputs.at(0).Get(numSamples);
            auto meanNegOut = &outputs.at(1).Get(numSamples);
            auto maxPosOut = &outputs.at(2).Get(numSamples);
            auto maxNegOut = &outputs.at(3).Get(numSamples);

            if (mPosPulseIntervalCount.empty()) {
                mPosPulseIntervalCount.push_back(0);
            }
            if (mNegPulseIntervalCount.empty()) {
                mNegPulseIntervalCount.push_back(0);
            }

            for (int32_t i = 0; i < numSamples; i++) {
                auto in = (*input++);

                if (in > 0.0f) {
                    mPosPulseIntervalCount.front() += 1.0f;
                }
                else {
                    mNegPulseIntervalCount.front() += 1.0f;
                }

                bool reversal = (in > 0.0f) != (mValuePrev1 > 0.0f);
                if (reversal && in > 0.0f) {
                    // positive pulse
                    mPosPulseIntervalCount.push_back(0.0f);
                    if (mPosPulseIntervalCount.size() > maxFlips) {
                        mPosPulseIntervalCount.resize(maxFlips);
                    }
                }
                else if (reversal && in < 0.0f){
                    // negative pulse
                    mNegPulseIntervalCount.push_back(0.0f);
                    if (mNegPulseIntervalCount.size() > maxFlips) {
                        mNegPulseIntervalCount.resize(maxFlips);
                    }
                }

                mValuePrev1 = in;

                float meanPos = 0.0f;
                float meanNeg = 0.0f;
                float maxPos = 0.0f;
                float maxNeg = 0.0f;

                for (auto count : mPosPulseIntervalCount) {
                    if (count > maxPos) {
                        maxPos = count;
                    }
                    meanPos += count;
                }
                for (auto count : mNegPulseIntervalCount) {
                    if (count > maxNeg) {
                        maxNeg = count;
                    }
                    meanNeg += count;
                }

                *meanPosOut++ = meanPos / static_cast<float>(mPosPulseIntervalCount.size());
                *meanNegOut++ = meanNeg / static_cast<float>(mNegPulseIntervalCount.size());
                *maxPosOut++ = maxPos;
                *maxNegOut++ = maxNeg;
            }
        }
    protected:
        std::vector<float> mPosPulseIntervalCount;
        std::vector<float> mNegPulseIntervalCount;
        float mValuePrev1;
    };
}

