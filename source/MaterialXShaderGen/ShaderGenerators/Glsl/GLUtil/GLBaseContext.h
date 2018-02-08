#ifndef MATERIALX_GLBASECONTEXT_H
#define MATERIALX_GLBASECONTEXT_H

#include "WindowWrapper.h"

#ifdef _WIN32
	#include "SimpleWindow.h"
#elif defined(OSMac_)
	#include <OpenGL/gl.h>
#endif

namespace MaterialX
{ 

/// Platform dependent definition of a hardware context
#if defined(_WIN32)
using HardwareContextHandle = HGLRC;
#elif defined(__linux__)
using HardwareContextHandle = GLXContext;
#elif defined(__APPLE__)
using HardwareContextHandle = void*;
#else
#error Not implemented on this OS
using HardwareContextHandle = int;  // To halt further compiler errors
#endif

/// @class GLBaseContext
/// Base OpenGL context singleton. 
/// Used as a arbitrary context to perform OpenGL operations from,
/// and context for resource sharing between contexts.
///
class GLBaseContext 
{
  public:
    /// Create base context singleton if not already created
#if defined(__linux__)
    static GLBaseContext* create(const WindowWrapper& windowWrapper, HardwareContextHandle context = 0);
#else
    static GLBaseContext* create(HardwareContextHandle context);
#endif
    /// Destroy singleton
    static void destroy();

    /// Return OpenGL context resource handle
	HardwareContextHandle dummyContext() const 
    { 
        return _dummyContext; 
    }

#if defined(__linux__)
    /// Return X display associated with context
    Display *display() const { return _display; }
#endif

    /// Return if context is valid
	bool isValid() const	
    { 
        return _isValid; 
    }
	
    /// Make the context "current" before execution of OpenGL operations
    int makeCurrent();

#ifdef _WIN32
    /// Share this context with an external one
	void shareLists(HardwareContextHandle context);
#endif

  protected:
#if defined(__linux__)
    /// Create the base context. Requires a window wrapper and optional
    /// OpenGL context to share with
    GLBaseContext(const WindowWrapper& windowWrapper, HardwareContextHandle context = 0);
#else
    /// Create the base context. A OpenGL context to share with can be passed in.
    GLBaseContext(HardwareContextHandle context = 0);
#endif
    ~GLBaseContext();

    /// Base context singleton
    static GLBaseContext* _globalGLBaseContext;

#ifdef _WIN32
    /// Offscreen window required for context operations
	SimpleWindow _dummyWindow;
#elif defined(__linux__)
    /// Offscreen window required for context operations
    Window _dummyWindow;
    /// X Display used by context operations
	Display *_display;
    /// Window wrapper used by context operations
	WindowWrapper _windowWrapper;
#endif

    /// OpenGL context
	HardwareContextHandle _dummyContext;

    /// Flag to indicate validity
	bool _isValid;
};

} // namespace MaterialX

#endif
