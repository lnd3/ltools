#pragma once

#include "logging/LoggingAll.h"

#include "nodegraph/core/NodeGraphGroup.h"
#include "nodegraph/operations/NodeGraphOpControl.h"
#include "nodegraph/operations/NodeGraphOpEffect.h"
#include "nodegraph/operations/NodeGraphOpFilter.h"
#include "nodegraph/operations/NodeGraphOpInput.h"
#include "nodegraph/operations/NodeGraphOpLogic.h"
#include "nodegraph/operations/NodeGraphOpNumeric.h"
#include "nodegraph/operations/NodeGraphOpOutput.h"
#include "nodegraph/operations/NodeGraphOpSource.h"
#include "nodegraph/operations/NodeGraphOpSignal.h"
#include "nodegraph/operations/NodeGraphOpDataBus.h"
#include "nodegraph/operations/NodeGraphOpUI.h"

#include <string>
#include <vector>
#include <map>
#include <typeinfo>
#include <type_traits>
#include <memory>

namespace l::nodegraph {

    struct UINodeDesc {
        std::string_view GetName() const {
            return mName;
        }
        int32_t GetId() const {
            return mId;
        }

        int32_t mId;
        std::string mName;
    };

    class NodeGraphSchema {
    public:

        using CustomCreateFunctionType = NodeGraphBase*(int32_t, NodeGraphGroup&);

        NodeGraphSchema(std::string name = "", bool useAllNodeTypes = false) :
            mName(name.empty() ? "Schema" : name)
        {
            if (useAllNodeTypes) {
                RegisterAllOf("Numeric");
                RegisterAllOf("Logic");
                RegisterAllOf("Signal");
                RegisterAllOf("SignalControl");
                RegisterAllOf("SignalFilter");
                RegisterAllOf("SignalEffect");
                RegisterAllOf("NodeGraphSource");
                RegisterAllOf("DeviceInput");
                RegisterAllOf("ExternalInput");
                RegisterAllOf("NodeGraphOutput");
                RegisterAllOf("DeviceOutput");
                RegisterAllOf("ExternalOutput");
                RegisterAllOf("Cache");
                RegisterAllOf("UI");
            }
        }

        ~NodeGraphSchema() = default;

        NodeGraphSchema& operator=(NodeGraphSchema&& other) noexcept {
            mMainNodeGraph = std::move(other.mMainNodeGraph);
            mName = std::move(other.mName);
            mRegisteredNodeTypes = std::move(other.mRegisteredNodeTypes);
            mCreateCustomNode = other.mCreateCustomNode;
            mKeyState = other.mKeyState;
            mAudioOutput = other.mAudioOutput;
            mMidiManager = other.mMidiManager;
            return *this;
        }
        NodeGraphSchema& operator=(const NodeGraphSchema& other) noexcept {
            mMainNodeGraph = other.mMainNodeGraph;
            mName = other.mName;
            mRegisteredNodeTypes = other.mRegisteredNodeTypes;
            mCreateCustomNode = other.mCreateCustomNode;
            mKeyState = other.mKeyState;
            mAudioOutput = other.mAudioOutput;
            mMidiManager = other.mMidiManager;
            return *this;
        }
        NodeGraphSchema(NodeGraphSchema&& other) noexcept {
            *this = std::move(other);
        }
        NodeGraphSchema(const NodeGraphSchema& other) noexcept {
            *this = other;
        }

        void SetName(std::string_view name) {
            mName = name;
        }
        std::string_view GetName() {
            return mName;
        }

        void SetCustomCreator(std::function<CustomCreateFunctionType> customCreator);
        void SetKeyState(l::hid::KeyState* keyState);
        void SetAudioOutput(l::audio::AudioStream* audioStream);
        void SetMidiManager(l::hid::midi::MidiManager* midiManager);

        int32_t NewNode(int32_t typeId);
        bool RemoveNode(int32_t nodeId);
        NodeGraphBase* GetNode(int32_t nodeId);
        void ForEachInputNode(std::function<bool(NodeGraphBase*)> cb);
        void ForEachOutputNode(std::function<bool(NodeGraphBase*)> cb);
        
        bool HasNodeType(const std::string& typeGroup, int32_t typeId);
        void ForEachNodeType(std::function<void(std::string_view, const std::vector<UINodeDesc>&)> cb) const;
        void RegisterNodeType(const std::string& typeGroup, int32_t uniqueTypeId, std::string_view typeName);
        void RegisterAllOf(const std::string& typeGroup);
        void ProcessSubGraph(int32_t numSamples, int32_t numCacheSamples = 0);
        void Tick(int32_t tickCount, float delta);
    protected:
        NodeGraphGroup mMainNodeGraph;
        std::string mName;

        std::function<CustomCreateFunctionType> mCreateCustomNode;
        l::hid::KeyState* mKeyState = nullptr;
        l::audio::AudioStream* mAudioOutput = nullptr;
        l::hid::midi::MidiManager* mMidiManager = nullptr;

        std::map<std::string, std::vector<UINodeDesc>> mRegisteredNodeTypes;
    };

}

