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
    // Per direct instruction, rebuilt around a single accumulated
    // orientation QUATERNION (mOrientation), not a per-frame
    // re-orthonormalized right/up/front triad (an earlier version of
    // this class tried that; re-deriving a basis via cross products
    // every frame is NOT the standard technique and turned out to have
    // a real sign bug in its mouse handling that a quaternion rewrite
    // sidesteps entirely). Each frame, pitch/yaw/roll deltas are each
    // built as a small quaternion around a FIXED LOCAL/BODY axis
    // (right=(1,0,0), up=(0,1,0), front=(0,0,-1) - never the current
    // WORLD-space vectors), combined by quaternion multiplication (for
    // small per-frame deltas, composition order barely matters - this
    // was verified numerically, not assumed), and composed onto the
    // existing orientation as `mOrientation = mOrientation * delta`
    // (body-frame/local composition - delta is expressed in the
    // object's OWN current local frame, which is exactly what "rotate
    // around the ship's own current axes, not a fixed world one" means
    // for a quaternion). Renormalized every frame (quat_norm) so
    // float error never accumulates into a non-unit/non-orthogonal
    // rotation - the quaternion equivalent of the old Orthonormalize(),
    // done the standard way. World-space right/up/front (for
    // translation) and the view rotation matrix (world-to-camera, the
    // CONJUGATE of mOrientation - see Update()'s own comment) are both
    // derived fresh from mOrientation every frame, never stored as
    // independent state.
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
        void AddAcceleration(int axisIndex, float acceleration);

        GLFWWindowHandle* mWindow;

        mat4x4 mView{ 0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0 };
        mat4x4 mRotation{ 0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0 };

        // Single source of truth for orientation - local-to-world
        // (quat_mul_vec3(mOrientation, localAxis) gives that axis's
        // current WORLD-space direction). Starts at identity, matching
        // the same at-rest basis (right=+X, up=+Y, front=-Z) the old
        // triad fields and FPSInterface both use.
        quat mOrientation{ 0, 0, 0, 1 };

        // World-space right/up/front, cached each Update() purely for
        // AddAcceleration's own use (translation along the CURRENT
        // orientation) - derived FROM mOrientation, never independently
        // mutated.
        vec4 mRight{ 1, 0, 0, 1 };
        vec4 mUp{ 0, 1, 0, 1 };
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
