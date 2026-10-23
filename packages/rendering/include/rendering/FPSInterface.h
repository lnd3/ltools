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

        // Per direct request (spacesim's PlayerMode switching needed to
        // carry the player's actual view position across a mode change
        // instead of popping back to wherever this camera was last
        // left) - overwrites mPosition only, zeroes mVelocity (no
        // leftover momentum). Direction/orientation (mTurnAngle/
        // mTiltAngle/mYawAngle) is deliberately left untouched here -
        // unlike FreeFlightInterface's single orientation quaternion,
        // this class's fixed-world-up polar angles don't have a cheap,
        // exact correspondence with the other modes' own orientation
        // representations, and position is the priority per direct
        // instruction ("position is the most important thing... unless
        // the scene requires something else") - direction continuity
        // for this specific (non-free-flight) camera is a known,
        // accepted simplification, not attempted here.
        void SetViewPosition(const vec4 position);

        // Returns the position accumulated since the last call (or since
        // SetViewPosition) and resets it to zero, leaving velocity,
        // and the view angles untouched - so motion carries on
        // smoothly. For callers that keep the camera's real position in
        // higher precision themselves (spacesim: doubles at ~1 AU): mPosition
        // is a float, and integrating small per-frame velocities into a
        // large float position quantizes movement to that float's spacing.
        // Consumed every frame, mPosition only ever holds one frame's small
        // movement, which float represents precisely. GetLookAt()'s
        // translation is then relative to the caller's own position.
        void ConsumePositionDelta(vec4 delta);

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