#include "testing/Test.h"
#include "logging/Log.h"
#include "rendering/GLFWWindow.h"


TEST(GLFWWindowTest, GLFW) {
    auto handle = l::rendering::CreateGLFW("GLFW Test window", 1024, 768);
    if (handle) {

    }

    TEST_TRUE(true, "");

    return 0;
}
