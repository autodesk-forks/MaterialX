#include "Platform.h"

#if defined(OSWin_)

#include <Windows.h>
#include "SimpleWindow.h"
#include "WindowWrapper.h"

namespace MaterialX
{ 

// Keep track of windows created
unsigned int SimpleWindow::_windowCount = 1;
std::map<HWND, SimpleWindow*> SimpleWindow::_windowsMap;

static LRESULT CALLBACK WndProc(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam);

SimpleWindow::SimpleWindow() :
    ISimpleWindow()
{
	// Give a unique ID to this window.
	//
	_id = _windowCount;
	_windowCount++;
	// Generate a unique string for our window class. 
    sprintf_s(_windowClassName, "_SW_%lu", _id);

    _processFurtherMessages = true;
}

void SimpleWindow::clearInternalState()
{
	ISimpleWindow::clearInternalState();
	_id = 0;
}

SimpleWindow* SimpleWindow::getWindow(HWND hWnd)
{
	return _windowsMap[hWnd];
}

MessageHandler* SimpleWindow::getHandler(HWND hWnd)
{
	SimpleWindow* winPtr = getWindow(hWnd);
    if (winPtr)
    {
        MessageHandler* handlerPtr = winPtr->handler();
        return handlerPtr;
    }
    return nullptr;
}

// Dummy window procedure
LRESULT CALLBACK NoOpProc(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam)
{
    switch (msg)
    {
        case WM_CLOSE:
        case WM_DESTROY:
            break;
        default:
            return DefWindowProc(hWnd, msg, wParam, lParam);
        break;
    }
	return 0;
}

// Basic window procedure
LRESULT CALLBACK WndProc(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
	SimpleWindow* simpleWindow = SimpleWindow::getWindow(hWnd);

	// Only process messages if:
	//	- we're not already processing another message.
	//  - we haven't yet processed the WM_CLOSE message
	//.
    static bool alreadyProcessingMessage = false;
    if (alreadyProcessingMessage ||
        (simpleWindow && !simpleWindow->processFurtherMessages()))
    {
        return DefWindowProc(hWnd, uMsg, wParam, lParam);
    }
    alreadyProcessingMessage = true;
	
	bool messageProcessed = false;
	switch (uMsg)
	{
		case WM_ACTIVATE:
		{
			MessageHandler* handlerPtr = SimpleWindow::getHandler(hWnd);
			if (handlerPtr)
            {
			    if (!HIWORD(wParam))
					SimpleWindow::getWindow(hWnd)->setActive(true);
			    else
					SimpleWindow::getWindow(hWnd)->setActive(false);

			    // Message has been handled.
			    messageProcessed = true;
            }
			break;
		}

        // Intercept System Commands
        case WM_SYSCOMMAND:
		{
			switch (wParam)
			{
				case SC_SCREENSAVE:
                    ;
				case SC_MONITORPOWER:
                    ;
				messageProcessed = true;							
			}
			break;
		}

        // Handle paint message
		case WM_PAINT:
		{
			HDC			hdc;
			PAINTSTRUCT	ps;

			MessageHandler* handlerPtr = SimpleWindow::getHandler(hWnd);
			if (handlerPtr)
			{
				hdc = ::BeginPaint(hWnd, &ps);
				handlerPtr->display();
				::EndPaint(hWnd, &ps);
				messageProcessed = true;
			}
			else
				messageProcessed = false;
			break;
		}
		
        // Handle move message
		case WM_MOVE:
		{
			MessageHandler* handlerPtr = SimpleWindow::getHandler(hWnd);
			if (handlerPtr)				
			{
				handlerPtr->windowMoved();
				messageProcessed = true;
			}
			else
				messageProcessed = false;
			break;
		}

        // Handle close message
		case WM_CLOSE:	
		{
			// Send a quit message.
			PostQuitMessage(0);
			messageProcessed = true;

			// Other messages should no longer be processed in that window.
			//
			MessageHandler* handlerPtr = SimpleWindow::getHandler(hWnd);
            if (handlerPtr)
            {
                SimpleWindow::getWindow(hWnd)->setProcessFurtherMessages(false);
            }
            break;
		}

        // Handle key-down
		case WM_KEYDOWN:
		{
			unsigned int nVirtKey = (unsigned int) wParam;    // virtual-key code 
			MessageHandler* handlerPtr = SimpleWindow::getHandler(hWnd);
			if (handlerPtr)
			{
				handlerPtr->keyboard( nVirtKey );
				messageProcessed = true;
			}
			else
				messageProcessed = false;
			break;
		}

        // Handle key-up
		case WM_KEYUP:
		{
			messageProcessed = true;
			break;
		}

		// Handle mouse move 
		case WM_MOUSEMOVE:
		{
			int xPos = LOWORD(lParam);
			int yPos = HIWORD(lParam);
			MessageHandler* handlerPtr = SimpleWindow::getHandler(hWnd);
			if (handlerPtr)
			{
				handlerPtr->mouseButton( (int)SimpleWindow::kNoInfo, (int)SimpleWindow::kMove, xPos, yPos);
				messageProcessed = true;
			}
			else
				messageProcessed = false;
			break;
		}

		// Handle left mouse button
		//
		case WM_LBUTTONUP:
		{
			int xPos = LOWORD(lParam);
			int yPos = HIWORD(lParam);
			MessageHandler* handlerPtr = SimpleWindow::getHandler(hWnd);
			if (handlerPtr)
			{
				handlerPtr->mouseButton( (int)SimpleWindow::kLMB, (int)SimpleWindow::kUp, xPos, yPos);
				messageProcessed = true;
			}
			else
				messageProcessed = false;
			break;
		}

        // Handle button down
		case WM_LBUTTONDOWN:
		{
			int xPos = LOWORD(lParam);
			int yPos = HIWORD(lParam);
			MessageHandler* handlerPtr = SimpleWindow::getHandler(hWnd);
			if (handlerPtr)
			{
				handlerPtr->mouseButton( (int)SimpleWindow::kLMB, (int)SimpleWindow::kDown, xPos, yPos);
				messageProcessed = true;
			}
			else
				messageProcessed = false;
			break;
		}

		// Handle middle mouse up
		//
		case WM_MBUTTONUP:
		{
			int xPos = LOWORD(lParam);
			int yPos = HIWORD(lParam);
			MessageHandler* handlerPtr = SimpleWindow::getHandler(hWnd);
			if (handlerPtr)
			{
				handlerPtr->mouseButton( (int)SimpleWindow::kMMB, (int)SimpleWindow::kUp, xPos, yPos);
				messageProcessed = true;
			}
			else
				messageProcessed = false;
			break;
		}

        // Handle middle mouse down
		case WM_MBUTTONDOWN:
		{
			int xPos = LOWORD(lParam);
			int yPos = HIWORD(lParam);
			
			MessageHandler* handlerPtr = SimpleWindow::getHandler(hWnd);
			if (handlerPtr)
			{
				handlerPtr->mouseButton( (int)SimpleWindow::kMMB, (int)SimpleWindow::kDown, xPos, yPos);
				messageProcessed = true;
			}
			else
				messageProcessed = false;
			break;
		}

		// Handle right mouse up
		//
		case WM_RBUTTONUP:
		{
			int xPos = LOWORD(lParam);
			int yPos = HIWORD(lParam);
			MessageHandler* handlerPtr = SimpleWindow::getHandler(hWnd);
			if (handlerPtr)
			{
				handlerPtr->mouseButton( (int)SimpleWindow::kRMB, (int)SimpleWindow::kUp, xPos, yPos);
				messageProcessed = true;
			}
			else
				messageProcessed = false;
			break;
		}

        // Handle mouse button down
		case WM_RBUTTONDOWN:
		{
			int xPos = LOWORD(lParam);
			int yPos = HIWORD(lParam);
			MessageHandler* handlerPtr = SimpleWindow::getHandler(hWnd);
			if (handlerPtr)
			{
				handlerPtr->mouseButton( (int)SimpleWindow::kRMB, (int)SimpleWindow::kDown, xPos, yPos);
				messageProcessed = true;
			}
			else
				messageProcessed = false;
			break;
		}
		
        // Handle resize
		case WM_SIZE:
		{
			// Resize the window
			unsigned int width = LOWORD(lParam);
			unsigned int height = HIWORD(lParam);
			MessageHandler* handlerPtr = SimpleWindow::getHandler(hWnd);
			if (handlerPtr)
			{
				handlerPtr->resize(width, height);
				messageProcessed = true;
			}
			else
				messageProcessed = false;
			break;
		}
	}

	// Finished processing messages
	alreadyProcessingMessage = false;

    // If handle message check if we need to invalidate
	if (messageProcessed)
	{
		MessageHandler* handlerPtr = SimpleWindow::getHandler(hWnd);
		if (handlerPtr)
        {
		    if( handlerPtr->needsRefresh() )
			    ::InvalidateRect(hWnd, 0, FALSE);
        }
		return 0;
	}

    // Pass unhandled messages default
    return DefWindowProc(hWnd,uMsg,wParam,lParam);
}

ISimpleWindow::ErrorCode SimpleWindow::create(char* title, 
											  unsigned int width, unsigned int height, 
											  MessageHandler* handler,
                                              void * /*applicationShell*/)
{
	HINSTANCE hInstance = GetModuleHandle(NULL);

	// Basic windows class structure
	//
	WNDCLASS	wc;
	wc.style = CS_HREDRAW | CS_VREDRAW | CS_OWNDC;	
    if (handler)
	    wc.lpfnWndProc = (WNDPROC) WndProc;				
    else
	    wc.lpfnWndProc = (WNDPROC) NoOpProc; 
	wc.cbClsExtra = 0;
	wc.cbWndExtra = 0;
	wc.hInstance = hInstance; // Set the instance to this application
	wc.hIcon = LoadIcon(NULL, IDI_WINLOGO);	
	wc.hCursor = LoadCursor(NULL, IDC_ARROW); 
	wc.hbrBackground = NULL; // No background required 
	wc.lpszMenuName = NULL;	 // No menu required
    wc.lpszClassName = _windowClassName; 

	if (!RegisterClass(&wc))
		return CANNOT_REGISTER_WINDOW_CLASS;
	
	// Window style and extended style
	//
	DWORD dwStyle = WS_OVERLAPPEDWINDOW;
	DWORD dwExStyle = WS_EX_APPWINDOW | WS_EX_WINDOWEDGE;

	// Set the rectangle of the client area.
	RECT		WindowRect;
	WindowRect.left		= (long) 0;
	WindowRect.top		= (long) 0;
	WindowRect.right	= (long) width;
	WindowRect.bottom	= (long) height;

	// Calculate the exact window size (including border) so that the 
	// client area has the desired dimensions.
	//
	AdjustWindowRectEx(&WindowRect, dwStyle, FALSE, dwExStyle);

	// Attempt to create the window.
	HWND hWnd = CreateWindowEx(dwExStyle, _windowClassName, title,
						  dwStyle |	WS_CLIPSIBLINGS | WS_CLIPCHILDREN,
						  0, 0,								// Window position
						  WindowRect.right-WindowRect.left,	// Window width (including borders)
						  WindowRect.bottom-WindowRect.top,	// Window height (including borders/title bar)
						  NULL,								// No parent window
						  NULL,								// No menu
						  hInstance,					    // Instance
						  NULL);							// Don't pass anything To WM_CREATE

	if (!hWnd)	
	{
		// Cancel everything we've done so far.
		return CANNOT_CREATE_WINDOW_INSTANCE;
	}

	setActive(true);

	_windowWrapper = WindowWrapper(hWnd, nullptr, nullptr);
	_windowsMap[hWnd] = this;
	_handler = handler;

	return SUCCESS;
}


void SimpleWindow::show()
{
	ShowWindow(_windowWrapper.externalHandle(), SW_SHOW);
}

void SimpleWindow::hide()
{
	ShowWindow(_windowWrapper.externalHandle(), SW_HIDE);
}

void SimpleWindow::setFocus()
{
	show();
	SetForegroundWindow(_windowWrapper.externalHandle());
	SetFocus(_windowWrapper.externalHandle());
}

bool
SimpleWindow::waitMessage()
{
	return ::WaitMessage() ? true : false;
}

// Message processor
ISimpleWindow::ProcessingResult SimpleWindow::processMessage()
{
	MSG msg;
	if (PeekMessage(&msg,NULL,0,0,PM_REMOVE))
	{
		if (msg.message == WM_QUIT)
		{
			return ISimpleWindow::QUIT_MESSAGE;
		}
		else									
		{
			TranslateMessage(&msg);
			DispatchMessage(&msg);
			return ISimpleWindow::MORE_MESSAGES;
		}
	}
	return ISimpleWindow::NO_MESSAGE;
}

SimpleWindow::~SimpleWindow()
{
	HWND hWnd = _windowWrapper.externalHandle();
	if (hWnd)
		_windowWrapper.release();

    DestroyWindow(hWnd);		
    UnregisterClass(_windowClassName, GetModuleHandle(NULL));
}

}

#endif