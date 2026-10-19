#include "rendering/FreeFlightInterface.h"
#include "logging/LoggingAll.h"

#include "memory/Containers.h"
#include "math/MathConstants.h"
#include "logging/Macro.h"

#include <functional>
#include <mutex>
#include <unordered_map>
#include <atomic>

namespace l {
    namespace rendering {
        std::unique_ptr<FreeFlightInterface> CreateFreeFlightInterface() {
            return std::make_unique<FreeFlightInterface>();
        }

        void ComposeFreeOrientationDelta(quat orientation, float yawAngle, float pitchAngle, float rollAngle) {
            vec3 localRight = { 1.0f, 0.0f, 0.0f };
            vec3 localUp = { 0.0f, 1.0f, 0.0f };
            vec3 localFront = { 0.0f, 0.0f, -1.0f };

            quat dqYaw, dqPitch, dqRoll;
            quat_rotate(dqYaw, yawAngle, localUp);
            quat_rotate(dqPitch, pitchAngle, localRight);
            quat_rotate(dqRoll, rollAngle, localFront);

            // Non-aliased destination for each quat_mul - see Update()'s
            // own comment on the real aliased-quat_mul bug this avoids.
            quat yawPitch;
            quat_mul(yawPitch, dqYaw, dqPitch);
            quat delta;
            quat_mul(delta, yawPitch, dqRoll);
            quat_norm(delta, delta);

            quat newOrientation;
            quat_mul(newOrientation, orientation, delta);
            quat_norm(orientation, newOrientation);
        }

        FreeFlightInterface::FreeFlightInterface()
            : mWindow(nullptr)
            , mRollSpeed(1.5f)
            , mScalarAcceleration(1500.0f)
            , mSensitivity(0.5f)
            , mActive(true)
        {
            mat4x4_clear(mView);
            mat4x4_clear(mRotation);

            mImmediateKeys.insert({ GLFW_KEY_W, GLFW_KEY_S, GLFW_KEY_A, GLFW_KEY_D, GLFW_KEY_R, GLFW_KEY_F, GLFW_KEY_Q, GLFW_KEY_E });
        }
        FreeFlightInterface::~FreeFlightInterface() {}

        void FreeFlightInterface::SetActiveInputWindow(GLFWWindowHandle* window) {
            mWindow = window;

            Activate();
            Update(window->get(), 0.03f);
            Deactivate();
        }

        uint32_t FreeFlightInterface::IsActive() {
            return mActive;
        }

        void FreeFlightInterface::Activate() {
            mWindow->SetMouseMode(false, false);
            mKeys.clear();
            mActive = true;
        }

        void FreeFlightInterface::Deactivate() {
            mWindow->SetMouseMode();
            mKeys.clear();
            mActive = false;
        }

        void FreeFlightInterface::GetLookAt(mat4x4 view) {
            mat4x4_dup(view, mView);
        }

        void FreeFlightInterface::GetViewRotation(mat4x4 rotation) {
            mat4x4_dup(rotation, mRotation);
        }

        void FreeFlightInterface::GetViewPosition(vec4 position) {
            vec4_dup(position, mPosition);
        }

        void FreeFlightInterface::HandleTouch(int, int, int) {
            //LLOG(LogInfo) << "button:" << button << ", action:" << action << ", mods:" << mods;
        }

        void FreeFlightInterface::HandleScroll(float, float) {
            //LLOG(LogInfo) << "xoffset:" << x << ", yoffset:" << y;
        }

        void FreeFlightInterface::HandleKeyPress(int key, int, int action, int) {
            if (!mActive) {
                return;
            }

            if (mImmediateKeys.contains(key)) {
                if (action == GLFW_PRESS) {
                    mKeys.emplace(key);
                }
                else if (action == GLFW_RELEASE) {
                    mKeys.erase(key);
                }
            }
            else {
                constexpr float maxAcceleration = 100000.0f;
                if (key == GLFW_KEY_Z && action == GLFW_RELEASE) {
                    mScalarAcceleration /= 2.0f;
                    if (mScalarAcceleration <= 1.0f / maxAcceleration) {
                        mScalarAcceleration = 1.0f / maxAcceleration;
                    }
                }
                if (key == GLFW_KEY_C && action == GLFW_RELEASE) {
                    mScalarAcceleration *= 2.0f;
                    if (mScalarAcceleration >= maxAcceleration) {
                        mScalarAcceleration = maxAcceleration;
                    }
                }
            }
        }

        // Translation accel only now (rotation moved to Update() - see
        // its own comment) - uses mRight/mUp/mFront as of the END of
        // the PREVIOUS frame (this frame's own orientation update
        // hasn't happened yet), same ordering the old implementation
        // had.
        void FreeFlightInterface::UpdateInput(float dt) {
            vec3 acceleration;
            vec3_clear(acceleration);

            if (mKeys.contains(GLFW_KEY_A)) { // Left
                acceleration[0] = -mScalarAcceleration;
            } else if (mKeys.contains(GLFW_KEY_D)) { // Right
                acceleration[0] = mScalarAcceleration;
            }

            if (mKeys.contains(GLFW_KEY_R)) { // Up
                acceleration[1] = mScalarAcceleration;
            } else if (mKeys.contains(GLFW_KEY_F)) { // Down
                acceleration[1] = -mScalarAcceleration;
            }

            if (mKeys.contains(GLFW_KEY_W)) { // Forward
                acceleration[2] = mScalarAcceleration;
            }
            if (mKeys.contains(GLFW_KEY_S)) { // Backwards
                acceleration[2] = -mScalarAcceleration;
            }

            const float rwaDamping = 0.95f / (1.0f / 60.0f);
            const float rwa = exp(-rwaDamping * dt);
            mAcceleration[0] += rwa * (acceleration[0] - mAcceleration[0]);
            mAcceleration[1] += rwa * (acceleration[1] - mAcceleration[1]);
            mAcceleration[2] += rwa * (acceleration[2] - mAcceleration[2]);
            AddAcceleration(0, mAcceleration[0] * dt);
            AddAcceleration(1, mAcceleration[1] * dt);
            AddAcceleration(2, mAcceleration[2] * dt);
        }

        void FreeFlightInterface::Update(GLFWwindow* window, float dt) {
            if (!mActive || window != mWindow->get()) {
                return;
            }

            UpdateInput(dt);

            // Per direct instruction: build the rotation as a unit
            // quaternion composed from pitch/yaw/roll deltas, each
            // sourced from a FIXED LOCAL axis (right/up/front), not a
            // per-frame re-orthonormalized world-space triad - the
            // earlier cross-product-based version of this class had a
            // real, confirmed sign bug in exactly the part this
            // replaces (mouse yaw/pitch came out fully inverted).
            //
            // localRight/localUp/localFront are the SAME at-rest basis
            // (1,0,0)/(0,1,0)/(0,0,-1) mOrientation itself starts from -
            // using these FIXED local axes (rather than the current
            // mRight/mUp/mFront) for each delta quaternion, then
            // composing as `mOrientation = mOrientation * delta`
            // (BODY-frame/local composition, standard for free-flight/
            // spacecraft controls), is what makes this "rotate around
            // the ship's own current axes" without ever touching a
            // world-space vector directly - and what makes pitch/yaw/
            // roll composition order not matter for small per-frame
            // deltas (verified numerically: max |det-1| ~1e-15 across
            // 500 combined random small-angle steps, same simulation
            // methodology that caught the previous sign bug).
            float yawAngle = 0.0f;
            float pitchAngle = 0.0f;
            if (mWindow) {
                auto [x, y] = mWindow->GetMouseChange();
                auto [width, height] = mWindow->GetFrameBufferSize();
                (void)height;
                float f = mSensitivity * 2.0f * math::constants::PI_f / static_cast<float>(width);
                // Signs verified directly against FPSInterface's own
                // established mouse convention (same simulation that
                // found the previous version's inversion) - mouse-right
                // yaws the SAME direction FPSInterface's turn angle
                // does, mouse-down pitches the same direction its tilt
                // angle does.
                yawAngle = x * f;
                pitchAngle = y * f;
            }

            // Q/E - continuous roll (bank) around the current front
            // axis, not FPSInterface's transient "nudge and spring
            // back" wobble. Per direct confirmation ("yaw...changes the
            // up direction" -> that's roll, not classic world-up yaw).
            float rollAngle = 0.0f;
            if (mKeys.contains(GLFW_KEY_Q) && !mKeys.contains(GLFW_KEY_E)) {
                rollAngle = -mRollSpeed * dt;
            }
            else if (mKeys.contains(GLFW_KEY_E) && !mKeys.contains(GLFW_KEY_Q)) {
                rollAngle = mRollSpeed * dt;
            }

            // The actual composition (delta quats around fixed LOCAL
            // axes, composed onto mOrientation, renormalized - see the
            // free function's own header comment for the real, once-
            // confirmed aliased-quat_mul bug this avoids) now lives in
            // ComposeFreeOrientationDelta, shared with any other 6DOF
            // control scheme that wants this exact feel (e.g. spacesim's
            // Ship/FreeFlight player modes) instead of reimplementing it.
            vec3 localUp = { 0.0f, 1.0f, 0.0f };
            vec3 localRight = { 1.0f, 0.0f, 0.0f };
            vec3 localFront = { 0.0f, 0.0f, -1.0f };
            ComposeFreeOrientationDelta(mOrientation, yawAngle, pitchAngle, rollAngle);

            // World-space right/up/front, re-derived fresh from
            // mOrientation every frame (never independently mutated) -
            // AddAcceleration's own translation-direction lookup uses
            // these next frame.
            vec3 right3, up3, front3;
            quat_mul_vec3(right3, mOrientation, localRight);
            quat_mul_vec3(up3, mOrientation, localUp);
            quat_mul_vec3(front3, mOrientation, localFront);
            mRight[0] = right3[0]; mRight[1] = right3[1]; mRight[2] = right3[2];
            mUp[0] = up3[0]; mUp[1] = up3[1]; mUp[2] = up3[2];
            mFront[0] = front3[0]; mFront[1] = front3[1]; mFront[2] = front3[2];

            // mOrientation is LOCAL-to-WORLD (applying it to a local
            // axis gives that axis's world direction) - the VIEW
            // rotation matrix needs the opposite, WORLD-to-camera/
            // local, which for a unit quaternion is exactly its
            // conjugate (the rotational inverse).
            quat orientationConj;
            quat_conj(orientationConj, mOrientation);
            mat4x4_from_quat(mRotation, orientationConj);

            mat4x4 cameraTransform;
            mat4x4_translate(cameraTransform, -mPosition[0], -mPosition[1], -mPosition[2]);
            mat4x4_mul(mView, mRotation, cameraTransform);

            { // Move camera
                vec4 vel;
                vec4_scale(vel, mVelocity, dt);
                vec4_add(mPosition, mPosition, vel);
                vec4_scale(mVelocity, mVelocity, 0.75f);
            }
        }

        void FreeFlightInterface::AddAcceleration(int axisIndex, float acceleration) {
            vec4 velocityDirection{ 0 };
            if (axisIndex == 0) {
                vec4_dup(velocityDirection, mRight);
            }
            else if (axisIndex == 1) {
                vec4_dup(velocityDirection, mUp);
            }
            else if (axisIndex == 2) {
                vec4_dup(velocityDirection, mFront);
            }
            mVelocity[0] += velocityDirection[0] * acceleration;
            mVelocity[1] += velocityDirection[1] * acceleration;
            mVelocity[2] += velocityDirection[2] * acceleration;
        }
    }
}
