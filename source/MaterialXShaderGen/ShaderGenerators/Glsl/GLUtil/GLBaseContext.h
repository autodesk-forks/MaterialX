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
    // Platform dependent definition of a hardware context
#if defined(_WIN32)
    using HardwareContextHandle = HGLRC;
#elif defined(__linux__)
    using HardwareContextHandle = GLXContext;
#elif defined(OSMac_)
    using HardwareContextHandle = void*;
#else
    #error Not implemented on this OS
    using HardwareContextHandle = int;  // To halt further compiler errors
#endif

class GLBaseContext 
{
  public:
#if defined(__linux__)
    static GLBaseContext* create(const WindowWrapper& windowWrapper, HardwareContextHandle context = 0);
#else
    static GLBaseContext* create(HardwareContextHandle context = 0);
#endif
    static void destroy();

	HardwareContextHandle dummyContext() const { return _dummyContext; }

#if defined(__linux__)
	Display *display() const { return _display; }
#endif

	bool isValid() const	
    { 
        return _isValid; 
    }
	
    int makeCurrent();

#ifdef _WIN32
	void shareLists(HGLRC context);
#endif

  protected:
#if defined(__linux__)
    GLBaseContext(const WindowWrapper& windowWrapper, HardwareContextHandle context = 0);
#else
    GLBaseContext(HardwareContextHandle context = 0);
#endif
    ~GLBaseContext();

    // Base context singleton
    static GLBaseContext* _globalGLBaseContext;

#ifdef _WIN32
	SimpleWindow _dummyWindow;
#elif defined(__linux__)
	Window _dummyWindow;
	Display *_display;
	WindowWrapper _windowWrapper;
#endif
	HardwareContextHandle _dummyContext;

	bool _isValid;
};

} // namespace MaterialX

#endif
