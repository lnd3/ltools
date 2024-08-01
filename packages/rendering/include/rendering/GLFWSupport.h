#pragma once

#include "rendering/ImageSupport.h"

#define GLAD_GL_IMPLEMENTATION
#include <glad/glad.h>

#define GLFW_INCLUDE_NONE
#include <GLFW/glfw3.h>

void gl_assert(std::string_view where = "");
#define GL_ASSERT() gl_assert()

#ifdef RENDERING_DEBUG
#include <stdio.h>
#define GL_CALL(_CALL)      do { _CALL; gl_assert(#_CALL);} while (0)  // Call with error check
#else
#define GL_CALL(_CALL)      _CALL   // Call without error check
#endif

namespace l {
namespace rendering {

    enum class BufferBindings {
        ARRAY_BUFFER = (GLenum)GL_ARRAY_BUFFER,
        ATOMIC_COUNTER_BUFFER = GL_ATOMIC_COUNTER_BUFFER,
        COPY_READ_BUFFER = GL_COPY_READ_BUFFER,
        COPY_WRITE_BUFFER = GL_COPY_WRITE_BUFFER,
        DISPATCH_INDIRECT_BUFFER = GL_DISPATCH_INDIRECT_BUFFER,
        DRAW_INDIRECT_BUFFER = GL_DRAW_INDIRECT_BUFFER,
        ELEMENT_ARRAY_BUFFER = GL_ELEMENT_ARRAY_BUFFER,
        PIXEL_PACK_BUFFER = GL_PIXEL_PACK_BUFFER,
        PIXEL_UNPACK_BUFFER = GL_PIXEL_UNPACK_BUFFER,
        QUERY_BUFFER = GL_QUERY_BUFFER,
        SHADER_STORAGE_BUFFER = GL_SHADER_STORAGE_BUFFER,
        TEXTURE_BUFFER = GL_TEXTURE_BUFFER,
        TRANSFORM_FEEDBACK_BUFFER = GL_TRANSFORM_FEEDBACK_BUFFER,
        UNIFORM_BUFFER = GL_UNIFORM_BUFFER
    };

    enum class BufferAccess {
        STREAM_DRAW = GL_STREAM_DRAW,
        STREAM_READ = GL_STREAM_READ,
        STREAM_COPY = GL_STREAM_COPY,
        STATIC_DRAW = GL_STATIC_DRAW,
        STATIC_READ = GL_STATIC_READ,
        STATIC_COPY = GL_STATIC_COPY,
        DYNAMIC_DRAW = GL_DYNAMIC_DRAW,
        DYNAMIC_READ = GL_DYNAMIC_READ,
        DYNAMIC_COPY = GL_DYNAMIC_COPY
    };

    enum class GLErrors {
        NoError = GL_NO_ERROR,
        InvalidEnum = GL_INVALID_ENUM, // 1280
        InvalidValue = GL_INVALID_VALUE, // 1281
        InvalidOperation = GL_INVALID_OPERATION, // 1282
        InvalidFramebufferOperation = GL_INVALID_FRAMEBUFFER_OPERATION,
        OutOfMemory = GL_OUT_OF_MEMORY, // 1285
        StackUnderflow = GL_STACK_UNDERFLOW,
        StackOverflow = GL_STACK_OVERFLOW
    };

    void CreateTexture(GLuint& textureId, size_t index, const l::rendering::image::image_u8& image);

}
}