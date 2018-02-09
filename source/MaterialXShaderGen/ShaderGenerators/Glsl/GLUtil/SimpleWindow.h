#ifndef MATERIALX_SIMPLEWINDOW_H
#define MATERIALX_SIMPLEWINDOW_H

#include "Platform.h"

#include <string>
//#include <HWFoundation/foundation/awHWFoundation_stdafx.h>
#include "ISimpleWindow.h"

#if defined(OSLinux_) || defined(OSWin_)
	#include <map>
#endif

namespace MaterialX
{

///
/// @class SimpleWindow
/// Basic platform dependent window class
///
class SimpleWindow : public ISimpleWindow
{
public:
    /// Default constructor
	SimpleWindow();

    /// Default destructor
    ~SimpleWindow() override;

    /// Window creator
	ErrorCode create(char* title, unsigned int width, unsigned int height, MessageHandler* handler,
					 void *applicationShell) override;

    /// Handle window show
	void show() override;

    /// Handle window hide
    void hide() override;

    /// Handle window focus setting
	void setFocus() override;

    /// Process window message
    ProcessingResult processMessage() override;

    /// Query if there are more messages to process
    bool processFurtherMessages() const
    {
        return _processFurtherMessages;
    }

    /// Set that there are more messages to process
    void setProcessFurtherMessages(bool val) 
    {
        _processFurtherMessages = val;
    }

    /// Clear internal state information
    void clearInternalState();

#if defined(OSWin_)
    /// Get window associated with a given window handle
    static SimpleWindow* getWindow(ExternalWindowHandle hWnd);

    static MessageHandler* getHandler(ExternalWindowHandle hWnd);

    bool waitMessage();

    enum MouseStates
    {
        kNoInfo, kLMB, kMMB, kRMB,
        kUp, kDown, kMove,
        kInvalid
    };
#elif defined(OSLinux_)
	static SimpleWindow* getWindow(Window hWnd);
	static MessageHandler* getHandler(Window hWnd);
	static inline Window getNative(SimpleWindow* sw) 
    { 
        return sw->_windowWrapper.hFBWnd(); 
    }
#elif defined(OSMac_)
	const WindowWrapper& windowWrapper() override;
#endif

  protected:
	/// Static counter for window ID generation.
	/// The counter starts from one, so that an id of 0 can be considered invalid.
	static unsigned int _windowCount;

#if defined(OSLinux_)
	static std::map<Window, SimpleWindow*> _windowsMap;
#elif defined(OSWin_)
	// Static map to quickly convert from HWND -> SimpleWindow,
	// useful in the event handling code.
	static std::map<HWND, SimpleWindow*> _windowsMap;
#endif

	// Unique window ID. Only valid if different than 0.
	unsigned int _id;

#if defined(OSWin_)
    // Class name for window (generated)
	char _windowClassName[128];

	// Should we process further messages?
    bool _processFurtherMessages;
#endif

};

}

#endif // _SimpleWindow_h_
