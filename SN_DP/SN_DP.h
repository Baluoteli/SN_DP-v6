#pragma once

#include "resource.h"

//rtc headers

#include <comdef.h>
#include <gdiplus.h>
#pragma comment(lib, "gdiplus.lib")
using namespace Gdiplus;

enum IDM_WINDOWS
{
	IDB_LEFTWINDOW = 200,
	IDB_RIGHTWINDOW,
	IDB_MODEBUTTON,
	IDB_MINBUTTON,
};

enum DISPLAY_MODE
{
	DISPLAY_GDI = 1,
	DISPLAY_D3D,
	DISPLAY_D2D,
	DISPLAY_ERR
};

BOOL initSystem();
BOOL quitSystem();