///*****************************************************************************/
// Copyright (c) 2016 Autodesk, Inc.
// All rights reserved.
//
// These coded instructions, statements, and computer programs contain
// unpublished proprietary information written by Autodesk, Inc., and are
// protected by Federal copyright law. They may not be disclosed to third
// parties or copied or duplicated in any form, in whole or in part, without
// the prior written consent of Autodesk, Inc.
//*****************************************************************************/

#if defined(__APPLE__)
    #include <OpenGL/gl.h>
    #include <OpenGL/glu.h>
#elif defined(_WIN32)
    #include <windows.h>
    #include <GL/gl.h>
    #include <GL/glu.h>
#else
    #include <GL/gl.h>
    #include <GL/glu.h>
#endif
