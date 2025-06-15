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
#include "nodegraph/operations/NodeGraphOpTradingDataIO.h"
#include "nodegraph/operations/NodeGraphOpTradingDetector.h"
#include "nodegraph/operations/NodeGraphOpTradingFilter.h"
#include "nodegraph/operations/NodeGraphOpTradingIndicator.h"
#include "nodegraph/operations/NodeGraphOpDeviceIOInput.h"
#include "nodegraph/operations/NodeGraphOpDeviceIOOutput.h"
#include "nodegraph/operations/NodeGraphOpSource.h"
#include "nodegraph/operations/NodeGraphOpDataIO.h"
#include "nodegraph/operations/NodeGraphOpUI.h"

#include <serialization/JsonSerializationBase.h>

#include <string>
#include <vector>
#include <map>
#include <typeinfo>
#include <type_traits>
#include <memory>
#include <filesystem>

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
        std::string mName;
        int32_t mId = 0;
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

    class NodeGraphSchema : public l::serialization::JsonSerializationBase, public NodeFactoryBase {
    public:
        const int32_t kVersionMajor = 1;
        const int32_t kVersionMinor = 1;

        using CustomCreateFunctionType = NodeGraphBase * (int32_t, NodeGraphGroup&);

        NodeGraphSchema(std::string name = "", std::string typeName = "", bool useAllNodeTypes = false) :
            mName(name.empty() ? "Schema" : name),
            mTypeName(typeName)
        {
            mMainNodeGraph.SetNodeFactory(this);
            if (useAllNodeTypes) {
                RegisterAllOperators();
            }
        }

        virtual ~NodeGraphSchema() {
        }

        NodeGraphSchema& operator=(NodeGraphSchema&& other) noexcept {
            mVersionMajor = other.mVersionMajor;
            mVersionMinor = other.mVersionMinor;
            mName = other.mName;
            mTypeName = other.mTypeName;
            mFileName = other.mFileName;
            mFullPath = other.mFullPath;
            mStringId = other.mStringId;
            mMainNodeGraph = std::move(other.mMainNodeGraph);
            mMainNodeGraph.SetNodeFactory(this); // must set anew since schema (this) was moved as well
            mRegisteredNodeTypes = std::move(other.mRegisteredNodeTypes);
            mCreateCustomNode = std::move(other.mCreateCustomNode);
            mKeyState = other.mKeyState;
            mAudioOutput = other.mAudioOutput;
            mMidiManager = other.mMidiManager;
            mRegisteredNodeTypes = std::move(other.mRegisteredNodeTypes);
            mPickerRootMenu = mPickerRootMenu;
            return *this;
        }

        NodeGraphSchema(NodeGraphSchema&& other) noexcept {
            *this = std::move(other);
        }

        NodeGraphSchema& operator=(const NodeGraphSchema&) = delete;
        NodeGraphSchema(const NodeGraphSchema&) = delete;
        
        virtual bool NodeGraphNewNode(int32_t typeId, int32_t id) override;
        virtual bool NodeGraphWireIO(int32_t srcId, int8_t srcChannel, int32_t dstid, int8_t dstChannel) override;

        void RegisterAllOperators();

        void SetName(std::string_view name) {
            mName = name;
        }
        void SetTypeName(std::string_view typeName) {
            mTypeName = typeName;
        }
        std::string_view GetName() {
            return mName;
        }
        std::string_view GetTypeName() {
            return mTypeName;
        }
        std::string_view GetFileName() {
            return mFileName;
        }
        std::string_view GetFullPath() {
            return mFullPath;
        }
        uint32_t GetStringId() {
            if (mStringId == 0) {
                mStringId = l::string::string_id(mFullPath);
            }
            return mStringId;
        }

        bool Load(std::filesystem::path file);
        bool Save(std::filesystem::path file, bool cloneOnly = false);

        virtual bool LoadArchiveData(l::serialization::JsonValue& jsonValue) override;
        virtual void GetArchiveData(l::serialization::JsonBuilder& jsonBuilder) override;

        void SetCustomCreator(std::function<CustomCreateFunctionType> customCreator);
        void SetKeyState(l::hid::KeyState* keyState);
        void SetAudioOutput(l::audio::AudioStream* audioStream);
        void SetMidiManager(l::hid::midi::MidiManager* midiManager);

        int32_t NewNode(int32_t typeId, int32_t id = -1);
        bool RemoveNode(int32_t id);
        NodeGraphBase* GetNode(int32_t id);
        void ForEachInputNode(std::function<bool(NodeGraphBase*)> cb);
        void ForEachOutputNode(std::function<bool(NodeGraphBase*)> cb);
        
        bool HasNodeType(const std::string& typeGroup, int32_t typeId);
        void ForEachNodeType(std::function<void(std::string_view, const std::vector<UINodeDesc>&)> cb) const;
        void RegisterNodeType(const std::string& typeGroup, int32_t uniqueTypeId, std::string_view typeName);
        void RegisterAllOf(const std::string& typeGroup);
        void ProcessSubGraph(int32_t numSamples, int32_t numCacheSamples = -1);
        void Tick(int32_t tickCount, float delta);

        TreeMenuNode& GetPickerRoot();
    protected:
        int32_t mVersionMajor = 0;
        int32_t mVersionMinor = 0;
        std::string mName;
        std::string mTypeName;
        std::string mFileName;
        std::string mFullPath;
        uint32_t mStringId = 0;

        NodeGraphGroup mMainNodeGraph;

        std::function<CustomCreateFunctionType> mCreateCustomNode;
        l::hid::KeyState* mKeyState = nullptr;
        l::audio::AudioStream* mAudioOutput = nullptr;
        l::hid::midi::MidiManager* mMidiManager = nullptr;

        std::map<std::string, std::vector<UINodeDesc>> mRegisteredNodeTypes;
        TreeMenuNode mPickerRootMenu;
    };

}

