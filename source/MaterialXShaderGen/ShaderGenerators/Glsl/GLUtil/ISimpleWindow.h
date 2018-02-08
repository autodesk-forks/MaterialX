#ifndef MATERIALX_ISIMPLEWINDOW_H
#define MATERIALX_ISIMPLEWINDOW_H

#include <MaterialXShaderGen/ShaderGenerators/Glsl/GLUtil/WindowWrapper.h>

namespace MaterialX
{

/// Base class message handler. By default performs no operation
class MessageHandler
{
public:
    virtual ~MessageHandler() {}
	virtual bool needsRefresh() const 
    { 
        return false; 
    }
	virtual void display() 
    { 
    }
	virtual void windowMoved() 
    {
    }
	virtual void resize(int /* width */, int /* height */) 
    { 
    }
	virtual void keyboard(unsigned int /* key */) 
    { 
    }
	virtual void mouseButton(int /* button */, int /* state */, int /* x */, int /* y */) 
    { 
    }
	virtual void visibility(bool /*visible*/) 
    { 
    }
};

/// Abstract base class for a simple window.
/// Derived classes provide platform specific implementations
class ISimpleWindow
{
 public:
	enum ErrorCode
	{
		SUCCESS,
		CANNOT_REGISTER_WINDOW_CLASS,		 
		CANNOT_CREATE_WINDOW_INSTANCE		
	};

	ISimpleWindow() 
    { 
        clearInternalState(); 
    }

	virtual ErrorCode create(char* title, unsigned int width, unsigned int height, MessageHandler* handler,
							 void *applicationShell) = 0;

	virtual void show() = 0;
	virtual void hide() = 0;
	virtual void setFocus() = 0;

	enum ProcessingResult
	{
		NO_MESSAGE,		    // No more messages pending
		MORE_MESSAGES,		// More messages pending
		QUIT_MESSAGE        // Quit message has been received
	};
	virtual ProcessingResult processMessage() = 0;

	MessageHandler* handler() 
    { 
        return _handler; 
    }
	
    virtual const WindowWrapper& windowWrapper()
    {
        return _windowWrapper;
    }

    bool active() const
    {
        return _active;
    }

    void setActive(bool value)
    {
        _active = value;
    }

protected:
    virtual ~ISimpleWindow()
    {
    }
	void clearInternalState() 
    { 
        _active = false; 
        _width = _height = 0; 
    }

protected:
    WindowWrapper _windowWrapper;
    MessageHandler* _handler;
    unsigned int _width, _height;
    bool _active;
};

} // namespace MaterialX

#endif
