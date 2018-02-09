#if defined(OSMac_)

#include "SimpleWindow.h"
// AGL wrappers to add
//#include "HWFoundationWrapperSets.h"

namespace MaterialX
{
SimpleWindow::SimpleWindow()
{
    clearInternalState();

    // Give a unique ID to this window.
    static unsigned int windowCount = 1;
    _id = windowCount;
    windowCount++;
}

bool SimpleWindow::create(char* title,
                        unsigned int width, unsigned int height,
                        void* /*applicationShell*/)
{
    void* win = aglToNSOpenGLCreateWindow(width, height, title, true);
    if (!win)
    {
        return false;
    }
    _windowWrapper = WindowWrapper(win);
    return true;
}

SimpleWindow::~SimpleWindow()
{
    void* hWnd = _windowWrapper.externalHandle();
    aglToNSOpenGLDisposeWindow(hWnd);
}

}
#endif