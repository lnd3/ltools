#pragma once

#include "Win32.h"

#include <vector>

#include <GL/GL.h>
#include <GL/GLU.h>

namespace l {
namespace win32 {
	namespace details {
		constexpr float fzn = 0.005f;
		constexpr float fzf = 1000.0f;

		static const std::vector<float> projectionmatrix = {
			1.5f, 0.00f,  0.0f,                    0.0f,
			0.0f, 1.5f,  0.0f,                    0.0f,
			0.0f, 0.00f, -(fzf + fzn) / (fzf - fzn),    -1.0f,
			0.0f, 0.00f, -2.0f*fzf*fzn / (fzf - fzn),  0.0f };
	}

	class OpenGLData {
	public:
		OpenGLData(HGLRC handleGLRC, HDC handleDC);
		HGLRC mHandleGLRC;
		HDC mHandleDC;
	};

	std::optional<OpenGLData> glCreateWindow(const WindowData& windowData);
	void glDestroyWindow(const OpenGLData& openGLData);

	bool glErrorCheck();

	void glClearScreen();
	void glSwapBuffers(const OpenGLData& openGLData);
	void glDraw2dPoints(const std::vector<float>& points, const float z, const float size);
	void glDraw3dPoints(const std::vector<float>& points, size_t numPoints, const float z, const float size);
	void glSetProjection(const std::vector<float>& projMat);
	void glSetDefaultProjection();
	void setOrthogonalProjection();

}
}
