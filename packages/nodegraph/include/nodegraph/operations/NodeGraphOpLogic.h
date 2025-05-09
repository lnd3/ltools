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
                bool bool1 = (*input1++) > 0.0f;
                bool bool2 = (*input2++) > 0.0f;
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
                bool bool1 = (*input1++) > 0.0f;
                bool bool2 = (*input2++) > 0.0f;
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
                bool bool1 = (*input1++) > 0.0f;
                bool bool2 = (*input2++) > 0.0f;
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
            AddOutput("Bull", 0.0f);
            AddOutput("Bear", 0.0f);
            AddOutput("Peak", 0.0f);
            AddOutput("Troph", 0.0f);
            AddOutput("Lift", 0.0f);
            AddOutput("Drop", 0.0f);
        }

        virtual ~GraphLogicalDetector() = default;
        virtual void Process(int32_t numSamples, std::vector<NodeGraphInput>& inputs, std::vector<NodeGraphOutput>& outputs) override {
            auto input = inputs.at(0).GetIterator(numSamples);
            auto outputBull = outputs.at(0).GetIterator(numSamples);
            auto outputBear = outputs.at(1).GetIterator(numSamples);
            auto outputPeak = outputs.at(2).GetIterator(numSamples);
            auto outputTroph = outputs.at(3).GetIterator(numSamples);
            auto outputBullish = outputs.at(4).GetIterator(numSamples);
            auto outputBearish = outputs.at(5).GetIterator(numSamples);

            for (int32_t i = 0; i < numSamples; i++) {
                float in = (*input++);
                float diff01 = in - mPrev1;
                float diff12 = mPrev1 - mPrev2;
                float diff02 = in - mPrev2;

                bool bull = false;
                bool bear = false;
                bool peak = false;
                bool troph = false;
                bool bullish = false;
                bool bearish = false;

                if (diff02 > 0.0f) {
                    bull = true;
                }
                if (diff02 < 0.0f) {
                    bear = true;
                }
                if (diff01 < 0.0f && diff12 > 0.0f) {
                    peak = true;
                }
                if (diff01 > 0.0f && diff12 < 0.0f) {
                    troph = true;
                }
                if (diff01 > 0.0f && diff12 > 0.0f && diff01 > diff12) {
                    bullish = true;
                }
                if (diff01 < 0.0f && diff12 < 0.0f && diff01 < diff12) {
                    bearish = true;
                }

                *outputBull++ = bull ? 1.0f : 0.0f;
                *outputBear++ = bear ? 1.0f : 0.0f;
                *outputPeak++ = peak ? 1.0f : 0.0f;
                *outputTroph++ = troph ? 1.0f : 0.0f;
                *outputBullish++ = bullish ? 1.0f : 0.0f;
                *outputBearish++ = bearish ? 1.0f : 0.0f;

                mPrev2 = mPrev1;
                mPrev1 = in;
            }
        }

    protected:
        float mPrev1 = 0.0f;
        float mPrev2 = 0.0f;
    };

}

