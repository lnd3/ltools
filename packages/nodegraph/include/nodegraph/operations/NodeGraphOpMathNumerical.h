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
            AddInput2("x");
            AddInput("Friction", 1.0f, 1, 0.0f, 1.0f);
            AddOutput2("Intgr(x)");
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
    class MathNumericalTemporalChange : public NodeGraphOp {
    public:
        MathNumericalTemporalChange(NodeGraphBase* node) :
            NodeGraphOp(node, "Temporal Change")
        {
            AddInput2("in");
            AddOutput2("out");
        }

        virtual ~MathNumericalTemporalChange() = default;
        virtual void Process(int32_t numSamples, int32_t numCacheSamples, std::vector<NodeGraphInput>& inputs, std::vector<NodeGraphOutput>& outputs) override;
    protected:
        int32_t mReadSamples = 0;

        float mInputPrev = 0.0f;
    };

    /*********************************************************************/
    class MathNumericalDiffNorm : public NodeGraphOp {
    public:
        MathNumericalDiffNorm(NodeGraphBase* node) :
            NodeGraphOp(node, "Temporal diff norm")
        {
            AddInput2("x");
            AddOutput2("Diff norm");
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
            NodeGraphOp(node, "Temporal Difference")
        {
            AddInput2("In");
            AddOutput2("Diff");
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
            AddInput2("In");
            AddInput2("Max");
            AddInput2("Min");
            AddInput("Num levels", 1.0f, 1, 1.0f, 100.0f, true, true);
            AddInput("Max", 1.0f, 1, 0.0f, 3.0f, true, true);
            AddInput("Min", 0.0f, 1, -3.0f, 1.0f, true, true);
            AddOutput2("Level");
            AddOutput2("Pulse");
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
            AddInput("Max", 1.0f, 1);
            AddInput("Min", 0.0f, 1);
            AddInput2("In");
            AddInput("Friction", 1.0f, 1, 0.0f, 1.0f);

            AddOutput2("Range");
            AddOutput2("Range Max");
            AddOutput2("Range Min");
            AddOutput2("Range Norm");
        }

        virtual ~MathNumericalMinMaxChannel() = default;
        virtual void Process(int32_t numSamples, int32_t numCacheSamples, std::vector<NodeGraphInput>& inputs, std::vector<NodeGraphOutput>& outputs) override;
    protected:
        int32_t mReadSamples = 0;

        float mCurRangeMax = 0.0f;
        float mCurRangeMin = 0.0f;
    };

    /*********************************************************************/
    class MathNumericalReconstructor : public NodeGraphOp {
    public:
        MathNumericalReconstructor(NodeGraphBase* node) :
            NodeGraphOp(node, "Reconstructor")
        {
            AddInput2("In");
            AddInput("Base", 0.0f, 1);
            AddInput("Friction1", 1.0f, 1, 0.0f, 1.0f);
            AddInput("Friction2", 1.0f, 1, 0.0f, 1.0f);
            AddInput("Scale1", 1.0f, 1, 0.0f, 100.0f);
            AddInput("Scale2", 1.0f, 1, 0.0f, 100.0f);

            AddOutput2("Diff");
            AddOutput2("Diff+base");
            AddOutput2("Intgr1");
            AddOutput2("Intgr+base");
            AddOutput2("Intgr2");
            AddOutput2("Intgr2+base");
        }

        virtual ~MathNumericalReconstructor() = default;
        virtual void Process(int32_t numSamples, int32_t numCacheSamples, std::vector<NodeGraphInput>& inputs, std::vector<NodeGraphOutput>& outputs) override;
    protected:
        int32_t mReadSamples = 0;
        float mInputPrev = 0.0f;
        float mDiffPrev = 0.0f;
        float mOutput1 = 0.0f;
        float mOutput2 = 0.0f;
    };

    /*********************************************************************/
    class MathNumericalReconstructor2 : public NodeGraphOp {
    public:
        MathNumericalReconstructor2(NodeGraphBase* node) :
            NodeGraphOp(node, "Reconstructor 2")
        {
            AddInput2("In1");
            AddInput2("In2");
            AddInput("Friction1", 1.0f, 1, 0.0f, 1.0f);
            AddInput("Friction2", 1.0f, 1, 0.0f, 1.0f);

            AddOutput2("Intgr1");
            AddOutput2("Intgr1");
            AddOutput2("Intgr Both");
        }

        virtual ~MathNumericalReconstructor2() = default;
        virtual void Process(int32_t numSamples, int32_t numCacheSamples, std::vector<NodeGraphInput>& inputs, std::vector<NodeGraphOutput>& outputs) override;
    protected:
        int32_t mReadSamples = 0;

        float mInPrev1 = 0.0f;
        float mIn1Prev1 = 0.0f;
        float mIn2Prev1 = 0.0f;

        float mInAccum = 0.0f;
        float mIn1Accum = 0.0f;
        float mIn2Accum = 0.0f;
        float mInAccumPrev1 = 0.0f;
        float mIn1AccumPrev1 = 0.0f;
        float mIn2AccumPrev1 = 0.0f;
    };

    /*********************************************************************/
    class MathNumericalNormalizer : public nodegraph::NodeGraphOp {
    public:
        MathNumericalNormalizer(nodegraph::NodeGraphBase* node) :
            NodeGraphOp(node, "Normalizer")
        {
            AddInput2("In");
            AddInput("Friction", 0.5f, 1, 0.0f, 1.0f);

            AddOutput2("Out");
        }

        virtual ~MathNumericalNormalizer() = default;
        virtual void Process(int32_t numSamples, int32_t numCacheSamples, std::vector<nodegraph::NodeGraphInput>& inputs, std::vector<nodegraph::NodeGraphOutput>& outputs) override;
    protected:
        float mInAbsPrev = 0.0f;
        float mTargetMagnitude = 1.0f;
    };

}