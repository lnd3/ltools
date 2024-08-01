#include "rendering/FPSInterface.h"
#include "logging/LoggingAll.h"

#include "memory/Containers.h"
#include "physics/Constants.h"
#include "logging/Macro.h"

#include <functional>
#include <mutex>
#include <unordered_map>
#include <atomic>

namespace l {
    namespace rendering {
        std::unique_ptr<FPSInterface> CreateFPSInterface() {
            return std::make_unique<FPSInterface>();
        }

		FPSInterface::FPSInterface()
			: mWindow(nullptr)
			, mTurnAngle(0.0f)
			, mTiltAngle(0.0f)
			, mYawAngle(0.0f)
			, mScalarAcceleration(1500.0f)
			, mSensitivity(0.5f)
			, mActive(true)
		{
			mat4x4_clear(mView);
			mat4x4_clear(mRotation);

			mImmediateKeys.insert({ GLFW_KEY_W, GLFW_KEY_S, GLFW_KEY_A, GLFW_KEY_D, GLFW_KEY_R, GLFW_KEY_F, GLFW_KEY_Q, GLFW_KEY_E });
		}
		FPSInterface::~FPSInterface() {}
		
		void FPSInterface::SetActiveInputWindow(GLFWWindowHandle* window) {
			mWindow = window;

			Activate();
			Update(window->get(), 0.03f);
			Deactivate();
		}

		uint32_t FPSInterface::IsActive() {
			return mActive;
		}

		void FPSInterface::Activate() {
			mWindow->SetMouseMode(false, false);
			mKeys.clear();
			mActive = true;
		}

		void FPSInterface::Deactivate() {
			mWindow->SetMouseMode();
			mKeys.clear();
			mActive = false;
		}

		void FPSInterface::GetLookAt(mat4x4 view) {
			mat4x4_dup(view, mView);
		}

		void FPSInterface::GetViewRotation(mat4x4 rotation) {
			mat4x4_dup(rotation, mRotation);
		}

		void FPSInterface::GetViewPosition(vec4 position) {
			vec4_dup(position, mPosition);
		}

		void FPSInterface::HandleTouch(int button, int action, int mods) {
			LOG(LogInfo) << "button:" << button << ", action:" << action << ", mods:" << mods;
		}

		void FPSInterface::HandleScroll(float x, float y) {
			LOG(LogInfo) << "xoffset:" << x << ", yoffset:" << y;
		}

		void FPSInterface::HandleKeyPress(int key, int, int action, int) {
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

		void FPSInterface::UpdateInput(float dt) {
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

			bool yawLeft = false;
			bool yawRight = false;

			if (mKeys.contains(GLFW_KEY_Q)) {
				yawLeft = true;
			}
			if (mKeys.contains(GLFW_KEY_E)) {
				yawRight = true;
			}

			float yawResetSpeed = sqrt(mScalarAcceleration / 10.0f);
			if (yawLeft && !yawRight) {
				mYawAngle += dt * yawResetSpeed * (0.25f - mYawAngle);
			}
			else if (yawRight) {
				mYawAngle += dt * yawResetSpeed * (-0.25f - mYawAngle);
			}
			else {
				mYawAngle += dt * yawResetSpeed * (-mYawAngle);
			}
		}

		void FPSInterface::Update(GLFWwindow* window, float dt) {
			if (!mActive || window != mWindow->get()) {
				return;
			}

			UpdateInput(dt);

			if (mWindow) {
				auto [x, y] = mWindow->GetMouseChange();
				auto [width, height] = mWindow->GetFrameBufferSize();
				float f = mSensitivity * 2.0f * constants::pi_f / static_cast<float>(width);
				mTurnAngle += x * f;
				mTiltAngle += y * f;
				if (fabs(x) > 0.0001f && fabs(y) > 0.0001f) {
					//LOG(LogInfo) << "Camera direction: [" << mFront[0] << "," << mFront[1] << "," << mFront[2] << "]" << "[" << mRight[0] << "," << mRight[1] << "," << mRight[2] << "]";
				}
			}

			if (mTiltAngle < -constants::pi_f / 2.0f + 0.001f) {
				mTiltAngle = -constants::pi_f / 2.0f + 0.001f;
			}
			if (mTiltAngle > constants::pi_f / 2.0f - 0.001f) {
				mTiltAngle = constants::pi_f / 2.0f - 0.001f;
			}

			// Open Gl resource
			// https://learnopengl.com/Getting-started/Camera
			// 
			// opengl coordinate system
			// x+ right
			// y+ up
			// z- front
			// 
			// column major
			//   up  right eye center
			// | ux   vx  -nx  -eyex |
			// | uy   vy  -ny  -eyey |
			// | uz   vz  -nz  -eyez |
			// | 0    0    0     1   |
			// 
			// row major
			// |  ux   uy   uz   0  | up
			// |  vx   vy   vz   0  | right
			// | -nx  -ny  -nz   0  | eye
			// | -ex  -ey  -ez   1  | center 
			// 
			// Compute camera rotation matrix
			FromPolarCoords(mRotation, mYawAngle, mTiltAngle, mTurnAngle);
			ToPrincipalAxis(mRight, mUp, mFront, mRotation);

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

		void FPSInterface::FromPolarCoords(mat4x4 dst, float yaw, float tilt, float turn) {
			mat4x4_identity(dst);
			mat4x4_rotate_Z(dst, dst, -yaw);
			mat4x4_rotate_X(dst, dst, -tilt);
			mat4x4_rotate_Y(dst, dst, -turn);
		}

		void FPSInterface::ToPrincipalAxis(vec4 right, vec4 up, vec4 front, const mat4x4 rotation) {
			for (int i = 0; i < 4; i++) {
				right[i] = rotation[i][0];
				up[i] = rotation[i][1];
				front[i] = -rotation[i][2];
			}
		}

		void FPSInterface::AddAcceleration(int axisIndex, float acceleration) {
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

			//LOG(LogInfo) << "Camera velocity: [" << mPosition[0] << "," << mPosition[1] << "," << mPosition[2] << "]";
		}
	}
}
