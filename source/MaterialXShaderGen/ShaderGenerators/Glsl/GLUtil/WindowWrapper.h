#ifndef MATERIALX_WINDOWWRAPPER_H
#define MATERIALX_WINDOWWRAPPER_H

namespace MaterialX
{

// OS specific type definitions
#if defined(_WIN32)
    #include <windows.h>
    using ExternalWindowHandle = HWND;
    using InternalWindowHandle = HDC;
    using DisplayHandle = void*;
#elif defined(__linux__)
    #include <X11/X.h> // for Window 
    #include <X11/Xlib.h> // for Display
	typedef struct _WidgetRec *Widget;
    using Widget = struct _WidgetRec *;
    using ExternalWindowHandle = Widget;
	using InternalWindowHandle = Window;
    using DisplayHandle = Display*;
#elif defined(__APPLE__)
	using ExternalWindowHandle = void*;
	using InternalWindowHandle = void*;
    using DisplayHandle = void*;
#else
	#error unsupported OS
#endif 

///
/// Generic wrapper for encapsulating a "window".
/// Each supported platform will have specific storage and logic.
///
class WindowWrapper
{ 
  public:
	WindowWrapper();
    virtual ~WindowWrapper();

	WindowWrapper(ExternalWindowHandle externalHandle, InternalWindowHandle internalHandle = nullptr,
                  DisplayHandle display = nullptr);

	WindowWrapper(const WindowWrapper& other);
	const WindowWrapper& operator=(const WindowWrapper& other);

	ExternalWindowHandle externalHandle() const 
    { 
        return _externalHandle; 
    }
	InternalWindowHandle internalHandle() const 
    { 
        return _internalHandle; 
    }

	// On both Windows and Unix, the window wrapper is only valid
	// if the internal handle (hDC for Win32, Window handle for X11) is
	// non-null.
	bool valid() const { return _internalHandle != 0; }

#if defined(__linux__)
    Window getFBWindow() const
    {
        return _framebufferWindow;
    }

    void setFBWindow(Window window)
    {
        _framebufferWindow = window;
    }

	Display* getDisplay() const 
    {
        return _display; 
}
#endif
    void release();

  protected:
    ExternalWindowHandle _externalHandle;
    InternalWindowHandle _internalHandle;

#if defined(__linux__)
    Window _framebufferWindow;  // Window ID of framebuffer instance created in wrapper
    Display *_display;          // Display
#endif
	
};

} // namespace MaterialX

#endif 

