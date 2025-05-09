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
        virtual void Process(int32_t numSamples, std::vector<NodeGraphInput>& inputs, std::vector<NodeGraphOutput>& outputs) override {
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
        virtual void Process(int32_t numSamples, std::vector<NodeGraphInput>& inputs, std::vector<NodeGraphOutput>& outputs) override {
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
        virtual void Process(int32_t numSamples, std::vector<NodeGraphInput>& inputs, std::vector<NodeGraphOutput>& outputs) override {
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
    class GraphLogicalDetector : public NodeGraphOp {
    public:
        GraphLogicalDetector(NodeGraphBase* node) :
            NodeGraphOp(node, "Detector")
        {
            AddInput("In", 0.0f);
            AddInput("Scale Trend", 0.5f, 1, 0.0f, 1.0f);
            AddInput("Scale Reversal", 0.5f, 1, 0.0f, 1.0f);
            AddInput("Scale Accel", 0.5f, 1, 0.0f, 1.0f);

            AddOutput("Trend", 0.0f);
            AddOutput("Reversal", 0.0f);
            AddOutput("Accel", 0.0f);
            AddOutput("Sum", 0.0f);
        }

        virtual ~GraphLogicalDetector() = default;
        virtual void Process(int32_t numSamples, std::vector<NodeGraphInput>& inputs, std::vector<NodeGraphOutput>& outputs) override {
            auto input0 = inputs.at(0).GetIterator(numSamples);
            auto input2 = inputs.at(1).GetIterator();
            auto input3 = inputs.at(2).GetIterator();
            auto input4 = inputs.at(3).GetIterator();

            auto outputTrend = outputs.at(0).GetIterator(numSamples);
            auto outputReversal = outputs.at(1).GetIterator(numSamples);
            auto outputAccel = outputs.at(2).GetIterator(numSamples);
            auto outputSum = outputs.at(3).GetIterator(numSamples);

            float scaleTrend = (*input2++);
            float scaleReversal = (*input3++);
            float scaleAccel = (*input4++);
            const float cSum = 1.0f / (scaleTrend + scaleReversal + scaleAccel); // scale to [-1,1]
            scaleTrend *= cSum;
            scaleReversal *= cSum;
            scaleAccel *= cSum;

            for (int32_t i = 0; i < numSamples; i++) {
                float in1 = (*input0++);

                float diff01 = in1 - mPrev1;
                float diff12 = mPrev1 - mPrev2;
                float diff23 = mPrev2 - mPrev3;
                float diff34 = mPrev3 - mPrev4;

                //float diff02 = in1 - mPrev2;
                //float diff03 = in1 - mPrev3;

                bool bull1 = diff01 > 0.0f;
                bool bull2 = bull1 && diff12 > 0.0f;
                bool bull3 = bull2 && diff23 > 0.0f;
                bool bull4 = bull3 && diff34 > 0.0f;

                bool bear1 = diff01 < 0.0f;
                bool bear2 = bear1 && diff12 < 0.0f;
                bool bear3 = bear2 && diff23 < 0.0f;
                bool bear4 = bear3 && diff34 < 0.0f;
                
                bool troph1 = bull1 && diff12 < 0.0f;
                bool troph2 = troph1 && diff23 < 0.0f;
                bool troph3 = troph2 && diff34 < 0.0f;

                bool peak1 = bear1 && diff12 > 0.0f;
                bool peak2 = peak1 && diff23 > 0.0f;
                bool peak3 = peak2 && diff34 > 0.0f;

                bool bullish1 = bull1 && diff01 > diff12;
                bool bullish2 = bullish1 && diff12 > diff23;
                bool bullish3 = bullish2 && diff23 > diff34;

                bool bearish1 = bear1 && diff01 < diff12;
                bool bearish2 = bearish1 && diff12 < diff23;
                bool bearish3 = bearish2 && diff23 < diff34;

                float bull = (bull1 ? 1.0f : 0.0f) + (bull2 ? 1.0f : 0.0f) + (bull3 ? 1.0f : 0.0f) + (bull4 ? 1.0f : 0.0f);
                float bear = (bear1 ? 1.0f : 0.0f) + (bear2 ? 1.0f : 0.0f) + (bear3 ? 1.0f : 0.0f) + (bear4 ? 1.0f : 0.0f);
                float troph = (troph1 ? 1.0f : 0.0f) + (troph2 ? 1.0f : 0.0f) + (troph3 ? 1.0f : 0.0f);
                float peak = (peak1 ? 1.0f : 0.0f) + (peak2 ? 1.0f : 0.0f) + (peak3 ? 1.0f : 0.0f);
                float bullish = (bullish1 ? 1.0f : 0.0f) + (bullish2 ? 1.0f : 0.0f) + (bullish3 ? 1.0f : 0.0f);
                float bearish = (bearish1 ? 1.0f : 0.0f) + (bearish2 ? 1.0f : 0.0f) + (bearish3 ? 1.0f : 0.0f);
                
                float trend = (bull - bear) * 0.25f;
                float reversal = (troph - peak) * 0.33f;
                float accel = (bullish - bearish) * 0.33f;

                *outputTrend++ = trend;
                *outputReversal++ = reversal;
                *outputAccel++ = accel;
                *outputSum++ = scaleTrend * trend + scaleReversal * reversal + scaleAccel * accel;

                mPrev4 = mPrev3;
                mPrev3 = mPrev2;
                mPrev2 = mPrev1;
                mPrev1 = in1;
            }
        }

    protected:
        float mPrev1 = 0.0f;
        float mPrev2 = 0.0f;
        float mPrev3 = 0.0f;
        float mPrev4 = 0.0f;
    };

}

