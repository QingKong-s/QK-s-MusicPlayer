#include "WndMain.h"

#include <Windows.h>
#include <commdlg.h>
#include <shlwapi.h>
#include <CommCtrl.h>
#include <vsstyle.h>
#include <Shobjidl.h>
#include <shlobj_core.h>
#include <windowsx.h>
#include <UxTheme.h>
#include <dwmapi.h>
#include <d2d1.h>
#include <d2d1_1.h>
#include <d2d1effects.h>
#include <malloc.h>

#include <math.h>
#include <stdio.h>

#include "bass.h"
#include "bass_fx.h"

#include "GlobalVar.h"
#include "Function.h"
#include "Resource.h"
#include "MyProject.h"
#include "QKCtrl.h"
#include "WndLrc.h"
#include "OLEDragDrop.h"
#include "WndEffect.h"
#include "WndList.h"

CURRMUSICINFO                   m_CurrSongInfo = { 0 };        //当前信息（在顶部显示）

ID2D1DCRenderTarget*            m_pD2DRTLeftBK              = NULL;
ID2D1DCRenderTarget*            m_pD2DRTLeftBK2             = NULL;

ID2D1DeviceContext*             m_pD2DDCLeftBK              = NULL;
ID2D1GdiInteropRenderTarget*    m_pD2DGdiInteropRTLeftBK    = NULL;
ID2D1Bitmap1*                   m_pD2DBmpLeftBK             = NULL;
ID2D1Bitmap1*                   m_pD2DBmpLeftBK2            = NULL;

IDXGISurface1*                  m_pDXGISfceLeftBK           = NULL;
IDXGISwapChain1*                m_pDXGIScLeftBK             = NULL;

ID2D1SolidColorBrush*           m_pD2DBrMyBlue              = NULL;
ID2D1SolidColorBrush*           m_pD2DBrMyBlue2             = NULL;

DWORD           m_uThreadFlagWaves          = THREADFLAG_STOP;  // 线程工作状态标志
HANDLE          m_htdWaves                  = NULL;         // 线程句柄
DWORD*          m_dwWavesData               = NULL;         // 指向波形信息数组，左声道：低WORD，右声道：高WORD
DWORD           m_dwWavesDataCount          = 0;            // 波形计数

BOOL            m_IsDraw[3]                 = { 0 };        // 绘制标志

BOOL            m_bLrcShow                  = TRUE;

RECT            m_rcLrcShow                 = { 0 };// 滚动歌词区
RECT            m_rcBtmBK                   = { 0 };// 底部按钮区
RECT            m_rcWaves                   = { 0 };// 波形区
RECT            m_rcSpe                     = { 0 };// 频谱区
RECT            m_rcAlbum                   = { 0 };// 旋转封面区

D2D_RECT_F      m_D2DRcLrcShow              = { 0 };// 滚动歌词区
D2D_RECT_F      m_D2DRcBtmBK                = { 0 };// 底部按钮区
D2D_RECT_F      m_D2DRcWaves                = { 0 };// 波形区
D2D_RECT_F      m_D2DRcSpe                  = { 0 };// 频谱区
D2D_RECT_F      m_D2DRcAlbum                = { 0 };// 旋转封面区

int             m_cxLeftBK                  = 0,
                m_cyLeftBK                  = 0,
                m_cxLrcShow                 = 0,
                m_cyLrcShow                 = 0,
                m_cyAlbum                   = 470;

int             m_iLrcSBPos                 = -1;
BOOL            m_bLockLrcScroll            = FALSE;

int             m_iLrcCenter                = -1;
int             m_iLrcMouseHover            = -1;
int             m_iLrcFixedIndex            = -1;

LRCHSCROLLINFO  m_LrcHScrollInfo            = { -1 };// 歌词水平滚动信息；仅适用于当前播放行
LRCVSCROLLINFO  m_LrcVScrollInfo            = { 0 };

int             m_iDrawingID                = 0;

int             m_iLastLrcIndex[2]          = { -1,-1 };// 0：上次中心；1：上次高亮

HDC             m_hdcLeftBK                 = NULL;

BOOL            m_bWndMinimized             = FALSE;

float           m_fRotationAngle            = 0.f;

HSTREAM m_hs;

HRESULT hr = S_OK;

void Settings_Read()
{
    WCHAR szBuffer[MAXPROFILEBUFFER];
    UINT u;
    GS.uDefTextCode = GetPrivateProfileIntW(PPF_SECTIONLRC, PPF_KEY_DEFTEXTCODE, 0, g_pszProfie);

    GetPrivateProfileStringW(PPF_SECTIONLRC, PPF_KEY_LRCDIR, NULL, szBuffer, MAXPROFILEBUFFER, g_pszProfie);
    delete[] GS.pszLrcDir;
    GS.pszLrcDir = new WCHAR[lstrlenW(szBuffer) + 1];
    lstrcpyW(GS.pszLrcDir, szBuffer);

    GS.bLrcAnimation = !GetPrivateProfileIntW(PPF_SECTIONLRC, PPF_KEY_DISABLEVANIMATION, FALSE, g_pszProfie);
    GS.bForceTwoLines = GetPrivateProfileIntW(PPF_SECTIONLRC, PPF_KEY_DISABLEWORDBREAK, TRUE, g_pszProfie);
    GS.bDTLrcShandow = !GetPrivateProfileIntW(PPF_SECTIONLRC, PPF_KEY_DISABLEDTLRCSHANDOW, FALSE, g_pszProfie);

    GetPrivateProfileStringW(PPF_SECTIONLRC, PPF_KEY_FONTNAME, L"微软雅黑", szBuffer, MAXPROFILEBUFFER, g_pszProfie);
    delete[] GS.pszDTLrcFontName;
    GS.pszDTLrcFontName = new WCHAR[lstrlenW(szBuffer) + 1];
    lstrcpyW(GS.pszDTLrcFontName, szBuffer);

    GS.uDTLrcFontSize = GetPrivateProfileIntW(PPF_SECTIONLRC, PPF_KEY_FONTSIZE, 40, g_pszProfie);
    GS.uDTLrcFontWeight = GetPrivateProfileIntW(PPF_SECTIONLRC, PPF_KEY_FONTWEIGHT, 400, g_pszProfie);

    GS.crDTLrc1 = GetPrivateProfileIntW(PPF_SECTIONLRC, PPF_KEY_DTLRCCLR1, 0x00FF00, g_pszProfie);
    GS.crDTLrc2 = GetPrivateProfileIntW(PPF_SECTIONLRC, PPF_KEY_DTLRCCLR2, 0x0000FF, g_pszProfie);

    GS.uDTLrcTransparent = GetPrivateProfileIntW(PPF_SECTIONLRC, PPF_KEY_DTLRCTRANSPARENT, 0xFF, g_pszProfie);

    GetPrivateProfileStringW(PPF_SECTIONLRC, PPF_KEY_DTLRCSPACELINE, L"♬♪♬♪♬", szBuffer, MAXPROFILEBUFFER, g_pszProfie);
    delete[] GS.pszDTLrcSpaceLine;
    GS.pszDTLrcSpaceLine = new WCHAR[lstrlenW(szBuffer) + 1];
    lstrcpyW(GS.pszDTLrcSpaceLine, szBuffer);
}
void Settings_Save()
{
    WCHAR sz[MAXPROFILEBUFFER];
    wsprintfW(sz, L"%u", GS.uDefTextCode);
    WritePrivateProfileStringW(PPF_SECTIONLRC, PPF_KEY_DEFTEXTCODE, sz, g_pszProfie);

    WritePrivateProfileStringW(PPF_SECTIONLRC, PPF_KEY_LRCDIR, GS.pszLrcDir, g_pszProfie);

    wsprintfW(sz, L"%u", !GS.bLrcAnimation);
    WritePrivateProfileStringW(PPF_SECTIONLRC, PPF_KEY_DISABLEVANIMATION, sz, g_pszProfie);

    wsprintfW(sz, L"%u", GS.bForceTwoLines);
    WritePrivateProfileStringW(PPF_SECTIONLRC, PPF_KEY_DISABLEWORDBREAK, sz, g_pszProfie);

    wsprintfW(sz, L"%u", !GS.bDTLrcShandow);
    WritePrivateProfileStringW(PPF_SECTIONLRC, PPF_KEY_DISABLEDTLRCSHANDOW, sz, g_pszProfie);

    WritePrivateProfileStringW(PPF_SECTIONLRC, PPF_KEY_FONTNAME, GS.pszDTLrcFontName, g_pszProfie);

    wsprintfW(sz, L"%u", GS.uDTLrcFontSize);
    WritePrivateProfileStringW(PPF_SECTIONLRC, PPF_KEY_FONTSIZE, sz, g_pszProfie);

    wsprintfW(sz, L"%u", GS.uDTLrcFontWeight);
    WritePrivateProfileStringW(PPF_SECTIONLRC, PPF_KEY_FONTWEIGHT, sz, g_pszProfie);

    wsprintfW(sz, L"%u", GS.crDTLrc1);
    WritePrivateProfileStringW(PPF_SECTIONLRC, PPF_KEY_DTLRCCLR1, sz, g_pszProfie);
    wsprintfW(sz, L"%u", GS.crDTLrc2);
    WritePrivateProfileStringW(PPF_SECTIONLRC, PPF_KEY_DTLRCCLR2, sz, g_pszProfie);

    wsprintfW(sz, L"%u", GS.uDTLrcTransparent);
    WritePrivateProfileStringW(PPF_SECTIONLRC, PPF_KEY_DTLRCTRANSPARENT, sz, g_pszProfie);

    WritePrivateProfileStringW(PPF_SECTIONLRC, PPF_KEY_DTLRCSPACELINE, GS.pszDTLrcSpaceLine, g_pszProfie);
}
INT_PTR CALLBACK DlgProc_Settings2(HWND hDlg, UINT message, WPARAM wParam, LPARAM lParam)
{
    static HBRUSH hbr1, hbr2;
    static HWND hST1, hST2;
    switch (message)
    {
    case WM_INITDIALOG:
    {
        hST1 = GetDlgItem(hDlg, IDC_ST_DTLRCCLR1);
        hST2 = GetDlgItem(hDlg, IDC_ST_DTLRCCLR2);

        HWND hCtrl = GetDlgItem(hDlg, IDC_CB_DEFTEXTCODE);
        SendMessageW(hCtrl, CB_INSERTSTRING, -1, (LPARAM)L"自动判断（不一定准确）");
        SendMessageW(hCtrl, CB_INSERTSTRING, -1, (LPARAM)L"GBK");
        SendMessageW(hCtrl, CB_INSERTSTRING, -1, (LPARAM)L"UTF-8");
        SendMessageW(hCtrl, CB_INSERTSTRING, -1, (LPARAM)L"UTF-16LE");
        SendMessageW(hCtrl, CB_INSERTSTRING, -1, (LPARAM)L"UTF-16BE");
        SendMessageW(hCtrl, CB_SETCURSEL, GS.uDefTextCode, 0);

        SetDlgItemTextW(hDlg, IDC_ED_LRCDIR, GS.pszLrcDir);

        SendDlgItemMessageW(hDlg, IDC_CB_DISABLEVANIMATION, BM_SETCHECK, GS.bLrcAnimation ? BST_UNCHECKED : BST_CHECKED, 0);
        SendDlgItemMessageW(hDlg, IDC_CB_DISABLEWORDBREAK, BM_SETCHECK, GS.bForceTwoLines ? BST_CHECKED : BST_UNCHECKED, 0);
        SendDlgItemMessageW(hDlg, IDC_CB_DISABLEDTLRCSHANDOW, BM_SETCHECK, GS.bDTLrcShandow ? BST_UNCHECKED : BST_CHECKED, 0);

        SetDlgItemTextW(hDlg, IDC_ED_DTLRCFONTINFO, GS.pszDTLrcFontName);

        SendDlgItemMessageW(hDlg, IDC_ED_DTLRCFONTINFO, EM_SETSEL, -2, -1);
        SendDlgItemMessageW(hDlg, IDC_ED_DTLRCFONTINFO, EM_REPLACESEL, FALSE, (LPARAM)L",");

        WCHAR sz[10];
        wsprintfW(sz, L"%u", GS.uDTLrcFontSize);
        SetPropW(GetDlgItem(hDlg, IDC_ED_DTLRCFONTINFO), PROP_DTLRCFONTSIZE, (HANDLE)GS.uDTLrcFontSize);
        SendDlgItemMessageW(hDlg, IDC_ED_DTLRCFONTINFO, EM_SETSEL, -2, -1);
        SendDlgItemMessageW(hDlg, IDC_ED_DTLRCFONTINFO, EM_REPLACESEL, FALSE, (LPARAM)sz);

        SendDlgItemMessageW(hDlg, IDC_ED_DTLRCFONTINFO, EM_SETSEL, -2, -1);
        SendDlgItemMessageW(hDlg, IDC_ED_DTLRCFONTINFO, EM_REPLACESEL, FALSE, (LPARAM)L",");

        wsprintfW(sz, L"%u", GS.uDTLrcFontWeight);
        SetPropW(GetDlgItem(hDlg, IDC_ED_DTLRCFONTINFO), PROP_DTLRCFONTWEIGHT, (HANDLE)GS.uDTLrcFontWeight);
        SendDlgItemMessageW(hDlg, IDC_ED_DTLRCFONTINFO, EM_SETSEL, -2, -1);
        SendDlgItemMessageW(hDlg, IDC_ED_DTLRCFONTINFO, EM_REPLACESEL, FALSE, (LPARAM)sz);

        hbr1 = CreateSolidBrush(QKCommonClrToGDIClr(GS.crDTLrc1));
        hbr2 = CreateSolidBrush(QKCommonClrToGDIClr(GS.crDTLrc2));
        SetPropW(GetDlgItem(hDlg, IDC_ST_DTLRCCLR1), PROP_DTLRCCLR1, (HANDLE)GS.crDTLrc1);
        SetPropW(GetDlgItem(hDlg, IDC_ST_DTLRCCLR2), PROP_DTLRCCLR2, (HANDLE)GS.crDTLrc2);

        SendDlgItemMessageW(hDlg, IDC_TB_DTLRCTRANSPARENT, TBM_SETRANGE, FALSE, MAKELPARAM(0, 0xFF));
        SendDlgItemMessageW(hDlg, IDC_TB_DTLRCTRANSPARENT, TBM_SETPOS, TRUE, GS.uDTLrcTransparent);

        SetDlgItemTextW(hDlg, IDC_ED_DTLRCSPACELINE, GS.pszDTLrcSpaceLine);

        DlgProc_Settings2(hDlg, WM_COMMAND, MAKEWPARAM(IDC_BT_REFRESHDEV, BN_CLICKED), 0);
    }
    return FALSE;
    case WM_NCPAINT:
        return TRUE;
    case WM_COMMAND:
    {
        switch (HIWORD(wParam))
        {
        case CBN_SELCHANGE:
        {
            BASS_Init(SendMessageW((HWND)lParam, CB_GETCURSEL, 0, 0) + 1, 44100, BASS_DEVICE_REINIT, g_hMainWnd, NULL);
        }
        case BN_CLICKED:
        {
            switch (LOWORD(wParam))
            {
            case IDC_BT_CHANGELRCDIR:
            {
                IFileOpenDialog* pfod;
                HRESULT hr = CoCreateInstance(CLSID_FileOpenDialog,
                    NULL,
                    CLSCTX_INPROC_SERVER,
                    IID_PPV_ARGS(&pfod));
                if (!SUCCEEDED(hr))
                    return 0;
                pfod->SetOptions(FOS_PICKFOLDERS | FOS_FORCEFILESYSTEM | FOS_PATHMUSTEXIST);
                pfod->Show(hDlg);
                IShellItem* psi;
                hr = pfod->GetResult(&psi);
                if (!SUCCEEDED(hr))
                {
                    pfod->Release();
                    return 0;
                }
                LPWSTR pszPath;
                hr = psi->GetDisplayName(SIGDN_DESKTOPABSOLUTEPARSING, &pszPath);
                psi->Release();
                pfod->Release();
                if (!SUCCEEDED(hr))
                    return 0;
                SetDlgItemTextW(hDlg, IDC_ED_LRCDIR, pszPath);
                CoTaskMemFree(pszPath);
            }
            return TRUE;
            case IDC_BT_CHANGEFONT:
            {
                LOGFONTW lf = { 0 };
                CHOOSEFONTW cf = { 0 };
                cf.lStructSize = sizeof(CHOOSEFONTW);
                cf.hwndOwner = hDlg;
                cf.lpLogFont = &lf;
                cf.Flags = CF_INITTOLOGFONTSTRUCT | CF_SCREENFONTS | CF_EFFECTS | CF_FORCEFONTEXIST | CF_NOVERTFONTS;

            }
            return TRUE;
            case IDC_BT_REFRESHDEV:
            {
                BASS_DEVICEINFO DevInfo;
                PWSTR psz;
                DWORD dwCurrDev = BASS_GetDevice();
                for (DWORD i = 1; BASS_GetDeviceInfo(i, &DevInfo); ++i)
                {
                    int iBufferSize = MultiByteToWideChar(CP_ACP, 0, DevInfo.name, -1, NULL, 0);
                    if (iBufferSize)
                    {
                        psz = new WCHAR[iBufferSize];//包含结尾NULL
                        MultiByteToWideChar(CP_ACP, 0, DevInfo.name, -1, psz, iBufferSize);
                    }
                    else
                        psz = NULL;

                    SendDlgItemMessageW(hDlg, IDC_CB_DEVICES, CB_INSERTSTRING, -1, (LPARAM)psz);
                    delete[] psz;
                }

                SendDlgItemMessageW(hDlg, IDC_CB_DEVICES, CB_SETCURSEL, BASS_GetDevice() - 1, 0);
            }
            return TRUE;
            }
        }
        }
    }
    return TRUE;
    case WM_CTLCOLORSTATIC:
    {
        if (lParam == (LPARAM)hST1)
            return (LRESULT)hbr1;
        else if (lParam == (LPARAM)hST2)
            return (LRESULT)hbr2;
    }
    return NULL;
    }
    return FALSE;
}
INT_PTR CALLBACK DlgProc_Settings(HWND hDlg, UINT message, WPARAM wParam, LPARAM lParam)
{
    static HWND hWnd;
    switch (message)
    {
    case WM_INITDIALOG:
    {
        int iGap = DPI(50);
        int cxClient;
        int cyClient;
        RECT rc;
        GetClientRect(hDlg, &rc);
        cxClient = rc.right;
        cyClient = rc.bottom;

        hWnd = CreateDialogParamW(g_hInst, MAKEINTRESOURCEW(IDD_OPTIONS2), NULL, DlgProc_Settings2, 0);
        GetClientRect(hWnd, &rc);
        SetParent(hWnd, hDlg);
        SetWindowPos(hWnd, NULL, 0, 0, rc.right, cyClient - iGap, SWP_NOZORDER);
        SetWindowLongPtrW(hWnd, GWL_STYLE, WS_CHILD);
        ShowWindow(hWnd, SW_SHOW);

        HWND hCtrl = GetDlgItem(hDlg, IDC_SB_SETTINGS);
        SetWindowPos(hCtrl, NULL, rc.right, 0, cxClient - rc.right, cyClient - iGap, SWP_NOZORDER);
        SCROLLINFO si;
        si.cbSize = sizeof(SCROLLINFO);
        si.fMask = SIF_ALL;
        si.nPos = 0;
        si.nMax = rc.bottom;
        si.nMin = 0;
        si.nPage = cyClient - iGap;
        SetScrollInfo(hCtrl, SB_CTL, &si, TRUE);
    }
    return FALSE;
    case WM_COMMAND:
    {
        switch (LOWORD(wParam))
        {
        case IDOK:
        {
            GS.uDefTextCode = SendDlgItemMessageW(hWnd, IDC_CB_DEFTEXTCODE, CB_GETCURSEL, 0, 0);

            delete[] GS.pszLrcDir;
            int iLength = GetWindowTextLengthW(GetDlgItem(hWnd, IDC_ED_LRCDIR));
            GS.pszLrcDir = new WCHAR[iLength + 1];
            GetDlgItemTextW(hWnd, IDC_ED_LRCDIR, GS.pszLrcDir, iLength + 1);

            GS.bLrcAnimation = !(SendDlgItemMessageW(hWnd, IDC_CB_DISABLEVANIMATION, BM_GETCHECK, 0, 0) == BST_CHECKED);
            GS.bForceTwoLines = (SendDlgItemMessageW(hWnd, IDC_CB_DISABLEWORDBREAK, BM_GETCHECK, 0, 0) == BST_CHECKED);
            GS.bDTLrcShandow = !(SendDlgItemMessageW(hWnd, IDC_CB_DISABLEDTLRCSHANDOW, BM_GETCHECK, 0, 0) == BST_CHECKED);

            delete[] GS.pszDTLrcFontName;
            iLength = GetWindowTextLengthW(GetDlgItem(hWnd, IDC_ED_FONTINFO));
            GS.pszDTLrcFontName = new WCHAR[iLength + 1];
            GetDlgItemTextW(hWnd, IDC_ED_FONTINFO, GS.pszDTLrcFontName, iLength + 1);

            int iPos = QKStrInStr(GS.pszDTLrcFontName, L",");
            *(GS.pszDTLrcFontName + iPos - 1) = L'\0';
            GS.uDTLrcFontSize = (UINT)GetPropW(GetDlgItem(hWnd, IDC_ED_DTLRCFONTINFO), PROP_DTLRCFONTSIZE);
            GS.uDTLrcFontWeight = (UINT)GetPropW(GetDlgItem(hWnd, IDC_ED_DTLRCFONTINFO), PROP_DTLRCFONTWEIGHT);

            GS.crDTLrc1 = (COLORREF)GetPropW(GetDlgItem(hWnd, IDC_ST_DTLRCCLR1), PROP_DTLRCCLR1);
            GS.crDTLrc2 = (COLORREF)GetPropW(GetDlgItem(hWnd, IDC_ST_DTLRCCLR2), PROP_DTLRCCLR2);

            GS.uDTLrcTransparent = SendDlgItemMessageW(hWnd, IDC_TB_DTLRCTRANSPARENT, TBM_GETPOS, 0, 0);
            delete[] GS.pszDTLrcSpaceLine;
            iLength = GetWindowTextLengthW(GetDlgItem(hWnd, IDC_ED_DTLRCSPACELINE));
            GS.pszDTLrcSpaceLine = new WCHAR[iLength + 1];
            GetDlgItemTextW(hWnd, IDC_ED_DTLRCSPACELINE, GS.pszDTLrcSpaceLine, iLength + 1);

            Settings_Save();
            EndDialog(hDlg, 0);
        }
        return TRUE;
        case IDCANCEL:
            EndDialog(hDlg, 0);
            return TRUE;
        }

    }
    return TRUE;
    case WM_MOUSEWHEEL:
    {
        int iDistance = -GET_WHEEL_DELTA_WPARAM(wParam) / WHEEL_DELTA;

        int iPos;
        SCROLLINFO si;
        si.cbSize = sizeof(SCROLLINFO);
        si.fMask = SIF_ALL;
        GetScrollInfo(GetDlgItem(hDlg, IDC_SB_SETTINGS), SB_CTL, &si);
        iPos = si.nPos;
        si.nPos += (si.nPage / 3 * iDistance);
        if (si.nPos < 0)
            si.nPos = 0;
        else if (si.nPos > si.nMax - si.nPage)
            si.nPos = si.nMax - si.nPage;

        SetScrollPos(GetDlgItem(hDlg, IDC_SB_SETTINGS), SB_CTL, si.nPos, TRUE);
        ScrollWindow(hWnd, 0, iPos - si.nPos, NULL, NULL);
    }
    return TRUE;
    case WM_VSCROLL:
    {
        int iPos;
        SCROLLINFO si;
        si.cbSize = sizeof(SCROLLINFO);
        si.fMask = SIF_ALL;
        GetScrollInfo((HWND)lParam, SB_CTL, &si);
        iPos = si.nPos;
        switch (LOWORD(wParam))
        {
        case SB_LEFT:
            si.nPos = 0;
            break;
        case SB_RIGHT:
            si.nPos = si.nMax;
            break;
        case SB_LINELEFT:
            si.nPos--;
            break;
        case SB_LINERIGHT:
            si.nPos++;
            break;
        case SB_PAGELEFT:
            si.nPos -= si.nPage;
            break;
        case SB_PAGERIGHT:
            si.nPos += si.nPage;
            break;
        case SB_THUMBTRACK:
            si.nPos = si.nTrackPos;
            break;
        }

        SetScrollPos((HWND)lParam, SB_CTL, si.nPos, TRUE);
        ScrollWindow(hWnd, 0, iPos - si.nPos, NULL, NULL);
    }
    return TRUE;
    case WM_CLOSE:
        EndDialog(hDlg, 0);
        return TRUE;
    }
    return FALSE;
}


void UI_PreProcessAlbumImage(IWICBitmap** ppWICBitmap)
{
    IWICBitmap* pWICBitmap = *ppWICBitmap;
    if (*ppWICBitmap)
    {
        UINT cx0, cy0, cx, cy;
        pWICBitmap->GetSize(&cx0, &cy0);

        if (max(cx0, cy0) > DPIS_LARGEIMAGE)// 限制图片大小
        {
            if (cx0 >= cy0)// 宽度较大
            {
                cy = (UINT)((float)cy0 / (float)cx0 * DPIS_LARGEIMAGE);
                cx = DPIS_LARGEIMAGE;
            }
            else
            {
                cx = (UINT)((float)cx0 / (float)cy0 * DPIS_LARGEIMAGE);
                cy = DPIS_LARGEIMAGE;
            }

            IWICBitmapScaler* pWICBitmapScaler;
            g_pWICFactory->CreateBitmapScaler(&pWICBitmapScaler);
            
            pWICBitmapScaler->Initialize(pWICBitmap, cx, cy, WICBitmapInterpolationModeCubic);
            pWICBitmap->Release();

            g_pWICFactory->CreateBitmapFromSource(pWICBitmapScaler, WICBitmapNoCache, ppWICBitmap);
            pWICBitmapScaler->Release();
        }
    }
    else
    {
		WICCreateBitmap(g_pszDefPic, ppWICBitmap);
    }
}
void MainWnd_ReleaseCurrInfo()
{
	delete[] m_CurrSongInfo.pszName;
    if (m_CurrSongInfo.pD2DBmpOrgAlbum)
        m_CurrSongInfo.pD2DBmpOrgAlbum->Release();
	MusicInfo_Release(&m_CurrSongInfo.mi);
    ZeroMemory(&m_CurrSongInfo, sizeof(CURRMUSICINFO));
}
void CALLBACK SyncProc_End(HSYNC handle, DWORD channel, DWORD data, void* user)
{
    WndProc_LeftBK(g_hBKLeft, LEFTBKM_SETPROGBARPOS, WndProc_LeftBK(g_hBKLeft, LEFTBKM_GETPROGBARMAX, 0, 0), TRUE);
    Playing_AutoNext();
}
/*
 * 目标：画一行歌词
 *
 * 参数：
 * iIndex 歌词索引
 * y 起始y坐标（顶边或底边），设为-1以使用上次绘画时的顶边，此时bTop参数无效
 * bTop 是否为顶边
 * bClearBK 是否擦除背景
 * bImmdShow 立即显示，若为FALSE，则需另外将后台位图拷贝到前台，设为TRUE，则自动剪辑歌词区域
 *
 * 返回值：返回已绘制的歌词高度
 * 操作简述：
 * 备注：必须保证索引合法；自动处理强制双行设置；自动处理热点和选中项目；擦除背景时仅擦除歌词矩形
 */
int Lrc_DrawItem(int iIndex, int y, BOOL bTop, BOOL bClearBK, BOOL bImmdShow, BOOL bCenterLine, int* yOut)
{
    if (m_cxLrcShow <= 0 || m_cyLrcShow <= 0)
        return -1;
    BOOL bCurr = (iIndex == g_iCurrLrcIndex);

    LRCDATA* p = (LRCDATA*)QKArray_Get(g_Lrc, iIndex);

    IDWriteTextLayout* pDWTextLayout1;
    IDWriteTextLayout* pDWTextLayout2;
    ID2D1SolidColorBrush* pD2DBrush;

    if (bCurr)
        m_pD2DDCLeftBK->CreateSolidColorBrush(D2D1::ColorF(D2D1::ColorF::Red), &pD2DBrush);
    else
    {
        pD2DBrush = m_pD2DBrMyBlue;
        pD2DBrush->AddRef();
    }

    D2D_RECT_F D2DRcF1, D2DRcF2;

    if (y == -1 && !bCenterLine)
    {
        if (p->iDrawID != m_iDrawingID)
            return -1;
        y = p->iLastTop;
        bTop = TRUE;
    }

    int cy1, cy2;
    int cx1, cx2;
    UINT32 uStrLen1, uStrLen2;
    DWRITE_TEXT_METRICS Metrics1, Metrics2;

    if (GS.bForceTwoLines)// 是否禁止换行
    {
        g_pDWTFBig->SetWordWrapping(DWRITE_WORD_WRAPPING_NO_WRAP);// 禁止自动换行
        if (p->iOrgLength == -1)// 只有一行
        {
            g_pDWTFBig->SetTextAlignment(DWRITE_TEXT_ALIGNMENT_LEADING);
            uStrLen1 = lstrlenW(p->pszLrc);

            if (bCurr)
            {
                if (iIndex != m_LrcHScrollInfo.iIndex)
                {
                    g_pDWFactory->CreateTextLayout(p->pszLrc, uStrLen1, g_pDWTFBig, m_cxLrcShow, m_cyLrcShow, &pDWTextLayout1);
					pDWTextLayout1->GetMetrics(&m_LrcHScrollInfo.Metrics1);
                    pDWTextLayout1->Release();

                    cx1 = m_LrcHScrollInfo.Metrics1.width;
                    cy1 = m_LrcHScrollInfo.Metrics1.height;

                    m_LrcHScrollInfo.iIndex = iIndex;
                    if (!m_LrcHScrollInfo.bWndSizeChangedFlag)
                        m_LrcHScrollInfo.x1 = m_LrcHScrollInfo.x2 = 0;
                    else
                        m_LrcHScrollInfo.bWndSizeChangedFlag = FALSE;

                    KillTimer(g_hMainWnd, IDT_ANIMATION);
                    if (cx1 > m_cxLrcShow)
                    {
						m_LrcHScrollInfo.cx1 = cx1;// 超长了，需要后续滚动
						m_LrcHScrollInfo.fNoScrollingTime1 = m_cxLrcShow * p->fDelay / m_LrcHScrollInfo.cx1 / 2;
						SetTimer(g_hMainWnd, IDT_ANIMATION, TIMERELAPSE_ANIMATION, TimerProc);
					}
					else
					{
						m_LrcHScrollInfo.cx1 = -1;
						m_LrcHScrollInfo.x1 = m_LrcHScrollInfo.x2 = 0;
					}
				}
                else// 横向滚动就別测高了
                {
                    cx1 = m_LrcHScrollInfo.Metrics1.width;
                    cy1 = m_LrcHScrollInfo.Metrics1.height;
                }
			}
            else
            {
                g_pDWFactory->CreateTextLayout(p->pszLrc, uStrLen1, g_pDWTFBig, m_cxLrcShow, m_cyLrcShow, &pDWTextLayout1);
                pDWTextLayout1->GetMetrics(&Metrics1);
                pDWTextLayout1->Release();
                cx1 = Metrics1.width;
                cy1 = Metrics1.height;
            }

            if (cx1 > m_cxLrcShow)
                g_pDWTFBig->SetTextAlignment(DWRITE_TEXT_ALIGNMENT_LEADING);
            else
                g_pDWTFBig->SetTextAlignment(DWRITE_TEXT_ALIGNMENT_CENTER);

            p->cy = cy1;

            if (bCenterLine)
            {
                y -= p->cy / 2;
                bTop = TRUE;
                *yOut = y;
            }

            if (bTop)
                D2DRcF2 = { (float)m_rcLrcShow.left,(float)y,(float)m_rcLrcShow.right,(float)(y + cy1) };
            else
                D2DRcF2 = { (float)m_rcLrcShow.left,(float)(y - p->cy),(float)m_rcLrcShow.right,(float)y };
            D2DRcF1 = D2DRcF2;

            
            if (bImmdShow)
            {
                m_pD2DDCLeftBK->BeginDraw();
                if (D2DRcF2.top < m_rcLrcShow.top)
                    D2DRcF2.top = m_rcLrcShow.top;
                if (D2DRcF2.bottom > m_rcLrcShow.bottom)
                    D2DRcF2.bottom = m_rcLrcShow.bottom;
                m_pD2DDCLeftBK->PushAxisAlignedClip(&D2DRcF2, D2D1_ANTIALIAS_MODE_PER_PRIMITIVE);
                m_pD2DDCLeftBK->DrawBitmap(m_pD2DBmpLeftBK2, &D2DRcF1, 1.0f, D2D1_BITMAP_INTERPOLATION_MODE_LINEAR, &D2DRcF1);// 画背景
            }

            if (bCurr)
                D2DRcF1.left += m_LrcHScrollInfo.x1;

            p->rcItem = { (LONG)D2DRcF1.left,(LONG)D2DRcF1.top,(LONG)D2DRcF1.right,(LONG)D2DRcF1.bottom };
            p->iLastTop = D2DRcF1.top;
            m_pD2DDCLeftBK->DrawTextW(p->pszLrc, uStrLen1, g_pDWTFBig, &D2DRcF1, pD2DBrush);
            
        }
        else// 有两行
        {
            g_pDWTFBig->SetTextAlignment(DWRITE_TEXT_ALIGNMENT_LEADING);
            uStrLen1 = p->iOrgLength;
            uStrLen2 = lstrlenW(p->pszLrc + p->iOrgLength + 1);

            if (bCurr)
            {
                if (iIndex != m_LrcHScrollInfo.iIndex)
                {
                    g_pDWFactory->CreateTextLayout(p->pszLrc, uStrLen1, g_pDWTFBig, m_cxLrcShow, m_cyLrcShow, &pDWTextLayout1);// 创建文本布局
                    g_pDWFactory->CreateTextLayout(p->pszLrc + p->iOrgLength + 1, uStrLen2, g_pDWTFBig, m_cxLrcShow, m_cyLrcShow, &pDWTextLayout2);// 创建文本布局
                    pDWTextLayout1->GetMetrics(&m_LrcHScrollInfo.Metrics1);
                    pDWTextLayout2->GetMetrics(&m_LrcHScrollInfo.Metrics2);
                    pDWTextLayout1->Release();
                    pDWTextLayout2->Release();

                    cx1 = m_LrcHScrollInfo.Metrics1.width;
                    cx2 = m_LrcHScrollInfo.Metrics2.width;
                    cy1 = m_LrcHScrollInfo.Metrics1.height;
                    cy2 = m_LrcHScrollInfo.Metrics2.height;

                    m_LrcHScrollInfo.iIndex = iIndex;
                    if (!m_LrcHScrollInfo.bWndSizeChangedFlag)
                        m_LrcHScrollInfo.x1 = m_LrcHScrollInfo.x2 = 0;
                    else
                        m_LrcHScrollInfo.bWndSizeChangedFlag = FALSE;

                    KillTimer(g_hMainWnd, IDT_ANIMATION);
                    if (cx1 > m_cxLrcShow)
                    {
                        m_LrcHScrollInfo.cx1 = cx1;// 超长了，需要后续滚动
                        m_LrcHScrollInfo.fNoScrollingTime1 = m_cxLrcShow * p->fDelay / m_LrcHScrollInfo.cx1 / 2;
                        SetTimer(g_hMainWnd, IDT_ANIMATION, TIMERELAPSE_ANIMATION, TimerProc);
                    }
                    else
                    {
                        m_LrcHScrollInfo.cx1 = -1;
                        m_LrcHScrollInfo.x1 = 0;
                    }

                    if (cx2 > m_cxLrcShow)
                    {
                        m_LrcHScrollInfo.cx2 = cx2;// 超长了，需要后续滚动
                        m_LrcHScrollInfo.fNoScrollingTime2 = m_cxLrcShow * p->fDelay / m_LrcHScrollInfo.cx2 / 2;
                        SetTimer(g_hMainWnd, IDT_ANIMATION, TIMERELAPSE_ANIMATION, TimerProc);
                    }
                    else
                    {
                        m_LrcHScrollInfo.cx2 = -1;
                        m_LrcHScrollInfo.x2 = 0;
                    }
                }
                else// 横向滚动就別测高了
                {
                    cx1 = m_LrcHScrollInfo.Metrics1.width;
                    cx2 = m_LrcHScrollInfo.Metrics2.width;
                    cy1 = m_LrcHScrollInfo.Metrics1.height;
                    cy2 = m_LrcHScrollInfo.Metrics2.height;
                }
            }
            else
            {
                g_pDWFactory->CreateTextLayout(p->pszLrc, uStrLen1, g_pDWTFBig, m_cxLrcShow, m_cyLrcShow, &pDWTextLayout1);// 创建文本布局
                g_pDWFactory->CreateTextLayout(p->pszLrc + p->iOrgLength + 1, uStrLen2, g_pDWTFBig, m_cxLrcShow, m_cyLrcShow, &pDWTextLayout2);// 创建文本布局
                pDWTextLayout1->GetMetrics(&Metrics1);
                pDWTextLayout2->GetMetrics(&Metrics2);
                pDWTextLayout1->Release();
                pDWTextLayout2->Release();
                cx1 = Metrics1.width;
                cx2 = Metrics2.width;
                cy1 = Metrics1.height;
                cy2 = Metrics2.height;
            }


            p->cy = cy1 + cy2;

            if (bCenterLine)
            {
                y -= p->cy / 2;
                bTop = TRUE;
                *yOut = y;
            }

            if (bTop)
            {
                D2DRcF2 = { (float)m_rcLrcShow.left,(float)y,(float)m_rcLrcShow.right,(float)(y + p->cy) };

                
                if (bImmdShow)
                {
                    m_pD2DDCLeftBK->BeginDraw();
                    D2DRcF1 = D2DRcF2;
                    if (D2DRcF2.top < m_rcLrcShow.top)
                        D2DRcF2.top = m_rcLrcShow.top;
                    if (D2DRcF2.bottom > m_rcLrcShow.bottom)
                        D2DRcF2.bottom = m_rcLrcShow.bottom;
                    m_pD2DDCLeftBK->PushAxisAlignedClip(&D2DRcF2, D2D1_ANTIALIAS_MODE_PER_PRIMITIVE);
                    m_pD2DDCLeftBK->DrawBitmap(m_pD2DBmpLeftBK2, &D2DRcF1, 1.0f, D2D1_BITMAP_INTERPOLATION_MODE_LINEAR, &D2DRcF1);
                }

                D2DRcF1.top = y;
                p->iLastTop = D2DRcF1.top;
                D2DRcF1.bottom = D2DRcF1.top + cy1;
                if (bCurr)
                    D2DRcF1.left = m_rcLrcShow.left + m_LrcHScrollInfo.x1;
                else
                    D2DRcF1.left = m_rcLrcShow.left;
                D2DRcF1.right = m_rcLrcShow.right;
                if (cx1 > m_cxLrcShow)
                    g_pDWTFBig->SetTextAlignment(DWRITE_TEXT_ALIGNMENT_LEADING);
                else
                    g_pDWTFBig->SetTextAlignment(DWRITE_TEXT_ALIGNMENT_CENTER);

                m_pD2DDCLeftBK->DrawTextW(p->pszLrc, uStrLen1, g_pDWTFBig, &D2DRcF1, pD2DBrush);

                D2DRcF1.top = D2DRcF1.bottom;
                D2DRcF1.bottom += cy2;
                if (bCurr)
                    D2DRcF1.left = m_rcLrcShow.left + m_LrcHScrollInfo.x2;

                D2DRcF1.right = m_rcLrcShow.right;
                if (cx2 > m_cxLrcShow)
                    g_pDWTFBig->SetTextAlignment(DWRITE_TEXT_ALIGNMENT_LEADING);
                else
                    g_pDWTFBig->SetTextAlignment(DWRITE_TEXT_ALIGNMENT_CENTER);

                m_pD2DDCLeftBK->DrawTextW(p->pszLrc + p->iOrgLength + 1, uStrLen2, g_pDWTFBig, &D2DRcF1, pD2DBrush);
                D2DRcF1.top = y;
                D2DRcF1.bottom = D2DRcF1.top + p->cy;
            }
            else
            {
                D2DRcF2 = { (float)m_rcLrcShow.left,(float)(y - p->cy),(float)m_rcLrcShow.right,(float)y };

                if (bImmdShow)
                {
                    m_pD2DDCLeftBK->BeginDraw();
                    D2DRcF1 = D2DRcF2;
                    if (D2DRcF2.top < m_rcLrcShow.top)
                        D2DRcF2.top = m_rcLrcShow.top;
                    if (D2DRcF2.bottom > m_rcLrcShow.bottom)
                        D2DRcF2.bottom = m_rcLrcShow.bottom;
                    m_pD2DDCLeftBK->PushAxisAlignedClip(&D2DRcF2, D2D1_ANTIALIAS_MODE_PER_PRIMITIVE);
                    m_pD2DDCLeftBK->DrawBitmap(m_pD2DBmpLeftBK2, &D2DRcF1, 1.0f, D2D1_BITMAP_INTERPOLATION_MODE_LINEAR, &D2DRcF1);
                }

                D2DRcF1.bottom = y;
                D2DRcF1.top = D2DRcF1.bottom - cy2;
                if (bCurr)
                    D2DRcF1.left = m_rcLrcShow.left + m_LrcHScrollInfo.x2;
                else
                    D2DRcF1.left = m_rcLrcShow.left;
                D2DRcF1.right = m_rcLrcShow.right;
                if (cx2 > m_cxLrcShow)
                    g_pDWTFBig->SetTextAlignment(DWRITE_TEXT_ALIGNMENT_LEADING);
                else
                    g_pDWTFBig->SetTextAlignment(DWRITE_TEXT_ALIGNMENT_CENTER);

                m_pD2DDCLeftBK->DrawTextW(p->pszLrc + p->iOrgLength + 1, uStrLen2, g_pDWTFBig, &D2DRcF1, pD2DBrush);

                D2DRcF1.bottom = D2DRcF1.top;
                D2DRcF1.top -= cy1;
                p->iLastTop = D2DRcF1.top;
                if (bCurr)
                    D2DRcF1.left = m_rcLrcShow.left + m_LrcHScrollInfo.x1;
                D2DRcF1.right = m_rcLrcShow.right;
                if (cx1 > m_cxLrcShow)
                    g_pDWTFBig->SetTextAlignment(DWRITE_TEXT_ALIGNMENT_LEADING);
                else
                    g_pDWTFBig->SetTextAlignment(DWRITE_TEXT_ALIGNMENT_CENTER);

                m_pD2DDCLeftBK->DrawTextW(p->pszLrc, uStrLen1, g_pDWTFBig, &D2DRcF1, pD2DBrush);
                D2DRcF1.bottom = y;
                D2DRcF1.top = D2DRcF1.bottom - p->cy;
            }
            p->rcItem = { (LONG)D2DRcF1.left,(LONG)D2DRcF1.top,(LONG)D2DRcF1.right,(LONG)D2DRcF1.bottom };
        }
    }
    else
    {
        g_pDWTFBig->SetTextAlignment(DWRITE_TEXT_ALIGNMENT_CENTER);
        g_pDWTFBig->SetWordWrapping(DWRITE_WORD_WRAPPING_WRAP);// 自动换行
        uStrLen1 = lstrlenW(p->pszLrc);
        g_pDWFactory->CreateTextLayout(p->pszLrc, uStrLen1, g_pDWTFBig, m_cxLrcShow, m_cyLrcShow, &pDWTextLayout1);
        pDWTextLayout1->GetMetrics(&Metrics1);
        pDWTextLayout1->Release();
        cy1 = Metrics1.height;
        p->cy = cy1;

        if (bCenterLine)
        {
            y -= p->cy / 2;
            bTop = TRUE;
            *yOut = y;
        }

        if (bTop)
            D2DRcF2 = { (float)m_rcLrcShow.left,(float)y,(float)m_rcLrcShow.right,(float)(y + p->cy) };
        else
            D2DRcF2 = { (float)m_rcLrcShow.left,(float)(y - p->cy),(float)m_rcLrcShow.right,(float)y };

        p->iLastTop = D2DRcF2.top;

        D2DRcF1 = D2DRcF2;

        if (bImmdShow)
        {
            m_pD2DDCLeftBK->BeginDraw();
            if (D2DRcF2.top < m_rcLrcShow.top)
                D2DRcF2.top = m_rcLrcShow.top;
            if (D2DRcF2.bottom > m_rcLrcShow.bottom)
                D2DRcF2.bottom = m_rcLrcShow.bottom;
            m_pD2DDCLeftBK->PushAxisAlignedClip(&D2DRcF2, D2D1_ANTIALIAS_MODE_PER_PRIMITIVE);
            m_pD2DDCLeftBK->DrawBitmap(m_pD2DBmpLeftBK2, &D2DRcF1, 1.0f, D2D1_BITMAP_INTERPOLATION_MODE_LINEAR, &D2DRcF1);
        }
        m_pD2DDCLeftBK->DrawTextW(p->pszLrc, uStrLen1, g_pDWTFBig, &D2DRcF1, pD2DBrush);
        p->rcItem = { (LONG)D2DRcF1.left,(LONG)D2DRcF1.top,(LONG)D2DRcF1.right,(LONG)D2DRcF1.bottom };
    }
    pD2DBrush->Release();
    if (m_iLrcMouseHover == iIndex)
    {
        m_pD2DDCLeftBK->CreateSolidColorBrush(D2D1::ColorF(QKGDIClrToCommonClr(QKCOLOR_CYANDEEPER), 0.1), &pD2DBrush);
        m_pD2DDCLeftBK->FillRectangle(D2DRcF1, pD2DBrush);
        pD2DBrush->Release();
    }

	if (bImmdShow)
	{
		m_pD2DDCLeftBK->PopAxisAlignedClip();
        //HDC hDC;
        //HRESULT hr = m_pD2DGdiInteropRTLeftBK->GetDC(D2D1_DC_INITIALIZE_MODE_COPY, &hDC);
        //HDC hDCWnd = GetDC(g_hBKLeft);
        //BitBlt(hDCWnd, p->rcItem.left, p->rcItem.top, p->rcItem.right - p->rcItem.left, p->rcItem.bottom - p->rcItem.top,
        //    hDC, p->rcItem.left, p->rcItem.top, SRCCOPY);
        //ReleaseDC(g_hBKLeft, hDCWnd);
        //m_pD2DGdiInteropRTLeftBK->ReleaseDC(NULL);
        m_pD2DDCLeftBK->EndDraw();
        m_pDXGIScLeftBK->Present(0, 0);
	}

	p->iDrawID = m_iDrawingID;// 已经绘制标记
	return p->cy;
}
/*
 * 目标：按索引播放列表中的文件
 *
 * 参数：
 * iIndex LV索引
 *
 * 返回值：
 * 操作简述：
 * 备注：
 */
void Playing_PlayFile(int iIndex)// 播放前将停止先前的播放
{
    //////////////清理遗留
    Playing_Stop(TRUE);
    //m_IsDraw[0] = m_IsDraw[1] = m_IsDraw[2] = TRUE;
    Lrc_ClearArray(g_Lrc);
    g_Lrc = QKArray_Create(0);
    delete[] m_dwWavesData;
    m_dwWavesData = NULL;
    //////////////取现行信息
    PLAYERLISTUNIT* p = List_GetArrayItem(iIndex);
    //////取文件名
    delete[] g_pszFile;// 释放先前的
    g_pszFile = new WCHAR[lstrlenW(p->pszFile) + 1];
    lstrcpyW(g_pszFile, p->pszFile);
    //////取消上一个播放标记
    int iLastPlayingIndex = g_iCurrFileIndex;
    g_iCurrFileIndex = -1;
    if (iLastPlayingIndex != -1)
        SendMessageW(g_hLV, LVM_REDRAWITEMS, iLastPlayingIndex, iLastPlayingIndex);
	//////开始播放
	g_hStream = BASS_OpenMusic(g_pszFile, BASS_SAMPLE_FX | BASS_STREAM_DECODE, BASS_SAMPLE_FX | BASS_MUSIC_PRESCAN | BASS_STREAM_DECODE);// 解码
	g_hStream = BASS_FX_TempoCreate(g_hStream, BASS_SAMPLE_FX | BASS_FX_FREESOURCE);// 创建重采样流
    BASS_ChannelSetSync(g_hStream, BASS_SYNC_END | BASS_SYNC_ONETIME, 0, SyncProc_End, NULL);// 设置同步过程用来跟踪歌曲播放完毕的事件
    
    if (!g_hStream)
    {
        g_iCurrFileIndex = -1;
		Global_ShowError(L"文件播放失败", NULL, ECODESRC_BASS, g_hMainWnd);
        return;
    }
    BASS_ChannelPlay(g_hStream, TRUE);
    g_bPlaying = TRUE;
	g_llLength = BASS_ChannelBytes2Seconds(g_hStream, BASS_ChannelGetLength(g_hStream, BASS_POS_BYTE)) * 1000;
    m_uThreadFlagWaves = THREADFLAG_WORKING;
    m_htdWaves = CreateThread(NULL, 0, Thread_GetWavesData, NULL, 0, NULL);// 启动线程获取波形数据
    if (g_pITaskbarList)
        g_pITaskbarList->SetProgressState(g_hTBGhost, TBPF_NORMAL);
    SendMessageW(g_hBKLeft, LEFTBKM_SETPLAYBTICON, FALSE, 0);
    //////置播放标记，判断是否要清除稍后播放标记
	g_iCurrFileIndex = iIndex;
	g_iLrcState = LRCSTATE_NOLRC;
	if (iIndex == g_iLaterPlay)
		g_iLaterPlay = -1;
	SendMessageW(g_hLV, LVM_REDRAWITEMS, iIndex, iIndex);
	//////////////取MP3信息
    MainWnd_ReleaseCurrInfo();
    //////取名称
    m_CurrSongInfo.pszName = new WCHAR[lstrlenW(p->pszName) + 1];
    lstrcpyW(m_CurrSongInfo.pszName, p->pszName);
    SetWindowTextW(g_hMainWnd, m_CurrSongInfo.pszName);
    //////取标签信息
	MusicInfo_Get(g_pszFile, &m_CurrSongInfo.mi);
    UI_PreProcessAlbumImage(&m_CurrSongInfo.mi.pWICBitmap);
    if (m_CurrSongInfo.mi.pWICBitmap)
    {
        m_pD2DDCLeftBK->CreateBitmapFromWicBitmap(m_CurrSongInfo.mi.pWICBitmap, &m_CurrSongInfo.pD2DBmpOrgAlbum);
        m_pD2DDCLeftBK->CreateBitmapBrush(m_CurrSongInfo.pD2DBmpOrgAlbum, &m_CurrSongInfo.pD2DBrushOrgAlbum);
    }

    DwmInvalidateIconicBitmaps(g_hTBGhost);
    UI_RefreshBmpBrush();
    //////解析Lrc歌词
    Lrc_ParseLrcData(g_pszFile, 0, TRUE, NULL, &g_Lrc, GS.uDefTextCode);
    if (!g_Lrc->iCount && m_CurrSongInfo.mi.pszLrc)
    {
        Lrc_ParseLrcData(
            m_CurrSongInfo.mi.pszLrc,
            (lstrlenW(m_CurrSongInfo.mi.pszLrc) + 1) * sizeof(WCHAR),
            FALSE, NULL, &g_Lrc, GS.uDefTextCode);
    }
    SendMessageW(g_hBKLeft, LEFTBKM_SETMAX, g_Lrc->iCount - 1, 0);
    LrcWnd_DrawLrc();
    UI_UpdateLeftBK();
    //////////////应用音效设置
    BASS_ChannelGetAttribute(g_hStream, BASS_ATTRIB_FREQ, &g_fDefSpeed);// 保存默认速度    
    g_llLength = (ULONGLONG)(BASS_ChannelBytes2Seconds(
        g_hStream,
        BASS_ChannelGetLength(g_hStream, BASS_POS_BYTE)
    ) * 1000);
    WndProc_LeftBK(g_hBKLeft, LEFTBKM_SETPROGBARMAX, g_llLength / 10, TRUE);

    if (g_fSpeedChanged != SBV_INVALIDVALUE)
        BASS_ChannelSetAttribute(g_hStream, BASS_ATTRIB_FREQ, g_fSpeedChanged * g_fDefSpeed);
    if (g_fBlanceChanged != SBV_INVALIDVALUE)
        BASS_ChannelSetAttribute(g_hStream, BASS_ATTRIB_PAN, g_fBlanceChanged);

    if (g_bSlient)
        BASS_ChannelSetAttribute(g_hStream, BASS_ATTRIB_VOL, 0);
    else if (g_fVolChanged != SBV_INVALIDVALUE)
		BASS_ChannelSetAttribute(g_hStream, BASS_ATTRIB_VOL, g_fVolChanged);

	if (g_GlobalEffect.fTempo != SBV_INVALIDVALUE)
		BASS_ChannelSetAttribute(g_hStream, BASS_ATTRIB_TEMPO, g_GlobalEffect.fTempo);

    if (g_GlobalEffect.hFXChorus)
    {
        g_GlobalEffect.hFXChorus = BASS_ChannelSetFX(g_hStream, BASS_FX_DX8_CHORUS, 1);
        BASS_FXSetParameters(g_GlobalEffect.hFXChorus, &g_GlobalEffect.Chorus);
    }
    if (g_GlobalEffect.hFXCompressor)
    {
        g_GlobalEffect.hFXCompressor = BASS_ChannelSetFX(g_hStream, BASS_FX_DX8_COMPRESSOR, 1);
        BASS_FXSetParameters(g_GlobalEffect.hFXCompressor, &g_GlobalEffect.Compressor);
    }
    if (g_GlobalEffect.hFXDistortion)
    {
        g_GlobalEffect.hFXDistortion = BASS_ChannelSetFX(g_hStream, BASS_FX_DX8_DISTORTION, 1);
        BASS_FXSetParameters(g_GlobalEffect.hFXDistortion, &g_GlobalEffect.Distortion);
    }
    if (g_GlobalEffect.hFXEcho)
    {
        g_GlobalEffect.hFXEcho = BASS_ChannelSetFX(g_hStream, BASS_FX_DX8_ECHO, 1);
        BASS_FXSetParameters(g_GlobalEffect.hFXEcho, &g_GlobalEffect.Echo);
    }
    if (g_GlobalEffect.hFXFlanger)
    {
        g_GlobalEffect.hFXFlanger = BASS_ChannelSetFX(g_hStream, BASS_FX_DX8_FLANGER, 1);
        BASS_FXSetParameters(g_GlobalEffect.hFXFlanger, &g_GlobalEffect.Flanger);
    }
    if (g_GlobalEffect.hFXGargle)
    {
        g_GlobalEffect.hFXGargle = BASS_ChannelSetFX(g_hStream, BASS_FX_DX8_GARGLE, 1);
        BASS_FXSetParameters(g_GlobalEffect.hFXGargle, &g_GlobalEffect.Gargle);
    }
    if (g_GlobalEffect.hFXI3DL2Reverb)
    {
        g_GlobalEffect.hFXI3DL2Reverb = BASS_ChannelSetFX(g_hStream, BASS_FX_DX8_I3DL2REVERB, 1);
        BASS_FXSetParameters(g_GlobalEffect.hFXI3DL2Reverb, &g_GlobalEffect.I3DL2Reverb);
    }
    if (g_GlobalEffect.hFXReverb)
    {
        g_GlobalEffect.hFXReverb = BASS_ChannelSetFX(g_hStream, BASS_FX_DX8_REVERB, 1);
        BASS_FXSetParameters(g_GlobalEffect.hFXReverb, &g_GlobalEffect.Reverb);
    }
    if (g_GlobalEffect.hFXEQ[0])
    {
        for (int i = 0; i < 10; ++i)
        {
            g_GlobalEffect.hFXEQ[i] = BASS_ChannelSetFX(g_hStream, BASS_FX_DX8_PARAMEQ, 1);
            BASS_FXSetParameters(g_GlobalEffect.hFXEQ[i], &g_GlobalEffect.EQ[i]);
        }
    }
    if (g_GlobalEffect.hFXRotate)
    {
        g_GlobalEffect.hFXRotate = BASS_ChannelSetFX(g_hStream, BASS_FX_BFX_ROTATE, 1);
        BASS_FXSetParameters(g_GlobalEffect.hFXRotate, &g_GlobalEffect.Rotate);
    }

	m_IsDraw[0] = m_IsDraw[1] = m_IsDraw[2] = TRUE;
    SetTimer(g_hBKLeft, IDT_PGS, TIMERELAPSE_PGS, NULL);
}
void Playing_Stop(BOOL bNoGap)
{
    KillTimer(g_hBKLeft, IDT_PGS);
    KillTimer(g_hMainWnd, IDT_ANIMATION);
    KillTimer(g_hMainWnd, IDT_ANIMATION2);

    StopThread_Waves();

    BASS_ChannelStop(g_hStream);
    BASS_FreeMusic(g_hStream);
    g_hStream = NULL;
    
    g_iCurrLrcIndex = -2;
    g_iLrcState = LRCSTATE_STOP;
    m_LrcHScrollInfo = { -1 };
    m_LrcVScrollInfo = { 0 };
    g_bPlaying = FALSE;
    m_iLrcSBPos = -1;
    m_iLrcFixedIndex = -1;
    m_iLrcMouseHover = -1;
    m_iLrcCenter = -1;
	m_iLastLrcIndex[0] = m_iLastLrcIndex[1] = -1;

    if (g_pITaskbarList)
        g_pITaskbarList->SetProgressState(g_hTBGhost, TBPF_NOPROGRESS);

    if (!bNoGap)
    {
        g_iCurrFileIndex = -1;
        m_IsDraw[0] = m_IsDraw[1] = m_IsDraw[2] = TRUE;
        
        LrcWnd_DrawLrc();
        SetWindowTextW(g_hMainWnd, L"未播放 - 晴空的音乐播放器");
        SendMessageW(g_hBKLeft, LEFTBKM_SETPLAYBTICON, TRUE, 0);
        m_IsDraw[0] = m_IsDraw[1] = m_IsDraw[2] = FALSE;
    }
    UI_UpdateLeftBK();
}
void Playing_PlayNext(BOOL bReverse)
{
    int iCount;
    if (g_iSearchResult != -1)
        iCount = g_iSearchResult;
    else
        iCount = g_ItemData->iCount;

    if (!iCount)
        return;

    int iIndex = g_iCurrFileIndex;
    if (g_iLaterPlay != -1)
    {
        iIndex = g_iLaterPlay;
        g_iLaterPlay = -1;
        SendMessageW(g_hLV, LVM_REDRAWITEMS, iIndex, iIndex);
        goto PlayFile;
    }

    if (bReverse)//倒序
    {
        for (int i = 0; i < g_ItemData->iCount; ++i)
        {
            --iIndex;
            if (iIndex < 0)
                iIndex = iCount - 1;

            if (!(List_GetArrayItem(iIndex)->dwFlags & QKLIF_IGNORED))
                goto PlayFile;
        }
    }
    else
    {
        for (int i = 0; i < g_ItemData->iCount; ++i)
        {
            ++iIndex;
            if (iIndex > iCount - 1)
                iIndex = 0;

            if (!(List_GetArrayItem(iIndex)->dwFlags & QKLIF_IGNORED))
                goto PlayFile;
        }
    }
    QKMessageBox(L"没有有效的项目", L"啥鼻，都忽略了你还听腻马啊", (HICON)TD_ERROR_ICON, L"错误");
    return;
PlayFile:
    Playing_PlayFile(iIndex);
}
void Playing_AutoNext()
{
    int i = SendMessageW(g_hBKLeft, LEFTBKM_GETREPEATMODE, 0, 0);
    switch (i)
    {
    case REPEATMODE_TOTALLOOP://整体循环
        Playing_PlayNext();
        break;
    case REPEATMODE_SINGLELOOP://单曲循环
        Playing_PlayFile(g_iCurrFileIndex);
        break;
    case REPEATMODE_RADOM://随机播放

        break;
    case REPEATMODE_SINGLE://单曲播放
        Playing_Stop();
        break;
    case REPEATMODE_TOTAL:

        break;
    }
}
void StopThread_Waves()
{
    DWORD dwExitCode;
    BOOL bResult = GetExitCodeThread(m_htdWaves, &dwExitCode);
    if (bResult && dwExitCode == STILL_ACTIVE)
    {
        m_uThreadFlagWaves = THREADFLAG_STOP;
        WaitForSingleObject(m_htdWaves, INFINITE);//等待线程退出
        m_uThreadFlagWaves = THREADFLAG_STOPED;
    }
    CloseHandle(m_htdWaves);
    m_htdWaves = NULL;//清空句柄
}
DWORD WINAPI Thread_GetWavesData(void* p)//调用前必须释放先前的内存
{
    HSTREAM hStream = BASS_OpenMusic(g_pszFile, BASS_STREAM_DECODE, BASS_MUSIC_DECODE | BASS_MUSIC_PRESCAN);
    int iCount = g_llLength / 20;
    if (iCount <= 0)
    {
        QKOutputDebugInt(BASS_ErrorGetCode());
        m_uThreadFlagWaves = THREADFLAG_ERROR;
        BASS_FreeMusic(hStream);
        return 0;
    }
    m_dwWavesData = new DWORD[iCount];
    for (int i = 0; i < iCount; i++)
    {
        m_dwWavesData[i] = BASS_ChannelGetLevel(hStream);
        if (m_uThreadFlagWaves == THREADFLAG_STOP)
            break;
    }
    BASS_FreeMusic(hStream);
    m_dwWavesDataCount = iCount;//计数
    m_uThreadFlagWaves = THREADFLAG_STOPED;//已停止
    m_IsDraw[0] = TRUE;//立即重画
    return 0;
}
/*
 * 目标：更新左侧内存位图并显示
 *
 * 参数：
 *
 * 返回值：
 * 操作简述：
 * 备注：ＤＩＲＥＣＴ　２Ｄ！
 */
void UI_UpdateLeftBK()
{
	if (m_cxLeftBK <= 0 || m_cyLeftBK <= 0)
		return;
	IWICBitmap* pWICBitmapOrg = m_CurrSongInfo.mi.pWICBitmap;// 原始WIC位图
	////////////////////绘制内存位图
	m_pD2DDCLeftBK->BeginDraw();
	HRESULT hr;
	m_pD2DDCLeftBK->SetTarget(m_pD2DBmpLeftBK2);
	m_pD2DDCLeftBK->Clear();

	UINT cx0, cy0;
	float cxRgn, cyRgn, cx, cy;
	D2D_RECT_F D2DRectF;

	ID2D1SolidColorBrush* pD2DBrush;
	if (pWICBitmapOrg)
	{
		ID2D1Bitmap1* pD2DBitmapOrg;// 原始D2D位图
		m_pD2DDCLeftBK->CreateBitmapFromWicBitmap(pWICBitmapOrg, &pD2DBitmapOrg);
		/*
		情况一，客户区宽为最大边
		cxClient   cyClient
		-------- = --------
		 cxPic      cyRgn
		情况二，客户区高为最大边
		cyClient   cxClient
		-------- = --------
		 cyPic      cxRgn
		*/
		////////////////////处理缩放与截取（无论怎么改变窗口大小，用来模糊的封面图都要居中充满整个窗口）
		D2D_POINT_2F D2DPtF;
		pWICBitmapOrg->GetSize(&cx0, &cy0);
		cyRgn = (float)m_cyLeftBK / (float)m_cxLeftBK * (float)cx0;
		if (cyRgn <= cy0)// 情况一
		{
			cx = m_cxLeftBK;
			cy = cx * cy0 / cx0;
			D2DPtF = { 0,(float)(m_cyLeftBK - cy) / 2 };
		}
		else// 情况二
		{
			cy = m_cyLeftBK;
			cx = cx0 * cy / cy0;
			D2DPtF = { (float)(m_cxLeftBK - cx) / 2,0 };
		}
		////////////缩放
		IWICBitmapScaler* pWICBitmapScaler;// WIC位图缩放器
		g_pWICFactory->CreateBitmapScaler(&pWICBitmapScaler);
		pWICBitmapScaler->Initialize(pWICBitmapOrg, cx, cy, WICBitmapInterpolationModeCubic);// 缩放
		IWICBitmap* pWICBmpScaled;// 缩放后的WIC位图
		g_pWICFactory->CreateBitmapFromSource(pWICBitmapScaler, WICBitmapNoCache, &pWICBmpScaled);
		pWICBitmapScaler->Release();// 释放WIC位图缩放器
		ID2D1Bitmap1* pD2DBmpScaled;// 缩放后的D2D位图
		m_pD2DDCLeftBK->CreateBitmapFromWicBitmap(pWICBmpScaled, &pD2DBmpScaled);// 转换为D2D位图
		pWICBmpScaled->Release();// 释放缩放后的WIC位图
		////////////模糊 
		ID2D1Image* pD2DBmpBlurred;
		ID2D1Effect* pD2DEffect;
		m_pD2DDCLeftBK->CreateEffect(CLSID_D2D1GaussianBlur, &pD2DEffect);
		pD2DEffect->SetInput(0, pD2DBmpScaled);
		pD2DEffect->SetValue(D2D1_GAUSSIANBLUR_PROP_STANDARD_DEVIATION, 50.0f);// 标准偏差
		pD2DEffect->SetValue(D2D1_GAUSSIANBLUR_PROP_BORDER_MODE, D2D1_BORDER_MODE_HARD);// 硬边缘
		pD2DEffect->GetOutput(&pD2DBmpBlurred);
		pD2DEffect->Release();
		pD2DBmpScaled->Release();
		////////////画模糊背景
		m_pD2DDCLeftBK->DrawImage(pD2DBmpBlurred, &D2DPtF);
		pD2DBmpBlurred->Release();
		////////////画半透明遮罩
		m_pD2DDCLeftBK->CreateSolidColorBrush(D2D1::ColorF(D2D1::ColorF::White, 0.5), &pD2DBrush);
		D2DRectF = { 0,0,(float)m_cxLeftBK,(float)m_cyLeftBK };
		m_pD2DDCLeftBK->FillRectangle(&D2DRectF, pD2DBrush);
		pD2DBrush->Release();
		////////////////////处理封面图位置
		////////////制封面框矩形
		if (m_bLrcShow)// 是否显示滚动歌词
		{
			cxRgn = DPIS_CXPIC + DPIS_EDGE * 2;
			cyRgn = cxRgn;
		}
		else
		{
			cxRgn = (float)m_cxLeftBK;
			cyRgn = (float)(m_cyLeftBK - DPIS_CYPROGBAR - DPIS_BT - DPIS_CYSPE - DPIS_CYTOPBK);
		}

		if (cxRgn >= cyRgn)// 高度较小
		{
			cy = cyRgn - DPIS_EDGE * 2;
			cx = cy;

			D2DRectF.left = (cxRgn - cx) / 2;
			D2DRectF.top = DPIS_EDGE + DPIS_CYTOPBK;
			D2DRectF.right = D2DRectF.left + cx;
			D2DRectF.bottom = D2DRectF.top + cy;
		}
		else// 宽度较小
		{
			cx = cxRgn - DPIS_EDGE * 2;
			cy = cx;

			D2DRectF.left = DPIS_EDGE;
			D2DRectF.top = (cyRgn - cy) / 2 + DPIS_CYTOPBK;
			D2DRectF.right = D2DRectF.left + cx;
			D2DRectF.bottom = D2DRectF.top + cy;
		}
		////////////////////绘制封面图
        if (!g_bShowAlbum)
        {
            ////////////比例缩放
            float f;
            if (cx0 >= cy0)// 高度较小
            {
                f = cx / cx0 * cy0;// 缩放到封面框中，要保证 水平 方向显示完全 高度 要多大？
                D2DRectF.top += ((cy - f) / 2);
                D2DRectF.bottom = D2DRectF.top + f;
            }
            else// 宽度较小
            {
                f = cy / cy0 * cx0;// 缩放到封面框中，要保证 垂直 方向显示完全 宽度 要多大？

                D2DRectF.left += ((cx - f) / 2);
                D2DRectF.right = D2DRectF.left + f;
            }
            ////////////画封面图
            m_pD2DDCLeftBK->DrawBitmap(pD2DBitmapOrg, &D2DRectF);
        }

		pD2DBitmapOrg->Release();// 销毁原始D2D位图
	}
	else// 没有图就全刷白吧
	{
		m_pD2DDCLeftBK->CreateSolidColorBrush(D2D1::ColorF(D2D1::ColorF::White), &pD2DBrush);
		D2DRectF = { 0,0,(float)m_cxLeftBK,(float)m_cyLeftBK };
		m_pD2DDCLeftBK->FillRectangle(&D2DRectF, pD2DBrush);
		pD2DBrush->Release();
	}
	////////////////////画顶部提示信息
	///////////制文本矩形
	D2DRectF = { (float)DPIS_EDGE,(float)DPIS_GAPTOPTIP,(float)(m_cxLeftBK - DPIS_EDGE),(float)DPIS_CYTOPTITLE };
	PCWSTR psz = m_CurrSongInfo.pszName ? m_CurrSongInfo.pszName : L"未播放";
	///////////制溢出裁剪对象
	DWRITE_TRIMMING DWTrimming =
	{
		DWRITE_TRIMMING_GRANULARITY_CHARACTER,// 按字符裁剪
		0,
		0
	};
	IDWriteInlineObject* pDWInlineObj;// 省略号裁剪内联对象
	g_pDWFactory->CreateEllipsisTrimmingSign(g_pDWTFBig, &pDWInlineObj);// 创建省略号裁剪
	///////////置文本格式
	g_pDWTFBig->SetTextAlignment(DWRITE_TEXT_ALIGNMENT_LEADING);
	g_pDWTFBig->SetTrimming(&DWTrimming, pDWInlineObj);// 置溢出裁剪
	g_pDWTFBig->SetWordWrapping(DWRITE_WORD_WRAPPING_NO_WRAP);// 不换行
	g_pDWTFNormal->SetTextAlignment(DWRITE_TEXT_ALIGNMENT_LEADING);
	g_pDWTFNormal->SetTrimming(&DWTrimming, pDWInlineObj);// 置溢出裁剪
	g_pDWTFNormal->SetWordWrapping(DWRITE_WORD_WRAPPING_NO_WRAP);// 不换行
	///////////画大标题
	m_pD2DDCLeftBK->DrawTextW(psz, lstrlenW(psz), g_pDWTFBig, &D2DRectF, m_pD2DBrMyBlue);
	///////////画其他信息
	m_pD2DDCLeftBK->CreateSolidColorBrush(D2D1::ColorF(D2D1::ColorF::Black), &pD2DBrush);// 创建一个黑色画刷
	const static PCWSTR pszTip[4] =
	{
		L"标题：",
		L"艺术家：",
		L"专辑：",
		L"备注："
	};

	PCWSTR pszTip2[4] =
	{
		m_CurrSongInfo.mi.pszTitle,
		m_CurrSongInfo.mi.pszArtist,
		m_CurrSongInfo.mi.pszAlbum,
		m_CurrSongInfo.mi.pszComment
	};

	D2DRectF.left = DPIS_EDGE;
	D2DRectF.right = D2DRectF.left + DPIS_CXTOPTIP;
	D2DRectF.top = DPIS_CYTOPTITLE + DPIS_GAPTOPTIP;
	D2DRectF.bottom = D2DRectF.top + DPIS_CYTOPTIP;

	for (int i = 0; i < sizeof(pszTip) / sizeof(PCWSTR); ++i)
	{
		m_pD2DDCLeftBK->DrawTextW(pszTip[i], lstrlenW(pszTip[i]), g_pDWTFNormal, &D2DRectF, pD2DBrush);
		if (pszTip2[i])
		{
			D2DRectF.left += DPIS_CXTOPTIP;
			D2DRectF.right = m_cxLeftBK - DPIS_EDGE;
			m_pD2DDCLeftBK->DrawTextW(pszTip2[i], lstrlenW(pszTip2[i]), g_pDWTFNormal, &D2DRectF, pD2DBrush);
			D2DRectF.left = DPIS_EDGE;
			D2DRectF.right = D2DRectF.left + DPIS_CXTOPTIP;
		}
		D2DRectF.top += DPIS_CYTOPTIP;
		D2DRectF.bottom = D2DRectF.top + DPIS_CYTOPTIP;
	}
	pD2DBrush->Release();// 删除画刷
    DWTrimming.granularity = DWRITE_TRIMMING_GRANULARITY_NONE;
    g_pDWTFBig->SetTrimming(&DWTrimming, NULL);
	g_pDWTFNormal->SetTrimming(&DWTrimming, NULL);// 置回原裁剪
	pDWInlineObj->Release();// 释放裁剪内联对象

	hr = m_pD2DDCLeftBK->EndDraw();
    m_pD2DDCLeftBK->SetTarget(m_pD2DBmpLeftBK);
	////////////////////写到交换链缓冲区
	m_pD2DDCLeftBK->BeginDraw();
	m_pD2DDCLeftBK->DrawBitmap(m_pD2DBmpLeftBK2);
	m_pD2DDCLeftBK->EndDraw();
    m_IsDraw[0] = m_IsDraw[1] = m_IsDraw[2] = TRUE;
    if (g_bShowAlbum)
    {
        UI_VEDrawAlbum();
    }
    else
    {
        UI_VEDrawWaves(FALSE);
        UI_VEDrawSpe(FALSE);
    }
    UI_VEProcLrcShowing(FALSE);
	WndProc_LeftBK(g_hBKLeft, LEFTBKM_REDRAWBTMBT, FALSE, TRUE);
    WndProc_LeftBK(g_hBKLeft, LEFTBKM_REDRAWTRACKBAR, FALSE, TRUE);
	////////////////////显示
	m_pDXGIScLeftBK->Present(0, 0);// 上屏
}
void UI_SeparateListWnd(BOOL b)
{
	static ULONG_PTR uCommStyle = WS_VISIBLE | WS_CLIPCHILDREN;
	RECT rc;
	if (b)
	{
		if (g_bListSeped)
			return;

		SetWindowLongPtrW(g_hBKList, GWL_STYLE, WS_OVERLAPPEDWINDOW | WS_POPUP | uCommStyle);
		SetParent(g_hBKList, NULL);
		ShowWindow(g_hSEB, SW_HIDE);
		g_bListSeped = TRUE;

		GetClientRect(g_hBKList, &rc);
		SendMessageW(g_hBKList, WM_SIZE, 0, MAKELONG(rc.right, rc.bottom));
		if (g_bListHidden)
			ShowWindow(g_hBKList, SW_HIDE);
	}
	else
	{
		if (!g_bListSeped)
			return;
		SetParent(g_hBKList, g_hMainWnd);
		SetWindowLongPtrW(g_hBKList, GWL_STYLE, WS_CHILD | uCommStyle);
		g_bListSeped = FALSE;
		if (g_bListHidden)
		{
			ShowWindow(g_hBKList, SW_HIDE);
			ShowWindow(g_hSEB, SW_HIDE);
		}
	}
	GetClientRect(g_hMainWnd, &rc);
	SendMessageW(g_hMainWnd, WM_SIZE, 0, MAKELONG(rc.right, rc.bottom));
}
void UI_ShowList(BOOL b)
{
	RECT rc;
	if (b)
	{
		if (!g_bListHidden)
			return;
		ShowWindow(g_hBKList, SW_SHOW);
		g_bListHidden = FALSE;
		if (!g_bListSeped)
			ShowWindow(g_hSEB, SW_SHOW);
	}
	else
	{
		if (g_bListHidden)
			return;
		ShowWindow(g_hBKList, SW_HIDE);
		ShowWindow(g_hSEB, SW_HIDE);
		g_bListHidden = TRUE;
	}
	GetClientRect(g_hMainWnd, &rc);
	SendMessageW(g_hMainWnd, WM_SIZE, 0, MAKELONG(rc.right, rc.bottom));
}
void UI_VEDrawAlbum(BOOL bImmdShow, BOOL bIndependlyDrawing)
{
    if (bIndependlyDrawing)
        m_pD2DDCLeftBK->BeginDraw();

    m_pD2DDCLeftBK->PushAxisAlignedClip(&m_D2DRcAlbum, D2D1_ANTIALIAS_MODE_ALIASED);// 设个剪辑区，防止边缘残留
    m_pD2DDCLeftBK->DrawBitmap(m_pD2DBmpLeftBK2, &m_D2DRcAlbum, 1.0f, D2D1_BITMAP_INTERPOLATION_MODE_LINEAR, &m_D2DRcAlbum);// 刷背景

    if (m_CurrSongInfo.pD2DBrushOrgAlbum && g_hStream)
    {
		ID2D1SolidColorBrush* pD2DBrush;
		m_pD2DDCLeftBK->CreateSolidColorBrush(D2D1::ColorF(D2D1::ColorF::White, 0.6f), &pD2DBrush);

		float cx = m_D2DRcAlbum.right - m_D2DRcAlbum.left, cy = m_D2DRcAlbum.bottom - m_D2DRcAlbum.top;
        /////////////////////////////准备数据
        const int iSampleCount = 100;
        int iBufSize = 2 * iSampleCount * sizeof(float);

        static float fData[200];
        static DWORD dwLevel;

        if (!g_bPlayIcon)
        {
            DWORD dw = BASS_ChannelGetData(g_hStream, fData, iBufSize);// 取频谱数据
            dwLevel = BASS_ChannelGetLevel(g_hStream);// 取电平

            m_fRotationAngle += 0.5f;
            if (m_fRotationAngle >= 360.f)
                m_fRotationAngle = 0;
        }
        /////////////////////////////画频谱
        float fStep = cx / iSampleCount;
		D2D1_POINT_2F D2DPt1, D2DPt2;
		int k, l;
		for (int j = 0; j < 2; ++j)
		{
			for (int i = 0; i < iSampleCount - 1; ++i)
			{
				k = (1 - fData[i * 2 + j]) * cy / 2;
				if (k < 0)
					k = 0;
				if (k > cy)
					k = cy;
				D2DPt1 = { m_D2DRcAlbum.left + i * fStep,(float)(m_D2DRcAlbum.top + k) };

				l = (1 - fData[(i + 1) * 2 + j]) * cy / 2;
				if (l < 0)
					l = 0;
				if (l > cy)
					l = cy;
				D2DPt2 = { m_D2DRcAlbum.left + (i + 1) * fStep,(float)(m_D2DRcAlbum.top + l) };
				m_pD2DDCLeftBK->DrawLine(D2DPt1, D2DPt2, m_pD2DBrMyBlue, DPIF(1.F));
			}
		}
        /////////////////////////////画封面边缘
        D2D1_ELLIPSE D2DEllipse;
        D2DEllipse.point = { m_D2DRcAlbum.left + cx / 2.f,m_D2DRcAlbum.top + cy / 2.f };
		float fRadius, f;
        fRadius = min(cx / 2.f - GC.DS_ALBUMLEVEL, cy / 2.f - GC.DS_ALBUMLEVEL) + GC.DS_ALBUMLEVEL;
        D2DEllipse.radiusX = D2DEllipse.radiusY = fRadius;

        m_pD2DDCLeftBK->FillEllipse(&D2DEllipse, pD2DBrush);// 画外圈
        float fOffset = 0.f;
        fRadius -= GC.DS_ALBUMLEVEL;
        
        if (dwLevel != -1)
        {
            fOffset = ((float)(LOWORD(dwLevel) + HIWORD(dwLevel)) / 2.f) / 32768.f * GC.DS_ALBUMLEVEL;
            fRadius += fOffset;
            D2DEllipse.radiusX = D2DEllipse.radiusY = fRadius;
            pD2DBrush->SetColor(D2D1::ColorF(D2D1::ColorF::White, 0.8f));
            m_pD2DDCLeftBK->FillEllipse(&D2DEllipse, pD2DBrush);// 画电平指示
        }
        pD2DBrush->Release();
        /////////////////////////////画封面
        D2D1_MATRIX_3X2_F Matrix = D2D1::Matrix3x2F::Rotation(m_fRotationAngle, D2DEllipse.point);// 制旋转矩阵
        m_pD2DDCLeftBK->SetTransform(Matrix);// 置旋转变换

        fRadius = fRadius - fOffset;
        D2DEllipse.radiusX = D2DEllipse.radiusY = fRadius;
        m_pD2DDCLeftBK->FillEllipse(&D2DEllipse, m_CurrSongInfo.pD2DBrushOrgAlbum);

        Matrix = D2D1::Matrix3x2F::Identity();
        m_pD2DDCLeftBK->SetTransform(Matrix);// 还原空变换
    }

    m_pD2DDCLeftBK->PopAxisAlignedClip();

    if (bIndependlyDrawing)
        m_pD2DDCLeftBK->EndDraw();
    if (bImmdShow)
        m_pDXGIScLeftBK->Present(0, 0);
}
BOOL UI_VEDrawWaves(BOOL bImmdShow, BOOL bIndependlyDrawing)
{
	if (!m_IsDraw[0])
		return FALSE;

	if (bIndependlyDrawing)
		m_pD2DDCLeftBK->BeginDraw();

	m_pD2DDCLeftBK->DrawBitmap(m_pD2DBmpLeftBK2, &m_D2DRcWaves, 1.0f, D2D1_BITMAP_INTERPOLATION_MODE_LINEAR, &m_D2DRcWaves);// 刷背景

	PCWSTR pszText = NULL;
	if (m_uThreadFlagWaves == THREADFLAG_WORKING)// 正在加载
		pszText = L"正在加载...";
	else if (!g_hStream)// 已停止
		pszText = L"未播放";
	else if (m_uThreadFlagWaves == THREADFLAG_ERROR)// 出错
		pszText = L"错误！";

	if (pszText)// 应当显示提示
	{
		m_IsDraw[0] = FALSE;// 下次不要再更新
		g_pDWTFBig->SetTextAlignment(DWRITE_TEXT_ALIGNMENT_CENTER);
		g_pDWTFBig->SetParagraphAlignment(DWRITE_PARAGRAPH_ALIGNMENT_CENTER);// 水平垂直都居中
		m_pD2DDCLeftBK->DrawTextW(pszText, lstrlenW(pszText), g_pDWTFBig, &m_D2DRcWaves, m_pD2DBrMyBlue);
		if (bIndependlyDrawing)
			m_pD2DDCLeftBK->EndDraw();
		return TRUE;
	}

	if (m_uThreadFlagWaves == THREADFLAG_STOPED)
	{
		int iCurrIndex = (int)(g_fTime * 1000.0 / 20.0);// 算数组索引    20ms一单位
		if (iCurrIndex < 0 || iCurrIndex > m_dwWavesDataCount - 1)
		{
			m_pD2DDCLeftBK->EndDraw();
			return TRUE;
		}

		int i = iCurrIndex;
		int x = m_rcWaves.left + DPIS_CXSPEHALF,
			y = m_rcWaves.top + DPIS_CYSPEHALF;
		D2D1_POINT_2F D2DPtF1, D2DPtF2;

		HDC hDC;
		m_pD2DDCLeftBK->SetAntialiasMode(D2D1_ANTIALIAS_MODE_ALIASED);
        m_pD2DDCLeftBK->PushAxisAlignedClip(&m_D2DRcWaves, D2D1_ANTIALIAS_MODE_PER_PRIMITIVE);
		// 上面是右声道，下面是左声道
		while (true)// 向右画
		{
			D2DPtF1 = { (float)x, (float)(y - HIWORD(m_dwWavesData[i]) * DPIS_CYSPEHALF / 32768) };
			D2DPtF2 = { (float)x, (float)(y + LOWORD(m_dwWavesData[i]) * DPIS_CYSPEHALF / 32768) };
			m_pD2DDCLeftBK->DrawLine(D2DPtF1, D2DPtF2, m_pD2DBrMyBlue2, DPIS_CXWAVESLINE);
			x += DPIS_CXWAVESLINE;
			i++;
			if (i > m_dwWavesDataCount - 1 || x >= m_rcWaves.left + DPIS_CXSPE)
				break;
		}
		i = iCurrIndex;
		x = m_rcWaves.left + DPIS_CXSPEHALF;
		while (true)// 向左画
		{
			D2DPtF1 = { (float)x, (float)(y - HIWORD(m_dwWavesData[i]) * DPIS_CYSPEHALF / 32768) };
			D2DPtF2 = { (float)x, (float)(y + LOWORD(m_dwWavesData[i]) * DPIS_CYSPEHALF / 32768) };
			m_pD2DDCLeftBK->DrawLine(D2DPtF1, D2DPtF2, m_pD2DBrMyBlue2, DPIS_CXWAVESLINE);
			x -= DPIS_CXWAVESLINE;
			i--;
			if (i < 0 || x < m_rcWaves.left)
				break;
		}
		x = m_rcWaves.left + DPIS_CXSPEHALF;

		D2DPtF1 = { (float)x, (float)m_rcWaves.top };
		D2DPtF2 = { (float)x,  (float)(m_rcWaves.top + DPIS_CYSPE) };
		ID2D1SolidColorBrush* pD2DBrush;
		m_pD2DDCLeftBK->CreateSolidColorBrush(D2D1::ColorF(D2D1::ColorF::Red), &pD2DBrush);
		m_pD2DDCLeftBK->DrawLine(D2DPtF1, D2DPtF2, pD2DBrush, DPIS_CXWAVESLINE);
		pD2DBrush->Release();
        m_pD2DDCLeftBK->PopAxisAlignedClip();
		m_pD2DDCLeftBK->SetAntialiasMode(D2D1_ANTIALIAS_MODE_PER_PRIMITIVE);
		//if (bImmdShow)
		//{
		//	m_pD2DGdiInteropRTLeftBK->GetDC(D2D1_DC_INITIALIZE_MODE_COPY, &hDC);// 取DC
		//	HDC hDCWnd = GetDC(g_hBKLeft);
		//	BitBlt(hDCWnd, m_rcWaves.left, m_rcWaves.top, DPIS_CXSPE, DPIS_CYSPE, hDC, m_rcWaves.left, m_rcWaves.top, SRCCOPY);// 区域显示
		//	ReleaseDC(g_hBKLeft, hDCWnd);
		//	m_pD2DGdiInteropRTLeftBK->ReleaseDC(&m_rcWaves);
		//}
        if (bIndependlyDrawing)
            m_pD2DDCLeftBK->EndDraw();
        if (bImmdShow)
            m_pDXGIScLeftBK->Present(0, 0);
	}
    else
    {
        if (bIndependlyDrawing)
            m_pD2DDCLeftBK->EndDraw();
    }
    return TRUE;
}
void UI_VEDrawSpe(BOOL bImmdShow, BOOL bIndependlyDrawing)
{
	int iCount = SPECOUNT;
	int m = DPIS_CXSPE - (DPIS_CXSPEBAR + DPIS_CXSPEBARDIV) * SPECOUNT;
	if (m >= DPIS_CXSPEBAR)
	{
		iCount += m / (DPIS_CXSPEBAR + DPIS_CXSPEBARDIV);
		if (iCount > 128)
			iCount = 128;
	}

	static int cySpeOld[128] = { 0 }, cySpe[128] = { 0 }, yMaxMark[128] = { 0 };
	static int iTime[128] = { 0 };
	static float fData[128] = { 0 };

	if (BASS_ChannelGetData(g_hStream, fData, BASS_DATA_FFT256) == -1)
	{
		ZeroMemory(cySpeOld, sizeof(cySpeOld));
		ZeroMemory(cySpe, sizeof(cySpe));
	}
	if (bIndependlyDrawing)
		m_pD2DDCLeftBK->BeginDraw();

	m_pD2DDCLeftBK->DrawBitmap(m_pD2DBmpLeftBK2, &m_D2DRcSpe, 1.0f, D2D1_BITMAP_INTERPOLATION_MODE_LINEAR, &m_D2DRcSpe);// 刷背景
	for (int i = 0; i < iCount; i++)
	{
		++iTime[i];
		cySpe[i] = abs((int)(fData[i] * 300.0f));
		//////////////////频谱条
		if (cySpe[i] > DPIS_CYSPE)// 超高
			cySpe[i] = DPIS_CYSPE;// 回来

		if (cySpe[i] > cySpeOld[i])// 当前的大于先前的
			cySpeOld[i] = cySpe[i];// 顶上去
		else
			cySpeOld[i] -= SPESTEP_BAR;// 如果不大于就继续落

		if (cySpeOld[i] < 3)// 太低了
			cySpeOld[i] = 3;// 回来
		//////////////////峰值
		if (iTime[i] > 10)// 时间已到
			yMaxMark[i] -= SPESTEP_MAX;// 下落

		if (cySpeOld[i] > yMaxMark[i])// 频谱条大于峰值
		{
			yMaxMark[i] = cySpeOld[i];// 峰值顶上去，重置时间
			iTime[i] = 0;
		}

		if (yMaxMark[i] < 3)// 太低了
			yMaxMark[i] = 3;// 回来
		//////////////////绘制
		//////////制频谱条矩形
        D2D_RECT_F D2DRectF;
		D2DRectF.left = m_rcSpe.left + (DPIS_CXSPEBAR + 1) * i;
		D2DRectF.top = m_rcSpe.top + DPIS_CYSPE - cySpeOld[i];
		D2DRectF.right = D2DRectF.left + DPIS_CXSPEBAR;
		D2DRectF.bottom = m_rcSpe.top + DPIS_CYSPE;
		m_pD2DDCLeftBK->FillRectangle(&D2DRectF, m_pD2DBrMyBlue2);
		//////////制峰值指示矩形
		D2DRectF.top = m_rcSpe.top + DPIS_CYSPE - yMaxMark[i];
		D2DRectF.bottom = D2DRectF.top + 3;
		m_pD2DDCLeftBK->FillRectangle(&D2DRectF, m_pD2DBrMyBlue2);
	}
	//if (bImmdShow)
	//{
	//	HDC hDC;
	//	HRESULT hr = m_pD2DGdiInteropRTLeftBK->GetDC(D2D1_DC_INITIALIZE_MODE_COPY, &hDC);
	//	HDC hDCWnd = GetDC(g_hBKLeft);
	//	BitBlt(hDCWnd, m_rcSpe.left, m_rcSpe.top, DPIS_CXSPE, DPIS_CYSPE, hDC, m_rcSpe.left, m_rcSpe.top, SRCCOPY);// 区域显示
	//	ReleaseDC(g_hBKLeft, hDCWnd);
	//	m_pD2DGdiInteropRTLeftBK->ReleaseDC(NULL);
	//}
	if (bIndependlyDrawing)
		m_pD2DDCLeftBK->EndDraw();
    if (bImmdShow)
        m_pDXGIScLeftBK->Present(0, 0);
}
void UI_VEDrawLrc(int yCenter, BOOL bImmdShow, BOOL bIndependlyDrawing)
{
    if (bIndependlyDrawing)
        m_pD2DDCLeftBK->BeginDraw();
	D2D_RECT_F D2DRectF = { (float)m_rcLrcShow.left,(float)m_rcLrcShow.top,(float)m_rcLrcShow.right,(float)m_rcLrcShow.bottom };
	m_pD2DDCLeftBK->DrawBitmap(m_pD2DBmpLeftBK2, &D2DRectF, 1.0f, D2D1_BITMAP_INTERPOLATION_MODE_LINEAR, &D2DRectF);// 刷背景
	m_pD2DDCLeftBK->PushAxisAlignedClip(&D2DRectF, D2D1_ANTIALIAS_MODE_ALIASED);

	int iTop;
	int iHeight = Lrc_DrawItem(m_iLrcCenter, yCenter, TRUE, FALSE, FALSE, TRUE, &iTop);
	int iBottom = iTop - DPIS_LRCSHOWGAP;
	iTop += (DPIS_LRCSHOWGAP + iHeight);
	int i = m_iLrcCenter;
	////////////////////////////////////////////////向上画
	while (iBottom > m_rcLrcShow.top)
	{
		if (i - 1 < 0)
			break;
		--i;
		iHeight = Lrc_DrawItem(i, iBottom, FALSE, FALSE, FALSE);
		iBottom -= (DPIS_LRCSHOWGAP + iHeight);
	}
	i = m_iLrcCenter;
	////////////////////////////////////////////////向下画
	while (iTop < m_rcLrcShow.bottom)
	{
		if (i + 1 >= g_Lrc->iCount)
			break;
		++i;
		iHeight = Lrc_DrawItem(i, iTop, TRUE, FALSE, FALSE);
		iTop += (DPIS_LRCSHOWGAP + iHeight);
	}

	m_pD2DDCLeftBK->PopAxisAlignedClip();
	//if (bImmdShow)
	//{
	//	HDC hDC;
	//	HRESULT hr = m_pD2DGdiInteropRTLeftBK->GetDC(D2D1_DC_INITIALIZE_MODE_COPY, &hDC);
	//	HDC hDCWnd = GetDC(g_hBKLeft);
	//	BitBlt(hDCWnd, m_rcLrcShow.left, m_rcLrcShow.top, m_rcLrcShow.right - m_rcLrcShow.left, m_rcLrcShow.bottom - m_rcLrcShow.top,
	//		hDC, m_rcLrcShow.left, m_rcLrcShow.top, SRCCOPY);// 区域显示
	//	ReleaseDC(g_hBKLeft, hDCWnd);
	//	m_pD2DGdiInteropRTLeftBK->ReleaseDC(NULL);
	//}
	if (bIndependlyDrawing)
		m_pD2DDCLeftBK->EndDraw();
    if (bImmdShow)
        m_pDXGIScLeftBK->Present(0, 0);
}
BOOL UI_VEProcLrcShowing(BOOL bImmdShow, BOOL bIndependlyDrawing, BOOL bOnlyDrawDTLrc)
{
	RECT rc;
    HDC hDC;
    BOOL bSwitchLrc;
    if (m_IsDraw[2])
    {
        PCWSTR pszText = NULL;
        if (!g_hStream)
        {
            pszText = L"晴空的音乐播放器 - VC++/Win32";
            g_iLrcState = LRCSTATE_STOP;
            g_iCurrLrcIndex = -2;
        }
        else if (!g_Lrc->iCount)
        {
            pszText = L"<无歌词>";
            g_iLrcState = LRCSTATE_NOLRC;
            g_iCurrLrcIndex = -2;
        }

        if (pszText)
        {
            if (bIndependlyDrawing)
                m_pD2DDCLeftBK->BeginDraw();

            D2D_RECT_F D2DRectF = { (float)m_rcLrcShow.left,(float)m_rcLrcShow.top,(float)m_rcLrcShow.right,(float)m_rcLrcShow.bottom };
            m_pD2DDCLeftBK->DrawBitmap(m_pD2DBmpLeftBK2, &D2DRectF, 1.0f, D2D1_BITMAP_INTERPOLATION_MODE_LINEAR, &D2DRectF);

            g_pDWTFBig->SetTextAlignment(DWRITE_TEXT_ALIGNMENT_CENTER);
            g_pDWTFBig->SetParagraphAlignment(DWRITE_PARAGRAPH_ALIGNMENT_NEAR);

            m_pD2DDCLeftBK->DrawTextW(pszText, lstrlenW(pszText), g_pDWTFBig, &D2DRectF, m_pD2DBrMyBlue, D2D1_DRAW_TEXT_OPTIONS_CLIP);
            //if (bImmdShow)
            //{
            //    HDC hDC;
            //    HRESULT hr = m_pD2DGdiInteropRTLeftBK->GetDC(D2D1_DC_INITIALIZE_MODE_COPY, &hDC);
            //    HDC hDCWnd = GetDC(g_hBKLeft);
            //    BitBlt(hDCWnd, m_rcLrcShow.left, m_rcLrcShow.top, m_rcLrcShow.right - m_rcLrcShow.left, m_rcLrcShow.bottom - m_rcLrcShow.top,
            //        hDC, m_rcLrcShow.left, m_rcLrcShow.top, SRCCOPY);// 区域显示
            //    ReleaseDC(g_hBKLeft, hDCWnd);
            //    m_pD2DGdiInteropRTLeftBK->ReleaseDC(NULL);
            //}
            if (bIndependlyDrawing)
                m_pD2DDCLeftBK->EndDraw();
            if (bImmdShow)
                m_pDXGIScLeftBK->Present(0, 0);
            return TRUE;
        }
    }

    m_iLrcCenter = -1;
	g_iCurrLrcIndex = -2;
	if (g_Lrc->iCount)
	{
		int iArrayCount = g_Lrc->iCount;
		float fTempTime;

		for (int i = 0; i < iArrayCount; i++)// 遍历歌词数组
		{
			fTempTime = ((LRCDATA*)QKArray_Get(g_Lrc, i))->fTime;
			if (g_fTime < ((LRCDATA*)QKArray_Get(g_Lrc, 0))->fTime)// 还没播到第一句
			{
				m_iLrcCenter = 0;
				g_iCurrLrcIndex = -1;
				break;
			}
			else if (i == iArrayCount - 1)// 最后一句
			{
				if (g_fTime >= fTempTime)
				{
					m_iLrcCenter = g_iCurrLrcIndex = i;
					break;
				}
			}
			else if (g_fTime >= fTempTime && g_fTime < ((LRCDATA*)QKArray_Get(g_Lrc, i + 1))->fTime)// 左闭右开，判断歌词区间
			{
				m_iLrcCenter = g_iCurrLrcIndex = i;
				break;
			}
		}
        
		if (m_iLrcSBPos != -1)
			m_iLrcCenter = m_iLrcSBPos;
        if (m_iLrcFixedIndex != -1)
            m_iLrcCenter = m_iLrcFixedIndex;
        // 索引查找完毕
        if (m_iLrcCenter != m_iLastLrcIndex[0] || g_iCurrLrcIndex != m_iLastLrcIndex[1])
        {
            m_IsDraw[2] = TRUE;
            bSwitchLrc = TRUE;
        }
        else
            bSwitchLrc = FALSE;
    }

    if (m_IsDraw[2])
    {
        ++m_iDrawingID;
        if (!m_iDrawingID)
            ++m_iDrawingID;
        g_iLrcState = LRCSTATE_NORMAL;
        if (!bOnlyDrawDTLrc)
        {
            if (bSwitchLrc && GS.bLrcAnimation && m_iLrcCenter != m_iLastLrcIndex[0] && m_iLrcSBPos == -1 && m_iLastLrcIndex[0] != -1)
            {
                if (m_iLastLrcIndex[0] > g_Lrc->iCount - 1)
                    m_iLastLrcIndex[0] = g_Lrc->iCount - 1;;
                LRCDATA* p1 = (LRCDATA*)QKArray_Get(g_Lrc, m_iLrcCenter), * p2 = (LRCDATA*)QKArray_Get(g_Lrc, m_iLastLrcIndex[0]);
                IDWriteTextLayout* pDWTextLayout;
                if (GS.bForceTwoLines)
                {
                    g_pDWTFBig->SetWordWrapping(DWRITE_WORD_WRAPPING_NO_WRAP);
                    g_pDWTFBig2->SetWordWrapping(DWRITE_WORD_WRAPPING_NO_WRAP);
                }
                else
                {
                    g_pDWTFBig->SetWordWrapping(DWRITE_WORD_WRAPPING_WRAP);
                    g_pDWTFBig2->SetWordWrapping(DWRITE_WORD_WRAPPING_WRAP);
                }
                DWRITE_TEXT_METRICS Metrics;
                g_pDWFactory->CreateTextLayout(p1->pszLrc, lstrlenW(p1->pszLrc), g_pDWTFBig2, m_cxLeftBK, m_cyLeftBK, &pDWTextLayout);
                pDWTextLayout->GetMetrics(&Metrics);
                int iHeight = Metrics.height;
                pDWTextLayout->Release();

                g_pDWFactory->CreateTextLayout(p2->pszLrc, lstrlenW(p2->pszLrc), g_pDWTFBig, m_cxLeftBK, m_cyLeftBK, &pDWTextLayout);
                pDWTextLayout->GetMetrics(&Metrics);
                int iHeight2 = Metrics.height;
                pDWTextLayout->Release();

                int iTop = m_rcLrcShow.top + (m_cyLrcShow - iHeight2) / 2;// 上一句顶边
                m_LrcVScrollInfo.iDestTop = m_rcLrcShow.top + m_cyLrcShow / 2;
                m_LrcVScrollInfo.fDelay = 0.1f;
                m_LrcVScrollInfo.fTime = g_fTime;
                float ff;
                if (m_iLrcCenter > m_iLastLrcIndex[0])// 下一句在上一句的下方
                {
                    m_LrcVScrollInfo.bDirection = TRUE;
                    m_LrcVScrollInfo.iSrcTop = m_LrcVScrollInfo.iDestTop + iHeight / 2 + iHeight2 / 2 + DPIS_LRCSHOWGAP;
                    m_LrcVScrollInfo.iDistance = m_LrcVScrollInfo.iSrcTop - m_LrcVScrollInfo.iDestTop;
                    ff = p1->fTime - p2->fTime;
                    if (m_LrcVScrollInfo.fDelay > ff)
                        m_LrcVScrollInfo.fDelay = ff / 2;
                }
                else// 下一句在上一句的上方
                {
                    m_LrcVScrollInfo.bDirection = FALSE;
                    m_LrcVScrollInfo.iSrcTop = m_LrcVScrollInfo.iDestTop - (iHeight / 2 + iHeight2 / 2 + DPIS_LRCSHOWGAP);
                    m_LrcVScrollInfo.iDistance = m_LrcVScrollInfo.iDestTop - m_LrcVScrollInfo.iSrcTop;
                    ff = p2->fTime - p1->fTime;
                    if (m_LrcVScrollInfo.fDelay > ff)
                        m_LrcVScrollInfo.fDelay = ff / 2;
                }
                KillTimer(g_hMainWnd, IDT_ANIMATION2);
                SetTimer(g_hMainWnd, IDT_ANIMATION2, TIMERELAPSE_ANIMATION2, TimerProc);
                LrcWnd_DrawLrc();
                m_iLastLrcIndex[0] = m_iLrcCenter;
                m_iLastLrcIndex[1] = g_iCurrLrcIndex;
                m_IsDraw[2] = FALSE;
                return TRUE;
            }

            UI_VEDrawLrc(m_rcLrcShow.top + m_cyLrcShow / 2, bImmdShow, bIndependlyDrawing);
        }

        m_iLastLrcIndex[0] = m_iLrcCenter;
        m_iLastLrcIndex[1] = g_iCurrLrcIndex;
        m_IsDraw[2] = FALSE;

        LrcWnd_DrawLrc();
        return TRUE;
    }
    else
        return FALSE;
}
void UI_RefreshBmpBrush()
{
    if (g_bShowAlbum && m_CurrSongInfo.pD2DBmpOrgAlbum)
    {
        if (m_CurrSongInfo.pD2DBrushOrgAlbum)
            m_CurrSongInfo.pD2DBrushOrgAlbum->Release();

        float fRadius, cx, cy;
        cx = m_D2DRcAlbum.right - m_D2DRcAlbum.left;
        cy = m_D2DRcAlbum.bottom - m_D2DRcAlbum.top;
		fRadius = min(cx / 2.f, cy / 2.f);
        float xStart = m_D2DRcAlbum.left + cx / 2.f - fRadius, yStart = m_D2DRcAlbum.top + cy / 2.f - fRadius;

        ID2D1BitmapBrush* pD2DBmpBrush;
        D2D1_MATRIX_3X2_F Matrix, Matrix2;

		Matrix = D2D1::Matrix3x2F::Translation(xStart, yStart);// 制平移矩阵
		D2D1::Matrix3x2F* MatrixObj1 = D2D1::Matrix3x2F::ReinterpretBaseType(&Matrix);// 转类

		D2D1_SIZE_F D2DSize = m_CurrSongInfo.pD2DBmpOrgAlbum->GetSize();

		Matrix2 = D2D1::Matrix3x2F::Scale((fRadius * 2) / D2DSize.width, (fRadius * 2) / D2DSize.height, D2D1::Point2F(xStart, yStart));// 制缩放矩阵
		D2D1::Matrix3x2F* MatrixObj2 = D2D1::Matrix3x2F::ReinterpretBaseType(&Matrix2);// 转类

		Matrix = static_cast<D2D1_MATRIX_3X2_F>((*MatrixObj1) * (*MatrixObj2));// 矩阵相乘

		D2D1_BRUSH_PROPERTIES D2DBrushProp = { 1.f,Matrix };

        m_pD2DDCLeftBK->CreateBitmapBrush(m_CurrSongInfo.pD2DBmpOrgAlbum, NULL, &D2DBrushProp, &m_CurrSongInfo.pD2DBrushOrgAlbum);
    }
}
LRESULT CALLBACK WndProc_TBGhost(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam)
{
    switch (message)
    {
    case WM_CREATE:
    {
        ChangeWindowMessageFilter(WM_DWMSENDICONICTHUMBNAIL, MSGFLT_ADD);
        ChangeWindowMessageFilter(WM_DWMSENDICONICLIVEPREVIEWBITMAP, MSGFLT_ADD);
        BOOL b = TRUE;
        DwmSetWindowAttribute(hWnd, DWMWA_HAS_ICONIC_BITMAP, &b, sizeof(BOOL));
        DwmSetWindowAttribute(hWnd, DWMWA_FORCE_ICONIC_REPRESENTATION, &b, sizeof(b));
    }
    return 0;
    case WM_DWMSENDICONICTHUMBNAIL:
    {
        if (!m_CurrSongInfo.mi.pWICBitmap)
            return 0;

        IWICBitmap* pWICBitmapOrg = m_CurrSongInfo.mi.pWICBitmap;
        pWICBitmapOrg->AddRef();

        UINT cx0, cy0, cx, cy, cxMax = HIWORD(lParam), cyMax = LOWORD(lParam);

        IWICBitmapLock* pWICBmpLock;
        WICRect WICRc = { 0 };
        pWICBitmapOrg->GetSize((UINT*)&WICRc.Width, (UINT*)&WICRc.Height);
        pWICBitmapOrg->Lock(&WICRc, WICBitmapLockRead, &pWICBmpLock);// 取位图锁

        UINT uBufSize;
        BYTE* pPicData;
        pWICBmpLock->GetDataPointer(&uBufSize, &pPicData);// 取数据指针
        UINT uStride;
        pWICBmpLock->GetStride(&uStride);// 取跨步
        pWICBmpLock->GetSize(&cx0, &cy0);// 取尺寸

        GpBitmap* pGdipBitmap;
        GdipCreateBitmapFromScan0(cx0, cy0, uStride, PixelFormat32bppArgb, pPicData, &pGdipBitmap);// WIC位图转GP位图

        HBITMAP hBitmap;
        if ((float)cxMax / (float)cyMax > (float)cx0 / (float)cy0)// y对齐
        {
            cy = cyMax;
            cx = cx0 * cy / cy0;
        }
        else// x对齐
        {
            cx = cxMax;
            cy = cx * cy0 / cx0;
        }
        GpBitmap* pGdipBitmapBK;
        GpGraphics* pGdipGraphics;
        GdipCreateBitmapFromScan0(cxMax, cyMax, 0, PixelFormat32bppArgb, NULL, &pGdipBitmapBK);// 建背景位图
        GdipGetImageGraphicsContext(pGdipBitmapBK, &pGdipGraphics);// 取背景图GP图形
        GdipGraphicsClear(pGdipGraphics, 0x00000000);
        GdipDrawImageRectRect(pGdipGraphics, pGdipBitmap,
            (cxMax - cx) / 2, (cyMax - cy) / 2, (float)cx, (float)cy,
            0, 0, (float)cx0, (float)cy0,
            UnitPixel, NULL, NULL, NULL);// 原始图缩放到背景图上
        GdipDeleteGraphics(pGdipGraphics);// 释放GP图形
        GdipCreateHBITMAPFromBitmap(pGdipBitmapBK, &hBitmap, 0x00000000);// GP位图转GDI位图
        DwmSetIconicThumbnail(hWnd, hBitmap, 0);// 置缩略图

        GdipDisposeImage(pGdipBitmap);
        GdipDisposeImage(pGdipBitmapBK);// 删除GP位图
        DeleteObject(hBitmap);// 删除GDI位图
        pWICBmpLock->Release();// 释放WIC位图锁
        pWICBitmapOrg->Release();
    }
    return 0;
    case WM_DWMSENDICONICLIVEPREVIEWBITMAP:
    {
        HBITMAP hBitmap;
        GpBitmap* pGdipImage;
        GdipCreateBitmapFromScan0(1, 1, 0, PixelFormat32bppArgb, NULL, &pGdipImage);// 1x1
        POINT pt = { 0 };
        GdipCreateHBITMAPFromBitmap(pGdipImage, &hBitmap, 0x00000000);
        DwmSetIconicLivePreviewBitmap(g_hTBGhost, hBitmap, &pt, 0);
        GdipDisposeImage(pGdipImage);
        DeleteObject(hBitmap);
    }
	return 0;
	case WM_ACTIVATE:// 窗口激活，转发（点击缩略图激活）
	{
        if (g_pITaskbarList)
            g_pITaskbarList->SetTabActive(hWnd, g_hMainWnd, 0);
		SetForegroundWindow(g_hMainWnd);
	}
	return 0;
    case WM_SYSCOMMAND:// 系统菜单，转发（最大化、最小化、关闭）
        return SendMessageW(g_hMainWnd, WM_SYSCOMMAND, wParam, lParam);
    case WM_COMMAND:// 任务栏工具栏按钮点击，转发
        return SendMessageW(g_hMainWnd, WM_COMMAND, wParam, lParam);
    }
    return DefWindowProcW(hWnd, message, wParam, lParam);
}
LRESULT CALLBACK WndProc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam)
{
    static int cxClient, cyClient;
    if (message == WM_TASKBARBUTTONCREATED)
    {
        g_pITaskbarList->UnregisterTab(g_hTBGhost);
        g_pITaskbarList->RegisterTab(g_hTBGhost, hWnd);
        g_pITaskbarList->SetTabOrder(g_hTBGhost, hWnd);
        THUMBBUTTON tb[3];
        THUMBBUTTONMASK dwMask = THB_ICON | THB_TOOLTIP;
        tb[0].dwMask = dwMask;
        tb[0].hIcon = GR.hiLast2;
        tb[0].iId = IDTBB_LAST;
        lstrcpyW(tb[0].szTip, L"上一曲");

        tb[1].dwMask = dwMask;
        tb[1].hIcon = GR.hiPlay2;
        tb[1].iId = IDTBB_PLAY;
        lstrcpyW(tb[1].szTip, L"播放");

        tb[2].dwMask = dwMask;
        tb[2].hIcon = GR.hiNext2;
        tb[2].iId = IDTBB_NEXT;
        lstrcpyW(tb[2].szTip, L"下一曲");
        g_pITaskbarList->ThumbBarAddButtons(g_hTBGhost, 3, tb);

        return 0;
    }
	switch (message)
	{
    case WM_COMMAND:
    {
        if (!lParam)//是菜单发出，lParam为0
        {
            
        }
        else//不是菜单或加速器发出
        {
            switch (LOWORD(wParam))
            {
            case IDTBB_LAST:
                SendMessageW(g_hBKLeft, LEFTBKM_DOBTOPE, 0, 0);
                return 0;
            case IDTBB_PLAY:
                SendMessageW(g_hBKLeft, LEFTBKM_DOBTOPE, 1, 0);
                return 0;
            case IDTBB_NEXT:
                SendMessageW(g_hBKLeft, LEFTBKM_DOBTOPE, 2, 0);
                return 0;
            }
        }
    }
    break;
    case WM_CREATE:
    {


        Settings_Read();
        GlobalEffect_ResetToDefault(EFFECT_ALL);
        
        g_Lrc = QKArray_Create(0);
        g_ItemData = QKArray_Create(0);
        ///////////////////////////初始化.....
        g_hMainWnd = hWnd;
        g_iDPI = QKGetDPIForWindow(hWnd);
        UI_UpdateDPISize();
        g_cxBKList = DPIS_DEFCXLV;
        
        HWND hCtrl, hCtrl2;
        ///////////////////////////左侧背景（剪辑子窗口）
        hCtrl2 = CreateWindowExW(0, BKWNDCLASS, NULL, WS_CHILD | WS_VISIBLE | WS_CLIPCHILDREN,
            0, 0, 0, 0,
            hWnd, (HMENU)IDC_BK_LEFT, g_hInst, NULL);
        g_hBKLeft = hCtrl2;
        RECT rc;
        GetClientRect(g_hBKLeft, &rc);

        DXGI_SWAP_CHAIN_DESC1 DXGIScDesc =
        {
            8,
            8,
            DXGI_FORMAT_B8G8R8A8_UNORM,
            FALSE,
            {1, 0},
            DXGI_USAGE_RENDER_TARGET_OUTPUT,
            2,
            DXGI_SCALING_STRETCH,
            DXGI_SWAP_EFFECT_DISCARD,
            DXGI_ALPHA_MODE_UNSPECIFIED,
            DXGI_SWAP_CHAIN_FLAG_GDI_COMPATIBLE// GDI兼容
        };
        HRESULT hr = g_pDXGIFactory->CreateSwapChainForHwnd(g_pDXGIDevice, g_hBKLeft, &DXGIScDesc, NULL, NULL, &m_pDXGIScLeftBK);// 创建交换链

        hr = g_pD2DDevice->CreateDeviceContext(D2D1_DEVICE_CONTEXT_OPTIONS_NONE, &m_pD2DDCLeftBK);// 创建设备上下文
        m_pD2DDCLeftBK->QueryInterface(IID_PPV_ARGS(&m_pD2DGdiInteropRTLeftBK));// 取GDI兼容渲染目标（鸣谢：福仔）
        m_pD2DDCLeftBK->CreateSolidColorBrush(c_D2DClrCyanDeeper, &m_pD2DBrMyBlue);
        m_pD2DDCLeftBK->CreateSolidColorBrush(D2D1::ColorF(0x469EDA), &m_pD2DBrMyBlue2);

        hr = m_pDXGIScLeftBK->GetBuffer(0, IID_PPV_ARGS(&m_pDXGISfceLeftBK));// 取缓冲区
        D2D1_BITMAP_PROPERTIES1 D2DBmpProp =
        {
            {DXGI_FORMAT_B8G8R8A8_UNORM,D2D1_ALPHA_MODE_PREMULTIPLIED},
            96,
            96,
            D2D1_BITMAP_OPTIONS_TARGET | D2D1_BITMAP_OPTIONS_CANNOT_DRAW | D2D1_BITMAP_OPTIONS_GDI_COMPATIBLE,
            NULL
        };

        hr = m_pD2DDCLeftBK->CreateBitmapFromDxgiSurface(m_pDXGISfceLeftBK, &D2DBmpProp, &m_pD2DBmpLeftBK);// 创建位图自DXGI表面

        D2D_SIZE_U D2DSizeU = { 8,8 };
        D2DBmpProp.bitmapOptions = D2D1_BITMAP_OPTIONS_TARGET | D2D1_BITMAP_OPTIONS_GDI_COMPATIBLE;
        m_pD2DDCLeftBK->CreateBitmap(D2DSizeU, NULL, 0, D2DBmpProp, &m_pD2DBmpLeftBK2);// 创建一张内存位图

        SetWindowLongPtrW(hCtrl2, GWLP_WNDPROC, (LONG_PTR)WndProc_LeftBK);
        SendMessageW(hCtrl2, LEFTBKM_INIT, 0, 0);

        ///////////////////////////分隔条
        hCtrl = CreateWindowExW(0, QKCCN_SEPARATEBAR, NULL, WS_CHILD | WS_VISIBLE,
            0, 0, 0, 0, hWnd, (HMENU)IDC_SEB, g_hInst, NULL);
        g_hSEB = hCtrl;
        SendMessageW(hCtrl, QKCSEBM_SETCOLOR, QKCOLOR_WHITE, 0);
        SendMessageW(hCtrl, QKCSEBM_SETCOLOR2, 0x9097D2CB, 0);
        SendMessageW(hCtrl, QKCSEBM_SETPROC, (WPARAM)QKCProc_SEB, 0);
        ///////////////////////////列表容器
		hCtrl = CreateWindowExW(0, WNDCLASS_LIST, NULL, WS_CHILD | WS_VISIBLE | WS_CLIPCHILDREN,
            0, 0, 0, 0,
            hWnd, (HMENU)IDC_BK_LIST, g_hInst, NULL);
        g_hBKList = hCtrl;

        UI_PreProcessAlbumImage(&m_CurrSongInfo.mi.pWICBitmap);
        if (m_CurrSongInfo.mi.pWICBitmap)
        {
            m_pD2DDCLeftBK->CreateBitmapFromWicBitmap(m_CurrSongInfo.mi.pWICBitmap, &m_CurrSongInfo.pD2DBmpOrgAlbum);
            m_pD2DDCLeftBK->CreateBitmapBrush(m_CurrSongInfo.pD2DBmpOrgAlbum, &m_CurrSongInfo.pD2DBrushOrgAlbum);
        }
        UI_RefreshBmpBrush();
        UI_UpdateLeftBK();
	}
	return 0;
	case WM_DESTROY:
	{
        Playing_Stop();
        DestroyWindow(g_hTBGhost);
        if (IsWindow(g_hLrcWnd))
            DestroyWindow(g_hLrcWnd);
        if (g_pITaskbarList)
            g_pITaskbarList->Release();

        m_pD2DBrMyBlue->Release();

        MainWnd_ReleaseCurrInfo();
        Lrc_ClearArray(g_Lrc);
        delete[] m_dwWavesData;

		PostQuitMessage(0);
	}
	return 0;
    case WM_SIZE:
    {
        if (wParam == SIZE_MINIMIZED)
        {
            m_bWndMinimized = TRUE;
            return 0;
        }
        else
            m_bWndMinimized = FALSE;
        cxClient = LOWORD(lParam);
        cyClient = HIWORD(lParam);

		int cxLeft;

        if (g_bListSeped || g_bListHidden)
            cxLeft = cxClient;
        else
        {
            cxLeft = cxClient - g_cxBKList - DPIS_GAP;
            SetWindowPos(g_hSEB, NULL,
                cxLeft,
                0,
                DPIS_GAP,
                cyClient,
                SWP_NOZORDER);

            SetWindowPos(g_hBKList, NULL,
                cxLeft + DPIS_GAP,
                0,
                g_cxBKList,
                cyClient,
                SWP_NOZORDER);

            SendMessageW(g_hSEB, QKCSEBM_SETRANGE, DPI(80), cxClient - DPI(80));
        }

        SetWindowPos(g_hBKLeft, NULL, 0, 0,
            cxLeft,
            cyClient,
            SWP_NOZORDER | SWP_NOMOVE);
    }
    return 0;
    case WM_GETMINMAXINFO:// 限制窗口大小
	{
		LPMINMAXINFO pInfo = (LPMINMAXINFO)lParam;
		pInfo->ptMinTrackSize.x = DPI(450);
		pInfo->ptMinTrackSize.y = DPI(640);
	}
	return 0;
	case WM_DPICHANGED:
	{
		g_iDPI = HIWORD(wParam);
		UI_UpdateDPISize();
        ImageList_Destroy((HIMAGELIST)SendMessageW(g_hLV, LVM_GETIMAGELIST, LVSIL_SMALL, 0));
        SendMessageW(g_hLV, LVM_SETIMAGELIST, LVSIL_SMALL, (LPARAM)ImageList_Create(1, DPIS_CYLVITEM, 0, 1, 0));// ILC_COLOR缺省
		RECT* p = (RECT*)lParam;
		SetWindowPos(hWnd, NULL, p->left, p->top, p->right - p->left, p->bottom - p->top, SWP_NOZORDER);
	}
	return 0;
    case WM_SETTEXT:// 设置标题，转发，否则预览时不会显示标题（鸣谢：nlmhc）
        SetWindowTextW(g_hTBGhost, (PCWSTR)lParam);
        break;
	}
    return DefWindowProcW(hWnd, message, wParam, lParam);
}
LRESULT CALLBACK WndProc_LeftBK(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam)
{
    static int iDelayTime = 0;
    static int iLastIndex = -1;
    static RECT rcLrcSB = { 0 };
    static RECT rcLrcSBThumb = { 0 };
    static int iThumbSize = 80, iSBMax = 0;
    static BOOL bSBLBtnDown = FALSE;
	static int iCursorOffest = 0;// 左键按下时，光标离滑块顶边的距离

    static WCHAR szTime[48] = L"00:00/00:00";
    static int iRepeatMode = REPEATMODE_TOTALLOOP;
    static RECT rcTimeText = { m_rcBtmBK.left,m_rcBtmBK.top,m_rcBtmBK.left + DPIS_CXTIME,m_rcBtmBK.bottom };
    static int iHot = -1, iPushed = -1, iLastHot = -1, iLastOver = -1;
    static BOOL bBTPLPushed = FALSE, bBTIGPushed = FALSE;
    static HWND hToolTip;
    static BOOL bLBTDown = FALSE;
    static TTTOOLINFOW ti = { sizeof(TTTOOLINFOW),0,hWnd,1,{0},g_hInst,NULL,0 };
    static HWND hDlg = NULL;
    static int i;
    static D2D_RECT_F D2DRcTimeLabel = { (float)m_rcBtmBK.left,(float)m_rcBtmBK.top,(float)(m_rcBtmBK.left + DPIS_CXTIME),(float)m_rcBtmBK.bottom };
    static RECT rcTimeLabel = { 0 };

    static RECT rcTrackBar = { 0 };
    static D2D_RECT_F D2DRcTrackBar = { 0 };
    static BOOL bTBLBtnDown = FALSE;
    static UINT uTBMax = 0;
    static UINT uTBPos = 0;
	switch (message)
	{
	case WM_PAINT:
	{
		ValidateRect(hWnd, NULL);
		if (m_pDXGIScLeftBK)
			m_pDXGIScLeftBK->Present(0, 0);
	}
	return 0;
    case WM_SIZE:
    {
        m_cxLeftBK = LOWORD(lParam);
        m_cyLeftBK = HIWORD(lParam);
        /////////////////////////调整窗口，更新绘制矩形
        int xSpe, ySpe, xWaves, yWaves;
        if (m_bLrcShow)
        {
            if (g_bShowAlbum)
            {
                ////////////////制滚动歌词区矩形
                m_rcLrcShow =
                {
                    DPIS_EDGE,
					DPIS_CYTOPBK + DPIS_EDGE * 3 + m_cyAlbum,
                    m_cxLeftBK - DPIS_EDGE,
                    m_cyLeftBK - GC.cyBT - DPIS_CYPROGBAR
                };
                m_cxLrcShow = m_rcLrcShow.right - m_rcLrcShow.left;
                m_cyLrcShow = m_rcLrcShow.bottom - m_rcLrcShow.top;
                m_D2DRcLrcShow = { (float)m_rcLrcShow.left,(float)m_rcLrcShow.top,(float)m_rcLrcShow.right,(float)m_rcLrcShow.bottom };
                
                xSpe = ySpe = 0;
                m_rcSpe = { 0 };
                m_D2DRcSpe = { 0 };
                
                xWaves = yWaves = 0;
                m_rcWaves = { 0 };
                m_D2DRcWaves = { 0 };

                m_rcAlbum =
                {
                    DPIS_EDGE,
                    DPIS_CYTOPBK + DPIS_EDGE * 2,
                    m_cxLeftBK - DPIS_EDGE,
                    DPIS_CYTOPBK + DPIS_EDGE * 2 + m_cyAlbum
                };
                m_D2DRcAlbum = { (float)m_rcAlbum.left,(float)m_rcAlbum.top,(float)m_rcAlbum.right,(float)m_rcAlbum.bottom };
            }
            else
            {
                ////////////////制滚动歌词区矩形
                m_rcLrcShow =
                {
                    DPIS_CXPIC + DPIS_EDGE * 2,
                    DPIS_CYTOPBK + DPIS_EDGE,
                    m_cxLeftBK - DPIS_EDGE,
                    m_cyLeftBK - GC.cyBT - DPIS_CYPROGBAR
                };
                m_cxLrcShow = m_rcLrcShow.right - m_rcLrcShow.left;
                m_cyLrcShow = m_rcLrcShow.bottom - m_rcLrcShow.top;
                m_D2DRcLrcShow = { (float)m_rcLrcShow.left,(float)m_rcLrcShow.top,(float)m_rcLrcShow.right,(float)m_rcLrcShow.bottom };
                ////////////////制频谱区矩形
                xSpe = DPIS_EDGE;
                ySpe = DPIS_CYTOPBK + DPIS_EDGE + DPIS_CXPIC + DPIS_EDGE;
                m_rcSpe = { xSpe,ySpe,xSpe + DPIS_CXSPE,ySpe + DPIS_CYSPE };
                m_D2DRcSpe = { (float)m_rcSpe.left,(float)m_rcSpe.top,(float)m_rcSpe.right,(float)m_rcSpe.bottom };
                ////////////////制波形区矩形
                xWaves = DPIS_EDGE;
                yWaves = m_rcSpe.top + DPIS_EDGE + DPIS_CYSPE;
                m_rcWaves = { xWaves,yWaves,xWaves + DPIS_CXSPE,yWaves + DPIS_CYSPE };
                m_D2DRcWaves = { (float)m_rcWaves.left,(float)m_rcWaves.top,(float)m_rcWaves.right,(float)m_rcWaves.bottom };
            }

            m_LrcHScrollInfo.iIndex = -1;
            m_LrcHScrollInfo.bWndSizeChangedFlag = TRUE;
        }
        else
        {
            if (g_bShowAlbum)
            {
                m_rcAlbum =
                {
                    DPIS_EDGE,
                    DPIS_CYTOPBK + DPIS_EDGE * 2,
                    m_cxLeftBK - DPIS_EDGE,
                    m_cyLeftBK - GC.cyBT - DPIS_CYPROGBAR
                };
                m_D2DRcAlbum = { (float)m_rcAlbum.left,(float)m_rcAlbum.top,(float)m_rcAlbum.right,(float)m_rcAlbum.bottom };
            }
            else
            {
                int cxSpe = (m_cxLeftBK - DPIS_EDGE * 2) / (DPIS_CXSPEBAR + DPIS_CXSPEBARDIV) * (DPIS_CXSPEBAR + DPIS_CXSPEBARDIV);
                if (cxSpe > DPIS_CXSPE)
                    cxSpe = DPIS_CXSPE;
                ////////////////清零滚动歌词区矩形
                m_rcLrcShow = { 0 };
                m_cxLrcShow = 0;
                m_cyLrcShow = 0;
                m_D2DRcLrcShow = { 0 };
                ////////////////制频谱区矩形
                xSpe = (m_cxLeftBK - (cxSpe * 2 + DPIS_EDGE)) / 2;
                ySpe = m_cyLeftBK - GC.cyBT - DPIS_CYPROGBAR - DPIS_CYSPE;
                m_rcSpe = { xSpe,ySpe,xSpe + DPIS_CXSPE,ySpe + DPIS_CYSPE };
                m_D2DRcSpe = { (float)m_rcSpe.left,(float)m_rcSpe.top,(float)m_rcSpe.right,(float)m_rcSpe.bottom };
                ////////////////制波形区矩形
                xWaves = m_rcSpe.left + DPIS_EDGE + DPIS_CXSPE;
                yWaves = m_rcSpe.top;
                m_rcWaves = { xWaves,yWaves,xWaves + DPIS_CXSPE,yWaves + DPIS_CYSPE };
                m_D2DRcWaves = { (float)m_rcWaves.left,(float)m_rcWaves.top,(float)m_rcWaves.right,(float)m_rcWaves.bottom };
            }
        }

        UI_RefreshBmpBrush();

        rcTrackBar = { DPI(6),m_cyLeftBK - GC.cyBT - DPIS_CYPROGBAR,m_cxLeftBK - DPI(6),0 };
        rcTrackBar.bottom = rcTrackBar.top + DPIS_CYPROGBAR;
        D2DRcTrackBar = { (float)rcTrackBar.left,(float)rcTrackBar.top,(float)rcTrackBar.right,(float)rcTrackBar.bottom };

        int xBtmBK = (m_cxLeftBK - GC.cxBKBtm) / 2;
        int yBtmBK = m_cyLeftBK - GC.cyBT;

        m_rcBtmBK = { xBtmBK,yBtmBK,xBtmBK + GC.cxBKBtm,yBtmBK + GC.cyBT };

        rcLrcSB =
        {
            m_cxLeftBK - DPIS_CXLRCTB * 3 / 4,
            m_rcLrcShow.top,
            m_cxLeftBK - DPIS_CXLRCTB / 4,
            m_rcLrcShow.bottom
        };// 制滚动条矩形

        if (iSBMax)
        {
            rcLrcSBThumb.left = rcLrcSB.left;
            rcLrcSBThumb.top = m_rcLrcShow.top + m_iLrcSBPos * (rcLrcSB.bottom - rcLrcSB.top - iThumbSize) / iSBMax;
            rcLrcSBThumb.right = rcLrcSB.right;
            rcLrcSBThumb.bottom = rcLrcSBThumb.top + iThumbSize;
        }

        if (!m_pDXGIScLeftBK || !m_pD2DDCLeftBK)
            return 0;

        m_pD2DDCLeftBK->SetTarget(NULL);// 移除引用
        m_pD2DBmpLeftBK->Release();
        m_pDXGISfceLeftBK->Release();
        HRESULT hr = m_pDXGIScLeftBK->ResizeBuffers(0, 0, 0, DXGI_FORMAT_UNKNOWN, DXGI_SWAP_CHAIN_FLAG_GDI_COMPATIBLE);

        hr = m_pDXGIScLeftBK->GetBuffer(0, IID_PPV_ARGS(&m_pDXGISfceLeftBK));
		D2D1_BITMAP_PROPERTIES1 D2DBmpProp =
		{
			{DXGI_FORMAT_B8G8R8A8_UNORM,D2D1_ALPHA_MODE_PREMULTIPLIED},
			96,
			96,
		    D2D1_BITMAP_OPTIONS_TARGET | D2D1_BITMAP_OPTIONS_CANNOT_DRAW | D2D1_BITMAP_OPTIONS_GDI_COMPATIBLE,
			NULL
		};
        hr = m_pD2DDCLeftBK->CreateBitmapFromDxgiSurface(m_pDXGISfceLeftBK, &D2DBmpProp, &m_pD2DBmpLeftBK);

        m_pD2DBmpLeftBK2->Release();
        D2D_SIZE_U D2DSizeU = { m_cxLeftBK,m_cyLeftBK };
        D2DBmpProp.bitmapOptions = D2D1_BITMAP_OPTIONS_TARGET | D2D1_BITMAP_OPTIONS_GDI_COMPATIBLE;
        m_pD2DDCLeftBK->CreateBitmap(D2DSizeU, NULL, 0, D2DBmpProp, &m_pD2DBmpLeftBK2);

        UI_UpdateLeftBK();
    }
    return 0;
    case WM_LBUTTONDOWN:
    {
        iDelayTime = 5;
        POINT pt = { GET_X_LPARAM(lParam), GET_Y_LPARAM(lParam) };
        if (PtInRect(&m_rcLrcShow, pt))// 在滚动歌词区
            SetFocus(hWnd);
        else if (PtInRect(&rcLrcSB, pt))// 在歌词滚动条区
        {
            if (PtInRect(&rcLrcSBThumb, pt))
            {
                bSBLBtnDown = TRUE;
                iCursorOffest = pt.y - m_rcLrcShow.top - m_iLrcSBPos * (rcLrcSB.bottom - rcLrcSB.top - iThumbSize) / iSBMax;
                SetCapture(hWnd);
            }
        }
        else if (PtInRect(&m_rcBtmBK, pt))// 在底部按钮区
        {
            POINT pt = { GET_X_LPARAM(lParam), GET_Y_LPARAM(lParam) };
            SetCapture(hWnd);
            iPushed = HitTest_BtmBK(pt);
            if (iPushed == -1)
                ReleaseCapture();
            else
				WndProc_LeftBK(hWnd, LEFTBKM_REDRAWBTMBT, TRUE, TRUE);
        }
        else if (PtInRect(&rcTrackBar, pt))// 在进度条区
        {
            bTBLBtnDown = TRUE;
            SetCapture(hWnd);
        }
    }
    return 0;
    case WM_LBUTTONUP:
    {
        static POINT pt;
        pt = { GET_X_LPARAM(lParam), GET_Y_LPARAM(lParam) };

        if (bSBLBtnDown)
        {
            bSBLBtnDown = FALSE;
            ReleaseCapture();
        }
        else if (bTBLBtnDown)
		{
			bTBLBtnDown = FALSE;
			ReleaseCapture();
			if (PtInRect(&rcTrackBar, pt))
			{
				BASS_ChannelSetPosition(
					g_hStream,
					BASS_ChannelSeconds2Bytes(
						g_hStream,
						(float)(pt.x - rcTrackBar.left) * (float)uTBMax / (float)(rcTrackBar.right - rcTrackBar.left) / 100.0f),
					BASS_POS_BYTE
				);// 设置位置
			}
		}
        else if (PtInRect(&m_rcBtmBK, pt))
        {
            if (iPushed == -1)
                return 0;
            i = iPushed;
            iPushed = -1;

            ReleaseCapture();

            if (i != HitTest_BtmBK(pt))
                return 0;
            ti.lpszText = NULL;
            SendMessageW(hToolTip, TTM_GETTOOLINFOW, 0, (LPARAM)&ti);
            SendMessageW(hToolTip, TTM_SETTOOLINFOW, 0, (LPARAM)&ti);
            WndProc_LeftBK(hWnd, LEFTBKM_REDRAWBTMBT, TRUE, TRUE);
            switch (i)
            {
            case 0:// 上一曲
            BTOpe_Last:
            {
                if (g_iCurrFileIndex == -1)
                    return 0;
                Playing_PlayNext(TRUE);
            }
            break;
            case 1:// 播放/暂停
            BTOpe_Play:
            {
                if (g_iCurrFileIndex == -1)
                    return 0;
                DWORD dwState = BASS_ChannelIsActive(g_hStream);
                if (dwState == BASS_ACTIVE_PLAYING)
                    WndProc_LeftBK(hWnd, LEFTBKM_SETPLAYBTICON, TRUE, 0);
                else if (dwState == BASS_ACTIVE_PAUSED)
                    WndProc_LeftBK(hWnd, LEFTBKM_SETPLAYBTICON, FALSE, 0);
            }
            break;
            case 2:// 停止
                if (g_iCurrFileIndex == -1)
                    return 0;
                Playing_Stop();
                break;
            case 3:// 下一曲
            BTOpe_Next:
                if (g_iCurrFileIndex == -1)
                    return 0;
                Playing_PlayNext();
                break;
            case 4:// 歌词
            {
                HMENU hMenu = CreatePopupMenu();
                AppendMenuW(hMenu, IsWindow(g_hLrcWnd) ? MF_CHECKED : 0, IDMI_LRC_SHOW, L"桌面歌词");
                AppendMenuW(hMenu, m_bLrcShow ? MF_CHECKED : 0, IDMI_LRC_LRCSHOW, L"滚动歌词");
                AppendMenuW(hMenu, GS.bForceTwoLines ? MF_CHECKED : 0, IDMI_LRC_FORCETWOLINES, L"禁止自动换行");
                RECT rc;
                GetWindowRect(hWnd, &rc);
                int iRet = TrackPopupMenu(hMenu, TPM_RETURNCMD,
                    rc.left + D2DRcTimeLabel.right + GC.cyBT * 4, rc.top + D2DRcTimeLabel.bottom, 0, g_hMainWnd, NULL);
                DestroyMenu(hMenu);
                switch (iRet)
                {
                case IDMI_LRC_SHOW:
                {
                    LrcWnd_Show();
                }
                break;
                case IDMI_LRC_LRCSHOW:
                {
                    m_bLrcShow = !m_bLrcShow;
                    RECT rc;
                    GetClientRect(g_hBKLeft, &rc);
                    SendMessageW(g_hBKLeft, WM_SIZE, 0, MAKELONG(rc.right, rc.bottom));
                }
                break;
                case IDMI_LRC_FORCETWOLINES:
                {
                    if (GS.bForceTwoLines)
                    {
                        GS.bForceTwoLines = FALSE;
                        KillTimer(g_hMainWnd, IDT_ANIMATION);
                    }
                    else
                        GS.bForceTwoLines = TRUE;

                    m_IsDraw[2] = TRUE;
                    TimerProc(NULL, 0, IDT_DRAWING_LRC, 0);
                }
                break;
                }
            }
            break;
            case 5:// 循环方式
                iRepeatMode++;
                if (iRepeatMode > 4)
                    iRepeatMode %= 5;
                WndProc_LeftBK(hWnd, LEFTBKM_REDRAWBTMBT, TRUE, TRUE);
                goto ShowToolTip;
                break;
            case 6:// 播放设置
                if (!IsWindow(hDlg))
                {
                    hDlg = CreateDialogParamW(g_hInst, MAKEINTRESOURCEW(IDD_EFFECT), hWnd, DlgProc_Effect, 0);
                    ShowWindow(hDlg, SW_SHOW);
                }
                else
                    SetFocus(hDlg);
                break;
            case 7:// 播放列表
            {
                HMENU hMenu = CreatePopupMenu();
                AppendMenuW(hMenu, g_bListSeped ? MF_CHECKED : 0, IDMI_PL_SEPARATE, L"将列表从主窗口拆离");//MF_STRING缺省
                AppendMenuW(hMenu, g_bListHidden ? 0 : MF_CHECKED, IDMI_PL_SHOW, L"显示播放列表");
                RECT rc;
                GetWindowRect(hWnd, &rc);
                int iRet = TrackPopupMenu(hMenu, TPM_RETURNCMD,
                    rc.left + D2DRcTimeLabel.right + GC.cyBT * 7, rc.top + D2DRcTimeLabel.bottom, 0, g_hMainWnd, NULL);
                DestroyMenu(hMenu);
                switch (iRet)
                {
                case IDMI_PL_SEPARATE:
                    UI_SeparateListWnd(!g_bListSeped);
                    break;
                case IDMI_PL_SHOW:
                    UI_ShowList(g_bListHidden);
                    break;
                }
            }
            break;
            case 8:// 设置
                DialogBoxParamW(g_hInst, MAKEINTRESOURCEW(IDD_OPTIONS), g_hMainWnd, DlgProc_Settings, 0);
                break;
            case 9:// 关于
                DialogBoxParamW(g_hInst, MAKEINTRESOURCEW(IDD_ABOUT), g_hMainWnd, DlgProc_About, 0);
                break;
            }
        }
    }
    return 0;
    case WM_MOUSEWHEEL:
    {
        if (!g_Lrc->iCount)
            return 0;

        iDelayTime = 5;
        if (m_iLrcSBPos == -1)
            m_iLrcSBPos = g_iCurrLrcIndex;

        int iDistance = -GET_WHEEL_DELTA_WPARAM(wParam) / WHEEL_DELTA;
        if (m_iLrcSBPos + iDistance < 0)
            m_iLrcSBPos = 0;
        else if (m_iLrcSBPos + iDistance > iSBMax)
            m_iLrcSBPos = iSBMax;
        else
            m_iLrcSBPos = m_iLrcSBPos + iDistance;
        WndProc_LeftBK(hWnd, LEFTBKM_REDRAWSB, FALSE, TRUE);
        m_IsDraw[2] = TRUE;
        UI_VEProcLrcShowing();
        SetTimer(hWnd, IDT_LRCSCROLL, TIMERELAPSE_LRCSCROLL, NULL);
    }
    return 0;
	case WM_TIMER:
	{
		switch (wParam)
		{
		case IDT_LRCSCROLL:
		{
			--iDelayTime;
			if (iDelayTime <= 0)
			{
				KillTimer(hWnd, IDT_LRCSCROLL);
				m_iLrcSBPos = -1;
				WndProc_LeftBK(hWnd, LEFTBKM_REDRAWSB, TRUE, TRUE);
			}
		}
        return 0;
        case IDT_PGS:
        {
			if (!g_hStream)
				return 0;

			if (BASS_ChannelIsActive(g_hStream) == BASS_ACTIVE_STOPPED && g_bPlaying)
			{
				SyncProc_End(NULL, 0, 0, NULL);
				return 0;
			}

            g_fTime = BASS_ChannelBytes2Seconds(
                g_hStream,
                BASS_ChannelGetPosition(g_hStream, BASS_POS_BYTE));

            if (m_bWndMinimized)
            {
                UI_VEProcLrcShowing(FALSE, TRUE);
            }
            else
            {
                m_pD2DDCLeftBK->BeginDraw();
                //////////////////////////画进度提示
                static int iMinOld = -1, iMin2Old = -1;

                int iMin = g_fTime / 60,
                    iMin2 = g_llLength / 1000 / 60;
                if (iMin != iMinOld || iMin2 != iMin2Old)
                {
                    wsprintfW(szTime, L"%02d:%02d/%02d:%02d",
                        iMin,
                        (int)g_fTime - iMin * 60,
                        iMin2,
                        (int)(g_llLength / 1000) - iMin2 * 60);

                    m_pD2DDCLeftBK->DrawBitmap(m_pD2DBmpLeftBK2, &D2DRcTimeLabel, 1.0f, D2D1_BITMAP_INTERPOLATION_MODE_LINEAR, &D2DRcTimeLabel);
                    ID2D1SolidColorBrush* pD2DBrush;
                    m_pD2DDCLeftBK->CreateSolidColorBrush(D2D1::ColorF(D2D1::ColorF::Black), &pD2DBrush);
                    g_pDWTFNormal->SetTextAlignment(DWRITE_TEXT_ALIGNMENT_TRAILING);
                    g_pDWTFNormal->SetParagraphAlignment(DWRITE_PARAGRAPH_ALIGNMENT_CENTER);
                    m_pD2DDCLeftBK->DrawTextW(szTime, lstrlenW(szTime), g_pDWTFNormal, &D2DRcTimeLabel, pD2DBrush, D2D1_DRAW_TEXT_OPTIONS_CLIP);
                    pD2DBrush->Release();
                }
                if (g_bShowAlbum)
                {
                    //////////////////////////画旋转封面
                    UI_VEDrawAlbum(FALSE, FALSE);
                }
                else
                {
                    //////////////////////////画波形
                    BOOL bWavesDraw = UI_VEDrawWaves(FALSE, FALSE);
                    //////////////////////////画频谱
                    UI_VEDrawSpe(FALSE, FALSE);
                }
                //////////////////////////画歌词
                BOOL bLrcDraw = UI_VEProcLrcShowing(FALSE, FALSE);
                //////////////////////////画进度条
                uTBPos = g_fTime * 100;
                WndProc_LeftBK(hWnd, LEFTBKM_REDRAWTRACKBAR, FALSE, FALSE);
                //GDI上屏狗都不用
                //HDC hDC;
                //m_pD2DGdiInteropRTLeftBK->GetDC(D2D1_DC_INITIALIZE_MODE_COPY, &hDC);
                //HDC hDCWnd = GetDC(hWnd);
                //HDC hDCWnd = m_hdcLeftBK;
                //
                //if (bWavesDraw)
                //	BitBlt(hDCWnd, m_rcWaves.left, m_rcWaves.top, DPIS_CXSPE, DPIS_CYSPE, hDC, m_rcWaves.left, m_rcWaves.top, SRCCOPY);// 显示波形
                //BitBlt(hDCWnd, m_rcSpe.left, m_rcSpe.top, DPIS_CXSPE, DPIS_CYSPE, hDC, m_rcSpe.left, m_rcSpe.top, SRCCOPY);// 显示频谱
                //BitBlt(hDCWnd, rcTimeLabel.left, rcTimeLabel.top, DPIS_CXTIME, GC.cyBT, hDC, rcTimeLabel.left, rcTimeLabel.top, SRCCOPY);// 显示进度提示
                //if (bLrcDraw)
                //	BitBlt(hDCWnd, m_rcLrcShow.left, m_rcLrcShow.top, m_cxLrcShow, m_cyLrcShow, hDC, m_rcLrcShow.left, m_rcLrcShow.top, SRCCOPY);// 显示歌词
                //ReleaseDC(hWnd, hDCWnd);
                //m_pD2DGdiInteropRTLeftBK->ReleaseDC(NULL);
                m_pD2DDCLeftBK->EndDraw();
                m_pDXGIScLeftBK->Present(0, 0);
            }
			if (g_pITaskbarList)
				g_pITaskbarList->SetProgressValue(g_hMainWnd, (ULONGLONG)g_fTime * 100, (ULONGLONG)g_llLength / 10);
        }
        return 0;
		}
	}
	return 0;
    case WM_MOUSEMOVE:
    {
        POINT pt = { GET_X_LPARAM(lParam), GET_Y_LPARAM(lParam) };
        static int iLastPos = -1;
        if (m_iLrcFixedIndex != -1)
            return 0;
        if (bSBLBtnDown)
        {
            iDelayTime = 5;
            int i = (pt.y - m_rcLrcShow.top - iCursorOffest) * iSBMax / (rcLrcSB.bottom - rcLrcSB.top - iThumbSize);
            if (i < 0)
                i = 0;
            else if (i > iSBMax)
                i = iSBMax;
            if (iLastPos != i)
            {
                iLastPos = m_iLrcSBPos = i;
                WndProc_LeftBK(hWnd, LEFTBKM_REDRAWSB, FALSE, TRUE);
                UI_VEDrawLrc(m_rcLrcShow.top + m_cyLeftBK / 2);
            }
        }
        else if (m_iLrcCenter >= 0 && m_iLrcFixedIndex == -1)
        {
            if (PtInRect(&m_rcLrcShow, pt))
            {
                int i = HitTest_LrcShow(pt);
                if (i != iLastIndex)
                {
                    m_iLrcMouseHover = i;
                    if (iLastIndex >= 0 && iLastIndex < g_Lrc->iCount)
                        Lrc_DrawItem(iLastIndex, -1, 0, TRUE, TRUE);

                    if (i != -1 && i < g_Lrc->iCount)
                        Lrc_DrawItem(i, -1, 0, TRUE, TRUE);

                    iLastIndex = i;
                }
                iDelayTime = 5;
                TRACKMOUSEEVENT tme;
                tme.cbSize = sizeof(TRACKMOUSEEVENT);
                tme.dwFlags = TME_LEAVE;
                tme.hwndTrack = hWnd;
                tme.dwHoverTime = 0;
                TrackMouseEvent(&tme);
            }
            else
            {
                if (iLastIndex != -1)
                {
                    m_iLrcMouseHover = -1;
                    if (iLastIndex < g_Lrc->iCount)
                        Lrc_DrawItem(iLastIndex, -1, 0, TRUE, TRUE);

                    iLastIndex = -1;
                }
            }
        }

        static BOOL bInBTBK = FALSE;
        if (PtInRect(&m_rcBtmBK, pt))
        {
            bInBTBK = TRUE;
            if (!bLBTDown)
            {
                POINT pt = { GET_X_LPARAM(lParam), GET_Y_LPARAM(lParam) };
                iHot = HitTest_BtmBK(pt);
                if (iLastHot != iHot)
                {
                    ti.lpszText = NULL;
                    SendMessageW(hToolTip, TTM_GETTOOLINFOW, 0, (LPARAM)&ti);
                    SendMessageW(hToolTip, TTM_SETTOOLINFOW, 0, (LPARAM)&ti);
                    iLastHot = iHot;
                    WndProc_LeftBK(hWnd, LEFTBKM_REDRAWBTMBT, TRUE, TRUE);
                }
                TRACKMOUSEEVENT tme;
                tme.cbSize = sizeof(tme);
                tme.hwndTrack = hWnd;
                tme.dwFlags = TME_LEAVE | TME_HOVER;
                tme.dwHoverTime = 200;
                TrackMouseEvent(&tme);
            }
        }
        else
        {
            if (bInBTBK)
            {
                bInBTBK = FALSE;
                iHot = -1;
                WndProc_LeftBK(hWnd, LEFTBKM_REDRAWBTMBT, TRUE, TRUE);
            }
        }
    }
    return 0;
    case WM_MOUSELEAVE:
    {
        if (iHot != -1)
        {
            ti.lpszText = NULL;
            SendMessageW(hToolTip, TTM_GETTOOLINFOW, 0, (LPARAM)&ti);
            SendMessageW(hToolTip, TTM_SETTOOLINFOW, 0, (LPARAM)&ti);
            iLastHot = iHot = -1;
            WndProc_LeftBK(hWnd, LEFTBKM_REDRAWBTMBT, TRUE, TRUE);
        }

        if (m_iLrcFixedIndex != -1)
            return 0;
        iLastIndex = m_iLrcMouseHover = -1;
        m_IsDraw[2] = TRUE;
        TimerProc(NULL, 0, IDT_DRAWING_LRC, 0);
    }
    return 0;
    case WM_MOUSEHOVER:
    {
        if (iHot != -1 && iPushed == -1 && iLastOver != iHot)
        {
        ShowToolTip:
            iLastOver = iHot;
            ti.lpszText = NULL;
            //SendMessageW(hToolTip, TTM_GETTOOLINFOW, 0, (LPARAM)&ti);
            if (iHot == 5)
                ti.lpszText = (LPWSTR)c_szBtmTip[BTMBKBTNCOUNT + iRepeatMode];
            else
                ti.lpszText = (LPWSTR)c_szBtmTip[iHot];
            //SendMessageW(hToolTip, TTM_SETTOOLINFOW, 0, (LPARAM)&ti);
            //SendMessageW(hToolTip, TTM_POPUP, 0, 0);
        }
    }
    return 0;
    case WM_RBUTTONUP:
    {
        POINT pt = { GET_X_LPARAM(lParam), GET_Y_LPARAM(lParam) };
        if (PtInRect(&m_rcLrcShow, pt))
        {
            int i = HitTest_LrcShow(pt);
            if (i != iLastIndex)
            {
                iLastIndex = m_iLrcMouseHover = i;
                m_IsDraw[2] = TRUE;
                TimerProc(NULL, 0, IDT_DRAWING_LRC, 0);
            }
            UINT uFlags = (i == -1) ? MF_GRAYED : 0;
            HMENU hMenu = CreatePopupMenu();
            AppendMenuW(hMenu, uFlags, IDMI_LS_PLAY, L"从此处播放");
            AppendMenuW(hMenu, uFlags, IDMI_LS_COPY, L"复制歌词");
            //SetMenuItemBitmaps()

            ClientToScreen(hWnd, &pt);
            m_iLrcFixedIndex = m_iLrcCenter;
            int iRet = TrackPopupMenu(hMenu, TPM_RETURNCMD, pt.x, pt.y, 0, hWnd, NULL);
            DestroyMenu(hMenu);
            switch (iRet)
            {
            case IDMI_LS_PLAY:// 从此处播放
            {
                BASS_ChannelSetPosition(
                    g_hStream,
                    BASS_ChannelSeconds2Bytes(
                        g_hStream,
                        ((LRCDATA*)QKArray_Get(g_Lrc, i))->fTime),
                    BASS_POS_BYTE
                );
            }
            break;
            case IDMI_LS_COPY:// 复制歌词
            {
                PWSTR pszLrc = ((LRCDATA*)QKArray_Get(g_Lrc, i))->pszLrc;
                HGLOBAL hGlobal = GlobalAlloc(GMEM_MOVEABLE, (lstrlenW(pszLrc) + 1) * sizeof(WCHAR));
                void* pGlobal = GlobalLock(hGlobal);
                lstrcpyW((PWSTR)pGlobal, pszLrc);
                GlobalUnlock(hGlobal);
                if (OpenClipboard(hWnd))
                {
                    EmptyClipboard();
                    SetClipboardData(CF_UNICODETEXT, hGlobal);
                    CloseClipboard();
                }
                else
                {
                    GlobalFree(hGlobal);
                    break;
                }
            }
            break;
            }
            m_iLrcFixedIndex = -1;
            m_IsDraw[2] = TRUE;
            TimerProc(NULL, 0, IDT_DRAWING_LRC, 0);
        }
    }
    return 0;
    case LEFTBKM_REDRAWSB:
    {
		if (iSBMax)
		{
			rcLrcSBThumb.left = rcLrcSB.left;
			rcLrcSBThumb.top = m_rcLrcShow.top + m_iLrcSBPos * (rcLrcSB.bottom - rcLrcSB.top - iThumbSize) / iSBMax;
			rcLrcSBThumb.right = rcLrcSB.right;
			rcLrcSBThumb.bottom = rcLrcSBThumb.top + iThumbSize;

			if (lParam)
				m_pD2DDCLeftBK->BeginDraw();

			D2D_RECT_F D2DRectF1 = { rcLrcSB.left,rcLrcSB.top,rcLrcSB.right,rcLrcSB.bottom },
				D2DRectF2 = { rcLrcSBThumb.left,rcLrcSBThumb.top,rcLrcSBThumb.right,rcLrcSBThumb.bottom };
			m_pD2DDCLeftBK->DrawBitmap(m_pD2DBmpLeftBK2, &D2DRectF1, 1.0f, D2D1_BITMAP_INTERPOLATION_MODE_LINEAR, &D2DRectF1);

			if (m_iLrcSBPos != -1)
				m_pD2DDCLeftBK->FillRectangle(&D2DRectF2, m_pD2DBrMyBlue);


			//ID2D1GdiInteropRenderTarget* pD2DGdiInteropRT;
			//m_pD2DDCLeftBK->QueryInterface(IID_PPV_ARGS(&pD2DGdiInteropRT));// 取GDI兼容渲染目标
			//HDC hDC;
			//HRESULT hr = pD2DGdiInteropRT->GetDC(D2D1_DC_INITIALIZE_MODE_COPY, &hDC);
			//HDC hDCWnd = GetDC(g_hBKLeft);
			//BitBlt(hDCWnd, rcLrcSB.left, rcLrcSB.top, rcLrcSB.right - rcLrcSB.left, rcLrcSB.bottom - rcLrcSB.top,
			//    hDC, rcLrcSB.left, rcLrcSB.top, SRCCOPY);// 区域显示
			//ReleaseDC(g_hBKLeft, hDCWnd);
			//pD2DGdiInteropRT->ReleaseDC(NULL);
			//pD2DGdiInteropRT->Release();
			if (lParam)
				m_pD2DDCLeftBK->EndDraw();
			if (wParam)
				m_pDXGIScLeftBK->Present(0, 0);
		}
    }
    return 0;
    case LEFTBKM_SETMAX:
    {
        iSBMax = wParam;
        WndProc_LeftBK(hWnd, LEFTBKM_REDRAWSB, TRUE, TRUE);
    }
    return 0;
    case WM_KEYDOWN:
    {
        if (wParam == VK_ESCAPE)
        {
            KillTimer(hWnd, IDT_LRCSCROLL);
            m_iLrcSBPos = -1;
            WndProc_LeftBK(hWnd, LEFTBKM_REDRAWSB, TRUE, TRUE);
            return 0;
        }
    }
    break;
    case LEFTBKM_REDRAWBTMBT:
    {
		HDC hDC;
		if (lParam)
			m_pD2DDCLeftBK->BeginDraw();

		D2D_RECT_F D2DRectF;
		if (m_rcBtmBK.right - m_rcBtmBK.left > m_cxLeftBK)
			D2DRectF = { 0,(float)m_rcBtmBK.top,(float)m_cxLeftBK,(float)m_rcBtmBK.bottom };
		else
			D2DRectF = { (float)m_rcBtmBK.left,(float)m_rcBtmBK.top,(float)m_rcBtmBK.right,(float)m_rcBtmBK.bottom };
		m_pD2DDCLeftBK->DrawBitmap(m_pD2DBmpLeftBK2, &D2DRectF, 1.0f, D2D1_BITMAP_INTERPOLATION_MODE_LINEAR, &D2DRectF);
		ID2D1SolidColorBrush* pD2DBrush;
		m_pD2DDCLeftBK->CreateSolidColorBrush(D2D1::ColorF(D2D1::ColorF::Black), &pD2DBrush);

		g_pDWTFNormal->SetTextAlignment(DWRITE_TEXT_ALIGNMENT_TRAILING);
		g_pDWTFNormal->SetParagraphAlignment(DWRITE_PARAGRAPH_ALIGNMENT_CENTER);
        D2DRcTimeLabel = { (float)m_rcBtmBK.left,(float)m_rcBtmBK.top,(float)(m_rcBtmBK.left + DPIS_CXTIME),(float)m_rcBtmBK.bottom };
        rcTimeLabel = { m_rcBtmBK.left,m_rcBtmBK.top,m_rcBtmBK.left + DPIS_CXTIME,m_rcBtmBK.bottom };
        m_pD2DDCLeftBK->DrawTextW(szTime, lstrlenW(szTime), g_pDWTFNormal, &D2DRcTimeLabel, pD2DBrush, D2D1_DRAW_TEXT_OPTIONS_CLIP);

        D2D_RECT_F D2DRectF2;
        
        int iIconOffest = (GC.cyBT - GC.iIconSize) / 2;
        int x = D2DRcTimeLabel.right, y = m_rcBtmBK.top + iIconOffest;
        RECT rc = { 0,0,0,GC.cyBT };
        if (iHot != -1 || iPushed != -1)
        {
            int i;
            if (iPushed != -1)
            {
                i = iPushed;
                pD2DBrush->SetColor(D2D1::ColorF(QKGDIClrToCommonClr(MYCLR_BTPUSHED)));
            }
            else
            {
                i = iHot;
                pD2DBrush->SetColor(D2D1::ColorF(QKGDIClrToCommonClr(MYCLR_BTHOT)));
            }

            D2DRectF2.left = x + GC.cyBT * i;
            D2DRectF2.top = D2DRectF.top;
            D2DRectF2.right = D2DRectF2.left + GC.cyBT;
            D2DRectF2.bottom = D2DRectF.bottom;
            m_pD2DDCLeftBK->FillRectangle(&D2DRectF2, pD2DBrush);
        }

        m_pD2DGdiInteropRTLeftBK->GetDC(D2D1_DC_INITIALIZE_MODE_COPY, &hDC);
        DrawIconEx(hDC, x + iIconOffest, y, GR.hiLast, 0, 0, 0, NULL, DI_NORMAL);// 1 上一曲
        x += GC.cyBT;
        DrawIconEx(hDC, x + iIconOffest, y, g_bPlayIcon ? GR.hiPlay : GR.hiPause, 0, 0, 0, NULL, DI_NORMAL);// 2 播放/暂停
        x += GC.cyBT;
        DrawIconEx(hDC, x + iIconOffest, y, GR.hiStop, 0, 0, 0, NULL, DI_NORMAL);// 3 停止
        x += GC.cyBT;
        DrawIconEx(hDC, x + iIconOffest, y, GR.hiNext, 0, 0, 0, NULL, DI_NORMAL);// 4 下一曲
        x += GC.cyBT;
        DrawIconEx(hDC, x + iIconOffest, y, GR.hiLrc, 0, 0, 0, NULL, DI_NORMAL);// 5 歌词
        x += GC.cyBT;
        HICON hi;
        switch (iRepeatMode)
        {
        case REPEATMODE_TOTALLOOP:
            hi = GR.hiArrowCircle;
            break;
        case REPEATMODE_RADOM:
            hi = GR.hiArrowCross;
            break;
        case REPEATMODE_SINGLE:
            hi = GR.hiArrowRight;
            break;
        case REPEATMODE_SINGLELOOP:
            hi = GR.hiArrowCircleOne;
            break;
        case REPEATMODE_TOTAL:
            hi = GR.hiArrowRightThree;
            break;
        default:
            hi = GR.hiArrowCircle;
            break;
        }

        DrawIconEx(hDC, x + iIconOffest, y, hi, 0, 0, 0, NULL, DI_NORMAL);// 6 循环方式
        x += GC.cyBT;

        DrawIconEx(hDC, x + iIconOffest, y, GR.hiPlaySetting, 0, 0, 0, NULL, DI_NORMAL);// 7 均衡器
        x += GC.cyBT;
        HBRUSH hBrush;
        if (bBTPLPushed)
        {
            hBrush = CreateSolidBrush(MYCLR_BTPUSHED);
            rc.left = x;
            rc.right = rc.left + GC.cyBT;
            FillRect(hDC, &rc, hBrush);
            DeleteObject(hBrush);
        }
        DrawIconEx(hDC, x + iIconOffest, y, GR.hiPlayList, 0, 0, 0, NULL, DI_NORMAL);// 8 显示播放列表
        x += GC.cyBT;
        if (bBTPLPushed)
        {
            hBrush = CreateSolidBrush(MYCLR_BTPUSHED);
            rc.left = x;
            rc.right = rc.left + GC.cyBT;
            FillRect(hDC, &rc, hBrush);
            DeleteObject(hBrush);
        }
        DrawIconEx(hDC, x + iIconOffest, y, GR.hiSettings, 0, 0, 0, NULL, DI_NORMAL);// 9 设置
        x += GC.cyBT;

        DrawIconEx(hDC, x + iIconOffest, y, GR.hiInfo, 0, 0, 0, NULL, DI_NORMAL);// 10 关于
        x += GC.cyBT;
        if (wParam)
        {
			HDC hDCWnd = GetDC(g_hBKLeft);
			BitBlt(hDCWnd, m_rcBtmBK.left, m_rcBtmBK.top, GC.cxBKBtm, GC.cyBT, hDC, m_rcBtmBK.left, m_rcBtmBK.top, SRCCOPY);
			ReleaseDC(g_hBKLeft, hDCWnd);
		}

		if (m_rcBtmBK.right - m_rcBtmBK.left > m_cxLeftBK)// 左边太小会导致DC释放失败（矩形超界）
		{
			RECT rc = { 0,m_rcBtmBK.top,m_cxLeftBK,m_rcBtmBK.bottom };
			m_pD2DGdiInteropRTLeftBK->ReleaseDC(&rc);
		}
		else
			m_pD2DGdiInteropRTLeftBK->ReleaseDC(&m_rcBtmBK);

		if (lParam)
			m_pD2DDCLeftBK->EndDraw();
    }
    return 0;
    case LEFTBKM_REDRAWTRACKBAR:
    {
        if (lParam)
            m_pD2DDCLeftBK->BeginDraw();
        m_pD2DDCLeftBK->DrawBitmap(m_pD2DBmpLeftBK2, D2DRcTrackBar, 1.0f, D2D1_BITMAP_INTERPOLATION_MODE_LINEAR, &D2DRcTrackBar);
        if (uTBMax)
        {
            D2D_RECT_F D2DRcF;
			D2DRcF.left = D2DRcTrackBar.left;
			D2DRcF.top = D2DRcTrackBar.top + (DPIS_CYPROGBAR - GC.DS_CYPROGBARCORE) / 2;
			D2DRcF.bottom = D2DRcF.top + GC.DS_CYPROGBARCORE;
			float fWidth = D2DRcTrackBar.right - D2DRcTrackBar.left;
			D2DRcF.right = D2DRcF.left + uTBPos * fWidth / uTBMax;
			if (D2DRcF.right - D2DRcF.left > fWidth)
				goto NoPgs;
			m_pD2DDCLeftBK->FillRectangle(&D2DRcF, m_pD2DBrMyBlue);
			D2DRcF.left = D2DRcF.right;
			D2DRcF.right = D2DRcTrackBar.right;
			ID2D1SolidColorBrush* pD2DBrush;
			m_pD2DDCLeftBK->CreateSolidColorBrush(D2D1::ColorF(D2D1::ColorF::Gray, 0.7f), &pD2DBrush);
			m_pD2DDCLeftBK->FillRectangle(&D2DRcF, pD2DBrush);
			pD2DBrush->Release();
		}
		else
        {
        NoPgs:
            D2D_RECT_F D2DRcF;
            D2DRcF.left = D2DRcTrackBar.left;
            D2DRcF.top = D2DRcTrackBar.top + (DPIS_CYPROGBAR - GC.DS_CYPROGBARCORE) / 2;
            D2DRcF.bottom = D2DRcF.top + GC.DS_CYPROGBARCORE;
            D2DRcF.right = D2DRcTrackBar.right;

            ID2D1SolidColorBrush* pD2DBrush;
            m_pD2DDCLeftBK->CreateSolidColorBrush(D2D1::ColorF(D2D1::ColorF::Gray, 0.7f), &pD2DBrush);
            m_pD2DDCLeftBK->FillRectangle(&D2DRcF, pD2DBrush);
            pD2DBrush->Release();
        }

        if (lParam)
            m_pD2DDCLeftBK->EndDraw();
        if (wParam)
            m_pDXGIScLeftBK->Present(0, 0);
    }
    return 0;
    case LEFTBKM_SETPROGBARPOS:
    {
        uTBPos = wParam;
        if (lParam)
            WndProc_LeftBK(hWnd, LEFTBKM_REDRAWTRACKBAR, TRUE, TRUE);
    }
    return 0;
    case LEFTBKM_GETPROGBARPOS:
        return uTBPos;
    case LEFTBKM_SETPROGBARMAX:
    {
        uTBMax = wParam;
        if (lParam)
            WndProc_LeftBK(hWnd, LEFTBKM_REDRAWTRACKBAR, TRUE, TRUE);
    }
    return 0;
    case LEFTBKM_GETPROGBARMAX:
        return uTBMax;
    case LEFTBKM_INIT:
    {
        hToolTip = CreateWindowExW(0, TOOLTIPS_CLASSW, NULL, 0, 0, 0, 0, 0, hWnd, NULL, NULL, NULL);
        //ShowWindow(hToolTip, SW_SHOWNOACTIVATE);
        RECT rc;
        GetClientRect(hWnd, &rc);
        ti.rect = rc;
        ti.uFlags = TTF_SUBCLASS;
        ti.hwnd = hWnd;
        ti.lpszText = (PWSTR)L"11111";
        SendMessageW(hToolTip, TTM_ADDTOOLW, 0, (LPARAM)&ti);
        //SendMessageW(hToolTip, TTM_POP, 0, 0);
    }
    return 0;
    case LEFTBKM_GETREPEATMODE:
        return iRepeatMode;
    case LEFTBKM_SETPLAYBTICON:
    {
        g_bPlayIcon = wParam;
        if (g_bPlayIcon)
        {
            THUMBBUTTON tb;
            tb.dwMask = THB_ICON | THB_TOOLTIP;
            tb.hIcon = GR.hiPlay2;
            tb.iId = IDTBB_PLAY;
            lstrcpyW(tb.szTip, L"播放");
            if (g_pITaskbarList)
                g_pITaskbarList->ThumbBarUpdateButtons(g_hTBGhost, 1, &tb);
            BASS_ChannelPause(g_hStream);
            KillTimer(hWnd, IDT_PGS);
        }
        else
        {
            THUMBBUTTON tb;
            tb.dwMask = THB_ICON | THB_TOOLTIP;
            tb.hIcon = GR.hiPause2;
            tb.iId = IDTBB_PLAY;
            lstrcpyW(tb.szTip, L"暂停");
            if (g_pITaskbarList)
                g_pITaskbarList->ThumbBarUpdateButtons(g_hTBGhost, 1, &tb);
            BASS_ChannelPlay(g_hStream, FALSE);
            SetTimer(hWnd, IDT_PGS, TIMERELAPSE_PGS, NULL);
        }
        WndProc_LeftBK(hWnd, LEFTBKM_REDRAWBTMBT, TRUE, TRUE);
        LrcWnd_DrawLrc();
    }
    return 0;
    case LEFTBKM_DOBTOPE:
        switch (wParam)
        {
        case 0:goto BTOpe_Last;
        case 1:goto BTOpe_Play;
        case 2:goto BTOpe_Next;
        }
        return 0;
    case WM_DESTROY:
        DestroyWindow(hToolTip);
        return 0;
    }
    return DefWindowProcW(hWnd, message, wParam, lParam);
}
INT_PTR CALLBACK DlgProc_License(HWND hDlg, UINT message, WPARAM wParam, LPARAM lParam)
{
    switch (message)
    {
    case WM_INITDIALOG:
	{
		HRSRC hResInfo = FindResourceW(g_hInst, MAKEINTRESOURCEW(IDR_LICENSE), L"BIN");
		HGLOBAL hRes = LoadResource(g_hInst, hResInfo);
		if (hRes)
		{
			PCCH p = (PCCH)LockResource(hRes);
			DWORD dwLength = SizeofResource(g_hInst, hResInfo);
			int iBufferSize = MultiByteToWideChar(CP_UTF8, 0, p, dwLength, NULL, 0);
			if (!iBufferSize)
				return NULL;
			PWSTR pBuffer = new WCHAR[iBufferSize + 1];
			MultiByteToWideChar(CP_UTF8, 0, p, dwLength, pBuffer, iBufferSize);
			*(pBuffer + iBufferSize) = L'\0';// 添加结尾NULL
			SetDlgItemTextW(hDlg, IDC_ED_LICENSE, pBuffer);
			delete[] pBuffer;
		}

		RECT rc;
		GetClientRect(hDlg, &rc);
		SetWindowPos(GetDlgItem(hDlg, IDC_ED_LICENSE), NULL, 0, 0, rc.right, rc.bottom, SWP_NOZORDER);
	}
	return TRUE;
	case WM_SIZE:
		SetWindowPos(GetDlgItem(hDlg, IDC_ED_LICENSE), NULL, 0, 0, LOWORD(lParam), HIWORD(lParam), SWP_NOZORDER);
		return TRUE;
    case WM_CLOSE:
        EndDialog(hDlg, 0);
        return TRUE;
	}
	return FALSE;
}
INT_PTR CALLBACK DlgProc_About(HWND hDlg, UINT message, WPARAM wParam, LPARAM lParam)
{
    static HBITMAP hBitmap;
    static HDC hCDC;
    static int cx0, cy0, cx, cy;
    switch (message)
    {
    case WM_INITDIALOG:
    {
        //////////////////////初始化图片
        hBitmap = (HBITMAP)LoadImageW(g_hInst, MAKEINTRESOURCEW(IDB_ABOUT), IMAGE_BITMAP, 0, 0, 0);
        BITMAP bmp;
        GetObjectW(hBitmap, sizeof(bmp), &bmp);
        RECT rc;
        GetClientRect(hDlg, &rc);

        cx0 = bmp.bmWidth;
        cy0 = bmp.bmHeight;
        cx = rc.right;
		cy = cx * cy0 / cx0;// 计算缩放尺寸

		hCDC = CreateCompatibleDC(NULL);
		SelectObject(hCDC, hBitmap);
        //////////////////////初始化对话框内容
        SetDlgItemTextW(hDlg, IDC_ED_MYQQ, L"639582106");// 我的QQ
        SetDlgItemTextW(hDlg, IDC_ED_BASSWEBSITE, L"www.un4seen.com");// Un4seen网址

		HRSRC hResInfo = FindResourceW(g_hInst, MAKEINTRESOURCEW(IDR_COMPILETIME), L"BIN");
		HGLOBAL hRes = LoadResource(g_hInst, hResInfo);
		if (hRes)
			SetDlgItemTextW(hDlg, IDC_ST_COMPILETIME, (PCWSTR)LockResource(hRes));// 编译时间

		WCHAR szCompileCount[48];
        hResInfo = FindResourceW(g_hInst, MAKEINTRESOURCEW(IDR_COMPILECOUNT), L"BIN");
        hRes = LoadResource(g_hInst, hResInfo);
        if (hRes)
        {
            wsprintfW(szCompileCount, L"%d", *(INT32*)LockResource(hRes));
            SetDlgItemTextW(hDlg, IDC_ST_COMPILECOUNT, szCompileCount);// 累计编译次数
        }

        SetDlgItemTextW(hDlg, IDC_ST_PROGVER, CURRVER);// 当前版本
	}
	return FALSE;
	case WM_CLOSE:
		EndDialog(hDlg, NULL);
		return TRUE;
	case WM_DESTROY:
		DeleteDC(hCDC);
		DeleteObject(hBitmap);
		return TRUE;
	case WM_COMMAND:
	{
        switch (LOWORD(wParam))
        {
        case IDC_BT_OK:
            EndDialog(hDlg, NULL);
            return TRUE;
        case IDC_BT_LICENSE:
            DialogBoxParamW(g_hInst, MAKEINTRESOURCEW(IDD_LICENSE), hDlg, DlgProc_License, 0);
            return TRUE;
        case IDC_BT_GITHUB:
            ShellExecuteW(NULL, L"open", L"https://github.com/QingKong-s/Player/", NULL, NULL, SW_SHOW);
            return TRUE;
        }
    }
    return FALSE;
    case WM_PAINT:
    {
        PAINTSTRUCT ps;
        HDC hDC = BeginPaint(hDlg, &ps);
        SetStretchBltMode(hDC, HALFTONE);// 置拉伸模式
        StretchBlt(hDC, 0, 0, cx, cy, hCDC, 0, 0, cx0, cy0, SRCCOPY);
        EndPaint(hDlg, &ps);
    }
    return TRUE;
    }
    return FALSE;
}
void CALLBACK TimerProc(HWND hWnd, UINT uMsg, UINT_PTR idEvent, DWORD dwTime)
{
	switch (idEvent)
	{
	case IDT_ANIMATION:
    {
		if (m_LrcHScrollInfo.iIndex == -1 || g_iCurrLrcIndex < 0)
            return;
        LRCDATA* p = (LRCDATA*)QKArray_Get(g_Lrc, g_iCurrLrcIndex);
        float fLastTime = g_fTime - p->fTime;
        static int iLastx1 = 0, iLastx2 = 0;// 上次左边，如果跟上次一样就不要再画了
        int ii;
        BOOL bRedraw = FALSE;
        if (m_LrcHScrollInfo.cx1 != -1)
        {
            if (fLastTime > m_LrcHScrollInfo.fNoScrollingTime1)
            {
                if (fLastTime < p->fDelay - m_LrcHScrollInfo.fNoScrollingTime1)
                {
                    ii = m_cxLrcShow / 2 - fLastTime * m_LrcHScrollInfo.cx1 / p->fDelay;
                    if (ii != iLastx1)
                    {
                        iLastx1 = m_LrcHScrollInfo.x1 = ii;
                        bRedraw = TRUE;
                    }
                }
                else
                {
                    ii = m_cxLrcShow - m_LrcHScrollInfo.cx1;
                    if (ii != iLastx1)
                    {
                        iLastx1 = m_LrcHScrollInfo.x1 = ii;
                        bRedraw = TRUE;
                    }
                }
            }
            else
            {
                if (iLastx1 != 0)
                {
                    iLastx1 = m_LrcHScrollInfo.x1 = 0;
                    bRedraw = TRUE;
                }
            }
        }

        if (m_LrcHScrollInfo.cx2 != -1)
        {
            if (fLastTime > m_LrcHScrollInfo.fNoScrollingTime2)
            {
                if (fLastTime < p->fDelay - m_LrcHScrollInfo.fNoScrollingTime2)
                {
                    ii = m_cxLrcShow / 2 - fLastTime * m_LrcHScrollInfo.cx2 / p->fDelay;
                    if (ii != iLastx2)
                    {
                        iLastx2 = m_LrcHScrollInfo.x2 = ii;
                        bRedraw = TRUE;
                    }
                }
                else
                {
                    ii = m_cxLrcShow - m_LrcHScrollInfo.cx2;
                    if (ii != iLastx2)
                    {
                        iLastx2 = m_LrcHScrollInfo.x2 = ii;
                        bRedraw = TRUE;
                    }
                }
            }
            else
            {
                if (iLastx2 != 0)
                {
                    iLastx2 = m_LrcHScrollInfo.x2 = 0;
                    bRedraw = TRUE;
                }
            }
        }

        if (bRedraw)
            Lrc_DrawItem(g_iCurrLrcIndex, -1, 0, TRUE, TRUE);
    }
    return;
    case IDT_ANIMATION2:
    {
        if (m_iLrcSBPos != -1 || m_iLrcCenter == -1)
        {
            m_IsDraw[2] = TRUE;
            UI_VEProcLrcShowing();
            KillTimer(g_hMainWnd, IDT_ANIMATION2);
            return;
        }
        int iTop;
        static int iLastTop = 0x80000000;
        float fLastTime = g_fTime - m_LrcVScrollInfo.fTime;
        if (m_LrcVScrollInfo.bDirection)
        {
            iTop = m_LrcVScrollInfo.iSrcTop - fLastTime * m_LrcVScrollInfo.iDistance / m_LrcVScrollInfo.fDelay;
            if (iTop <= m_LrcVScrollInfo.iDestTop)
            {
                m_IsDraw[2] = TRUE;
                UI_VEProcLrcShowing();
                KillTimer(g_hMainWnd, IDT_ANIMATION2);
                return;
            }
            else if (iTop != iLastTop)
            {
                iLastTop = iTop;
            }
            else
                return;
        }
        else
        {
            iTop = m_LrcVScrollInfo.iSrcTop + fLastTime * m_LrcVScrollInfo.iDistance / m_LrcVScrollInfo.fDelay;
            if (iTop >= m_LrcVScrollInfo.iDestTop)
            {
                m_IsDraw[2] = TRUE;
                UI_VEProcLrcShowing();
                KillTimer(g_hMainWnd, IDT_ANIMATION2);
                return;
            }
            else if (iTop != iLastTop)
            {
                iLastTop = iTop;
            }
            else
                return;
        }

        UI_VEDrawLrc(iTop);
    }
    return;
    }
}
LRESULT CALLBACK QKCProc_SEB(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
    if (uMsg == QKCSEPN_DRAGEND)
    {
        RECT rc;
        GetClientRect(g_hMainWnd, &rc);
        g_cxBKList = rc.right - wParam - DPIS_GAP;
        UI_SetRitCtrlPos();
        SendMessageW(g_hMainWnd, WM_SIZE, 0, MAKELONG(rc.right, rc.bottom));
    }
    return 0;
}
int HitTest_BtmBK(POINT pt)
{
    if (!PtInRect(&m_rcBtmBK,pt))
        return -1;

	for (int i = 0; i < BTMBKBTNCOUNT; i++)
    {
        if (pt.x > m_rcBtmBK.left + DPIS_CXTIME + i * GC.cyBT && pt.x < m_rcBtmBK.left + DPIS_CXTIME + (i + 1) * GC.cyBT)
            return i;
    }
    return -1;
}
int HitTest_LrcShow(POINT pt)
{
    if (!g_Lrc->iCount || !g_hStream)
        return -1;

    LRCDATA* p = (LRCDATA*)QKArray_Get(g_Lrc, m_iLrcCenter);
    int i = m_iLrcCenter;
    if (pt.y >= p->rcItem.top)// 落在下半部分（包括中间一项）
    {
        while (p->iDrawID == m_iDrawingID)
        {
            if (PtInRect(&p->rcItem, pt))
                return i;
            ++i;
            if (i >= g_Lrc->iCount)
                break;
            p = (LRCDATA*)QKArray_Get(g_Lrc, i);
        }
    }
    else// 落在上半部分
    {
        --i;// 中间一项就不判断了，跳过
        if (i < 0)
            return -1;
        p = (LRCDATA*)QKArray_Get(g_Lrc, i);
        while (p->iDrawID == m_iDrawingID)
        {
            if (PtInRect(&p->rcItem, pt))
                return i;
            --i;
            if (i < 0)
                break;
            p = (LRCDATA*)QKArray_Get(g_Lrc, i);
        }
    }

    return -1;
}