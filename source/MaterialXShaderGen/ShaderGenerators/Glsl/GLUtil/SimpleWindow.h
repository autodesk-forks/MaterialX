#ifndef MATERIALX_SIMPLEWINDOW_H
#define MATERIALX_SIMPLEWINDOW_H

#include "Platform.h"
#include "WindowWrapper.h"

namespace MaterialX
{
///
/// @class SimpleWindow
/// Basic platform dependent window class
///
class SimpleWindow
{
  public:
    /// Default constructor
    SimpleWindow();

    /// Default destructor
    ~SimpleWindow();

    /// Window creator
    bool create(char* title, unsigned int width, unsigned int height,
        void *applicationShell);

    /// Return windowing information for the window
    const WindowWrapper& windowWrapper()
    {
        return _windowWrapper;
    }

    /// Return width of window
    unsigned int width() const
    {
        return _width;
    }

    /// Return height of window
    unsigned int height() const
    {
        return _height;
    }

    /// Check validity
    bool isValid() const
    {
        return (_id != 0);
    }

  protected:
    /// Clear internal state information
    void clearInternalState()
    {
        _width = _height = 0;
        _id = 0;
    }

    /// Windowing information
    WindowWrapper _windowWrapper;
    
    /// Dimensions of window
    unsigned int _width;
    unsigned int _height;

    /// Unique window identifier Only valid if different than 0.
    unsigned int _id;

#if defined(OSWin_)
    /// Class name for window (generated)
    char _windowClassName[128];
#endif

};

#if defined(OSUnsupported_)
//
// Stubs for unsupported OS
//
SimpleWindow::SimpleWindow() :
    _id(0)
{
}

SimpleWindow::~SimpleWindow()
{
}

ISimpleWindow::bool SimpleWindow::create(char* /*title*/,
                                            unsigned int /*width*/,
                                            unsigned int /*height*/,
                                            void* /*applicationShell*/)
{
    return false;
}

#endif

}

#endif // _SimpleWindow_h_
