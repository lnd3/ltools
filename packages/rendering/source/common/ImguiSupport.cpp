#include "rendering/ImguiSupport.h"

#include <memory>

namespace {
    bool CtrlKeyCombo(ImGuiKey key) {
        return ImGui::GetIO().KeyCtrl && ImGui::IsKeyPressed(key);
    }

    bool AltKeyCombo(ImGuiKey key) {
        return ImGui::GetIO().KeyAlt && ImGui::IsKeyPressed(key);
    }

    bool ShiftKeyCombo(ImGuiKey key) {
        return ImGui::GetIO().KeyShift && ImGui::IsKeyPressed(key);
    }
}

namespace l {
    namespace rendering {
        std::unique_ptr<GLFWImguiHandle> CreateImgui(
            GLFWwindow* parent
        ) {
            if (parent) {
                return std::make_unique<GLFWImguiHandle>(parent);
            }
            return nullptr;
        }

        GLFWImguiHandle::GLFWImguiHandle(
            GLFWwindow* parent
        ) : mParent(parent) 
        {
            const char* glsl_version = "#version 130";

            IMGUI_CHECKVERSION();
            ImGui::CreateContext();

            ImGuiIO& io = ImGui::GetIO();
            io.ConfigFlags |= ImGuiConfigFlags_NavEnableKeyboard;     // Enable Keyboard Controls
            io.ConfigFlags |= ImGuiConfigFlags_NavEnableGamepad;      // Enable Gamepad Controls
            io.ConfigFlags |= ImGuiConfigFlags_DockingEnable;         // Enable Docking
            io.ConfigFlags |= ImGuiConfigFlags_ViewportsEnable;       // Enable Multi-Viewport / Platform Windows
            //io.ConfigViewportsNoAutoMerge = true;
            //io.ConfigViewportsNoTaskBarIcon = true;

            // Setup Dear ImGui style
            ImGui::StyleColorsDark();
            //ImGui::StyleColorsLight();

            // When viewports are enabled we tweak WindowRounding/WindowBg so platform windows can look identical to regular ones.
            ImGuiStyle& style = ImGui::GetStyle();
            if (io.ConfigFlags & ImGuiConfigFlags_ViewportsEnable)
            {
                style.WindowRounding = 0.0f;
                style.Colors[ImGuiCol_WindowBg].w = 1.0f;
            }

            // Setup Platform/Renderer backends
            ImGui_ImplGlfw_InitForOpenGL(mParent, true);
            ImGui_ImplOpenGL3_Init(glsl_version);
        }

        GLFWImguiHandle::~GLFWImguiHandle() {
            ImGui_ImplOpenGL3_Shutdown();
            ImGui_ImplGlfw_Shutdown();
            ImGui::DestroyContext();
        }

        void GLFWImguiHandle::Render() {
            // Start the Dear ImGui frame
            ImGui_ImplOpenGL3_NewFrame();
            ImGui_ImplGlfw_NewFrame();
            ImGui::NewFrame();

            ImGui::CaptureKeyboardFromApp();
            ImGui::CaptureMouseFromApp();

            if (mBuilder) {
                mBuilder(*this);
            }

            ImGui::EndFrame();

            ImGui::Render();
            ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());

            // Update and Render additional Platform Windows
            // (Platform functions may change the current OpenGL context, so we save/restore it to make it easier to paste this code elsewhere.
            //  For this specific demo app we could also call glfwMakeContextCurrent(window) directly)
            
            ImGuiIO& io = ImGui::GetIO();
            if (io.ConfigFlags & ImGuiConfigFlags_ViewportsEnable) {
                GLFWwindow* backup_current_context = glfwGetCurrentContext();
                ImGui::UpdatePlatformWindows();
                ImGui::RenderPlatformWindowsDefault();
                glfwMakeContextCurrent(backup_current_context);
            }
        }

        void GLFWImguiHandle::SetGuiBuilder(ImguiHandler builder) {
            mBuilder = std::move(builder);
        }


	}
}
