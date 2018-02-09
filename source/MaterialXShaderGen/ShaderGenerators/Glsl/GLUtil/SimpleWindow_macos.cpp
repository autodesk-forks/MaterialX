#if defined(OSMac_)

#include "SimpleWindow.h"
//#include <Foundation/include/Hmac.h>
//#if defined(OSMac_MachO_)
//#include <Carbon/Carbon.h>
//#endif
//#include "HWFoundationWrapperSets.h"

namespace MaterialX
{

unsigned int SimpleWindow::_windowCount = 1;

SimpleWindow::SimpleWindow()
{
	clearInternalState();

	// Give a unique ID to this window.
	_Id = _windowCount;
	_windowCount++;
}

void SimpleWindow::clearInternalState()
{
	ISimpleWindow::clearInternalState();
	_Id = 0;
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
	return kSuccess;
}

void SimpleWindow::show()
{
	aglToNSOpenGLShowWindow(_windowWrapper.externalHandle());
}

void SimpleWindow::hide()
{
	aglToNSOpenGLHideWindow(_windowWrapper.externalHandle());
}

void SimpleWindow::setFocus()
{
	aglToNSOpenGLSetFocus(_windowWrapper.externalHandle());
}

// HERE
/* virtual */
ISimpleWindow::ProcessingResult SimpleWindow::processMessage()
{
	return ISimpleWindow::kNoMessage;
}

/* virtual */
SimpleWindow::~SimpleWindow()
{
	void* hWnd = _windowWrapper.externalHandle();
	aglToNSOpenGLDisposeWindow(hWnd);
}
/* virtual */ const WindowWrapper& SimpleWindow::windowWrapper()
{
#ifdef _DEBUG_SIMPLEWINDOW
	DBOUT("Get wrapper %x from simplewindow %x\n", &_windowWrapper, this);
#endif
	return _windowWrapper;
}

}
#endif