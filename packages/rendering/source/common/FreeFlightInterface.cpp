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

        void FreeFlightInterface::HandleTouch(int button, int action, int mods) {
            LLOG(LogInfo) << "button:" << button << ", action:" << action << ", mods:" << mods;
        }

        void FreeFlightInterface::HandleScroll(float x, float y) {
            LLOG(LogInfo) << "xoffset:" << x << ", yoffset:" << y;
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

            // Q/E - continuous roll (bank) around the CURRENT front axis,
            // not FPSInterface's transient "nudge and spring back" wobble.
            // Per direct confirmation ("yaw...changes the up direction" ->
            // that's roll, not classic world-up yaw): rolling tilts
            // mRight/mUp, matching the whole point of this interface.
            if (mKeys.contains(GLFW_KEY_Q) && !mKeys.contains(GLFW_KEY_E)) {
                RotateFrame(mFront[0], mFront[1], mFront[2], -mRollSpeed * dt);
            }
            else if (mKeys.contains(GLFW_KEY_E) && !mKeys.contains(GLFW_KEY_Q)) {
                RotateFrame(mFront[0], mFront[1], mFront[2], mRollSpeed * dt);
            }
        }

        void FreeFlightInterface::Update(GLFWwindow* window, float dt) {
            if (!mActive || window != mWindow->get()) {
                return;
            }

            UpdateInput(dt);

            if (mWindow) {
                auto [x, y] = mWindow->GetMouseChange();
                auto [width, height] = mWindow->GetFrameBufferSize();
                (void)height;
                float f = mSensitivity * 2.0f * math::constants::PI_f / static_cast<float>(width);

                // Mouse X -> yaw around the CURRENT up axis (not world
                // Y); mouse Y -> pitch around the CURRENT right axis
                // (not world X) - this is the core difference from
                // FPSInterface::FromPolarCoords, which always rotates
                // around the fixed world axes regardless of current
                // orientation. Rotating the frame IN PLACE around its
                // own live axes is what lets pitch naturally carry the
                // up vector with it instead of it staying pinned to
                // world-up.
                //
                // NOT negated (x*f/y*f, not -x*f/-y*f) - a real,
                // confirmed bug, found via a direct report ("viewing
                // was completely inverted - mouse up moves view down,
                // mouse left was view right") and verified by directly
                // comparing against FPSInterface::FromPolarCoords with
                // identical mouse input in a standalone numeric
                // simulation. FPSInterface applies -turn/-tilt to the
                // VIEW matrix itself (world-to-camera); this function
                // instead rotates the camera's own WORLD-SPACE basis
                // vectors directly - the mathematical INVERSE of a view-
                // matrix rotation, where negating the view-matrix angle
                // does NOT correspond to negating this one (it's the
                // opposite sign). The earlier -x*f/-y*f copied
                // FPSInterface's own sign convention verbatim without
                // accounting for that transpose relationship, inverting
                // both axes.
                if (x != 0.0f) {
                    RotateFrame(mUp[0], mUp[1], mUp[2], x * f);
                }
                if (y != 0.0f) {
                    RotateFrame(mRight[0], mRight[1], mRight[2], y * f);
                }
            }

            Orthonormalize();
            FromPrincipalAxis(mRotation);

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

        void FreeFlightInterface::RotateFrame(float x, float y, float z, float angle) {
            mat4x4 rot;
            mat4x4_identity(rot);
            mat4x4_rotate(rot, rot, x, y, z, angle);

            vec4 newRight, newUp, newFront;
            mat4x4_mul_vec4(newRight, rot, mRight);
            mat4x4_mul_vec4(newUp, rot, mUp);
            mat4x4_mul_vec4(newFront, rot, mFront);
            vec4_dup(mRight, newRight);
            vec4_dup(mUp, newUp);
            vec4_dup(mFront, newFront);
        }

        void FreeFlightInterface::Orthonormalize() {
            // Re-derives a clean orthonormal basis from mFront/mUp every
            // frame (standard drift-correction for an incrementally
            // rotated frame, same idea the walk-mode surface-camera code
            // already relies on at a smaller scale) - mFront is taken as
            // the authoritative look direction, mRight/mUp are rebuilt
            // from it via cross products so float error never
            // accumulates into a non-orthogonal or non-unit frame.
            vec3 front3{ mFront[0], mFront[1], mFront[2] };
            vec3 up3{ mUp[0], mUp[1], mUp[2] };
            vec3_norm(front3, front3);

            vec3 right3;
            vec3_mul_cross(right3, front3, up3);
            vec3_norm(right3, right3);

            vec3 upFixed3;
            vec3_mul_cross(upFixed3, right3, front3);
            vec3_norm(upFixed3, upFixed3);

            mFront[0] = front3[0]; mFront[1] = front3[1]; mFront[2] = front3[2];
            mRight[0] = right3[0]; mRight[1] = right3[1]; mRight[2] = right3[2];
            mUp[0] = upFixed3[0]; mUp[1] = upFixed3[1]; mUp[2] = upFixed3[2];
        }

        void FreeFlightInterface::FromPrincipalAxis(mat4x4 dst) {
            // Inverse of FPSInterface::ToPrincipalAxis: builds a
            // rotation matrix directly from the current right/up/front
            // basis vectors instead of deriving right/up/front FROM a
            // rotation matrix (there's no polar-angle intermediate here
            // at all - the basis vectors themselves ARE the state).
            for (int i = 0; i < 4; i++) {
                dst[i][0] = mRight[i];
                dst[i][1] = mUp[i];
                dst[i][2] = -mFront[i];
                dst[i][3] = (i == 3) ? 1.0f : 0.0f;
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
