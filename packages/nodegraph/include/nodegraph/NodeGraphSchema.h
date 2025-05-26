#pragma once

#include "logging/LoggingAll.h"

#include "nodegraph/core/NodeGraphGroup.h"
#include "nodegraph/operations/NodeGraphOpSignalGenerator.h"
#include "nodegraph/operations/NodeGraphOpSignalControl.h"
#include "nodegraph/operations/NodeGraphOpSignalEffect.h"
#include "nodegraph/operations/NodeGraphOpSignalFilter.h"
#include "nodegraph/operations/NodeGraphOpMathLogic.h"
#include "nodegraph/operations/NodeGraphOpMathNumerical.h"
#include "nodegraph/operations/NodeGraphOpMathAritmethic.h"
#include "nodegraph/operations/NodeGraphOpFuzzyDetector.h"
#include "nodegraph/operations/NodeGraphOpFuzzyFilter.h"
#include "nodegraph/operations/NodeGraphOpDeviceIOInput.h"
#include "nodegraph/operations/NodeGraphOpDeviceIOOutput.h"
#include "nodegraph/operations/NodeGraphOpSource.h"
#include "nodegraph/operations/NodeGraphOpDataIO.h"
#include "nodegraph/operations/NodeGraphOpUI.h"

#include <string>
#include <vector>
#include <map>
#include <typeinfo>
#include <type_traits>
#include <memory>

namespace l::nodegraph {

    class TreeMenuNode {
    public:
        TreeMenuNode() = default;
        TreeMenuNode(std::string_view pathPart, std::string_view name, int32_t id) : mPathPart(pathPart), mName(name), mId(id) {}
        ~TreeMenuNode() = default;

        std::string_view GetPathPart() const {
            return mPathPart;
        }
        std::string_view GetName() const {
            return mName;
        }
        int32_t GetId() const {
            return mId;
        }

        std::vector<TreeMenuNode> mChildren;
        std::string mPathPart;
    protected:
        int32_t mId = 0;
        std::string mName;
    };

    TreeMenuNode* findOrCreateChild(TreeMenuNode& node, std::string_view pathPart);
    void insertPath(TreeMenuNode& root, std::string_view path, std::string_view name, int32_t nodeId);

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
                RegisterAllOf("NodeGraph.Source");
                RegisterAllOf("NodeGraph.Output");
                RegisterAllOf("NodeGraph.Cache");

                RegisterAllOf("Math.Aritmethic");
                RegisterAllOf("Math.Numerical");
                RegisterAllOf("Math.Logic");

                RegisterAllOf("Fuzzy.Detector");
                RegisterAllOf("Fuzzy.Filter");

                RegisterAllOf("Signal.Generator");
                RegisterAllOf("Signal.Control");
                RegisterAllOf("Signal.Filter");
                RegisterAllOf("Signal.Effect");

                RegisterAllOf("Device.Input");
                RegisterAllOf("Device.Output");

                RegisterAllOf("External.Input");
                RegisterAllOf("External.Output");

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

        TreeMenuNode& GetPickerRoot();
    protected:
        NodeGraphGroup mMainNodeGraph;
        std::string mName;

        std::function<CustomCreateFunctionType> mCreateCustomNode;
        l::hid::KeyState* mKeyState = nullptr;
        l::audio::AudioStream* mAudioOutput = nullptr;
        l::hid::midi::MidiManager* mMidiManager = nullptr;

        std::map<std::string, std::vector<UINodeDesc>> mRegisteredNodeTypes;
        TreeMenuNode mPickerRootMenu;
    };

}

