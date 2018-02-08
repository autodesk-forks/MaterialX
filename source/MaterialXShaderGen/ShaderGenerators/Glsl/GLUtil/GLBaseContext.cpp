

#if defined(_WIN32)
#include <Windows.h>

#elif defined(__linux__)
#include <X11/Intrinsic.h> // for XtDisplay etc

#elif defined(__APPLE__)
//#include <HWGL/src/macos/HWGLWrapperSets.h>
#endif

#include <MaterialXShaderGen/ShaderGenerators/Glsl/GLew/glew.h>
#include <MaterialXShaderGen/ShaderGenerators/Glsl/GLUtil/GLBaseContext.h>

namespace MaterialX
{

// Global base context 
GLBaseContext* GLBaseContext::_globalGLBaseContext = nullptr;

//
// Windows implementation
//
#if defined(_WIN32)
GLBaseContext::GLBaseContext(HardwareContextHandle sharedWithContext)
{
	_isValid = false;

	// For windows, we need a HDC to create an OpenGL context.
	// Create a dummy 1x1 window and use it' HDC.
    _dummyWindow.create("__GL_BASE_CONTEXT_DUMMY_WINDOW__", 1, 1, 0, nullptr);
	WindowWrapper dummyWindowWrapper = _dummyWindow.windowWrapper();

	// Use a generic pixel format to create the context
	static PIXELFORMATDESCRIPTOR pfd =
	{
		sizeof(PIXELFORMATDESCRIPTOR),
		1,
		PFD_DRAW_TO_WINDOW | PFD_SUPPORT_OPENGL | PFD_DOUBLEBUFFER,
		PFD_TYPE_RGBA,
		32,
		0, 0, 0, 0, 0, 0,
		0, 0, 0, 0, 0, 0, 0,
		16, 0, 0, PFD_MAIN_PLANE, 0, 0, 0, 0
	};

	int chosenPixelFormat = ChoosePixelFormat(dummyWindowWrapper.internalHandle(), &pfd);
	if (chosenPixelFormat)
	{
		if (SetPixelFormat(dummyWindowWrapper.internalHandle(), chosenPixelFormat, &pfd))
		{
			_dummyContext = wglCreateContext(dummyWindowWrapper.internalHandle());
			if (_dummyContext != 0)
			{
				if (sharedWithContext)
					shareLists(sharedWithContext);

				int makeCurrentOk = wglMakeCurrent(dummyWindowWrapper.internalHandle(), _dummyContext);
				if (makeCurrentOk)
				{
                    // Initialize glew here?
                    _isValid = true;
				}
			}
		}
	}
}

//
// Linux context implementation
//
#elif defined(__linux__)
GLBaseContext::GLBaseContext(const WindowWrapper& windowWrapper,
								   HardwareContextHandle sharedWithContext)
{
	_isValid = false;

	//
	// Unix implementation
	//
	_windowWrapper = windowWrapper;

	// Get connection to X Server
	_display = windowWrapper.display();

	void *libHandle = dlopen("libGL.so", RTLD_LAZY);

	char *error;
	XVisualInfo * ( * ChooseVisualFuncPtr)(Display *, int, int *);
	ChooseVisualFuncPtr = (XVisualInfo *(*)(Display *, int, int *))
									dlsym(libHandle, "glXChooseVisual");
	if((error = dlerror()) != 0)  
    {
        return;
	}

	GLXContext ( * CreateContextFuncPtr)(Display *, XVisualInfo *, GLXContext, Bool);
	CreateContextFuncPtr = (GLXContext (*)(Display *, XVisualInfo *, GLXContext, Bool))
									dlsym(libHandle, "glXCreateContext");
    if((error = dlerror()) != 0)  
    {
        return;
    }

	Bool ( * MakeCurrentFuncPtr)(Display *, GLXDrawable, GLXContext);
	MakeCurrentFuncPtr = (Bool (*)(Display *, GLXDrawable, GLXContext))
									dlsym(libHandle, "glXMakeCurrent");
	if((error = dlerror()) != 0)  
    {
        return;
	}

	GLXDrawable ( * GetDrawableFuncPtr)();
	GetDrawableFuncPtr = (GLXDrawable (*)())dlsym(libHandle, "glXGetCurrentDrawable");
	if((error = dlerror()) != 0)  
    {
        return;
	}

	GLXContext ( * GetContextFuncPtr)();
	GetContextFuncPtr = (GLXContext (*)())dlsym(libHandle, "glXGetCurrentContext");
	if((error = dlerror()) != 0)  
    {
        return;
	}

	if(ChooseVisualFuncPtr == 0 || CreateContextFuncPtr == 0 || MakeCurrentFuncPtr == 0 || GetDrawableFuncPtr == 0 || GetContextFuncPtr == 0)
	{
        return;
	}

	int list[30];
	int i = 0;

	list[i++] = GLX_RGBA;
	list[i++] = GLX_DOUBLEBUFFER;
	list[i++] = GLX_RED_SIZE; list[i++] = 8;
	list[i++] = GLX_GREEN_SIZE; list[i++] = 8;
	list[i++] = GLX_BLUE_SIZE; list[i++] = 8;
	list[i++] = GLX_ALPHA_SIZE; list[i++] = 8;
	list[i++] = GLX_DEPTH_SIZE; list[i++] = 24;
	list[i++] = GLX_STENCIL_SIZE; list[i++] = 8;

	list[i++] = None;
	XVisualInfo *vinfo = ChooseVisualFuncPtr(_display, DefaultScreen(_display), list);
	if(vinfo == 0)
	{
		_dummyContext = 0;
		return;
	}

	// Create context that shares display lists and texture objects across contexts
    if (sharedWithContext)
    {
        _dummyContext = CreateContextFuncPtr(_display, vinfo, sharedWithContext, GL_TRUE);
    }
	else
		_dummyContext = CreateContextFuncPtr(_display, vinfo, 0, GL_TRUE);
	if(_dummyContext == 0)
	{
		_dummyContext = 0;
		return;
	}

	// For glX need a window to make the context created above current, creating
	// minimal requirements for an OpenGL window

	Window root = RootWindow(_display, DefaultScreen(_display));
	Colormap cmap = XCreateColormap(_display, root, vinfo->visual, AllocNone); 
	XSetWindowAttributes wa;
	unsigned long attribMask;
	attribMask=CWBackPixmap | CWBorderPixel | CWColormap;
	wa.background_pixmap = None;
	wa.border_pixel = 0;
	wa.colormap = cmap;

	// Create an X window with the visual requested above
	_dummyWindow= XCreateWindow(_display, root, 0, 0, 10, 10, 0, vinfo->depth, InputOutput, 
                                vinfo->visual, attribMask, &wa);
	if(_dummyWindow == 0)
	{
		_dummyContext = 0;
		return;
	}

	//	Save the current context.
	GLXDrawable oldDrawable = GetDrawableFuncPtr();
	GLXContext	oldContext = GetContextFuncPtr();
	bool haveOldContext = (NULL != oldContext);
	bool haveOldDrawable = (None != oldDrawable);

	MakeCurrentFuncPtr(_display, _dummyWindow, _dummyContext);

	// Populate Function Table
	if(_display)
	{
        // Initialize glew here?
        _isValid = true;

        //	Restore the previous context
        if (haveOldContext && haveOldDrawable)
        {
            MakeCurrentFuncPtr(_display, oldDrawable, oldContext);
        }
	}
}

GLBaseContext::GLBaseContext(HardwareContextHandle sharedWithContext)
{
	_dummyWindow = 0;
	_dummyContext = 0;
	_display = 0;

	_isValid = false;
}

//
// OSX implementation
//
#elif defined(__APPLE__)

#if defined(__APPLE__MachO_)
#include <Carbon/Carbon.h>
#endif

GLBaseContext::GLBaseContext(HardwareContextHandle sharedWithContext)
{	
	_isValid = false;

	void* pixelFormat = aglToNSOpenGLChoosePixelFormatWrapper(true, 0,  32, 24, 8, 0, 0, false, 
                                                              false, false, false, false);
	if (!pixelFormat)
	{
		return;
	}
		
	// Create the context, but do not share against other contexts.
	// (Instead, all other contexts will share against this one.)
	//
    _dummyContext = aglToNSOpenGLCreateContextWrapper(pixelFormat, sharedWithContext);

	aglCheckError();
           
	aglToNSOpenGLReleasePixelFormat(pixelFormat);
	aglCheckError();

	aglToNSOpenGLMakeCurrent( _dummyContext );
	aglCheckError();
       	
    // Initialize glew here?
    _isValid = true;
}
#endif

// Destroy the startup context.
GLBaseContext::~GLBaseContext()
{
	// Only do this portion if the context is valid
	if (_isValid) 
	{
#if _WIN32
		// Release the dummy context.
		wglDeleteContext(_dummyContext);

#elif defined(__linux__)
		glXMakeCurrent(_display, None, NULL);
		
		// This needs to be done after all the GL object
		// created with this context are destroyed.
		if(_dummyContext != 0)
			glXDestroyContext(_display, _dummyContext);
		if(_dummyWindow != 0)
			XDestroyWindow(_display, _dummyWindow);

#elif defined(__APPLE__)
		// This needs to be done after all the GL object
		// created with this context are destroyed.
		if(_dummyContext != 0)
		{
			DBOUT(("Destroy the startup context"));
			aglToNSOpenGLDestroyCurrentContext(&_dummyContext);
		}
#endif
	}

	GLBaseContext::_globalGLBaseContext = 0;
}

int GLBaseContext::makeCurrent()
{
    if (!_isValid) return 0;

    int makeCurrentOk = 0;
#if defined(_WIN32)
    makeCurrentOk = wglMakeCurrent(_dummyWindow.windowWrapper().internalHandle(), _dummyContext);
#elif defined(__linux__)
    makeCurrentOk = glXMakeCurrent(_display, _dummyWindow, _dummyContext);
#elif defined(__APPLE__)
    aglToNSOpenGLMakeCurrent(_dummyContext);
    if (aglToNSOpenGLGetCurrentContextWrapper() == _dummyContext)
        makeCurrentOk = 1;
    aglCheckError();
#else
#error OS not supported
#endif
    return makeCurrentOk;
}

#if defined(_WIN32)
void GLBaseContext::shareLists(HardwareContextHandle context)
{
    if (_isValid)
        wglShareLists(_dummyContext, context);
}
#endif

#if defined(__linux__)
GLBaseContext* GLBaseContext::create(const WindowWrapper& windowWrapper, HardwareContextHandle context)
{
    if (!_globalGLBaseContext)
    {
        _globalGLBaseContext = new GLBaseContext(windowWrapper, context);
    }
    return _globalGLBaseContext;
}
#else
GLBaseContext* GLBaseContext::create(HardwareContextHandle context)
{
    if (!_globalGLBaseContext)
    {
        _globalGLBaseContext = new GLBaseContext(context);
    }
    return _globalGLBaseContext;
}
#endif

void GLBaseContext::destroy()
{
    if (_globalGLBaseContext)
    {
        delete _globalGLBaseContext;
        _globalGLBaseContext = nullptr;
    }
}

}

