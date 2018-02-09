#ifndef MATERIALX_SIMPLEWINDOW_H
#define MATERIALX_SIMPLEWINDOW_H

#include "Platform.h"
#include "ISimpleWindow.h"

#include <string>

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

    /// Clear internal state information
    void clearInternalState() override
    {
        ISimpleWindow::clearInternalState();
        _id = 0;
    }

  protected:
	// Unique window ID. Only valid if different than 0.
	unsigned int _id;

#if defined(OSWin_)
    // Class name for window (generated)
	char _windowClassName[128];
#endif

};

}

#endif // _SimpleWindow_h_
