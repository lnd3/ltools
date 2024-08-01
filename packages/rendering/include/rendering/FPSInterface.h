#pragma once

#include "logging/LoggingAll.h"

#include "memory/Handle.h"
#include "rendering/GLFWWindow.h"
#include "various/linmathext.h"

#include <string_view>
#include <memory>
#include <functional>
#include <optional>
#include <tuple>
#include <span>
#include <array>
#include <unordered_set>

namespace l {
namespace rendering {

    class FPSInterface {
    public:
        FPSInterface();
        ~FPSInterface();

        void SetActiveInputWindow(GLFWWindowHandle* window);
        void GetLookAt(mat4x4 view);

        void GetViewRotation(mat4x4 rotation);
        void GetViewPosition(vec4 position);

        void HandleTouch(int button, int action, int mods);
        void HandleScroll(float x, float y);
        void HandleKeyPress(int key, int scancode, int action, int mods);

        void Update(GLFWwindow* window, float dt);

        uint32_t IsActive();
        void Activate();
        void Deactivate();
    private:
        void UpdateInput(float dt);
        void FromPolarCoords(mat4x4 dst, float yaw, float tilt, float turn);
        void ToPrincipalAxis(vec4 right, vec4 up, vec4 front, const mat4x4 rotation);
        void AddAcceleration(int axisIndex, float acceleration);

        GLFWWindowHandle* mWindow;

        mat4x4 mView{ 0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0 };
        vec4 mRight{0, 0, 0, 1}; // x
        vec4 mUp{ 0, 0, 0, 1 };    // y
        vec4 mFront{ 0, 0, 0, 1 };

        mat4x4 mRotation{ 0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0 };
        vec4 mPosition{ 0, 0, 0, 1 };   // z

        vec4 mVelocity{ 0, 0, 0, 1 };
        vec4 mAcceleration{ 0, 0, 0, 1 };
        float mTurnAngle;
        float mTiltAngle;
        float mYawAngle;
        float mScalarAcceleration;
        float mSensitivity;

        std::unordered_set<int> mKeys;
        std::unordered_set<int> mImmediateKeys;
        bool mActive;
    };

    std::unique_ptr<FPSInterface> CreateFPSInterface();
}
}