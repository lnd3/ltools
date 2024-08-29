#pragma once

#include "rendering/ui/UIContainer.h"
#include "nodegraph/NodeGraphSchema.h"

#include <unordered_set>

namespace l::ui {

    class UIUpdate : public UIVisitor {
    public:
        UIUpdate() {}
        ~UIUpdate() = default;

        virtual bool ShouldUpdateContainer();
    };

    class UIZoom : public UIVisitor {
    public:
        virtual bool Active(UIContainer& container, const InputState& input);
        virtual bool Visit(UIContainer& container, const InputState& input);
    };

    class UIDrag : public UIVisitor {
    public:
        virtual bool Active(UIContainer& container, const InputState& input);
        virtual bool Visit(UIContainer& container, const InputState& input);
    protected:
        bool mDragging = false;
        UIContainer* mSourceContainer = nullptr;
    };

    class UIMove : public UIVisitor {
    public:
        virtual bool Active(UIContainer& container, const InputState& input);
        virtual bool Visit(UIContainer& container, const InputState& input);
    protected:
        bool mMoving = false;
        UIContainer* mSourceContainer = nullptr;
    };

    class UIResize : public UIVisitor {
    public:
        virtual bool Visit(UIContainer& container, const InputState& input);
    protected:
        bool mResizing = false;
        UIContainer* mSourceContainer = nullptr;
    };

    class UISelect : public UIVisitor {
    public:
        UISelect(UIStorage& uiStorage) : mUIStorage(uiStorage) {}

        virtual bool Visit(UIContainer& container, const InputState& input);

        void SetNGSchema(l::nodegraph::NodeGraphSchema* ngSchema) {
            mNGSchema = ngSchema;
        }
    protected:
        std::unordered_set<UIContainer*> mSelectedContainers;
        UIStorage& mUIStorage;
        l::nodegraph::NodeGraphSchema* mNGSchema = nullptr;
    };

    class UIEdit : public UIVisitor {
    public:
        virtual bool Visit(UIContainer& container, const InputState& input);

        void SetNGSchema(l::nodegraph::NodeGraphSchema* ngSchema) {
            mNGSchema = ngSchema;
        }
    protected:
        bool mEditing = false;
        UIContainer* mSourceContainer = nullptr;
        l::nodegraph::NodeGraphSchema* mNGSchema = nullptr;
    };

    class UIDraw : public UIVisitor {
    public:
        UIDraw(ImDrawList* drawList = nullptr) : mDrawList(drawList) {}
        ~UIDraw() = default;

        virtual bool Visit(UIContainer& container, const InputState& input);

        void SetDrawList(ImDrawList* drawList) {
            mDrawList = drawList;
        }

        void SetNGSchema(l::nodegraph::NodeGraphSchema* ngSchema) {
            mNGSchema = ngSchema;
        }

    protected:
        ImDrawList* mDrawList;
        l::nodegraph::NodeGraphSchema* mNGSchema = nullptr;
    };

    class UILinkIO : public UIVisitor {
    public:
        using HandlerFunctionType = bool(int32_t, int32_t, int32_t, int32_t, bool);

        UILinkIO(UIStorage& uiStorage) : mUIStorage(uiStorage) {}
        ~UILinkIO() = default;

        virtual bool Active(UIContainer& container, const InputState& input);
        virtual bool Visit(UIContainer& container, const InputState& input);

        bool LinkHandler(int32_t linkInputId, int32_t linkOutputId, int32_t inputChannel, int32_t outputChannel, bool connected) {
            if (mNGSchema == nullptr) {
                return false;
            }

            auto inputNode = mNGSchema->GetNode(linkInputId);
            if (inputNode == nullptr) {
                return false;
            }
            if (connected) {
                auto outputNode = mNGSchema->GetNode(linkOutputId);
                return outputNode != nullptr && inputNode->SetInput(static_cast<int8_t>(inputChannel), *outputNode, static_cast<int8_t>(outputChannel));
            }
            return inputNode->ClearInput(static_cast<int8_t>(inputChannel));
        }

        void SetNGSchema(l::nodegraph::NodeGraphSchema* ngSchema) {
            mNGSchema = ngSchema;
        }

    protected:
        bool mDragging = false;
        UIHandle mLinkContainer;
        UIStorage& mUIStorage;
        l::nodegraph::NodeGraphSchema* mNGSchema = nullptr;
    };


}
