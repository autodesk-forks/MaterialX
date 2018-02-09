#ifndef MATERIALX_WINDOWWRAPPER_H
#define MATERIALX_WINDOWWRAPPER_H

#include "Platform.h"

#if defined(OSWin_)
#include <windows.h>
#elif defined(OSLinux_)
#include <X11/X.h> // for Window 
#include <X11/Xlib.h> // for Display
#endif

namespace MaterialX
{
/// OS specific type windowing definitions
#if defined(OSWin_)
using ExternalWindowHandle = HWND;
using InternalWindowHandle = HDC;
using DisplayHandle = void*;
#elif defined(OSLinux_)
using ExternalWindowHandle = Widget;
using InternalWindowHandle = Window;
using DisplayHandle = Display*;
using Widget = struct _WidgetRec*;
#elif defined(OSMac_)
using ExternalWindowHandle = void*;
using InternalWindowHandle = void*;
using DisplayHandle = void*;
#else
using Widget = void*;
using ExternalWindowHandle = void*;
using InternalWindowHandle = void*;
using DisplayHandle = void*;
#endif 

///
/// @class WindowWrapper
/// Generic wrapper for encapsulating a "window" construct
/// Each supported platform will have specific storage and logic.
///
class WindowWrapper
{
  public:
    /// Default constructor
    WindowWrapper();

    /// Default destructor
    virtual ~WindowWrapper();

    /// Contruct a wrapper using window information
    WindowWrapper(ExternalWindowHandle externalHandle, InternalWindowHandle internalHandle = nullptr,
        DisplayHandle display = nullptr);

    /// Copy constructor
    WindowWrapper(const WindowWrapper& other);

    /// Assignment operator
    const WindowWrapper& operator=(const WindowWrapper& other);

    /// Return "external" handle
    ExternalWindowHandle externalHandle() const
    {
        return _externalHandle;
    }

    /// Return "internal" handle
    InternalWindowHandle internalHandle() const
    {
        return _internalHandle;
    }

    /// Check that there is a valid OS handle set.
    /// It is sufficient to just check the internal handle
    bool isValid() const 
    { 
        return _internalHandle != 0; 
    }

#if defined(OSLinux_)
    /// Return frame buffer X window
    Window getFBWindow() const
    {
        return _framebufferWindow;
    }

    /// Set frame buffer X window
    void setFBWindow(Window window)
    {
        _framebufferWindow = window;
    }

    /// Rreturn X display
    Display* getDisplay() const
    {
        return _display;
    }
#endif
    void release();

  protected:
    ExternalWindowHandle _externalHandle;
    InternalWindowHandle _internalHandle;

#if defined(OSLinux_)
    /// Window ID of framebuffer instance created in wrapper
    Window _framebufferWindow;
    /// X Display
    Display* _display;
#endif

};

} // namespace MaterialX

#endif 

