#pragma once

#include "logging/LoggingAll.h"

#include <functional>
#include <memory>

namespace l {
namespace memory {

    template<class T>
    class Handle {
    public:
        using ConstructorType = T*();
        using DestructorType = void(T*);

        template<class T>
        std::unique_ptr<Handle<T>> Create(std::function<ConstructorType> constructor, std::function<DestructorType> destructor) {
            return std::make_unique<Handle<T>>(std::move(constructor), std::move(destructor));
        }

        template<class T>
        bool construct() {
            if (!mInstance && mConstruct) {
                mInstance = mConstruct();
                if (mInstance) {
                    return true;
                }
            }
            return false;
        }

        template<class T>
        void destruct() noexcept {
            if (mInstance) {
                if (mDestruct) {
                    mDestruct(mInstance);
                }
            }
            mInstance = nullptr;
        }

        T* get() {
            return mInstance;
        }

        Handle() : mConstruct(nullptr), mDestruct(nullptr), mInstance(nullptr) {}
        Handle(std::function<ConstructorType> constructor, std::function<DestructorType> destructor)
            : mConstruct(std::move(constructor))
            , mDestruct(std::move(destructor))
            , mInstance(nullptr) {
            construct<T>();
        }

        Handle(const Handle& other) = delete;
        Handle& operator=(const Handle& other) = delete;

        Handle& operator=(Handle&& other) noexcept {
            destruct<T>();
            mConstruct = std::move(other.mConstruct);
            mDestruct = std::move(other.mDestruct);
            mInstance = std::move(other.mInstance);
            return *this;
        }
        Handle(Handle&& other) noexcept {
            *this = std::move(other);
        }
        bool operator()() {
            return mInstance != nullptr;
        }

        ~Handle() {
            destruct<T>();
        }

    protected:
        T* mInstance;
        std::function<T* ()> mConstruct;
        std::function<void(T*)> mDestruct;
    };

}
}