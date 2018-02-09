#if defined(OSMac_)

#include "SimpleWindow.h"
//#include <Foundation/include/Hmac.h>
//#include "HWFoundationWrapperSets.h"

namespace MaterialX
{

static unsigned int SimpleWindow::_windowCount = 1;

SimpleWindow::SimpleWindow()
{
	clearInternalState();

	// Give a unique ID to this window.
	_Id = _windowCount;
	_windowCount++;
}

ISimpleWindow::ErrorCode SimpleWindow::create(char* title,
											unsigned int width, unsigned int height,
											MessageHandler* handler,
                                            void* /*applicationShell*/)
{
	void* win = aglToNSOpenGLCreateWindow(width, height, title, true);
	if (!win)
	{
		return CANNOT_CREATE_WINDOW_INSTANCE;
	}
	_windowWrapper = WindowWrapper(win);
	return SUCCESS;
}

SimpleWindow::~SimpleWindow()
{
	void* hWnd = _windowWrapper.externalHandle();
	aglToNSOpenGLDisposeWindow(hWnd);
}

}
#endif