#include "rendering/GLFWSupport.h"
#include "logging/LoggingAll.h"

#define GLAD_GL_IMPLEMENTATION
#include <glad/glad.h>

void gl_assert(std::string_view where) {
    auto error = glGetError();
    ASSERT(!error) << "[" << where.data() << "] GL Error: " << error;
}


namespace l::rendering {

    void CreateTexture(GLuint& textureId, size_t index, const l::rendering::image::image_u8& image) {
        glGenTextures(1, &textureId);
        glActiveTexture(static_cast<GLenum>(GL_TEXTURE0 + index));
        glBindTexture(GL_TEXTURE_2D, textureId);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
        glTexImage2D(GL_TEXTURE_2D, 0, GL_SRGB8_ALPHA8, image.width(), image.height(), 0, GL_RGBA, GL_UNSIGNED_BYTE, (void*)&image.get_pixels()[0]);
        GL_CALL(glGenerateMipmap(GL_TEXTURE_2D));

        //GL_ARB_sparse_texture();
        //GL_ARB_bindless_texture();
        //GL_ARB_sparse_texture2();
        //GL_ARB_sparse_texture_clamp();
    }
};
