// Emacs style mode select   -*- C++ -*-
//-----------------------------------------------------------------------------
//
// Copyright (C) 1998-2000 by DooM Legacy Team.
//
// This program is free software; you can redistribute it and/or
// modify it under the terms of the GNU General Public License
// as published by the Free Software Foundation; either version 2
// of the License, or (at your option) any later version.
//
// This program is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU General Public License for more details.
//
//-----------------------------------------------------------------------------
/// \file
/// \brief SDL specific part of the OpenGL API for SRB2

#ifdef _MSC_VER
#pragma warning(disable : 4214 4244)
#endif

#ifdef HAVE_SDL
#define _MATH_DEFINES_DEFINED

#include "SDL.h"

#include "sdlmain.h"

#ifdef _MSC_VER
#pragma warning(default : 4214 4244)
#endif

#include "../doomdef.h"

#ifdef HWRENDER
#include "../hardware/r_opengl/r_opengl.h"
#include "ogl_sdl.h"
#include "../i_system.h"
#include "hwsym_sdl.h"
#include "../m_argv.h"

#ifdef DEBUG_TO_FILE
#include <stdarg.h>
#if defined (_WIN32) && !defined (__CYGWIN__)
#include <direct.h>
#else
#include <unistd.h>
#endif
#include <sys/types.h>
#include <sys/stat.h>
#endif

#ifdef USE_WGL_SWAP
PFNWGLEXTSWAPCONTROLPROC wglSwapIntervalEXT = NULL;
#else
typedef int (*PFNGLXSWAPINTERVALPROC) (int);
PFNGLXSWAPINTERVALPROC glXSwapIntervalSGIEXT = NULL;
#endif

/**	\brief SDL video display surface
*/
INT32 oglflags = 0;
void *GLUhandle = NULL;
SDL_GLContext sdlglcontext = 0;

/**	\brief	The OglSdlSurface function

	\param	w	width
	\param	h	height
	\param	isFullscreen	if true, go fullscreen

	\return	if true, changed video mode
*/
boolean OglSdlSurface(INT32 w, INT32 h)
{
	INT32 cbpp;
	const GLvoid *glvendor = NULL, *glrenderer = NULL, *glversion = NULL;

	cbpp = cv_scr_depth.value < 16 ? 16 : cv_scr_depth.value;

	glvendor = glGetString(GL_VENDOR);
	// Get info and extensions.
	//BP: why don't we make it earlier ?
	//Hurdler: we cannot do that before intialising gl context
	glrenderer = glGetString(GL_RENDERER);
	glversion = glGetString(GL_VERSION);
	gl_extensions = glGetString(GL_EXTENSIONS);

	DBG_Printf("Vendor     : %s\n", glvendor);
	DBG_Printf("Renderer   : %s\n", glrenderer);
	DBG_Printf("Version    : %s\n", glversion);
	DBG_Printf("Extensions : %s\n", gl_extensions);
	oglflags = 0;

	if (isExtAvailable("GL_EXT_texture_filter_anisotropic", gl_extensions))
		glGetIntegerv(GL_MAX_TEXTURE_MAX_ANISOTROPY_EXT, &maximumAnisotropy);
	else
		maximumAnisotropy = 1;

	granisotropicmode_cons_t[1].value = maximumAnisotropy;

	SDL_GL_SetSwapInterval(cv_vidwait.value ? 1 : 0);

	SetModelView(w, h);
	SetStates();
	glClear(GL_COLOR_BUFFER_BIT|GL_DEPTH_BUFFER_BIT);

	HWR_Startup();
	textureformatGL = cbpp > 16 ? GL_RGBA : GL_RGB5_A1;

	return true;
}
/**	\brief	The OglSdlFinishUpdate function

	\param	vidwait	wait for video sync

	\return	void
*/
void OglSdlFinishUpdate(boolean waitvbl)
{
	static boolean oldwaitvbl = false;
	int sdlw, sdlh;
	if (oldwaitvbl != waitvbl)
	{
		SDL_GL_SetSwapInterval(waitvbl ? 1 : 0);
	}

	oldwaitvbl = waitvbl;

	SDL_GetWindowSize(window, &sdlw, &sdlh);

	HWR_MakeScreenFinalTexture();
	HWR_DrawScreenFinalTexture(sdlw, sdlh);
	SDL_GL_SwapWindow(window);

	GClipRect(0, 0, realwidth, realheight, NZCLIP_PLANE);

	HWR_DrawScreenFinalTexture(realwidth, realheight);
}

EXPORT void HWRAPI( OglSdlSetPalette) (RGBA_t *palette, RGBA_t *pgamma)
{
	INT32 i = -1;
	UINT32 redgamma = pgamma->s.red, greengamma = pgamma->s.green,
		bluegamma = pgamma->s.blue;

	for (i = 0; i < 256; i++)
	{
		myPaletteData[i].s.red   = (UINT8)MIN((palette[i].s.red   * redgamma)  /127, 255);
		myPaletteData[i].s.green = (UINT8)MIN((palette[i].s.green * greengamma)/127, 255);
		myPaletteData[i].s.blue  = (UINT8)MIN((palette[i].s.blue  * bluegamma) /127, 255);
		myPaletteData[i].s.alpha = palette[i].s.alpha;
	}
	Flush();
}

#endif //HWRENDER
#endif //SDL
