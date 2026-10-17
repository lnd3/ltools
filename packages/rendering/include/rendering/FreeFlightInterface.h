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

    // Free (6DOF), no-fixed-up camera interface - a sibling to
    // FPSInterface, not a replacement for it. FPSInterface composes
    // orientation from polar angles around the FIXED world Y/X/Z axes
    // (FromPolarCoords), which is correct for walking a planet surface
    // but wrong for flying a ship in open space: pitching the nose up
    // should tilt the ship's own up vector along with it, and there's
    // no way to bank/roll at all there (Q/E there is a transient,
    // spring-loaded "turn and release" wobble, not a real rotation).
    //
    // This class instead stores an orthonormal local frame
    // (mRight/mUp/mFront, no stored angles) and rotates it IN PLACE
    // every frame around its own CURRENT axes (mouse X -> yaw around
    // mUp, mouse Y -> pitch around mRight, Q/E -> continuous roll
    // around mFront), re-orthonormalizing afterward. Pitch and roll
    // therefore naturally carry the up vector along with them instead
    // of snapping back to a fixed world reference - the whole point of
    // this interface, per direct request ("no fixed up... will work
    // perfectly for ship controls").
    class FreeFlightInterface {
    public:
        FreeFlightInterface();
        ~FreeFlightInterface();

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
        void Orthonormalize();
        void RotateFrame(float x, float y, float z, float angle);
        void FromPrincipalAxis(mat4x4 dst); // inverse of FPSInterface::ToPrincipalAxis - builds mRotation from mRight/mUp/mFront
        void AddAcceleration(int axisIndex, float acceleration);

        GLFWWindowHandle* mWindow;

        mat4x4 mView{ 0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0 };
        mat4x4 mRotation{ 0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0 };
        vec4 mRight{ 1, 0, 0, 1 }; // x
        vec4 mUp{ 0, 1, 0, 1 };    // y
        vec4 mFront{ 0, 0, -1, 1 };

        vec4 mPosition{ 0, 0, 0, 1 };

        vec4 mVelocity{ 0, 0, 0, 1 };
        vec4 mAcceleration{ 0, 0, 0, 1 };
        float mRollSpeed; // radians/sec while Q/E held
        float mScalarAcceleration;
        float mSensitivity;

        std::unordered_set<int> mKeys;
        std::unordered_set<int> mImmediateKeys;
        bool mActive;
    };

    std::unique_ptr<FreeFlightInterface> CreateFreeFlightInterface();
}
}
