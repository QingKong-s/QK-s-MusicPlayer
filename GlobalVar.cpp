#include <Windows.h>
#include <Shobjidl.h>
#include <d2d1.h>
#include <dwrite.h>
#include <d2d1_1.h>
#include <d3d11.h>
#include <dxgi.h>
#include <dxgi1_2.h>

#include "bass.h"

#include "MyProject.h"
#include "resource.h"
#include "Function.h"
#include "WndEffect.h"

HWND			g_hMainWnd;
HWND			g_hLV;
HWND			g_hTBProgess;
HWND			g_hLrcWnd				= NULL;
HWND			g_hBKLeft;
//HWND			g_hBKBtm;
HWND			g_hBKRight;
HWND			g_hSEB;
HWND			g_hBKList;

HINSTANCE		g_hInst;
GLOBALRES		GR						= { 0 };
GLOBALCONTEXT	GC;
SETTINGS		GS;
HFONT           g_hFontDrawing;
HFONT			g_hFont;
HFONT			g_hFontCenterLrc;
int				WM_TASKBARBUTTONCREATED;
UINT			g_uMyClipBoardFmt;
ITaskbarList4*	g_pITaskbarList			= NULL;
pFuncGetDpiForSystem pGetDpiForSystem;
pFuncGetDpiForWindow pGetDpiForWindow;
int				g_iDPI;
PWSTR			g_pszDefPic;
PWSTR			g_pszDataDir;
PWSTR			g_pszListDir;
PWSTR			g_pszCurrDir;
PWSTR			g_pszProfie;
ID2D1Factory1*	g_pD2DFactory;
IDWriteFactory* g_pDWFactory;
ID2D1Device*	g_pD2DDevice;
IDXGIDevice1*	g_pDXGIDevice;
IWICImagingFactory* g_pWICFactory;
IDXGIFactory2*	g_pDXGIFactory;
IDWriteTextFormat* g_pDWTFNormal;
IDWriteTextFormat* g_pDWTFBig;
IDWriteTextFormat* g_pDWTFBig2;

QKARRAY			g_Lrc;
int             g_iLrcState				= LRCSTATE_STOP;
QKARRAY			g_ItemData;

HSTREAM			g_hStream				= NULL;
int				g_iMusicType			= MUSICTYPE_NORMAL;
BOOL			g_bPlaying				= FALSE;

ULONGLONG       g_llLength = 0;
float			g_fTime = 0;

PWSTR			g_pszFile				= NULL;
int				g_iCurrFileIndex		= -1;
int				g_iCurrLrcIndex			= -2;
int				g_iLaterPlay			= -1;

GLOBALEFFECT    g_GlobalEffect			= { SBV_INVALIDVALUE };
BOOL            g_bSlient				= FALSE;
float           g_fDefSpeed				= 0,
				g_fSpeedChanged			= SBV_INVALIDVALUE,
				g_fBlanceChanged		= SBV_INVALIDVALUE,
				g_fVolChanged			= SBV_INVALIDVALUE;

BOOL			g_bListSeped			= FALSE;
BOOL			g_bListHidden			= FALSE;
int             g_cxBKList				= 0;
int             g_iSearchResult			= -1;
int             g_bSort					= FALSE;

BOOL			g_bPlayIcon				= FALSE;