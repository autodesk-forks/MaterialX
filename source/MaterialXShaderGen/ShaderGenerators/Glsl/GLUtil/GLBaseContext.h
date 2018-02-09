#ifndef MATERIALX_GLBASECONTEXT_H
#define MATERIALX_GLBASECONTEXT_H

#include "WindowWrapper.h"

#if defined(OSWin_)
	#include "SimpleWindow.h"
#elif defined(OSMac_)
	#include <OpenGL/gl.h>
#endif

namespace MaterialX
{ 

/// Platform dependent definition of a hardware context
#if defined(OSWin_)
using HardwareContextHandle = HGLRC;
#elif defined(OSLinux_)
using HardwareContextHandle = GLXContext;
#elif defined(OSMac_)
using HardwareContextHandle = void*;
#else
using HardwareContextHandle = void*;  
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
    static GLBaseContext* create(const WindowWrapper& windowWrapper, HardwareContextHandle context = 0);

    /// Get base context singleton
    static GLBaseContext* get()
    {
        return _globalGLBaseContext;
    }

    /// Destroy base context singleton
    static void destroy();

    /// Return OpenGL context resource handle
	HardwareContextHandle dummyContext() const 
    { 
        return _dummyContext; 
    }

#if defined(OSLinux_)
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

#if defined(OSWin_)
    /// Share this context with an external one
	void shareLists(HardwareContextHandle context);
#endif

  protected:
    /// Create the base context. A OpenGL context to share with can be passed in.
    GLBaseContext(const WindowWrapper& windowWrapper, HardwareContextHandle context = 0);
    ~GLBaseContext();

    /// Base context singleton
    static GLBaseContext* _globalGLBaseContext;

#if defined(OSWin_)
    /// Offscreen window required for context operations
	SimpleWindow _dummyWindow;
#elif defined(OSLinux_)
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
