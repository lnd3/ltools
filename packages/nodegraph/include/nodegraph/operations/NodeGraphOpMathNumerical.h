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
    class MathNumericalIntegral : public NodeGraphOp {
    public:
        MathNumericalIntegral(NodeGraphBase* node) :
            NodeGraphOp(node, "Integral")
        {
            AddInput("In", 0.0f, 1);
            AddInput("Friction", 1.0f, 1, 0.0f, 1.0f);
            AddInput("Lod", 0.0f, 1, 0.0f, 1.0f);
            AddOutput("Out", 0.0f, 1);
        }

        virtual ~MathNumericalIntegral() = default;
        virtual void Process(int32_t numSamples, int32_t numCacheSamples, std::vector<NodeGraphInput>& inputs, std::vector<NodeGraphOutput>& outputs) override;
        virtual void Reset() override {
            mOutput = 0.0f;
        }
    protected:
        int32_t mReadSamples = 0;

        float mOutput = 0.0f;
    };

    /*********************************************************************/
    class MathNumericalDerivate : public NodeGraphOp {
    public:
        MathNumericalDerivate(NodeGraphBase* node) :
            NodeGraphOp(node, "Derivate")
        {
            AddInput("In", 0.0f, 1);
            AddInput("Lod", 0.0f, 1, 0.0f, 1.0f);
            AddOutput("Out", 0.0f, 1);
        }

        virtual ~MathNumericalDerivate() = default;
        virtual void Process(int32_t numSamples, int32_t numCacheSamples, std::vector<NodeGraphInput>& inputs, std::vector<NodeGraphOutput>& outputs) override;
    protected:
        int32_t mReadSamples = 0;

        float mInputPrev = 0.0f;
    };

    /*********************************************************************/
    class MathNumericalDiffNorm : public NodeGraphOp {
    public:
        MathNumericalDiffNorm(NodeGraphBase* node) :
            NodeGraphOp(node, "Difference Normalized")
        {
            AddInput("In", 0.0f, 1);
            AddInput("Lod", 0.0f, 1, 0.0f, 1.0f);
            AddOutput("Out", 0.0f, 1);
        }

        virtual ~MathNumericalDiffNorm() = default;
        virtual void Process(int32_t numSamples, int32_t numCacheSamples, std::vector<NodeGraphInput>& inputs, std::vector<NodeGraphOutput>& outputs) override;
    protected:
        int32_t mReadSamples = 0;

        float mInputPrev = 0.0f;
    };

    /*********************************************************************/
    class MathNumericalDiff : public NodeGraphOp {
    public:
        MathNumericalDiff(NodeGraphBase* node) :
            NodeGraphOp(node, "Difference")
        {
            AddInput("In", 0.0f, 1);
            AddInput("Lod", 0.0f, 1, 0.0f, 1.0f);
            AddOutput("Out", 0.0f, 1);
        }

        virtual ~MathNumericalDiff() = default;
        virtual void Process(int32_t numSamples, int32_t numCacheSamples, std::vector<NodeGraphInput>& inputs, std::vector<NodeGraphOutput>& outputs) override;
    protected:
        int32_t mReadSamples = 0;

        float mInputPrev = 0.0f;
    };

    /*********************************************************************/
    class MathNumericalLevelTrigger : public NodeGraphOp {
    public:
        MathNumericalLevelTrigger(NodeGraphBase* node) :
            NodeGraphOp(node, "Level Trigger")
        {
            AddInput2("In", 1, InputFlags(false, false, false, false));
            AddInput2("Max", 1, InputFlags(false, false, false, false));
            AddInput2("Min", 1, InputFlags(false, false, false, false));
            AddInput("Num levels", 1.0f, 1, 1.0f, 100.0f, true, true);
            AddInput("Max%", 1.0f, 1, 0.0f, 1.0f, true, true);
            AddInput("Min%", 0.0f, 1, 0.0f, 1.0f, true, true);
            AddOutput("Level", 0.0f, 1);
            AddOutput("Pulse", 0.0f, 1);
        }

        virtual ~MathNumericalLevelTrigger() = default;
        virtual void Process(int32_t numSamples, int32_t, std::vector<NodeGraphInput>& inputs, std::vector<NodeGraphOutput>& outputs) override;
    protected:
        float mLevelPrev = 0.0f;
    };

    /*********************************************************************/
    class MathNumericalMinMaxChannel : public NodeGraphOp {
    public:
        MathNumericalMinMaxChannel(NodeGraphBase* node) :
            NodeGraphOp(node, "Minmax Channel")
        {
            AddInput("Upper Bound", 0.0f, 1);
            AddInput("Lower Bound", 0.0f, 1);
            AddInput("Bounded Value", 0.0f, 1);

            AddOutput("Range", 0.0f, 1);
            AddOutput("Range Max", 0.0f, 1);
            AddOutput("Range Min", 0.0f, 1);
            AddOutput("Value Norm", 0.0f, 1);
        }

        virtual ~MathNumericalMinMaxChannel() = default;
        virtual void Process(int32_t numSamples, int32_t numCacheSamples, std::vector<NodeGraphInput>& inputs, std::vector<NodeGraphOutput>& outputs) override;
    protected:
        int32_t mReadSamples = 0;

        float mCurRangeMax = 0.0f;
        float mCurRangeMin = 0.0f;
    };

    /*********************************************************************/
    class MathNumericalDerivate2 : public NodeGraphOp {
    public:
        MathNumericalDerivate2(NodeGraphBase* node) :
            NodeGraphOp(node, "Derivate2")
        {
            AddInput("In", 0.0f, 1);
            AddInput("Base", 0.0f, 1);
            AddInput("Friction1", 1.0f, 1, 0.0f, 1.0f);
            AddInput("Friction2", 1.0f, 1, 0.0f, 1.0f);
            AddInput("Scale1", 1.0f, 1, 0.0f, 100.0f);
            AddInput("Scale2", 1.0f, 1, 0.0f, 100.0f);

            AddOutput("d1dt", 0.0f, 1);
            AddOutput("d2dt", 0.0f, 1);
            AddOutput("d1+base", 0.0f, 1);
            AddOutput("d2+base", 0.0f, 1);
        }

        virtual ~MathNumericalDerivate2() = default;
        virtual void Process(int32_t numSamples, int32_t numCacheSamples, std::vector<NodeGraphInput>& inputs, std::vector<NodeGraphOutput>& outputs) override;
    protected:
        int32_t mReadSamples = 0;
        float mInputPrev1 = 0.0f;
        float mOutput1 = 0.0f;
        float mInputPrev2 = 0.0f;
        float mOutput2 = 0.0f;
    };
}