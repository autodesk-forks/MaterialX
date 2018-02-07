#ifndef MATERIALX_SIMPLEWINDOW_H
#define MATERIALX_SIMPLEWINDOW_H

//#include <HWFoundation/foundation/awHWFoundation_stdafx.h>
#include "ISimpleWindow.h"

#if defined(OSUnix_) || defined(OSWin_)
	#include <map>
#endif

namespace MaterialX
{

class SimpleWindow : public ISimpleWindow
{
public:
	SimpleWindow();
	ErrorCode create(char* title, unsigned int width, unsigned int height, MessageHandler* handler,
					 void *applicationShell) override;
	void show() override;
	void hide() override;
	void setFocus() override;
	~SimpleWindow() override;

#if defined(OSUnix_)
	static SimpleWindow* getWindow(Window hWnd);
	static MessageHandler* getHandler(Window hWnd);
	static inline Window getNative(SimpleWindow* sw) { return sw->fWindowWrapper.hFBWnd(); }
#elif defined(OSWin_)
	static SimpleWindow* getWindow(HWND hWnd);
	static MessageHandler* getHandler(HWND hWnd);
#endif

	ProcessingResult processMessage() override;

#if defined(OSWin_)
	virtual bool			 waitMessage();
#elif defined(OSMac_)
	const WindowWrapper& windowWrapper() override;
#endif

#ifdef OSWin_
	enum MouseStates
	{
		kNoInfo, kLMB, kMMB, kRMB,
		kUp, kDown, kMove,
		kInvalid
	};
#endif

	void clearInternalState();

	// Static counter for window ID generation.
	// The counter starts from one, so that an id of 0 can be considered invalid.
	static unsigned int _windowCount;

#if defined(OSUnix_)
	static std::map<Window, SimpleWindow*> _windowsMap;
#elif defined(OSWin_)
	// Static map to quickly convert from HWND -> SimpleWindow,
	// useful in the event handling code.
	static std::map<HWND, SimpleWindow*> _windowsMap;
#endif

	// Unique window ID. Only valid if different than 0.
	unsigned int fID;

#ifdef OSWin_
	char _windowClassName[48];
	// Should we process further messages?
	// This will be set to false when the window is getting closed.
	// [claforte] Currently only implemented on Win32. Is this required on
	// unix?
	bool _processFurtherMessages;
#endif

};

}

#endif // _SimpleWindow_h_
