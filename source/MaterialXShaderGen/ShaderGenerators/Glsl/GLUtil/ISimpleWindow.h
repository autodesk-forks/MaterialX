#ifndef MATERIALX_ISIMPLEWINDOW_H
#define MATERIALX_ISIMPLEWINDOW_H

#include "Platform.h"
#include <MaterialXShaderGen/ShaderGenerators/Glsl/GLUtil/WindowWrapper.h>

namespace MaterialX
{

/// @class MessageHandler
/// Base class message handler. By default performs no operations
///
class MessageHandler
{
public:
    virtual ~MessageHandler() {}

    /// Is a new refresh required
	virtual bool needsRefresh() const 
    { 
        return false; 
    }

    /// Display results
	virtual void display() 
    { 
    }

    /// Handle window movement
	virtual void windowMoved() 
    {
    }

    /// Handle window resize
	virtual void resize(int /* width */, int /* height */) 
    { 
    }

    /// Handle keyboard input
	virtual void keyboard(unsigned int /* key */) 
    { 
    }

    /// Handle mouse buttons
	virtual void mouseButton(int /* button */, int /* state */, int /* x */, int /* y */) 
    { 
    }

    /// Handle visibility changes
	virtual void visibility(bool /*visible*/) 
    { 
    }
};

/// @class ISimpleWindow
/// Abstract base class for a simple window.
/// Derived classes provide platform specific implementations
class ISimpleWindow
{
 public:
    /// Window creation status codes
	enum ErrorCode
	{
		SUCCESS,
		CANNOT_REGISTER_WINDOW_CLASS,		 
		CANNOT_CREATE_WINDOW_INSTANCE		
	};

    /// Default constructor
	ISimpleWindow() 
    { 
        clearInternalState(); 
    }

    /// Window creator
	virtual ErrorCode create(char* title, unsigned int width, unsigned int height, MessageHandler* handler,
							 void *applicationShell) = 0;

	/// Show the window. Derived classes must override this method
    virtual void show() = 0;

    /// Hide the window. Derived classes must override this method
    virtual void hide() = 0;
	
    /// Set focus to the window. Derived classes must override this method
    virtual void setFocus() = 0;

    /// Message processing events
	enum ProcessingResult
	{
		NO_MESSAGE,		    // No more messages pending
		MORE_MESSAGES,		// More messages pending
		QUIT_MESSAGE        // Quit message has been received
	};

    /// Process input coming messages. Derived classes must implement this method
	virtual ProcessingResult processMessage() = 0;

    /// Return any associated message handler
	MessageHandler* handler() 
    { 
        return _handler; 
    }
	
    /// Return windowing information for the window
    const WindowWrapper& windowWrapper()
    {
        return _windowWrapper;
    }

    /// Return if window is active
    bool active() const
    {
        return _active;
    }

    /// Set window to be active
    void setActive(bool value)
    {
        _active = value;
    }

protected:
    /// Default destructor
    virtual ~ISimpleWindow()
    {
    }

    /// Clear state information
	void clearInternalState() 
    { 
        _active = false; 
        _width = _height = 0; 
    }

protected:
    /// Windowing information
    WindowWrapper _windowWrapper;
    /// Associated message handler. May be empty.
    MessageHandler* _handler;
    /// Width and height of window
    unsigned int _width, _height;
    /// Flag to indicate if window is "active"
    bool _active;
};

} // namespace MaterialX

#endif
