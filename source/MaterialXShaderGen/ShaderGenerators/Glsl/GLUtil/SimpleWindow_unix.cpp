#if defined(__linux__)

#include <Root/aw.h>
#include "../SimpleWindow.h"
#include <assert.h>
#include <X11/StringDefs.h>
#include <X11/Shell.h>
#include <X11/Xlib.h> // for XEvent definition
#include <X11/Intrinsic.h> // for XtCallbackProc definition

#include <map>

std::map<Window, SimpleWindow*> SimpleWindow::fWindowsMap;

int processEvent(XEvent *event);
XtCallbackProc WndProc(Widget w, caddr_t client_data, caddr_t call_data);

inline Display *display(const WindowWrapper& windowWrapper)
{
	return windowWrapper.display();
}

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

SimpleWindow* SimpleWindow::getWindow(Window hWnd)
{
	return fWindowsMap[hWnd];
}

MessageHandler* SimpleWindow::getHandler(Window hWnd)
{
	SimpleWindow* winPtr = getWindow(hWnd);
	assert(winPtr);

	MessageHandler* handlerPtr = winPtr->handler();
	assert(handlerPtr);

	return handlerPtr;
}

int processEvent(XEvent *event)
{
	if(! event)
		return 0;
	if(!((XAnyEvent *)event)->window)
		return 0;
	if(!((XAnyEvent *)event)->display)
		return 0;

	// Not the simple window
	if(! SimpleWindow::getWindow(((XAnyEvent *)event)->window)) {
		return 0;
	}

	switch(event->type)
	{
		case Expose:
			XEvent ahead;
			// Compress expose events
			while(XEventsQueued(((XAnyEvent *)event)->display, QueuedAfterReading) > 0) {
				XPeekEvent(((XAnyEvent *)event)->display, &ahead);
				if(ahead.type != Expose)
					XNextEvent(((XAnyEvent *)event)->display,  event);
			}
		case MapNotify:
		case VisibilityNotify:
		{
			// Resize the window
			Window root;
			int x, y;
			unsigned int width, height, border_width, depth;
			unsigned int child_width, child_height;
			
			XGetGeometry(((XAnyEvent *)event)->display, ((XAnyEvent *)event)->window, &root, &x, &y, &width, &height, &border_width, &depth);

			SimpleWindow* winPtr = SimpleWindow::getWindow(((XAnyEvent *)event)->window);
			Window child = SimpleWindow::getNative(winPtr);
			if(! child)
				return 0;
			XGetGeometry(((XAnyEvent *)event)->display, child, &root, &x, &y, &child_width, &child_height, &border_width, &depth);
			if(width != child_width ||  height != child_height) {
				 XResizeWindow(((XAnyEvent *)event)->display, child, width, height);
				 SimpleWindow::getHandler(((XAnyEvent *)event)->window)->resize(width, height);
			}

			SimpleWindow::getHandler(((XAnyEvent *)event)->window)->display();
			return 0;
		}
		case KeyPress:
			return 0;
		case KeyRelease:
			return 0;
		case ConfigureNotify:
		{
			// Resize the window
			Window root;
			int x, y;
			unsigned int width, height, border_width, depth;
			unsigned int child_width, child_height;
			
			XGetGeometry(((XAnyEvent *)event)->display, ((XAnyEvent *)event)->window, &root, &x, &y, &width, &height, &border_width, &depth);

			SimpleWindow* winPtr = SimpleWindow::getWindow(((XAnyEvent *)event)->window);
			Window child = SimpleWindow::getNative(winPtr);
			if(! child)
				return 0;
			XGetGeometry(((XAnyEvent *)event)->display, child, &root, &x, &y, &child_width, &child_height, &border_width, &depth);
			if(width != child_width ||  height != child_height)
				 XResizeWindow(((XAnyEvent *)event)->display, child, width, height);

			SimpleWindow::getHandler(((XAnyEvent *)event)->window)->resize(width, height);

			SimpleWindow::getHandler(((XAnyEvent *)event)->window)->display();
			return 0;
		}
		case ButtonPress:
		case ButtonRelease:
			// HERE - TODO not handled
			return 0;
		case CirculateNotify:
			return 0;
	}

	// dispatches to widget callback any unhandled events
	XtDispatchEvent(event);
	return 0;
}

XtCallbackProc WndProc(Widget w, caddr_t client_data, caddr_t call_data)

{
	// this is the SimpleWindow popup shell widget callback
	// nothing implemented yet
	return (XtCallbackProc)0;
}

ISimpleWindow::ErrorCode SimpleWindow::create(char* title, 
											  unsigned int width, unsigned int height, 
											  MessageHandler* handler,
											  void *applicationShell /* = NULL */)
{
	int n=0;

	XtAppContext appContext;	
	Widget shell;
	static Widget batchShell;
	static bool initializedBatchOnce = false;
	if (!applicationShell)
	{
		// Connect to the X Server
		if (!initializedBatchOnce) {
			//printf("Call XtOpenApplication !\n");
			batchShell = XtOpenApplication(&appContext, "HWRender", 0, 0, &n, 0, 0, 
				applicationShellWidgetClass ,0,0);
			initializedBatchOnce = true;
		}
		shell = batchShell;
	}
	else
	{
		// Reuse existing application shell;
		// printf("Reuse existing X application shell !\n");
		shell = (Widget )applicationShell;
	}

	if(! shell) {
		assert(0);
		return kCannotCreateWindowInstance;;
	}
	

	Arg args[6];
	n=0;
	XtSetArg(args[n], XtNx, 0); n++;
	XtSetArg(args[n], XtNy, 0); n++;
	XtSetArg(args[n], XtNwidth, width); n++;
	XtSetArg(args[n], XtNheight, height); n++;
	Widget widget = XtCreatePopupShell(title, topLevelShellWidgetClass, shell, args, n);
	if(! widget) {
		assert(0);
		return kCannotCreateWindowInstance;
	}
	
	XtRealizeWidget(widget);
	assert(XtWindow(widget));

	fActive = true;

	fWindowWrapper = WindowWrapper(widget, XtWindow(widget), XtDisplay(widget));

	// TODO: Make this section thread-safe?

	fWindowsMap[XtWindow(widget)] = this;
	fHandler = handler;

	return kSuccess;
}


void SimpleWindow::show()
{
	// TODO: some state tracking.
	XtPopup(fWindowWrapper.externalHandle(), XtGrabNone);
}


void SimpleWindow::hide()
{
	XtPopdown(fWindowWrapper.externalHandle());
}

void SimpleWindow::setFocus()
{
	show();
	XRaiseWindow(display(windowWrapper()), fWindowWrapper.internalHandle());
}

// HERE 
/* virtual */
ISimpleWindow::ProcessingResult SimpleWindow::processMessage()
{
	XEvent event;	
	Display *_display = display(windowWrapper());
	assert(_display);
	XFlush(_display);
	if(XPending(_display)) {
		XNextEvent(_display, &event);
		if(event.type == DestroyNotify) {
			return ISimpleWindow::kQuitMessage;
		} else {

			processEvent(&event);
			for(; XPending(_display); XNextEvent(_display, &event))
				processEvent(&event);

			// if there is not a widget callback, perhaps the event needs to
			// be processed here
			return ISimpleWindow::kMoreMessages;
		}
	}

	return ISimpleWindow::kNoMessage;
}

/* virtual */
SimpleWindow::~SimpleWindow()
{

	Widget hWnd = fWindowWrapper.externalHandle();
	if (hWnd)
	{
		// Unrealize the widget first to avoid X calls to it
		XtUnrealizeWidget(hWnd);
		// Used to be XFree. Make a proper call to destroy
		// the widget.
		XtDestroyWidget(hWnd);
		hWnd = NULL;
	}
}

#endif
