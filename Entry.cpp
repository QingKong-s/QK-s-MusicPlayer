/*
* Entry.cpp
* 应用程序入口点，加载全局设置，执行全局初始化
*/
// 使用6.0版通用组件库，老版的太难看√都不用
#pragma comment(linker,"\"/manifestdependency:type='win32' \
name='Microsoft.Windows.Common-Controls' version='6.0.0.0' \
processorArchitecture='*' publicKeyToken='6595b64144ccf1df' language='*'\"")
// 导入静态库
#pragma comment(lib,"Shlwapi.lib")
#pragma comment(lib,"UxTheme.lib")
#pragma comment(lib,"Comctl32.lib")
#pragma comment(lib,"Msimg32.lib")
#pragma comment(lib,"gdiplus.lib")
#pragma comment(lib,"D2d1.lib")
#pragma comment(lib,"dwrite.lib")
#pragma comment(lib,"dwmapi.lib")
#pragma comment(lib,"D3D11.lib")
#pragma comment(lib,"dxguid.lib")

#pragma comment(lib,"bass.lib")
#pragma comment(lib,"bass_fx.lib")
#pragma comment(lib,"bassmidi.lib")

#include <Windows.h>
#include <WinUser.h>
#include <d2d1.h>
#include <dwrite.h>
#include <d3d11.h>

#include "bass.h"
#include "bass_fx.h"
#include "bassmidi.h"

#include "MyProject.h"
#include "GlobalVar.h"
#include "QKCtrl.h"
#include "WndLrc.h"
#include "WndMain.h"
#include "WndList.h"
#include "resource.h"

/*
 * 目标：
 *
 * 参数：
 *
 * 返回值：
 * 操作简述：
 * 备注：
 */

 /*
  * 目标：入口点
  *
  * 参数：
  * hInstance 实例句柄
  * hPrevInstance 先前实例句柄（恒为NULL）
  * lpCmdLine 命令行
  * nCmdShow 显示方式
  *
  * 返回值：
  * 操作简述：
  * 备注：
  *
  */
int APIENTRY wWinMain(_In_ HINSTANCE hInstance, _In_opt_ HINSTANCE hPrevInstance, _In_ LPWSTR lpCmdLine, _In_ int nCmdShow)
{
	BOOL bSuccessful = TRUE;
    g_hInst = hInstance;
    HMODULE hLib = LoadLibraryW(L"User32.dll");
    if (!hLib)
    {
        Global_ShowError(L"User32.dll加载失败", L"你确定你的电脑还能用吗", ECODESRC_WINSDK);
        return 1;
	}
	pGetDpiForSystem = (pFuncGetDpiForSystem)GetProcAddress(hLib, "GetDpiForSystem");
	pGetDpiForWindow = (pFuncGetDpiForWindow)GetProcAddress(hLib, "GetDpiForWindow");
	//////////////初始化COM和OLE
	HRESULT hr;
	hr = CoInitialize(NULL);
	if (FAILED(hr))
	{
		Global_ShowError(L"COM库初始化失败", NULL, ECODESRC_OTHERS, NULL, hr);
		return 1;
	}
	hr = OleInitialize(NULL);
	if (FAILED(hr))
	{
		Global_ShowError(L"OLE库初始化失败", NULL, ECODESRC_OTHERS, NULL, hr);
		return 1;
	}
    //////////////启动GDI+
    GdiplusStartupInput gpsi = { 0 };
    gpsi.GdiplusVersion = 1;
	ULONG_PTR uGPToken;
    GpStatus GPRet;
    GPRet = GdiplusStartup(&uGPToken, &gpsi, NULL);
	if (GPRet != Ok)
    {
		Global_ShowError(L"GDI+启动失败", NULL, ECODESRC_OTHERS, NULL, GPRet);
        return 1;
    }

	D2D1CreateFactory(D2D1_FACTORY_TYPE_SINGLE_THREADED, __uuidof(ID2D1Factory1), (void**)&g_pD2DFactory);
	DWriteCreateFactory(DWRITE_FACTORY_TYPE_SHARED, __uuidof(IDWriteFactory), reinterpret_cast<IUnknown**>(&g_pDWFactory));
    ID3D11Device* pD3DDevice;
    D3D11CreateDevice(NULL, D3D_DRIVER_TYPE_HARDWARE, NULL, D3D11_CREATE_DEVICE_BGRA_SUPPORT
#ifndef NDEBUG
        | D3D11_CREATE_DEVICE_DEBUG
#endif // NDEBUG
        , NULL, 0, D3D11_SDK_VERSION, &pD3DDevice, NULL, NULL);
    pD3DDevice->QueryInterface(__uuidof(IDXGIDevice1), (void**)&g_pDXGIDevice);
    pD3DDevice->Release();

    IDXGIAdapter* pDXGIAdapter;
    g_pDXGIDevice->GetAdapter(&pDXGIAdapter);

    pDXGIAdapter->GetParent(IID_PPV_ARGS(&g_pDXGIFactory));
    pDXGIAdapter->Release();

    g_pD2DFactory->CreateDevice(g_pDXGIDevice, &g_pD2DDevice);
    CoCreateInstance(CLSID_WICImagingFactory, NULL, CLSCTX_INPROC_SERVER, IID_PPV_ARGS(&g_pWICFactory));
    //////////////载入资源，填充全局上下文
    if (pGetDpiForSystem)
        Res_Load(pGetDpiForSystem() * 16 / 96);
    else
		Res_Load(QKGetDPIForWindow(NULL) * 16 / 96);
    GC =
	{
		CreateSolidBrush(QKCOLOR_CYANDEEPER),
		CreateSolidBrush(0xDA9E46),
		0
	};
	//////////////创建字体
	g_hFont = QKCreateFont(L"微软雅黑", 9);
	g_hFontDrawing = QKCreateFont(L"微软雅黑", 13);
	g_hFontCenterLrc = QKCreateFont(L"微软雅黑", 15, 700);

    g_pDWFactory->CreateTextFormat(L"微软雅黑", NULL, DWRITE_FONT_WEIGHT_NORMAL, DWRITE_FONT_STYLE_NORMAL, DWRITE_FONT_STRETCH_NORMAL,
        9, L"zh-cn", &g_pDWTFNormal);
    g_pDWFactory->CreateTextFormat(L"微软雅黑", NULL, DWRITE_FONT_WEIGHT_NORMAL, DWRITE_FONT_STYLE_NORMAL, DWRITE_FONT_STRETCH_NORMAL,
        13, L"zh-cn", &g_pDWTFBig);
    g_pDWFactory->CreateTextFormat(L"微软雅黑", NULL, DWRITE_FONT_WEIGHT_BOLD, DWRITE_FONT_STYLE_NORMAL, DWRITE_FONT_STRETCH_NORMAL,
        15, L"zh-cn", &g_pDWTFBig2);
	//////////////保存运行目录
	PWSTR p = new WCHAR[MAX_PATH];
	GetModuleFileNameW(NULL, p, MAX_PATH);
	PathRemoveFileSpecW(p);

    g_pszDefPic = new WCHAR[lstrlenW(p) + lstrlenW(DEFPICFILENAME) + 1];
    lstrcpyW(g_pszDefPic, p);
    lstrcatW(g_pszDefPic, DEFPICFILENAME);

    g_pszDataDir = new WCHAR[lstrlenW(p) + lstrlenW(DATADIR) + 1];
    lstrcpyW(g_pszDataDir, p);
    lstrcatW(g_pszDataDir, DATADIR);

    g_pszListDir = new WCHAR[lstrlenW(p) + lstrlenW(LISTDIR) + 1];
    lstrcpyW(g_pszListDir, p);
    lstrcatW(g_pszListDir, LISTDIR);

    g_pszCurrDir = new WCHAR[lstrlenW(p) + 2];
    lstrcpyW(g_pszCurrDir, p);
    lstrcatW(g_pszCurrDir, L"\\");

    g_pszProfie = new WCHAR[lstrlenW(p) + lstrlenW(PROFILENAME) + 1];
    lstrcpyW(g_pszProfie, p);
    lstrcatW(g_pszProfie, PROFILENAME);

    delete[] p;
    //////////////注册任务栏按钮创建消息
    WM_TASKBARBUTTONCREATED = RegisterWindowMessageW(L"TaskbarButtonCreated");
    g_uMyClipBoardFmt = RegisterClipboardFormatW(CLIPBOARDFMT_MYDRAGDROP);
    //////////////注册窗口类
    bSuccessful = QKCtrlInit() && bSuccessful;
    bSuccessful = LrcWnd_Init() && bSuccessful;
    WNDCLASSEXW wcex = { 0 };
    wcex.cbSize = sizeof(WNDCLASSEX);
    wcex.style = CS_HREDRAW | CS_VREDRAW;
    wcex.hInstance = hInstance;
    wcex.hIcon = (HICON)LoadImageW(hInstance, MAKEINTRESOURCEW(IDI_MAIN), IMAGE_ICON, 256, 256, LR_SHARED);
    wcex.hCursor = LoadCursorW(NULL, IDC_ARROW);
    wcex.hbrBackground = (HBRUSH)GetStockObject(WHITE_BRUSH);
    /////////注册主窗口
    wcex.lpfnWndProc = WndProc;
    wcex.lpszClassName = MAINWNDCLASS;
    bSuccessful = RegisterClassExW(&wcex) && bSuccessful;
    /////////注册任务栏幽灵（注意要与主窗口图标保持一致）
    wcex.hbrBackground = NULL;
    wcex.lpszClassName = TBGHOSTWNDCLASS;
    wcex.lpfnWndProc = WndProc_TBGhost;
    bSuccessful = RegisterClassExW(&wcex) && bSuccessful;
    /////////注册列表容器（保持图标）
    wcex.lpszClassName = WNDCLASS_LIST;
    wcex.lpfnWndProc = WndProc_PlayList;
    bSuccessful = RegisterClassExW(&wcex) && bSuccessful;
    /////////注册通用背景
	wcex.hIcon = NULL;
    wcex.style |= CS_OWNDC;
	wcex.lpfnWndProc = (WNDPROC)GetProcAddress(hLib, "DefWindowProcW");
	wcex.lpszClassName = BKWNDCLASS;
	bSuccessful = RegisterClassExW(&wcex) && bSuccessful;
	if (!bSuccessful)
	{
		Global_ShowError(L"窗口类注册失败", NULL);
		return 1;
	}
	//////////////创建窗口
	if (!BASS_Init(-1, 44100, 0, g_hMainWnd, NULL))// 初始化Bass
		Global_ShowError(L"Bass初始化失败", L"稍后请尝试更换输出设备", ECODESRC_BASS);
	if (pGetDpiForSystem)
		g_hMainWnd = CreateWindowExW(0, MAINWNDCLASS, L"未播放 - 晴空的音乐播放器", WS_OVERLAPPEDWINDOW | WS_CLIPCHILDREN,
			CW_USEDEFAULT, 0, pGetDpiForSystem() * 1000 / 96, pGetDpiForSystem() * 640 / 96, NULL, NULL, hInstance, NULL);
	else
		g_hMainWnd = CreateWindowExW(0, MAINWNDCLASS, L"未播放 - 晴空的音乐播放器", WS_OVERLAPPEDWINDOW | WS_CLIPCHILDREN,
			CW_USEDEFAULT, 0, 1000, 640, NULL, NULL, hInstance, NULL);

    if (!g_hMainWnd)
    {
        Global_ShowError(L"创建主窗口失败", NULL, ECODESRC_WINSDK);
        return 1;
    }

    ShowWindow(g_hMainWnd, nCmdShow);
    UpdateWindow(g_hMainWnd);
    //////////////消息循环
    MSG msg;
    while (GetMessageW(&msg, NULL, 0, 0))
    {
        TranslateMessage(&msg);
        DispatchMessageW(&msg);
    }
    //////////////清理全局资源
    FreeLibrary(hLib);
    BASS_Free();
    DeleteObject(g_hFontCenterLrc);
    DeleteObject(g_hFont);
    DeleteObject(g_hFontDrawing);
    GdiplusShutdown(uGPToken);
    OleUninitialize();
    CoUninitialize();
    return (int)msg.wParam;
}
void Global_ShowError(PCWSTR pszTitle, PCWSTR pszContent, int iErrCodeSrc, HWND hParent, DWORD dwOtherErrCode)
{
    PWSTR psz = NULL;
    DWORD dwErrCode;
    if (pszContent)
    {
        psz = new WCHAR[lstrlenW(pszContent) + 20];
        lstrcpyW(psz, pszContent);

        if (iErrCodeSrc != ECODESRC_NONE)
        {
            if (iErrCodeSrc == ECODESRC_BASS)
                dwErrCode = BASS_ErrorGetCode();
            else if (iErrCodeSrc == ECODESRC_WINSDK)
                dwErrCode = GetLastError();
            else
                dwErrCode = dwOtherErrCode;
            lstrcatW(psz, L"\n错误代码：0x");
            wsprintfW(psz + lstrlenW(psz), L"%08x", dwErrCode);
        }
    }
    else if (iErrCodeSrc != ECODESRC_NONE)
    {
        if (iErrCodeSrc == ECODESRC_BASS)
            dwErrCode = BASS_ErrorGetCode();
        else if (iErrCodeSrc == ECODESRC_WINSDK)
            dwErrCode = GetLastError();
        else
            dwErrCode = dwOtherErrCode;
        psz = new WCHAR[20];
        lstrcpyW(psz, L"错误代码：0x");
        wsprintfW(psz + lstrlenW(psz), L"%08x", dwErrCode);
    }

	QKMessageBox(pszTitle, psz, (HICON)TD_ERROR_ICON, L"错误", hParent);
	delete[] psz;
}
ULONG_PTR BASS_OpenMusic(PWSTR pszFile, DWORD dwFlagsHS, DWORD dwFlagsHM, DWORD dwFlagsHMIDI)
{
	ULONG_PTR h;
	h = BASS_StreamCreateFile(FALSE, pszFile, 0, 0, BASS_UNICODE | dwFlagsHS);
	g_iMusicType = MUSICTYPE_NORMAL;
	if (!h && BASS_ErrorGetCode() == BASS_ERROR_FILEFORM)
	{
		h = BASS_MusicLoad(FALSE, pszFile, 0, 0, BASS_UNICODE | dwFlagsHM, 0);
		g_iMusicType = MUSICTYPE_MOD;
		if (!h && BASS_ErrorGetCode() == BASS_ERROR_FILEFORM)
		{
			h = BASS_MIDI_StreamCreateFile(FALSE, pszFile, 0, 0, dwFlagsHMIDI, 1);
			g_iMusicType = MUSICTYPE_MIDI;
		}
		else
			g_iMusicType = MUSICTYPE_NORMAL;
	}

	return h;
}
BOOL BASS_FreeMusic(ULONG_PTR h)
{
    if (!h)
        return FALSE;

    switch (g_iMusicType)
    {
    case MUSICTYPE_NORMAL:
    case MUSICTYPE_MIDI:
        return BASS_StreamFree(h);
    case MUSICTYPE_MOD:
        return BASS_MusicFree(h);
    default:
        return FALSE;
    }
}
void UI_UpdateDPISize()
{
    GC.DS_CYPROGBAR = DPI(SIZE_CYPROGBAR);
    GC.DS_CYPROGBARCORE = DPI(SIZE_CYPROGBARCORE);
    GC.DS_CYBTBK = DPI(SIZE_CYBTBK);
    GC.DS_CYSPE = DPI(SIZE_CYSPE);
    GC.DS_CYSPEHALF = DPI(SIZE_CYSPEHALF);
    GC.DS_CXSPEBAR = DPI(SIZE_CXSPEBAR);
    GC.DS_CXSPEBARDIV = DPI(SIZE_CXSPEBARDIV);
    GC.DS_CXSPE = DPI(SIZE_CXSPE);
    GC.DS_CXSPEHALF = DPI(SIZE_CXSPEHALF);
    GC.DS_CXBTMBTBK = DPI(SIZE_CXBTMBTBK);
    GC.DS_CXPIC = DPI(SIZE_CXPIC);
    GC.DS_EDGE = DPI(SIZE_EDGE);
    GC.DS_CYTOPBK = DPI(SIZE_CYTOPBK);
    GC.DS_BT = DPI(SIZE_BT);
    GC.DS_CXRITBT = DPI(SIZE_CXRITBT);
    GC.DS_GAP = DPI(SIZE_GAP);
    GC.DS_LARGEIMAGE = DPI(SIZE_LARGEIMAGE);
    GC.DS_CYRITBK = DPI(SIZE_CYRITBK);
    GC.DS_CYSTLISTNAME = DPI(SIZE_CYSTLISTNAME);
    GC.DS_CXTIME = DPI(SIZE_CXTIME);
    GC.DS_CXWAVESLINE = DPI(SIZE_CXWAVESLINE);
    GC.DS_CYTOPTITLE = DPI(SIZE_CYTOPTITLE);
    GC.DS_CXTOPTIP = DPI(SIZE_CXTOPTIP);
    GC.DS_CYTOPTIP = DPI(SIZE_CYTOPTIP);
    GC.DS_GAPTOPTIP = DPI(SIZE_GAPTOPTIP);
    GC.DS_CXABOUTPIC = DPI(SIZE_CXABOUTPIC);
    GC.DS_CYABOUTPIC = DPI(SIZE_CYABOUTPIC);
    GC.DS_LVTEXTSPACE = DPI(SIZE_LVTEXTSPACE);
    GC.DS_CXLRCTB = DPI(SIZE_CXLRCTB);
    GC.DS_DEFCXLV = DPI(SIZE_DEFCXLV);
    GC.DS_DTLRCEDGE = DPI(SIZE_DTLRCEDGE);
    GC.DS_DTLRCFRAME = DPI(SIZE_DTLRCFRAME);
    GC.DS_STDICON = DPI(SIZE_STDICON);
    GC.DS_CXDTLRCBTNRGN = DPIS_DTLRCEDGE * 3 + DPIS_BT * 4;
    GC.DS_CYLVITEM = DPI(SIZE_CYLVITEM);
    GC.DS_LRCSHOWGAP = DPI(SIZE_LRCSHOWGAP);
	GC.DS_CXDRAGDROPICON = DPI(SIZE_CXDRAGDROPICON);
	GC.DS_CYDRAGDROPICON = DPI(SIZE_CYDRAGDROPICON);
	GC.DS_LVDRAGEDGE = DPI(SIZE_LVDRAGEDGE);

	GC.iIconSize = DPI(16);
	GC.cyBT = GC.iIconSize * 10 / 5;
    GC.cxBKBtm = DPIS_CXTIME + BTMBKBTNCOUNT * GC.cyBT;

    SAFE_RELEASE(g_pDWTFNormal);
    SAFE_RELEASE(g_pDWTFBig);
    SAFE_RELEASE(g_pDWTFBig2);

    g_pDWFactory->CreateTextFormat(L"微软雅黑", NULL, DWRITE_FONT_WEIGHT_NORMAL, DWRITE_FONT_STYLE_NORMAL, DWRITE_FONT_STRETCH_NORMAL,
        DPI(12), L"zh-cn", &g_pDWTFNormal);
    g_pDWFactory->CreateTextFormat(L"微软雅黑", NULL, DWRITE_FONT_WEIGHT_SEMI_LIGHT, DWRITE_FONT_STYLE_NORMAL, DWRITE_FONT_STRETCH_NORMAL,
        DPI(18), L"zh-cn", &g_pDWTFBig);
    g_pDWFactory->CreateTextFormat(L"微软雅黑", NULL, DWRITE_FONT_WEIGHT_BOLD, DWRITE_FONT_STYLE_NORMAL, DWRITE_FONT_STRETCH_NORMAL,
        DPI(20), L"zh-cn", &g_pDWTFBig2);
}
void Res_Free()
{
    for (int i = 0; i <= sizeof(GR); i += sizeof(HANDLE))
    {
        DeleteObject(*(HGDIOBJ*)((BYTE*)&GR + i));
    }
}
void Res_Load(int iSize)
{
    Res_Free();
    GR =
    {
        NULL,
        (HICON)LoadImageW(g_hInst, MAKEINTRESOURCEW(IDI_LOCATE), IMAGE_ICON, iSize, iSize, 0),
        (HICON)LoadImageW(g_hInst, MAKEINTRESOURCEW(IDI_PLUS), IMAGE_ICON, iSize, iSize, 0),
        (HICON)LoadImageW(g_hInst, MAKEINTRESOURCEW(IDI_READFILE), IMAGE_ICON, iSize, iSize, 0),
        (HICON)LoadImageW(g_hInst, MAKEINTRESOURCEW(IDI_DISK), IMAGE_ICON, iSize, iSize, 0),
        (HICON)LoadImageW(g_hInst, MAKEINTRESOURCEW(IDI_CROSS), IMAGE_ICON, iSize, iSize, 0),
        (HICON)LoadImageW(g_hInst, MAKEINTRESOURCEW(IDI_SEARCH), IMAGE_ICON, iSize, iSize, 0),
        (HICON)LoadImageW(g_hInst, MAKEINTRESOURCEW(IDI_THREETRACKBARS), IMAGE_ICON, iSize, iSize, 0),
        (HICON)LoadImageW(g_hInst, MAKEINTRESOURCEW(IDI_PLAYLIST), IMAGE_ICON, iSize, iSize, 0),
        (HICON)LoadImageW(g_hInst, MAKEINTRESOURCEW(IDI_ARROWCROSS), IMAGE_ICON, iSize, iSize, 0),
        (HICON)LoadImageW(g_hInst, MAKEINTRESOURCEW(IDI_ARROWRIGHT), IMAGE_ICON, iSize, iSize, 0),
        (HICON)LoadImageW(g_hInst, MAKEINTRESOURCEW(IDI_ARROWRIGHTTHREE), IMAGE_ICON, iSize, iSize, 0),
        (HICON)LoadImageW(g_hInst, MAKEINTRESOURCEW(IDI_ARROWCIRCLEONE), IMAGE_ICON, iSize, iSize, 0),
        (HICON)LoadImageW(g_hInst, MAKEINTRESOURCEW(IDI_ARROWCIRCLE), IMAGE_ICON, iSize, iSize, 0),
        (HICON)LoadImageW(g_hInst, MAKEINTRESOURCEW(IDI_LAST), IMAGE_ICON, iSize, iSize, 0),
        (HICON)LoadImageW(g_hInst, MAKEINTRESOURCEW(IDI_PLAY), IMAGE_ICON, iSize, iSize, 0),
        (HICON)LoadImageW(g_hInst, MAKEINTRESOURCEW(IDI_PAUSE), IMAGE_ICON, iSize, iSize, 0),
        (HICON)LoadImageW(g_hInst, MAKEINTRESOURCEW(IDI_STOP), IMAGE_ICON, iSize, iSize, 0),
        (HICON)LoadImageW(g_hInst, MAKEINTRESOURCEW(IDI_NEXT), IMAGE_ICON, iSize, iSize, 0),
        (HICON)LoadImageW(g_hInst, MAKEINTRESOURCEW(IDI_LRC), IMAGE_ICON, iSize, iSize, 0),
        (HICON)LoadImageW(g_hInst, MAKEINTRESOURCEW(IDI_SETTINGS), IMAGE_ICON, iSize, iSize, 0),
        (HICON)LoadImageW(g_hInst, MAKEINTRESOURCEW(IDI_INFO), IMAGE_ICON, iSize, iSize, 0),
        (HICON)LoadImageW(g_hInst, MAKEINTRESOURCEW(IDI_PLAYLISTMANAGE), IMAGE_ICON, iSize, iSize, 0),
        (HICON)LoadImageW(g_hInst, MAKEINTRESOURCEW(IDI_LAST2), IMAGE_ICON, iSize, iSize, 0),
        (HICON)LoadImageW(g_hInst, MAKEINTRESOURCEW(IDI_PLAY2), IMAGE_ICON, iSize, iSize, 0),
        (HICON)LoadImageW(g_hInst, MAKEINTRESOURCEW(IDI_PAUSE2), IMAGE_ICON, iSize, iSize, 0),
        (HICON)LoadImageW(g_hInst, MAKEINTRESOURCEW(IDI_NEXT2), IMAGE_ICON, iSize, iSize, 0),
        (HICON)LoadImageW(g_hInst, MAKEINTRESOURCEW(IDI_CROSS2), IMAGE_ICON, iSize, iSize, 0),
        (HICON)LoadImageW(g_hInst, MAKEINTRESOURCEW(IDI_TICK), IMAGE_ICON, iSize, iSize, 0),
        (HICON)LoadImageW(g_hInst, MAKEINTRESOURCEW(IDI_TICK2), IMAGE_ICON, iSize, iSize, 0),
        (HICON)LoadImageW(g_hInst, MAKEINTRESOURCEW(IDI_SPEED), IMAGE_ICON, iSize, iSize, 0),
        (HICON)LoadImageW(g_hInst, MAKEINTRESOURCEW(IDI_BLANCE), IMAGE_ICON, iSize, iSize, 0),
        (HICON)LoadImageW(g_hInst, MAKEINTRESOURCEW(IDI_VOL), IMAGE_ICON, iSize, iSize, 0),
        (HICON)LoadImageW(g_hInst, MAKEINTRESOURCEW(IDI_VOLSLIENT), IMAGE_ICON, iSize, iSize, 0),
        (HICON)LoadImageW(g_hInst, MAKEINTRESOURCEW(IDI_TEMPO), IMAGE_ICON, iSize, iSize, 0)
    };
}