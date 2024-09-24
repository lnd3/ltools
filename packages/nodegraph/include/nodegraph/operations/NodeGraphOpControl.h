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

    class GraphControlBase : public NodeGraphOp {
    public:

        static const int8_t mNumDefaultInputs = 6;
        static const int8_t mNumDefaultOutputs = 2;

        GraphControlBase(NodeGraphBase* node, std::string_view name) :
            NodeGraphOp(node, name),
            mNodeInputManager(*this)
        {
            mNodeInputManager.AddInput(InputTypeBase::SAMPLED, AddInput("Sync", 0.0f, 1, 0.0f, 1.0f));
            mNodeInputManager.AddInput(InputTypeBase::SAMPLED, AddInput("Rate", 256.0f, 1, 1.0f, 2048.0f));
            mNodeInputManager.AddInput(InputTypeBase::SAMPLED, AddInput("Velocity", 0.5f, 1, 0.0f, 1.0f));
            mNodeInputManager.AddInput(InputTypeBase::SAMPLED, AddInput("Fade", 0.1f, 1, 0.0001f, 1.0f));
            mNodeInputManager.AddInput(InputTypeBase::SAMPLED, AddInput("Attack", 50.0f, 1, 1.0f, 10000.0f));
            mNodeInputManager.AddInput(InputTypeBase::SAMPLED, AddInput("Release", 50.0f, 1, 1.0f, 10000.0f));
            mNodeInputManager.AddCustom(InputTypeBase::CUSTOM_INTERP_RWA_MS); // 100
            mNodeInputManager.AddCustom(InputTypeBase::CUSTOM_INTERP_RWA_MS); // 101

            AddOutput("Freq");
            AddOutput("Volume");

            mSamplesUntilUpdate = 0.0f;
        }

        virtual ~GraphControlBase() = default;
        virtual void Process(int32_t numSamples, std::vector<NodeGraphInput>& inputs, std::vector<NodeGraphOutput>& outputs) override;
        virtual void UpdateSignal(NodeInputManager& inputManager) = 0;
        virtual std::pair<float, float> ProcessSignal(NodeInputManager& inputManager) = 0;
    protected:
        NodeInputManager mNodeInputManager;
        float mUpdateRate = 256.0f;

        float mDeltaTime = 0.0f;
        float mSamplesUntilUpdate = 0.0f;
    };

    /*********************************************************************/
    class GraphControlEnvelope : public GraphControlBase {
    public:
        GraphControlEnvelope(NodeGraphBase* node) :
            GraphControlBase(node, "Envelope")
        {
            mNodeInputManager.AddInput(InputTypeBase::SAMPLED_RWA, AddInput("Freq"));
            mEnvelope = 0.0f;
        }

        virtual ~GraphControlEnvelope() = default;
        virtual void UpdateSignal(NodeInputManager& inputManager) override;
        virtual std::pair<float, float> ProcessSignal(NodeInputManager& inputManager) override;
    protected:
        bool mNoteOn = false;
        int32_t mFrameCount = 0;
        int32_t mAttackFrames = 0;
        int32_t mReleaseFrames = 0;
        float mAttackFactor = 0.0f;
        float mReleaseFactor = 0.0f;
        float mEnvelopeTarget = 0.0f;
        float mFreqTarget = 0.0f;
        float mFreqTargetPrev = 0.0f;
        float mEnvelope = 0.0f;
    };

    /*********************************************************************/
    class GraphControlArpeggio: public NodeGraphOp {
    public:

        const float gArpeggioUpdateRate = 16.0f;

        const static int32_t gPolyphony = 12;
        GraphControlArpeggio(NodeGraphBase* node) :
            NodeGraphOp(node, "Arpeggio"),
            mNodeInputManager(*this)
        {
            mNodeInputManager.AddInput(InputTypeBase::SAMPLED, AddInput("Sync", 0.0f, 1, 0.0f, 1.0f));
            mNodeInputManager.AddInput(InputTypeBase::SAMPLED_RWA, AddInput("Bpm", 60.0f, 1, 1.0f, 1000.0f));
            mNodeInputManager.AddInput(InputTypeBase::SAMPLED, AddInput("Velocity", 0.5f, 1, 0.0f, 1.0f));
            mNodeInputManager.AddInput(InputTypeBase::SAMPLED, AddInput("Fade", 1.0f, 1, 0.0001f, 1.0f));
            mNodeInputManager.AddInput(InputTypeBase::SAMPLED, AddInput("Attack", 5.0f, 1, 1.0f, 10000.0f));
            mNodeInputManager.AddInput(InputTypeBase::SAMPLED, AddInput("Release", 20.0f, 1, 1.0f, 10000.0f));
            mNodeInputManager.AddInput(InputTypeBase::CONSTANT_ARRAY, AddInput("Note On", l::audio::gNoNote_f, gPolyphony, -499.0, 500.0));
            mNodeInputManager.AddInput(InputTypeBase::CONSTANT_ARRAY, AddInput("Note Off", l::audio::gNoNote_f, gPolyphony, -499.0, 500.0));
            mNodeInputManager.AddCustom(InputTypeBase::CUSTOM_INTERP_RWA_MS); // 100
            mNodeInputManager.AddCustom(InputTypeBase::CUSTOM_INTERP_RWA_MS); // 101

            AddOutput("Freq");
            AddOutput("Volume");

            mGainTarget = 0.0f;
        }

        virtual ~GraphControlArpeggio() = default;
        virtual void Process(int32_t numSamples, std::vector<NodeGraphInput>& inputs, std::vector<NodeGraphOutput>& outputs) override;
        void UpdateSignal(NodeInputManager& inputManager);
    protected:
        NodeInputManager mNodeInputManager;

        float mSamplesUntilUpdate = 0.0f;
        float mUpdateRate = 256.0f;
        int32_t mUpdateCounter = 0;

        float mGainTarget = 0.0f;
        float mGain = 0.0f;
        float mGainAttack = 0.01f;
        float mGainRelease = 0.01f;

        float mFreqSmoothing = 0.1f;
        float mFreqTarget = 0.0f;
        float mFreq = 0.0f;
        std::vector<int32_t> mNotes;
        int32_t mNoteIndex = 0;

    };
}

