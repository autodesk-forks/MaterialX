#include <MaterialXShaderGen/ShaderGenerators/Glsl/GLUtil/WindowWrapper.h>

#if defined(__linux__)
	#include <X11/Intrinsic.h> // for XtWindow
#elif defined(__APPLE__)
	//#include <Foundation/include/Hmac.h>
	//#include "macos/HWFoundationWrapperSets.h"
#endif

namespace MaterialX
{
    
//
// Windows code
//
#if defined(_WIN32)

// Default constructor.
WindowWrapper::WindowWrapper()
{
    _externalHandle = 0;
	_internalHandle = 0;
}

WindowWrapper::WindowWrapper(ExternalWindowHandle externalHandle, InternalWindowHandle internalHandle, 
                             DisplayHandle /*display*/)
{
	_externalHandle = externalHandle;

	if (_externalHandle && !internalHandle)
	{
		// Cache our own HDC that corresponds to the hWnd.
		_internalHandle = GetDC(_externalHandle);
	}
	else
	{
		// This window wrapper is wrapping an offline DC.
		_internalHandle = internalHandle;
	}
}

WindowWrapper::WindowWrapper(const WindowWrapper& other)
{
	_externalHandle = other._externalHandle;

	if (_externalHandle && !_internalHandle)
	{
		// Cache our own HDC that corresponds to the hWnd.
		_internalHandle = GetDC(_externalHandle);
	}
	else
	{
		// This window wrapper is wrapping an offline DC.
		_internalHandle = other._internalHandle;
	}
}

const WindowWrapper& WindowWrapper::operator=(const WindowWrapper& other)
{
	release();

	_externalHandle = other._externalHandle;
	if (_externalHandle && !_internalHandle)
	{
		// Cache our own HDC that corresponds to the hWnd.
		_internalHandle = GetDC(_externalHandle);
	}
	else
	{
		// This window wrapper is wrapping an offline DC.
		_internalHandle = other._internalHandle;
	}

	return *this;
}

WindowWrapper::~WindowWrapper()
{
	release();
}

void WindowWrapper::release()
{
	if (_externalHandle)
	{
		// On-screen window wrapper.
		ReleaseDC(_externalHandle, _internalHandle);
	}
	else
	{
		// Offscreen window wrapper.
		// User is in charge of releasing the HDC.
	}

	_externalHandle = 0;
	_internalHandle = 0;
}

//
// Linux (X11-specific) code
//
#elif defined(__linux__)

// Default constructor.
WindowWrapper::WindowWrapper() :
    _externalHandle(0),
    _internalHandle(0),
    _framebufferWindow(0),
    _display(0)
{
}

WindowWrapper::WindowWrapper(ExternalWindowHandle externalHandle, InternalWindowHandle internalHandle, DisplayHandle display)
{
	_display = display;
	_framebufferWindow = 0;
	_externalHandle = externalHandle;
	// Cache a pointer to the window.
	if (internalHandle)
	  _internalHandle = internalHandle;
	else
	  _internalHandle = XtWindow(externalHandle);
}

WindowWrapper::WindowWrapper(const WindowWrapper& other)
{
	_framebufferWindow = other._framebufferWindow;
	_externalHandle = other._externalHandle;
	_internalHandle = other._internalHandle;
	_display = other._display;
}

const WindowWrapper& WindowWrapper::operator=(const WindowWrapper& other)
{
	_framebufferWindow = other._framebufferWindow;
	_externalHandle = other._externalHandle;
	_internalHandle = other._internalHandle;
	_display = other._display;
	return *this;
}

WindowWrapper::~WindowWrapper()
{
	release();
}

void WindowWrapper::release()
{
	// No release is required.
	_externalHandle = 0;
	_internalHandle = 0;
	_framebufferWindow = 0;
	_display = 0;
}

//
// OSX (Apple) specific code
//
#elif defined(__APPLE__)

WindowWrapper::WindowWrapper()
{
	_externalHandle = 0;
	_internalHandle = 0;
}

WindowWrapper::WindowWrapper(ExternalWindowHandle externalHandle, InternalWindowHandle internalHandle, DisplayHandle display)
{
	_externalHandle = externalHandle;
	// Cache a pointer to the window.
	_internalHandle = aglToNSOpenGLGetView(externalHandle);
}

WindowWrapper::WindowWrapper(const WindowWrapper& other)
{
	_externalHandle = other._externalHandle;
	_internalHandle = aglToNSOpenGLGetView(_externalHandle);
}

const WindowWrapper& WindowWrapper::operator=(const WindowWrapper& other)
{
	_externalHandle = other._externalHandle;
	_internalHandle = aglToNSOpenGLGetView(_externalHandle);
	return *this;
}

WindowWrapper::~WindowWrapper()
{
	release();
}

void WindowWrapper::release()
{
	_externalHandle = 0;
	_internalHandle = 0;
}
#endif 

}
