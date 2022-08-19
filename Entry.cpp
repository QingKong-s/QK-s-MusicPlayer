/*
* Entry.cpp
* Ӧ�ó�����ڵ㣬����ȫ�����ã�ִ��ȫ�ֳ�ʼ��
*/
// ʹ��6.0��ͨ������⣬�ϰ��̫�ѿ��̶�����
#pragma comment(linker,"\"/manifestdependency:type='win32' \
name='Microsoft.Windows.Common-Controls' version='6.0.0.0' \
processorArchitecture='*' publicKeyToken='6595b64144ccf1df' language='*'\"")
// ���뾲̬��
#pragma comment(lib,"Shlwapi.lib")
#pragma comment(lib,"UxTheme.lib")
#pragma comment(lib,"Comctl32.lib")
#pragma comment(lib,"Msimg32.lib")
#pragma comment(lib,"gdiplus.lib")
#pragma comment(lib,"D2d1.lib")
#pragma comment(lib,"dwrite.lib")
#pragma comment(lib,"dwmapi.lib")

#pragma comment(lib,"bass.lib")

#include <Windows.h>
#include <WinUser.h>
#include <d2d1.h>
#include <dwrite.h>

#include "MyProject.h"
#include "GlobalVar.h"
#include "QKCtrl.h"
#include "WndLrc.h"
#include "WndMain.h"
#include "WndList.h"
#include "resource.h"

/*
 * Ŀ�꣺
 *
 * ������
 *
 * ����ֵ��
 * ����������
 * ��ע��
 */

 /*
  * Ŀ�꣺��ڵ�
  *
  * ������
  * hInstance ʵ�����
  * hPrevInstance ��ǰʵ���������ΪNULL��
  * lpCmdLine ������
  * nCmdShow ��ʾ��ʽ
  *
  * ����ֵ��
  * ����������
  * ��ע��
  *
  */
int APIENTRY wWinMain(_In_ HINSTANCE hInstance, _In_opt_ HINSTANCE hPrevInstance, _In_ LPWSTR lpCmdLine, _In_ int nCmdShow)
{
	BOOL bSuccessful = TRUE;
    g_hInst = hInstance;
    HMODULE hLib = LoadLibraryW(L"User32.dll");
    if (!hLib)
    {
        Global_ShowError(L"User32.dll����ʧ��", L"��ȷ����ĵ��Ի�������", ECODESRC_WINSDK);
        return 1;
	}
	pGetDpiForSystem = (pFuncGetDpiForSystem)GetProcAddress(hLib, "GetDpiForSystem");
	pGetDpiForWindow = (pFuncGetDpiForWindow)GetProcAddress(hLib, "GetDpiForWindow");
	//////////////��ʼ��COM��OLE
	HRESULT hr;
	hr = CoInitialize(NULL);
	if (FAILED(hr))
	{
		Global_ShowError(L"COM���ʼ��ʧ��", NULL, ECODESRC_OTHERS, NULL, hr);
		return 1;
	}
	hr = OleInitialize(NULL);
	if (FAILED(hr))
	{
		Global_ShowError(L"OLE���ʼ��ʧ��", NULL, ECODESRC_OTHERS, NULL, hr);
		return 1;
	}
    //////////////����GDI+
    GdiplusStartupInput gpsi = { 0 };
    gpsi.GdiplusVersion = 1;
	ULONG_PTR uGPToken;
    GpStatus GPRet;
    GPRet = GdiplusStartup(&uGPToken, &gpsi, NULL);
	if (GPRet != Ok)
    {
		Global_ShowError(L"GDI+����ʧ��", NULL, ECODESRC_OTHERS, NULL, GPRet);
        return 1;
    }
    //////////////������Դ�����ȫ��������
    if (pGetDpiForSystem)
        Res_Load(pGetDpiForSystem() * 16 / 96);
    else
        Res_Load(16);
    GC =
	{
		CreateSolidBrush(QKCOLOR_CYANDEEPER),
		CreateSolidBrush(0xDA9E46),
		0
	};
	//////////////��������
	g_hFont = QKCreateFont(L"΢���ź�", 9);
	g_hFontDrawing = QKCreateFont(L"΢���ź�", 13);
	g_hFontCenterLrc = QKCreateFont(L"΢���ź�", 15, 700);

    D2D1CreateFactory(D2D1_FACTORY_TYPE_SINGLE_THREADED, &g_pD2DFactory);
    DWriteCreateFactory(DWRITE_FACTORY_TYPE_SHARED, __uuidof(IDWriteFactory), reinterpret_cast<IUnknown**>(&g_pDWFactory));

	//////////////��������Ŀ¼
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
    //////////////ע����������ť������Ϣ
    WM_TASKBARBUTTONCREATED = RegisterWindowMessageW(L"TaskbarButtonCreated");
    g_uMyClipBoardFmt = RegisterClipboardFormatW(CLIPBOARDFMT_MYDRAGDROP);
    //////////////ע�ᴰ����
    bSuccessful = QKCtrlInit() && bSuccessful;
    bSuccessful = LrcWnd_Init() && bSuccessful;
    WNDCLASSEXW wcex = { 0 };
    wcex.cbSize = sizeof(WNDCLASSEX);
    wcex.style = CS_HREDRAW | CS_VREDRAW;
    wcex.hInstance = hInstance;
    wcex.hIcon = (HICON)LoadImageW(hInstance, MAKEINTRESOURCEW(IDI_MAIN), IMAGE_ICON, 0, 0, LR_SHARED);
    wcex.hCursor = LoadCursorW(NULL, IDC_ARROW);
    wcex.hbrBackground = (HBRUSH)GetStockObject(WHITE_BRUSH);
    /////////ע��������
    wcex.lpfnWndProc = WndProc;
    wcex.lpszClassName = MAINWNDCLASS;
    bSuccessful = RegisterClassExW(&wcex) && bSuccessful;
    /////////ע�����������飨ע��Ҫ��������ͼ�걣��һ�£�
    wcex.hbrBackground = NULL;
    wcex.lpszClassName = TBGHOSTWNDCLASS;
    wcex.lpfnWndProc = WndProc_TBGhost;
    bSuccessful = RegisterClassExW(&wcex) && bSuccessful;
    /////////ע���б�����������ͼ�꣩
    wcex.lpszClassName = WNDCLASS_LIST;
    wcex.lpfnWndProc = WndProc_PlayList;
    bSuccessful = RegisterClassExW(&wcex) && bSuccessful;
    /////////ע��ͨ�ñ���
	wcex.hIcon = NULL;
	wcex.lpfnWndProc = (WNDPROC)GetProcAddress(hLib, "DefWindowProcW");
	wcex.lpszClassName = BKWNDCLASS;
	bSuccessful = RegisterClassExW(&wcex) && bSuccessful;
	if (!bSuccessful)
	{
		Global_ShowError(L"������ע��ʧ��", NULL);
		return 1;
	}
	//////////////��������
	if (!BASS_Init(-1, 44100, 0, g_hMainWnd, NULL))// ��ʼ��Bass
		Global_ShowError(L"Bass��ʼ��ʧ��", L"�Ժ��볢�Ը�������豸", ECODESRC_BASS);
	if (pGetDpiForSystem)
		g_hMainWnd = CreateWindowExW(0, MAINWNDCLASS, L"δ���� - ��յ����ֲ�����", WS_OVERLAPPEDWINDOW | WS_CLIPCHILDREN,
			CW_USEDEFAULT, 0, pGetDpiForSystem() * 1000 / 96, pGetDpiForSystem() * 640 / 96, NULL, NULL, hInstance, NULL);
	else
		g_hMainWnd = CreateWindowExW(0, MAINWNDCLASS, L"δ���� - ��յ����ֲ�����", WS_OVERLAPPEDWINDOW | WS_CLIPCHILDREN,
			CW_USEDEFAULT, 0, 1000, 640, NULL, NULL, hInstance, NULL);

    if (!g_hMainWnd)
    {
        Global_ShowError(L"����������ʧ��", NULL, ECODESRC_WINSDK);
        return 1;
    }

    ShowWindow(g_hMainWnd, nCmdShow);
    UpdateWindow(g_hMainWnd);
    //////////////��Ϣѭ��
    MSG msg;
    while (GetMessageW(&msg, NULL, 0, 0))
    {
        TranslateMessage(&msg);
        DispatchMessageW(&msg);
    }
    //////////////����ȫ����Դ
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
            lstrcatW(psz, L"\n������룺0x");
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
        lstrcpyW(psz, L"������룺0x");
        wsprintfW(psz + lstrlenW(psz), L"%08x", dwErrCode);
    }

	QKMessageBox(pszTitle, psz, (HICON)TD_ERROR_ICON, L"����", hParent);
    delete[] psz;
}
ULONG_PTR BASS_OpenMusic(PWSTR pszFile, DWORD dwFlagsHS, DWORD dwFlagsHM)
{
    ULONG_PTR h;
    h = BASS_StreamCreateFile(FALSE, pszFile, 0, 0, BASS_UNICODE | dwFlagsHS);
    g_bHMUSIC = FALSE;
    if (!h && BASS_ErrorGetCode() == BASS_ERROR_FILEFORM)
    {
        h = BASS_MusicLoad(FALSE, pszFile, 0, 0, BASS_UNICODE | dwFlagsHM, 0);
        g_bHMUSIC = TRUE;
    }

    return h;
}
BOOL BASS_FreeMusic(ULONG_PTR h)
{
    if (g_bHMUSIC)
        return BASS_MusicFree(h);
    else
        return BASS_StreamFree(h);
}
void UI_UpdateDPISize()
{
    GC.DS_CYPROGBAR = DPI(SIZE_CYPROGBAR);
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
        (HICON)LoadImageW(g_hInst, MAKEINTRESOURCEW(IDI_LOCATE), IMAGE_ICON, iSize, 0, 0),
        (HICON)LoadImageW(g_hInst, MAKEINTRESOURCEW(IDI_PLUS), IMAGE_ICON, iSize, 0, 0),
        (HICON)LoadImageW(g_hInst, MAKEINTRESOURCEW(IDI_READFILE), IMAGE_ICON, iSize, 0, 0),
        (HICON)LoadImageW(g_hInst, MAKEINTRESOURCEW(IDI_DISK), IMAGE_ICON, iSize, 0, 0),
        (HICON)LoadImageW(g_hInst, MAKEINTRESOURCEW(IDI_CROSS), IMAGE_ICON, iSize, 0, 0),
        (HICON)LoadImageW(g_hInst, MAKEINTRESOURCEW(IDI_SEARCH), IMAGE_ICON, iSize, 0, 0),
        (HICON)LoadImageW(g_hInst, MAKEINTRESOURCEW(IDI_THREETRACKBARS), IMAGE_ICON, iSize, 0, 0),
        (HICON)LoadImageW(g_hInst, MAKEINTRESOURCEW(IDI_PLAYLIST), IMAGE_ICON, iSize, 0, 0),
        (HICON)LoadImageW(g_hInst, MAKEINTRESOURCEW(IDI_ARROWCROSS), IMAGE_ICON, iSize, 0, 0),
        (HICON)LoadImageW(g_hInst, MAKEINTRESOURCEW(IDI_ARROWRIGHT), IMAGE_ICON, iSize, 0, 0),
        (HICON)LoadImageW(g_hInst, MAKEINTRESOURCEW(IDI_ARROWRIGHTTHREE), IMAGE_ICON, iSize, 0, 0),
        (HICON)LoadImageW(g_hInst, MAKEINTRESOURCEW(IDI_ARROWCIRCLEONE), IMAGE_ICON, iSize, 0, 0),
        (HICON)LoadImageW(g_hInst, MAKEINTRESOURCEW(IDI_ARROWCIRCLE), IMAGE_ICON, iSize, 0, 0),
        (HICON)LoadImageW(g_hInst, MAKEINTRESOURCEW(IDI_LAST), IMAGE_ICON, iSize, 0, 0),
        (HICON)LoadImageW(g_hInst, MAKEINTRESOURCEW(IDI_PLAY), IMAGE_ICON, iSize, 0, 0),
        (HICON)LoadImageW(g_hInst, MAKEINTRESOURCEW(IDI_PAUSE), IMAGE_ICON, iSize, 0, 0),
        (HICON)LoadImageW(g_hInst, MAKEINTRESOURCEW(IDI_STOP), IMAGE_ICON, iSize, 0, 0),
        (HICON)LoadImageW(g_hInst, MAKEINTRESOURCEW(IDI_NEXT), IMAGE_ICON, iSize, 0, 0),
        (HICON)LoadImageW(g_hInst, MAKEINTRESOURCEW(IDI_LRC), IMAGE_ICON, iSize, 0, 0),
        (HICON)LoadImageW(g_hInst, MAKEINTRESOURCEW(IDI_SETTINGS), IMAGE_ICON, iSize, 0, 0),
        (HICON)LoadImageW(g_hInst, MAKEINTRESOURCEW(IDI_INFO), IMAGE_ICON, iSize, 0, 0),
        (HICON)LoadImageW(g_hInst, MAKEINTRESOURCEW(IDI_PLAYLISTMANAGE), IMAGE_ICON, iSize, 0, 0),
        (HICON)LoadImageW(g_hInst, MAKEINTRESOURCEW(IDI_LAST2), IMAGE_ICON, iSize, 0, 0),
        (HICON)LoadImageW(g_hInst, MAKEINTRESOURCEW(IDI_PLAY2), IMAGE_ICON, iSize, 0, 0),
        (HICON)LoadImageW(g_hInst, MAKEINTRESOURCEW(IDI_PAUSE2), IMAGE_ICON, iSize, 0, 0),
        (HICON)LoadImageW(g_hInst, MAKEINTRESOURCEW(IDI_NEXT2), IMAGE_ICON, iSize, 0, 0),
        (HICON)LoadImageW(g_hInst, MAKEINTRESOURCEW(IDI_CROSS2), IMAGE_ICON, iSize, 0, 0),
        (HICON)LoadImageW(g_hInst, MAKEINTRESOURCEW(IDI_TICK), IMAGE_ICON, iSize, 0, 0)
    };
}