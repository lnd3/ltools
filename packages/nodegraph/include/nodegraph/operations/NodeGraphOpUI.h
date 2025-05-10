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
    class GraphUICheckbox : public NodeGraphOp {
    public:
        GraphUICheckbox(NodeGraphBase* node) :
            NodeGraphOp(node, "UI Checkbox")
        {
            AddInput("In", 0.0f, 1, 0.0f, 1.0f);
            AddOutput("Out");
        }

        virtual ~GraphUICheckbox() = default;
        virtual void Process(int32_t numSamples, std::vector<NodeGraphInput>& inputs, std::vector<NodeGraphOutput>& outputs) override;
        virtual void Tick(int32_t, float) override;

        bool& GetStatePtr();
        void ExternallyChanged();
    protected:
        bool mExternallyChanged = false;
        bool mState = false;
    };

    /*********************************************************************/
    class GraphUISlider : public NodeGraphOp {
    public:
        GraphUISlider(NodeGraphBase* node, float min, float max, float power) :
            NodeGraphOp(node, "UI Slider")
        {
            AddInput("In", 0.0f, 1, min, max);
            AddInput("Min", 0.0f);
            AddInput("Max", 1.0f);
            AddInput("Power", power);
            AddOutput("Out");
        }

        virtual ~GraphUISlider() = default;
        virtual void Process(int32_t numSamples, std::vector<NodeGraphInput>& inputs, std::vector<NodeGraphOutput>& outputs) override;
        virtual void Tick(int32_t, float) override;

        float& GetMin();
        float& GetMax();
        float& GetValue();
        void ExternallyChanged();
    protected:
        bool mExternallyChanged = false;
        float mValue = 0.0f;
        float mMin = 0.0f;
        float mMax = 1.0f;
    };

    /*********************************************************************/
    class GraphUIChartLine : public NodeGraphOp {
    public:
        GraphUIChartLine(NodeGraphBase* node) :
            NodeGraphOp(node, "Chart Lines"),
            mInputManager(*this)
        {
            mInputManager.AddInput(InputIterationType::SAMPLED, AddInput("x", 0.0f, 1, -l::math::constants::FLTMAX, l::math::constants::FLTMAX, false, false));
            mInputManager.AddInput(InputIterationType::SAMPLED, AddInput("y", 0.0f, 1, -l::math::constants::FLTMAX, l::math::constants::FLTMAX, false, false));
            AddOutput("Interleaved Data");
        }
        virtual ~GraphUIChartLine() = default;

        virtual void Process(int32_t numSamples, std::vector<NodeGraphInput>& inputs, std::vector<NodeGraphOutput>& outputs) override;
    protected:
        InputManager mInputManager;
        float mStepsUntilUpdate = 0.0f;
        float mUpdateRate = 16.0f;
    };

    /*********************************************************************/
    class GraphUICandleSticks : public NodeGraphOp {
    public:
        GraphUICandleSticks(NodeGraphBase* node) :
            NodeGraphOp(node, "Candle Sticks"),
            mInputManager(*this)
        {
            mInputManager.AddInput(InputIterationType::SAMPLED, AddInput("unixtime", 0.0f, 1, -l::math::constants::FLTMAX, l::math::constants::FLTMAX, false, false));
            mInputManager.AddInput(InputIterationType::SAMPLED, AddInput("open", 0.0f, 1, -l::math::constants::FLTMAX, l::math::constants::FLTMAX, false, false));
            mInputManager.AddInput(InputIterationType::SAMPLED, AddInput("close", 0.0f, 1, -l::math::constants::FLTMAX, l::math::constants::FLTMAX, false, false));
            mInputManager.AddInput(InputIterationType::SAMPLED, AddInput("high", 0.0f, 1, -l::math::constants::FLTMAX, l::math::constants::FLTMAX, false, false));
            mInputManager.AddInput(InputIterationType::SAMPLED, AddInput("low", 0.0f, 1, -l::math::constants::FLTMAX, l::math::constants::FLTMAX, false, false));
            mInputManager.AddInput(InputIterationType::SAMPLED, AddInput("volume", 0.0f, 1, -l::math::constants::FLTMAX, l::math::constants::FLTMAX, false, false));
            AddOutput("Interleaved Data");
        }
        virtual ~GraphUICandleSticks() = default;

        virtual void Process(int32_t numSamples, std::vector<NodeGraphInput>& inputs, std::vector<NodeGraphOutput>& outputs) override;
    protected:
        InputManager mInputManager;
        float mStepsUntilUpdate = 0.0f;
        float mUpdateRate = 16.0f;
    };
}

