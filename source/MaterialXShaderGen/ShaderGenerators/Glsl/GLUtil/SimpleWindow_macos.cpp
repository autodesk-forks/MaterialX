#if defined(__APPLE__)

#include <Root/aw.h>
#include "../SimpleWindow.h"
#include <assert.h>
#include <Foundation/include/Hmac.h>
#if defined(OSMac_MachO_)
#include <Carbon/Carbon.h>
#endif
#include "HWFoundationWrapperSets.h"
#include <HWFoundation/foundation/HWEnvironment.h>

//#define _DEBUG_SIMPLEWINDOW
#ifdef _DEBUG_SIMPLEWINDOW
#include <Foundation/include/TdebugOutput.h> // For debugging
#define DBOUT	rprintf
#endif

unsigned int SimpleWindow::fWindowCount = 1;

SimpleWindow::SimpleWindow()
{
	clearInternalState();

	// Give a unique ID to this window.
	//
	fID = fWindowCount;
	fWindowCount++;
}

void SimpleWindow::clearInternalState()
{
	ISimpleWindow::clearInternalState();
	fID = 0;
}

#if 0
SimpleWindow* SimpleWindow::getWindow(Window wnd)
{
	return fWindowsMap[wnd];
}

MessageHandler* SimpleWindow::getHandler(Window wnd)
{
	return NULL;
}
#endif

ISimpleWindow::ErrorCode SimpleWindow::create(char* title,
													unsigned int width, unsigned int height,
													MessageHandler* handler)
{
#ifdef _DEBUG_SIMPLEWINDOW
	DBOUT("*** Create simple window size %d by %d\n", width, height);
#endif

	// Attempt to create the window.

#ifdef _DEBUG_SIMPLEWINDOW
	DBOUT("Create SimpleWindow\n");
#endif
	void* hWnd = aglToNSOpenGLCreateWindow(width, height, title, HWEnvironment::maya_batch_mode);

	if (!hWnd)
	{
#ifdef _DEBUG_SIMPLEWINDOW
		DBOUT("Can't create window abort !!!!\n");
#endif
		// Cancel everything we've done so far.
		assert(0); //KillGLWindow();
		return kCannotCreateWindowInstance;
	}

	fWindowWrapper = WindowWrapper(hWnd);

	// TODO: Make this section thread-safe?
	//assert(fWindowsMap[hWnd] == NULL);
	//fWindowsMap[hWnd] = this;
	//fHandler = handler;

	return kSuccess;
}


void SimpleWindow::show()
{
	aglToNSOpenGLShowWindow(fWindowWrapper.externalHandle());
}

void SimpleWindow::hide()
{
	aglToNSOpenGLHideWindow(fWindowWrapper.externalHandle());
}

void SimpleWindow::setFocus()
{
	aglToNSOpenGLSetFocus(fWindowWrapper.externalHandle());
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
	void* hWnd = fWindowWrapper.externalHandle();
	aglToNSOpenGLDisposeWindow(hWnd);
}
/* virtual */ const WindowWrapper& SimpleWindow::windowWrapper()
{
#ifdef _DEBUG_SIMPLEWINDOW
	DBOUT("Get wrapper %x from simplewindow %x\n", &fWindowWrapper, this);
#endif
	return fWindowWrapper;
}

#endif