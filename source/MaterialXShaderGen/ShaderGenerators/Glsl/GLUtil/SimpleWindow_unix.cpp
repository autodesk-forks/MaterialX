#if defined(OSLinux_)

#include "SimpleWindow.h"
#include <X11/StringDefs.h>
#include <X11/Shell.h>
#include <X11/Xlib.h> // for XEvent definition
#include <X11/Intrinsic.h> // for XtCallbackProc definition

namespace MaterialX
{ 

SimpleWindow::SimpleWindow()
{
	clearInternalState();

	// Give a unique ID to this window.
	//
    static unsigned int windowCount = 1;
    _id = windowCount;
    windowCount++;
}

SimpleWindow* SimpleWindow::getWindow(Window hWnd)
{
	return _windowsMap[hWnd];
}

MessageHandler* SimpleWindow::getHandler(Window hWnd)
{
	SimpleWindow* winPtr = getWindow(hWnd);
	MessageHandler* handlerPtr = winPtr->handler();
	return handlerPtr;
}

ISimpleWindow::ErrorCode SimpleWindow::create(char* title, 
											  unsigned int width, unsigned int height, 
											  MessageHandler* handler,
											  void *applicationShell)
{
	int n = 0;

	XtAppContext appContext;	
	Widget shell;
	static Widget batchShell;
	static bool initializedBatchOnce = false;
	if (!applicationShell)
	{
		// Connect to the X Server
		if (!initializedBatchOnce) 
        {
			batchShell = XtOpenApplication(&appContext, "__dummy__app__", 
                                           0, 0, &n, 0, 0, 
				                           applicationShellWidgetClass ,0,0);
			initializedBatchOnce = true;
		}
		shell = batchShell;
	}
	else
	{
		// Reuse existing application shell;
		shell = (Widget)applicationShell;
	}

	if(!shell) 
    {
		return CANNOT_CREATE_WINDOW_INSTANCE;;
	}
	
	Arg args[6];
	n = 0;
	XtSetArg(args[n], XtNx, 0); n++;
	XtSetArg(args[n], XtNy, 0); n++;
	XtSetArg(args[n], XtNwidth, width); n++;
	XtSetArg(args[n], XtNheight, height); n++;
	Widget widget = XtCreatePopupShell(title, topLevelShellWidgetClass, shell, args, n);
	if (!widget) 
    {
		return CANNOT_CREATE_WINDOW_INSTANCE;
	}
	
	XtRealizeWidget(widget);

	_windowWrapper = WindowWrapper(widget, XtWindow(widget), XtDisplay(widget));
	_handler = handler;

    _active = true;

	return SUCCESS;
}

SimpleWindow::~SimpleWindow()
{
	Widget widget = _windowWrapper.externalHandle();
	if (widget)
	{
		// Unrealize the widget first to avoid X calls to it
		XtUnrealizeWidget(widget);
		// Used to be XFree. Make a proper call to destroy
		// the widget.
		XtDestroyWidget(widget);
        widget = nullptr;
	}
}

}
#endif
