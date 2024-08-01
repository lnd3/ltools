#pragma once

#pragma comment(lib, "opengl32.lib")
#pragma comment(lib, "glu32.lib")

#include "Opengl.h"

#include "logging/Log.h"

#include <optional>


//#include <gl/glew.h>
#include <gl/GL.h>
#include <gl/GLU.h>
//#include <gl/glut.h>
//#include <gl/glext.h>

namespace l {
namespace win32 {

	bool glErrorCheck() {
		bool hasError = false;
		GLenum error;
		while((error = glGetError()) != GL_NO_ERROR) {
			LOG(LogError) << "GL error: " << error;
			hasError = true;
		}
		return hasError;
	}

	OpenGLData::OpenGLData(HGLRC handleGLRC, HDC handleDC) : mHandleGLRC(handleGLRC), mHandleDC(handleDC) {
	}

	std::optional<OpenGLData> glCreateWindow(const WindowData& windowData) {
		static const PIXELFORMATDESCRIPTOR pfd =
		{
			sizeof(PIXELFORMATDESCRIPTOR),  //  size of this pfd  
			1,                     // version number  
			PFD_DRAW_TO_WINDOW |   // support window  
			PFD_SUPPORT_OPENGL |   // support OpenGL  
			PFD_DOUBLEBUFFER,      // double buffered  
			PFD_TYPE_RGBA,         // RGBA type  
			24,                    // 24-bit color depth  
			0, 0, 0, 0, 0, 0,      // color bits ignored  
			0,                     // no alpha buffer  
			0,                     // shift bit ignored  
			0,                     // no accumulation buffer  
			0, 0, 0, 0,            // accum bits ignored  
			32,                    // 32-bit z-buffer      
			0,                     // no stencil buffer  
			0,                     // no auxiliary buffer  
			PFD_MAIN_PLANE,        // main layer  
			0,                     // reserved  
			0, 0, 0                // layer masks ignored  
		};

		HDC hDC = GetDC(windowData.mHWnd);
		if (!hDC) {
			LOG(LogError) << "Failed to get window draw context (dc)";
			return std::nullopt;
		}

		int format = ChoosePixelFormat(hDC, &pfd);
		if (format == 0) {
			LOG(LogError) << "Failed to choose pixel format";
			return std::nullopt;
		}


		if (!SetPixelFormat(hDC, format, &pfd)) {
			LOG(LogError) << "Failed to set pixel format";
			return std::nullopt;
		}
		
		HGLRC hRC = wglCreateContext(hDC);
		if (!wglMakeCurrent(hDC, hRC)) {
			LOG(LogError) << "Failed to make current context";
			return std::nullopt;
		}

		LOG(LogInfo) << "Available opengl version: " << glGetString(GL_VERSION);

		glEnable(GL_BLEND);

		return std::make_optional<OpenGLData>(hRC, hDC);
	}

	void glDestroyWindow(const OpenGLData& openGLData) {
		wglMakeCurrent(NULL, NULL);
		wglDeleteContext(openGLData.mHandleGLRC);
	}

	void glClearScreen() {
		glClearColor(0.0f, 0.0f, 0.0f, 0.0f);
		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT | GL_ACCUM_BUFFER_BIT | GL_STENCIL_BUFFER_BIT);
	}

	void glSwapBuffers(const OpenGLData& openGLData) {
		if (!wglSwapLayerBuffers(openGLData.mHandleDC, WGL_SWAP_MAIN_PLANE)) {
			LOG(LogError) << "Failed to swap buffers";
		}
	}

	void glDraw2dPoints(const std::vector<float>& points, const float z, const float size) {
		glPointSize(size);
		if (size > 1.99f) {
			glEnable(GL_POINT_SMOOTH);
		}
		else {
			glDisable(GL_POINT_SMOOTH);
		}
		glBegin(GL_POINTS);
		for (size_t i = 0; i < points.size() / 2; i++) {
			glVertex3f(points[2 * i], points[2 * i + 1], z);
		}
		glEnd();
	}

	void glDraw3dPoints(const std::vector<float>& points, size_t numPoints, const float z, const float size) {
		glPointSize(size);
		if (size > 1.99f) {
			glEnable(GL_POINT_SMOOTH);
		}
		else {
			glDisable(GL_POINT_SMOOTH);
		}
		glBegin(GL_POINTS);
		for (size_t i = 0; i < numPoints; i++) {
			glVertex3f(points[3 * i], points[3 * i + 1], z + points[3 * i + 2]);
		}
		glEnd();
	}

	void glSetProjection(const std::vector<float>& projMat) {
		if (projMat.empty()) {
			glMatrixMode(GL_PROJECTION);
			glLoadIdentity();
		}
		else {
			glMatrixMode(GL_PROJECTION);
			glLoadMatrixf(projMat.data());
		}
	}

	void glSetDefaultProjection() {
		glSetProjection(details::projectionmatrix);
	}

	void setOrthogonalProjection() {
		glSetProjection({});
	}

}
}
