﻿#include "WndMain.h"

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

#include <math.h>
#include <stdio.h>

#include "bass.h"

#include "GlobalVar.h"
#include "Function.h"
#include "Resource.h"
#include "MyProject.h"
#include "QKCtrl.h"
#include "WndLrc.h"
#include "OLEDragDrop.h"
#include "WndEffect.h"
#include "WndList.h"

CURRMUSICINFO       m_CurrSongInfo = { 0 };        //当前信息（在顶部显示）

HBITMAP         m_hbmpLeftBK                = NULL;
HBITMAP         m_hbmpLeftBK2               = NULL;
HBITMAP         m_hbmpBookMark              = NULL;

HDC             m_hcdcLeftBK                = NULL;         // 原始（除了封面和顶部提示信息外没有其他视觉元素）
HDC             m_hcdcLeftBK2               = NULL;         // 所有绘制工作均已完成
HDC             m_hcdcBookMark              = NULL;


DWORD           m_uThreadFlagWaves          = THREADFLAG_STOP;  //线程工作状态标志
HANDLE          m_htdWaves                  = NULL;         //线程句柄
DWORD*          m_dwWavesData               = NULL;         //指向波形信息数组，左声道：低WORD，右声道：高WORD
DWORD           m_dwWavesDataCount          = 0;            //波形计数

BOOL            m_IsDraw[3]                 = { 0 };        //绘制标志
DRAWING_TIME    m_TimeStru_VU[2]            = { 0 };        //延迟下落
DRAWING_TIME    m_TimeStru_Spe[128]    = { 0 };             //延迟下落

RECT            m_LeftStaticRect            = { 0 };        //右侧静态客户区



BOOL            m_bLrcShow                  = TRUE;
RECT            m_rcLrcShow                 = { 0 };


int             m_xSpe                      = 0,
                m_ySpe                      = 0,
                m_xBtmBK                    = 0,
                m_yBtmBK                    = 0,
                m_xWaves                    = 0,
                m_yWaves                    = 0,
                m_cxLeftBK                  = 0,
                m_cyLeftBK                  = 0,
                m_cxLrcShow                 = 0,
                m_cyLrcShow                 = 0;

int             m_iLrcSBPos                 = -1;
BOOL            m_bLockLrcScroll            = FALSE;

int             m_iLrcCenter                = -1;
int             m_iLrcMouseHover            = -1;
int             m_iLrcFixedIndex            = -1;

LRCHSCROLLINFO  m_LrcHScrollInfo            = { -1 };// 歌词水平滚动信息；仅适用于当前播放行
LRCVSCROLLINFO  m_LrcVScrollInfo            = { 0 };

HWND            m_hTBGhost                  = NULL;

int             m_iDrawingID                = 0;

int             m_iLastLrcIndex[2]          = { -1,-1 };// 0：上次中心；1：上次高亮

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


void MainWnd_ReleaseCurrInfo()
{
	delete[] m_CurrSongInfo.pszName;
	MusicInfo_Release(&m_CurrSongInfo.mi);
    ZeroMemory(&m_CurrSongInfo, sizeof(CURRMUSICINFO));
}
void CALLBACK SyncProc_End(HSYNC handle, DWORD channel, DWORD data, void* user)
{
    SendMessageW(g_hTBProgess, QKCTBM_SETPOS, TRUE, (LPARAM)SendMessageW(g_hTBProgess, QKCTBM_GETRANGE, 0, 0));
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
int Lrc_DrawItem(int iIndex, int y, BOOL bTop, BOOL bClearBK, BOOL bImmdShow)
{
	BOOL bCurr = (iIndex == g_iCurrLrcIndex);
	if (bCurr)
		SetTextColor(m_hcdcLeftBK2, QKCOLOR_RED);
	else
		SetTextColor(m_hcdcLeftBK2, QKCOLOR_CYANDEEPER);

	if (iIndex == m_iLrcCenter)
		SelectObject(m_hcdcLeftBK2, g_hFontCenterLrc);
	else
		SelectObject(m_hcdcLeftBK2, g_hFontDrawing);
	LRCDATA* p = (LRCDATA*)QKArray_Get(g_Lrc, iIndex);
	if (y == -1)
	{
		if (p->iDrawID != m_iDrawingID)
			return -1;
        y = p->iLastTop;
        bTop = TRUE;
    }

	RECT rc;
	RECT rc2;
	UINT uFlags;
	int iHeight, iHeight2;
    int cx1, cx2;
	if (GS.bForceTwoLines)
	{
		uFlags = DT_NOPREFIX | DT_CALCRECT;
        rc = m_rcLrcShow;
        if (p->iOrgLength == -1)
        {
            iHeight = DrawTextW(m_hcdcLeftBK2, p->pszLrc, -1, &rc, uFlags);
            cx1 = rc.right - rc.left;
            if (bCurr)
            {
                if (iIndex != m_LrcHScrollInfo.iIndex)
                {
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
            }

            if (cx1 > m_cxLrcShow)
                uFlags = DT_NOPREFIX;
            else
                uFlags = DT_NOPREFIX | DT_CENTER;

            p->cy = iHeight;

            if (bTop)
            {
                rc2 = { m_rcLrcShow.left,y,m_rcLrcShow.right,y + p->cy };
                
                if (bClearBK)
                    BitBlt(m_hcdcLeftBK2, rc2.left, rc2.top, rc2.right - rc2.left, rc2.bottom - rc2.top, m_hcdcLeftBK, rc2.left, rc2.top, SRCCOPY);

                if (bImmdShow)
                {
                    HRGN hRgn1 = CreateRectRgnIndirect(&rc2);
                    HRGN hRgn2 = CreateRectRgnIndirect(&m_rcLrcShow);
                    CombineRgn(hRgn1, hRgn1, hRgn2, RGN_AND);
                    SelectClipRgn(m_hcdcLeftBK2, hRgn1);// 设置剪辑区
                    DeleteObject(hRgn1);
                    DeleteObject(hRgn2);
                }

				rc = rc2;
				if (bCurr)
					rc.left += m_LrcHScrollInfo.x1;
				p->iLastTop = rc.top;
				DrawTextW(m_hcdcLeftBK2, p->pszLrc, -1, &rc, uFlags);// 绘制
            }
            else
            {
                rc2 = { m_rcLrcShow.left,y - p->cy,m_rcLrcShow.right,y };
                if (bClearBK)
                    BitBlt(m_hcdcLeftBK2, rc2.left, rc2.top, rc2.right - rc2.left, rc2.bottom - rc2.top, m_hcdcLeftBK, rc2.left, rc2.top, SRCCOPY);

                if (bImmdShow)
                {
                    HRGN hRgn1 = CreateRectRgnIndirect(&rc2);
                    HRGN hRgn2 = CreateRectRgnIndirect(&m_rcLrcShow);
                    CombineRgn(hRgn1, hRgn1, hRgn2, RGN_AND);
                    SelectClipRgn(m_hcdcLeftBK2, hRgn1);// 设置剪辑区
                    DeleteObject(hRgn1);
                    DeleteObject(hRgn2);
				}

				rc = rc2;
				if (bCurr)
					rc.left += m_LrcHScrollInfo.x1;
				p->iLastTop = rc.top;
				DrawTextW(m_hcdcLeftBK2, p->pszLrc, -1, &rc, uFlags);// 绘制
            }
        }
        else// 有两行
        {
            iHeight = DrawTextW(m_hcdcLeftBK2, p->pszLrc, p->iOrgLength, &rc, uFlags);// 测量第一行
            iHeight2 = DrawTextW(m_hcdcLeftBK2, p->pszLrc + p->iOrgLength + 1, -1, &rc2, uFlags);// 测量第二行
            cx1 = rc.right - rc.left;
            cx2 = rc2.right - rc2.left;
            if (bCurr)
            {
                if (iIndex != m_LrcHScrollInfo.iIndex)
                {
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
            }

            p->cy = iHeight + iHeight2;

            
            if (bTop)
            {
                rc2 = { m_rcLrcShow.left,y,m_rcLrcShow.right,y + p->cy };
                if (bClearBK)
                {
                    BitBlt(m_hcdcLeftBK2, rc2.left, rc2.top, rc2.right - rc2.left, rc2.bottom - rc2.top, m_hcdcLeftBK, rc2.left, rc2.top, SRCCOPY);
                }

                if (bImmdShow)
                {
                    HRGN hRgn1 = CreateRectRgnIndirect(&rc2);
                    HRGN hRgn2 = CreateRectRgnIndirect(&m_rcLrcShow);
                    CombineRgn(hRgn1, hRgn1, hRgn2, RGN_AND);
                    SelectClipRgn(m_hcdcLeftBK2, hRgn1);// 设置剪辑区
                    DeleteObject(hRgn1);
                    DeleteObject(hRgn2);
                }

                rc.top = y;
                p->iLastTop = rc.top;
                rc.bottom = rc.top + iHeight;
                if (bCurr)
                    rc.left = m_rcLrcShow.left + m_LrcHScrollInfo.x1;
                else
                    rc.left = m_rcLrcShow.left;
                rc.right = m_rcLrcShow.right;
                if (cx1 > m_cxLrcShow)
                    uFlags = DT_NOPREFIX;
                else
                    uFlags = DT_NOPREFIX | DT_CENTER;
                DrawTextW(m_hcdcLeftBK2, p->pszLrc, p->iOrgLength, &rc, uFlags);// 绘制第一行

                rc.top = rc.bottom;
                rc.bottom += iHeight2;
                if (bCurr)
                    rc.left = m_rcLrcShow.left + m_LrcHScrollInfo.x2;

                rc.right = m_rcLrcShow.right;
                if (cx2 > m_cxLrcShow)
                    uFlags = DT_NOPREFIX;
                else
                    uFlags = DT_NOPREFIX | DT_CENTER;
                DrawTextW(m_hcdcLeftBK2, p->pszLrc + p->iOrgLength + 1, -1, &rc, uFlags);// 绘制第二行
            }
            else
            {
                rc2 = { m_rcLrcShow.left,y - p->cy,m_rcLrcShow.right,y };
                if (bClearBK)
                    BitBlt(m_hcdcLeftBK2, rc2.left, rc2.top, rc2.right - rc2.left, rc2.bottom - rc2.top, m_hcdcLeftBK, rc2.left, rc2.top, SRCCOPY);

                if (bImmdShow)
                {
                    HRGN hRgn1 = CreateRectRgnIndirect(&rc2);
                    HRGN hRgn2 = CreateRectRgnIndirect(&m_rcLrcShow);
                    CombineRgn(hRgn1, hRgn1, hRgn2, RGN_AND);
                    SelectClipRgn(m_hcdcLeftBK2, hRgn1);// 设置剪辑区
                    DeleteObject(hRgn1);
                    DeleteObject(hRgn2);
                }

                rc.bottom = y;
                rc.top = rc.bottom - iHeight2;
                if (bCurr)
                    rc.left = m_rcLrcShow.left + m_LrcHScrollInfo.x2;
                else
                    rc.left = m_rcLrcShow.left;
                rc.right = m_rcLrcShow.right;
                if (cx2 > m_cxLrcShow)
                    uFlags = DT_NOPREFIX;
                else
                    uFlags = DT_NOPREFIX | DT_CENTER;
                DrawTextW(m_hcdcLeftBK2, p->pszLrc + p->iOrgLength + 1, -1, &rc, uFlags);// 绘制第二行

				rc.bottom = rc.top;
				rc.top -= iHeight;
				p->iLastTop = rc.top;
				if (bCurr)
					rc.left = m_rcLrcShow.left + m_LrcHScrollInfo.x1;
				rc.right = m_rcLrcShow.right;
				if (cx1 > m_cxLrcShow)
					uFlags = DT_NOPREFIX;
				else
                    uFlags = DT_NOPREFIX | DT_CENTER;
                DrawTextW(m_hcdcLeftBK2, p->pszLrc, p->iOrgLength, &rc, uFlags);// 绘制第一行
            }
        }
        if (m_iLrcMouseHover == iIndex)
            FrameRect(m_hcdcLeftBK2, &rc2, GC.hbrCyanDeeper);
	}
	else
	{
        rc2 = m_rcLrcShow;
        uFlags = DT_NOPREFIX | DT_WORDBREAK | DT_CENTER | DT_CALCRECT;
        iHeight = DrawTextW(m_hcdcLeftBK2, p->pszLrc, -1, &rc2, uFlags);
        p->cy = iHeight;

        uFlags = DT_NOPREFIX | DT_WORDBREAK | DT_CENTER;

        if (bTop)
        {
            rc2.top = y;
            rc2.bottom = rc2.top + iHeight;
        }
        else
        {
            rc2.bottom = y;
            rc2.top = rc2.bottom - iHeight;
		}
		p->iLastTop = rc2.top;
		rc2.left = m_rcLrcShow.left;
		rc2.right = m_rcLrcShow.right;
        if (bClearBK)
            BitBlt(m_hcdcLeftBK2, rc2.left, rc2.top, rc2.right - rc2.left, rc2.bottom - rc2.top, m_hcdcLeftBK, rc2.left, rc2.top, SRCCOPY);

		if (bImmdShow)
		{
			HRGN hRgn1 = CreateRectRgnIndirect(&rc2);
			HRGN hRgn2 = CreateRectRgnIndirect(&m_rcLrcShow);
			CombineRgn(hRgn1, hRgn1, hRgn2, RGN_AND);
			SelectClipRgn(m_hcdcLeftBK2, hRgn1);// 设置剪辑区
			DeleteObject(hRgn1);
			DeleteObject(hRgn2);
		}

		DrawTextW(m_hcdcLeftBK2, p->pszLrc, -1, &rc2, uFlags);
		if (m_iLrcMouseHover == iIndex)
			FrameRect(m_hcdcLeftBK2, &rc2, GC.hbrCyanDeeper);
	}

	if (bImmdShow)
	{
		SelectClipRgn(m_hcdcLeftBK2, NULL);
		HDC hDC = GetDC(g_hBKLeft);
		BitBlt(hDC, rc2.left, rc2.top, rc2.right - rc2.left, rc2.bottom - rc2.top, m_hcdcLeftBK2, rc2.left, rc2.top, SRCCOPY);
		ReleaseDC(g_hBKLeft, hDC);
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
    g_hStream = BASS_OpenMusic(g_pszFile, BASS_SAMPLE_FX, BASS_SAMPLE_FX | BASS_MUSIC_PRESCAN);
    BASS_ChannelSetSync(g_hStream, BASS_SYNC_END | BASS_SYNC_ONETIME, 0, SyncProc_End, NULL);// 设置同步过程用来跟踪歌曲播放完毕的事件
    // Bass这个结束判定跟闹弹似的，如果播到最后突然改变位置同步过程就不会触发，你真给我整不会了😅😅😅😅😅
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
        g_pITaskbarList->SetProgressState(m_hTBGhost, TBPF_NORMAL);
    SendMessageW(g_hBKBtm, BTMBKM_SETPLAYBTICON, FALSE, 0);
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
    if (!m_CurrSongInfo.mi.pGdipImage)// 没有图片，读入默认图片
        GdipLoadImageFromFile(g_pszDefPic, &m_CurrSongInfo.mi.pGdipImage);
    else// 有图片，判断尺寸是否过大
    {
        UINT cx0, cy0, cx, cy;
        GdipGetImageWidth(m_CurrSongInfo.mi.pGdipImage, &cx0);
        GdipGetImageHeight(m_CurrSongInfo.mi.pGdipImage, &cy0);
        if (max(cx0, cy0) > DPIS_LARGEIMAGE)// 限制图片大小
        {
            GpBitmap* pGdipImage;
            GpGraphics* pGdipGraphics;
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
            GdipCreateBitmapFromScan0(cx, cy, 0, PixelFormat32bppRGB, NULL, &pGdipImage);
            GdipGetImageGraphicsContext(pGdipImage, &pGdipGraphics);
            GdipDrawImageRectRect(pGdipGraphics, m_CurrSongInfo.mi.pGdipImage,
                0, 0, (float)cx, (float)cy,
                0, 0, (float)cx0, (float)cy0,
                UnitPixel, NULL, NULL, NULL);
            GdipDeleteGraphics(pGdipGraphics);
            GdipDisposeImage(m_CurrSongInfo.mi.pGdipImage);
            m_CurrSongInfo.mi.pGdipImage = pGdipImage;
        }
    }
    DwmInvalidateIconicBitmaps(m_hTBGhost);
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
    SendMessageW(g_hTBProgess, QKCTBM_SETRANGE, FALSE, (LPARAM)g_llLength / 10);// 设定最大位置

    if (g_fSpeedChanged != SBV_INVALIDVALUE)
        BASS_ChannelSetAttribute(g_hStream, BASS_ATTRIB_FREQ, g_fSpeedChanged * g_fDefSpeed);
    if (g_fBlanceChanged != SBV_INVALIDVALUE)
        BASS_ChannelSetAttribute(g_hStream, BASS_ATTRIB_PAN, g_fBlanceChanged);

    if (g_bSlient)
        BASS_ChannelSetAttribute(g_hStream, BASS_ATTRIB_VOL, 0);
    else if (g_fVolChanged != SBV_INVALIDVALUE)
        BASS_ChannelSetAttribute(g_hStream, BASS_ATTRIB_VOL, g_fVolChanged);

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

	m_IsDraw[0] = m_IsDraw[1] = m_IsDraw[2] = TRUE;
    SetTimer(g_hMainWnd, IDT_DRAWING_LRC, TIMERELAPSE_LRC, TimerProc);
    SetTimer(g_hMainWnd, IDT_DRAWING_SPE, TIMERELAPSE_VU_SPE, TimerProc);
    SetTimer(g_hMainWnd, IDT_DRAWING_WAVES, TIMERELAPSE_WAVES, TimerProc);
}
void Playing_Stop(BOOL bNoGap)
{
    KillTimer(g_hMainWnd, IDT_DRAWING_LRC);
    KillTimer(g_hMainWnd, IDT_DRAWING_SPE);
    KillTimer(g_hMainWnd, IDT_DRAWING_WAVES);
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
        g_pITaskbarList->SetProgressState(m_hTBGhost, TBPF_NOPROGRESS);

    if (!bNoGap)
    {
        g_iCurrFileIndex = -1;
        m_IsDraw[0] = m_IsDraw[1] = m_IsDraw[2] = TRUE;
        GdipLoadImageFromFile(g_pszDefPic, &m_CurrSongInfo.mi.pGdipImage);
        LrcWnd_DrawLrc();
        SetWindowTextW(g_hMainWnd, L"未播放 - 晴空的音乐播放器");
        SendMessageW(g_hBKBtm, BTMBKM_SETPLAYBTICON, TRUE, 0);
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
        iCount = QKArray_GetCount(g_ItemData);

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
    int i = SendMessageW(g_hBKBtm, BTMBKM_GETREPEATMODE, 0, 0);
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
 * 备注：ＧＤＩＰＬＵＳ
 */
void UI_UpdateLeftBK()
{
    GpGraphics* pGdipGraphic;
    GpGraphics* pGdipGraphic2;
    GpBitmap* pGdipBitmap = m_CurrSongInfo.mi.pGdipImage;
    GpBitmap* pGdipBitmapBlur = NULL;
    GpEffect* pGdipEffect;

    if (m_cxLeftBK <= 0 || m_cyLeftBK <= 0)
        return;
    UINT cx, cy;
    float cxRgn, cyRgn;
    RectF rcF;
    SelectClipRgn(m_hcdcLeftBK, NULL);
    GdipCreateFromHDC(m_hcdcLeftBK, &pGdipGraphic);
    if (pGdipBitmap)
    {
        /*
        情况一，内存位图宽为最大边
        cxClient   cyClient
        -------- = --------
         cxPic      cyRgn
        情况二，内存位图高为最大边
        cyClient   cxClient
        -------- = --------
         cyPic      cxRgn
        */
        ////////////////////处理截取（无论怎么改变窗口大小，用来模糊的封面图都要居中充满整个窗口）
        GdipGetImageWidth(pGdipBitmap, &cx);
        GdipGetImageHeight(pGdipBitmap, &cy);
        cyRgn = (float)m_cyLeftBK / (float)m_cxLeftBK * (float)cx;
        if (cyRgn <= cy)// 情况一
        {
            rcF.Left = 0;
            rcF.Top = ((float)cy - cyRgn) / 2;
            rcF.Width = (float)cx;
            rcF.Height = cyRgn;
        }
        else// 情况二
        {
            cxRgn = (float)m_cxLeftBK / (float)m_cyLeftBK * (float)cy;
            rcF.Left = ((float)cx - cxRgn) / 2;
            rcF.Top = 0;
            rcF.Width = cxRgn;
            rcF.Height = (float)cy;
        }
        ////////////////////创建       
        GdipCreateBitmapFromScan0(m_cxLeftBK, m_cyLeftBK, 0, PixelFormat32bppRGB, NULL, &pGdipBitmapBlur);//创建模糊背景图
        GdipGetImageGraphicsContext(pGdipBitmapBlur, &pGdipGraphic2);
        ////////////////////准备原图
        GdipDrawImageRectRect(
            pGdipGraphic2, pGdipBitmap,
            0, 0, (float)m_cxLeftBK, (float)m_cyLeftBK,
            rcF.Left, rcF.Top, rcF.Width, rcF.Height,
            UnitPixel, NULL, NULL, NULL);
        GdipDeleteGraphics(pGdipGraphic2);
        ////////////////////置模糊效果
        // {633C80A4-1843-482b-9EF2-BE2834C5FDD4}
        const static GUID BlurEffectGuid = { 0x633c80a4, 0x1843, 0x482b, { 0x9e, 0xf2, 0xbe, 0x28, 0x34, 0xc5, 0xfd, 0xd4 } };
        GdipCreateEffect(BlurEffectGuid, &pGdipEffect);
        BlurParams bp;
        bp.radius = 100;// 模糊半径
        bp.expandEdge = FALSE;
        GdipSetEffectParameters(pGdipEffect, &bp, sizeof(BlurParams));
        GdipBitmapApplyEffect(pGdipBitmapBlur, pGdipEffect, NULL, FALSE, NULL, NULL);
        GdipDeleteEffect(pGdipEffect);
        ////////////////////画模糊图作为背景
        GdipDrawImageRectRect(
            pGdipGraphic, pGdipBitmapBlur,
            0, 0, (float)m_cxLeftBK, (float)m_cyLeftBK,
            0, 0, (float)m_cxLeftBK, (float)m_cyLeftBK,
            UnitPixel, NULL, NULL, NULL);
        GdipDisposeImage(pGdipBitmapBlur);
        ////////////////////画白色半透明遮罩
        GpBrush* pGdipBrush;
        GdipCreateSolidFill(0xB2FFFFFF, &pGdipBrush);// 0xB2==178   70%透明度
        GdipFillRectangle(pGdipGraphic, pGdipBrush, 0, 0, (float)m_cxLeftBK, (float)m_cyLeftBK);
        GdipDeleteBrush(pGdipBrush);
        ////////////////////处理封面图位置
        ////////////制封面框矩形
        if (m_bLrcShow)// 是否显示歌词秀？
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
            rcF.Height = cyRgn - DPIS_EDGE * 2;
            rcF.Width = rcF.Height;
            rcF.Top = DPIS_EDGE + DPIS_CYTOPBK;
            rcF.Left = (cxRgn - rcF.Width) / 2;
        }
        else// 宽度较小
        {
            rcF.Width = cxRgn - DPIS_EDGE * 2;
            rcF.Height = rcF.Width;
            rcF.Left = DPIS_EDGE;
            rcF.Top = (cyRgn - rcF.Height) / 2 + DPIS_CYTOPBK;
        }
        ////////////////////绘制封面图
        ////////////比例缩放
        float f;
        if (cx >= cy)// 高度较小
        {
            f = rcF.Width / cx * cy;// 缩放到封面框中，要保证 水平 方向显示完全 高度 要多大？
            rcF.Top = rcF.Top + (rcF.Height - f) / 2;// 偏移
            rcF.Height = f;
        }
        else// 宽度较小
        {
            f = rcF.Height / cy * cx;// 缩放到封面框中，要保证 垂直 方向显示完全 宽度 要多大？
            rcF.Left = rcF.Left + (rcF.Width - f) / 2;// 偏移
            rcF.Width = f;
        }
        GdipDrawImageRectRect(pGdipGraphic, pGdipBitmap,
            rcF.Left,
            rcF.Top,
            rcF.Width,
            rcF.Height,
            0,
            0,
            (float)cx,
            (float)cy,
            UnitPixel, NULL, NULL, NULL);
    }
    else// 没有图就全刷白吧
    {
        GpBrush* pGdipBrush;
        GdipCreateSolidFill(0xFFFFFFFF, &pGdipBrush);
        GdipFillRectangle(pGdipGraphic, pGdipBrush, 0, 0, (float)m_cxLeftBK, (float)m_cyLeftBK);
        GdipDeleteBrush(pGdipBrush);
    }
    GdipDeleteGraphics(pGdipGraphic);
    ////////////////////画顶部提示信息
    ///////////画大标题
    SetTextColor(m_hcdcLeftBK, QKCOLOR_CYANDEEPER);
    RECT rcText = { DPIS_EDGE, 5, m_cxLeftBK - DPIS_EDGE, DPIS_CYTOPTITLE };
    if (!m_CurrSongInfo.pszName)
        DrawTextW(m_hcdcLeftBK, L"未播放", -1, &rcText, DT_VCENTER | DT_SINGLELINE | DT_END_ELLIPSIS | DT_NOPREFIX);
    else
        DrawTextW(m_hcdcLeftBK, m_CurrSongInfo.pszName, -1, &rcText, DT_VCENTER | DT_SINGLELINE | DT_END_ELLIPSIS | DT_NOPREFIX);
    ///////////画其他信息
    SelectObject(m_hcdcLeftBK, g_hFont);// 切换字体
    SetTextColor(m_hcdcLeftBK, QKCOLOR_BLACK);

    rcText.left = DPIS_EDGE;
    rcText.top = DPIS_CYTOPTITLE + DPIS_GAPTOPTIP;
    rcText.right = rcText.left + DPIS_CXTOPTIP;
    rcText.bottom = rcText.top + DPIS_CYTOPTIP;
    DrawTextW(m_hcdcLeftBK, L"标题：", -1, &rcText, DT_VCENTER | DT_SINGLELINE);

    rcText.top += DPIS_CYTOPTIP;
    rcText.bottom = rcText.top + DPIS_CYTOPTIP;
    DrawTextW(m_hcdcLeftBK, L"艺术家：", -1, &rcText, DT_VCENTER | DT_SINGLELINE);

    rcText.top += DPIS_CYTOPTIP;
    rcText.bottom = rcText.top + DPIS_CYTOPTIP;
    DrawTextW(m_hcdcLeftBK, L"专辑：", -1, &rcText, DT_VCENTER | DT_SINGLELINE);

    rcText.top += DPIS_CYTOPTIP;
    rcText.bottom = rcText.top + DPIS_CYTOPTIP;
    DrawTextW(m_hcdcLeftBK, L"备注：", -1, &rcText, DT_VCENTER | DT_SINGLELINE);

    rcText.left = DPIS_CXTOPTIP + DPIS_EDGE;
    rcText.right = m_cxLeftBK - DPIS_EDGE;
    rcText.top = DPIS_CYTOPTITLE + DPIS_GAPTOPTIP;
    rcText.bottom = rcText.top + DPIS_CYTOPTIP;
    DrawTextW(m_hcdcLeftBK, m_CurrSongInfo.mi.pszTitle, -1, &rcText, DT_VCENTER | DT_SINGLELINE | DT_END_ELLIPSIS | DT_NOPREFIX);

    rcText.top += DPIS_CYTOPTIP;
    rcText.bottom = rcText.top + DPIS_CYTOPTIP;
    DrawTextW(m_hcdcLeftBK, m_CurrSongInfo.mi.pszArtist, -1, &rcText, DT_VCENTER | DT_SINGLELINE | DT_END_ELLIPSIS | DT_NOPREFIX);

    rcText.top += DPIS_CYTOPTIP;
    rcText.bottom = rcText.top + DPIS_CYTOPTIP;
    DrawTextW(m_hcdcLeftBK, m_CurrSongInfo.mi.pszAlbum, -1, &rcText, DT_VCENTER | DT_SINGLELINE | DT_END_ELLIPSIS | DT_NOPREFIX);

    rcText.top += DPIS_CYTOPTIP;
    rcText.bottom = rcText.top + DPIS_CYTOPTIP;
    DrawTextW(m_hcdcLeftBK, m_CurrSongInfo.mi.pszComment, -1, &rcText, DT_VCENTER | DT_SINGLELINE | DT_END_ELLIPSIS | DT_NOPREFIX);

    SelectObject(m_hcdcLeftBK, g_hFontDrawing);

    BitBlt(m_hcdcLeftBK2, 0, 0, m_cxLeftBK, m_cyLeftBK, m_hcdcLeftBK, 0, 0, SRCCOPY);
    m_IsDraw[0] = m_IsDraw[1] = m_IsDraw[2] = TRUE;
    TimerProc(NULL, 0, IDT_DRAWING_LRC, 0);
    TimerProc(NULL, 0, IDT_DRAWING_SPE, 0);
    TimerProc(NULL, 0, IDT_DRAWING_WAVES, 0);
    InvalidateRect(g_hTBProgess, NULL, FALSE);
    UpdateWindow(g_hTBProgess);
    InvalidateRect(g_hBKBtm, NULL, FALSE);
    UpdateWindow(g_hBKBtm);
    SendMessageW(g_hBKLeft, LEFTBKM_REDRAWSB, FALSE, 0);
    InvalidateRect(g_hBKLeft, NULL, FALSE);
    UpdateWindow(g_hBKLeft);
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
        if (!m_CurrSongInfo.mi.pGdipImage)
            return 0;
        HBITMAP hBitmap;
        UINT cx0, cy0, cx, cy, cxMax = HIWORD(lParam), cyMax = LOWORD(lParam);
        GdipGetImageWidth(m_CurrSongInfo.mi.pGdipImage, &cx0);
        GdipGetImageHeight(m_CurrSongInfo.mi.pGdipImage, &cy0);

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
        GpBitmap* pGdipImage;
        GpGraphics* pGdipGraphics;
        GdipCreateBitmapFromScan0(cxMax, cyMax, 0, PixelFormat32bppArgb, NULL, &pGdipImage);
        GdipGetImageGraphicsContext(pGdipImage, &pGdipGraphics);
        GdipDrawImageRectRect(pGdipGraphics, m_CurrSongInfo.mi.pGdipImage,
            (cxMax - cx) / 2, (cyMax - cy) / 2, (float)cx, (float)cy,
            0, 0, (float)cx0, (float)cy0,
            UnitPixel, NULL, NULL, NULL);
        GdipDeleteGraphics(pGdipGraphics);
        GdipCreateHBITMAPFromBitmap(pGdipImage, &hBitmap, 0x00000000);
        DwmSetIconicThumbnail(hWnd, hBitmap, 0);
        GdipDisposeImage(pGdipImage);
        DeleteObject(hBitmap);
    }
    return 0;
    case WM_DWMSENDICONICLIVEPREVIEWBITMAP:
    {
        HBITMAP hBitmap;
        GpBitmap* pGdipImage;
        GdipCreateBitmapFromScan0(1, 1, 0, PixelFormat32bppArgb, NULL, &pGdipImage);// 很小且全透明
        POINT pt = {};
        GdipCreateHBITMAPFromBitmap(pGdipImage, &hBitmap, 0x00000000);
        DwmSetIconicLivePreviewBitmap(m_hTBGhost, hBitmap, &pt, 0);
        GdipDisposeImage(pGdipImage);
        DeleteObject(hBitmap);
    }
	return 0;
    case WM_ACTIVATE:// 窗口激活，转发（就是点击缩略图激活的那个操作）
    {
        SetForegroundWindow(g_hMainWnd);
        if (!g_pITaskbarList)
            return 0;
        g_pITaskbarList->SetTabActive(hWnd, g_hMainWnd, 0);
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
        g_pITaskbarList->UnregisterTab(m_hTBGhost);
        g_pITaskbarList->RegisterTab(m_hTBGhost, hWnd);
        g_pITaskbarList->SetTabOrder(m_hTBGhost, NULL);

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
        g_pITaskbarList->ThumbBarAddButtons(m_hTBGhost, 3, tb);
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
                SendMessageW(g_hBKBtm, BTMBKM_DOBTOPE, 0, 0);
                return 0;
            case IDTBB_PLAY:
                SendMessageW(g_hBKBtm, BTMBKM_DOBTOPE, 1, 0);
                return 0;
            case IDTBB_NEXT:
                SendMessageW(g_hBKBtm, BTMBKM_DOBTOPE, 2, 0);
                return 0;
            }
        }
    }
    break;
    case WM_CREATE:
    {
        CoCreateInstance(CLSID_TaskbarList, NULL, CLSCTX_INPROC_SERVER, IID_PPV_ARGS(&g_pITaskbarList));// 创建ITaskbarList4对象
        g_pITaskbarList->HrInit();// 初始化

        Settings_Read();
        GlobalEffect_ResetToDefault(EFFECT_ALL);

        m_hcdcLeftBK = CreateCompatibleDC(NULL);
        m_hcdcLeftBK2 = CreateCompatibleDC(NULL);
        SelectObject(m_hcdcLeftBK, g_hFontDrawing);
        SelectObject(m_hcdcLeftBK2, g_hFontDrawing);
        SetBkMode(m_hcdcLeftBK, TRANSPARENT);
        SetBkMode(m_hcdcLeftBK2, TRANSPARENT);

        GdipLoadImageFromFile(g_pszDefPic, &m_CurrSongInfo.mi.pGdipImage);
        UI_UpdateLeftBK();

        g_Lrc = QKArray_Create(0);
        g_ItemData = QKArray_Create(0);
        ///////////////////////////创建用于支持任务栏操作的幽灵窗口（生命周期与主窗口相同）
        g_hMainWnd = hWnd;
        m_hTBGhost = CreateWindowExW(0, TBGHOSTWNDCLASS, ((CREATESTRUCTW*)lParam)->lpszName, 
			WS_POPUP | WS_CAPTION, -32000, -32000, 10, 10, NULL, NULL, g_hInst, NULL);
        ///////////////////////////初始化.....
        g_iDPI = QKGetDPIForWindow(hWnd);
        UI_UpdateDPISize();
        g_cxBKList = DPIS_DEFCXLV;
        
        HWND hCtrl, hCtrl2;
        ///////////////////////////左侧背景（剪辑子窗口）
        hCtrl2 = CreateWindowExW(0, BKWNDCLASS, NULL, WS_CHILD | WS_VISIBLE | WS_CLIPCHILDREN,
            0, 0, 0, 0,
            hWnd, (HMENU)IDC_BK_LEFT, g_hInst, NULL);
        g_hBKLeft = hCtrl2;
        SetWindowLongPtrW(hCtrl2, GWLP_WNDPROC, (LONG_PTR)WndProc_LeftBK);
        ///////////////////////////进度滑块条
        hCtrl = CreateWindowExW(0, QKCCN_TRACKBAR, NULL, WS_CHILD | WS_VISIBLE,
            0, 0, 0, 0,
            hCtrl2, (HMENU)IDC_TB_PGS, g_hInst, NULL);
        g_hTBProgess = hCtrl;
        SendMessageW(hCtrl, QKCTBM_SETPROC, FALSE, (LPARAM)QKCProc_TBPaint);
        SendMessageW(hCtrl, QKCTBM_GETRANGE, FALSE, MAKELONG(0, 10));
        ///////////////////////////底部按钮容器
        hCtrl2 = CreateWindowExW(0, BKWNDCLASS, NULL, WS_CHILD | WS_VISIBLE,
            0, 0, GC.cxBKBtm, GC.cyBT,
            hCtrl2, (HMENU)IDC_BK_BOTTOMBTBK, g_hInst, NULL);
        g_hBKBtm = hCtrl2;
        SetWindowLongPtrW(hCtrl2, GWLP_WNDPROC, (LONG_PTR)WndProc_BtmBK);
        SendMessageW(hCtrl2, BTMBKM_INIT, 0, 0);
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
	}
	return 0;
	case WM_DESTROY:
	{
        Playing_Stop();
        DestroyWindow(m_hTBGhost);
        if (IsWindow(g_hLrcWnd))
            DestroyWindow(g_hLrcWnd);
        if (g_pITaskbarList)
            g_pITaskbarList->Release();

        GDIObj_LeftBK(GDIOBJOPE_DELETE);
        MainWnd_ReleaseCurrInfo();
        Lrc_ClearArray(g_Lrc);
        delete[] m_dwWavesData;

		PostQuitMessage(0);
	}
	return 0;
    case WM_SIZE:
    {
        if (wParam == SIZE_MINIMIZED)
            return 0;
        cxClient = LOWORD(lParam);
        cyClient = HIWORD(lParam);

		int cxLeft;

        if (g_bListSeped || g_bListHidden)
        {
            cxLeft = cxClient;
        }
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
        SetWindowTextW(m_hTBGhost, (PCWSTR)lParam);
        break;
	}
    return DefWindowProcW(hWnd, message, wParam, lParam);
}
LRESULT CALLBACK WndProc_BtmBK(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam)
{
    static WCHAR szTime[20] = L"00:00/00:00";
    static int iRepeatMode = REPEATMODE_TOTALLOOP;
    static RECT rcTimeText = { 0,0,DPIS_CXTIME,GC.cyBT };
    static int iHot = -1, iPushed = -1, iLastHot = -1, iLastOver = -1;
    static BOOL bBTPLPushed = FALSE, bBTIGPushed = FALSE;
    static HWND hToolTip;
    static BOOL bLBTDown = FALSE;
    static TTTOOLINFOW ti = { sizeof(TTTOOLINFOW),0,hWnd,1 };
    static HWND hDlg = NULL;
    static int i;
    static HDC hCDC;
    static HBITMAP hBitmap;
    switch (message)
    {
    case BTMBKM_INIT:
    {
        hToolTip = CreateWindowExW(0, TOOLTIPS_CLASSW, NULL, 0, 0, 0, 0, 0, hWnd, NULL, NULL, NULL);
        RECT rc;
        GetClientRect(hWnd, &rc);
        ti.rect = rc;
        ti.uFlags = TTF_SUBCLASS;
        SendMessageW(hToolTip, TTM_ADDTOOLW, 0, (LPARAM)&ti);
        SendMessageW(hToolTip, TTM_POP, 0, 0);
        SetTimer(hWnd, IDT_PGS, TIMERELAPSE_PGS, NULL);

        HDC hDC = GetDC(hWnd);
        hBitmap = CreateCompatibleBitmap(hDC, GC.cxBKBtm, GC.cyBT);
        hCDC = CreateCompatibleDC(hDC);
        ReleaseDC(hWnd, hDC);
        SelectObject(hCDC, hBitmap);
        BitBlt(hCDC, 0, 0, GC.cxBKBtm, GC.cyBT, m_hcdcLeftBK, m_xBtmBK, m_yBtmBK, SRCCOPY);
        SelectObject(hCDC, g_hFont);
        SetBkMode(hCDC, TRANSPARENT);
    }
    return 0;
    case BTMBKM_GETREPEATMODE:
        return iRepeatMode;
    case BTMBKM_SETPLAYBTICON:
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
                g_pITaskbarList->ThumbBarUpdateButtons(m_hTBGhost, 1, &tb);
            BASS_ChannelPause(g_hStream);
            KillTimer(g_hMainWnd, IDT_DRAWING_LRC);
            KillTimer(g_hMainWnd, IDT_DRAWING_SPE);
            KillTimer(g_hMainWnd, IDT_DRAWING_WAVES);
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
                g_pITaskbarList->ThumbBarUpdateButtons(m_hTBGhost, 1, &tb);
            BASS_ChannelPlay(g_hStream, FALSE);
            SetTimer(g_hMainWnd, IDT_DRAWING_LRC, TIMERELAPSE_LRC, TimerProc);
            SetTimer(g_hMainWnd, IDT_DRAWING_SPE, TIMERELAPSE_VU_SPE, TimerProc);
            SetTimer(g_hMainWnd, IDT_DRAWING_WAVES, TIMERELAPSE_WAVES, TimerProc);
            SetTimer(hWnd, IDT_PGS, TIMERELAPSE_PGS, NULL);
        }

        InvalidateRect(hWnd, NULL, FALSE);
        UpdateWindow(hWnd);
        LrcWnd_DrawLrc();
    }
    return 0;
    case BTMBKM_DOBTOPE:
        switch (wParam)
        {
        case 0:goto BTOpe_Last;
        case 1:goto BTOpe_Play;
        case 2:goto BTOpe_Next;
        }
        return 0;
    case WM_PAINT:
    {
        PAINTSTRUCT ps;
        HDC hDC = BeginPaint(hWnd, &ps);
        BitBlt(hCDC, 0, 0, GC.cxBKBtm, GC.cyBT, m_hcdcLeftBK, m_xBtmBK, m_yBtmBK, SRCCOPY);
        DrawTextW(hCDC, szTime, -1, &rcTimeText, DT_RIGHT | DT_SINGLELINE | DT_VCENTER);
        int x = rcTimeText.right + 3;
        int iIconOffest = (GC.cyBT - GC.iIconSize) / 2;
        HBRUSH hBrush;
        RECT rc = { 0,0,0,GC.cyBT };
        if (iHot != -1 || iPushed != -1)
        {
            int i;
            if (iPushed != -1)
            {
                i = iPushed;
                hBrush = CreateSolidBrush(MYCLR_BTPUSHED);
            }
            else
            {
                i = iHot;
                hBrush = CreateSolidBrush(MYCLR_BTHOT);
            }
            rc.left = x + GC.cyBT * i;
            rc.right = rc.left + GC.cyBT;
            FillRect(hCDC, &rc, hBrush);
            DeleteObject(hBrush);
        }


        DrawIconEx(hCDC, x + iIconOffest, iIconOffest, GR.hiLast, 0, 0, 0, NULL, DI_NORMAL);// 1 上一曲
        x += GC.cyBT;
        DrawIconEx(hCDC, x + iIconOffest, iIconOffest, g_bPlayIcon ? GR.hiPlay : GR.hiPause, 0, 0, 0, NULL, DI_NORMAL);// 2 播放/暂停
        x += GC.cyBT;
        DrawIconEx(hCDC, x + iIconOffest, iIconOffest, GR.hiStop, 0, 0, 0, NULL, DI_NORMAL);// 3 停止
        x += GC.cyBT;
        DrawIconEx(hCDC, x + iIconOffest, iIconOffest, GR.hiNext, 0, 0, 0, NULL, DI_NORMAL);// 4 下一曲
        x += GC.cyBT;
        DrawIconEx(hCDC, x + iIconOffest, iIconOffest, GR.hiLrc, 0, 0, 0, NULL, DI_NORMAL);// 5 歌词
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

        DrawIconEx(hCDC, x + iIconOffest, iIconOffest, hi, 0, 0, 0, NULL, DI_NORMAL);// 6 循环方式
        x += GC.cyBT;

        DrawIconEx(hCDC, x + iIconOffest, iIconOffest, GR.hiPlaySetting, 0, 0, 0, NULL, DI_NORMAL);// 7 均衡器
        x += GC.cyBT;
        if (bBTPLPushed)
        {
            hBrush = CreateSolidBrush(MYCLR_BTPUSHED);
            rc.left = x;
            rc.right = rc.left + GC.cyBT;
            FillRect(hCDC, &rc, hBrush);
            DeleteObject(hBrush);
        }
        DrawIconEx(hCDC, x + iIconOffest, iIconOffest, GR.hiPlayList, 0, 0, 0, NULL, DI_NORMAL);// 8 显示播放列表
        x += GC.cyBT;
        if (bBTPLPushed)
        {
            hBrush = CreateSolidBrush(MYCLR_BTPUSHED);
            rc.left = x;
            rc.right = rc.left + GC.cyBT;
            FillRect(hCDC, &rc, hBrush);
            DeleteObject(hBrush);
        }
        DrawIconEx(hCDC, x + iIconOffest, iIconOffest, GR.hiSettings, 0, 0, 0, NULL, DI_NORMAL);// 9 设置
        x += GC.cyBT;

        DrawIconEx(hCDC, x + iIconOffest, iIconOffest, GR.hiInfo, 0, 0, 0, NULL, DI_NORMAL);// 10 关于
        x += GC.cyBT;

        BitBlt(hDC, 0, 0, GC.cxBKBtm, GC.cyBT, hCDC, 0, 0, SRCCOPY);

        EndPaint(hWnd, &ps);
    }
    return 0;
    case WM_MOUSEMOVE:
    {
        if (!bLBTDown)
        {
            iHot = HitTest_BtmBK(GET_X_LPARAM(lParam), GET_Y_LPARAM(lParam));
            if (iLastHot != iHot)
            {
                ti.lpszText = NULL;
                SendMessageW(hToolTip, TTM_GETTOOLINFOW, 0, (LPARAM)&ti);
                SendMessageW(hToolTip, TTM_SETTOOLINFOW, 0, (LPARAM)&ti);
                iLastHot = iHot;
                InvalidateRect(hWnd, NULL, FALSE);
            }
            TRACKMOUSEEVENT tme;
            tme.cbSize = sizeof(tme);
            tme.hwndTrack = hWnd;
            tme.dwFlags = TME_LEAVE | TME_HOVER;
            tme.dwHoverTime = 200;
            TrackMouseEvent(&tme);
        }
    }
    return 0;
    case WM_MOUSEHOVER:
    {
        if (iHot != -1 && iPushed == -1 && iLastOver != iHot)
        {
        ShowToolTip:
            iLastOver = iHot;
            ti.lpszText = NULL;
            SendMessageW(hToolTip, TTM_GETTOOLINFOW, 0, (LPARAM)&ti);
            if (iHot == 5)
                ti.lpszText = (LPWSTR)c_szBtmTip[BTMBKBTNCOUNT + iRepeatMode];
            else
                ti.lpszText = (LPWSTR)c_szBtmTip[iHot];
            SendMessageW(hToolTip, TTM_SETTOOLINFOW, 0, (LPARAM)&ti);
            SendMessageW(hToolTip, TTM_POPUP, 0, 0);
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
            InvalidateRect(hWnd, NULL, FALSE);
        }
    }
    return 0;
    case WM_LBUTTONDOWN:
    {
        SetCapture(hWnd);
        iPushed = HitTest_BtmBK(GET_X_LPARAM(lParam), GET_Y_LPARAM(lParam));
        if (iPushed == -1)
            ReleaseCapture();
        else
        {
            InvalidateRect(hWnd, NULL, FALSE);
            UpdateWindow(hWnd);
        }
    }
    return 0;
    case WM_LBUTTONUP:
    {
        ReleaseCapture();
        if (iPushed == -1)
            return 0;
        i = iPushed;
        iPushed = -1;
        if (i != HitTest_BtmBK(GET_X_LPARAM(lParam), GET_Y_LPARAM(lParam)))
            return 0;
        ti.lpszText = NULL;
        SendMessageW(hToolTip, TTM_GETTOOLINFOW, 0, (LPARAM)&ti);
        SendMessageW(hToolTip, TTM_SETTOOLINFOW, 0, (LPARAM)&ti);
        InvalidateRect(hWnd, NULL, FALSE);
        UpdateWindow(hWnd);
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
                WndProc_BtmBK(hWnd, BTMBKM_SETPLAYBTICON, TRUE, 0);
            else if (dwState == BASS_ACTIVE_PAUSED)
                WndProc_BtmBK(hWnd, BTMBKM_SETPLAYBTICON, FALSE, 0);
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
            GetWindowRect(g_hBKBtm, &rc);
            int iRet = TrackPopupMenu(hMenu, TPM_RETURNCMD, rc.left + DPIS_CXTIME + DPIS_BT * 4, rc.top + DPIS_BT, 0, g_hMainWnd, NULL);
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
            InvalidateRect(hWnd, NULL, FALSE);
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
            GetWindowRect(g_hBKBtm, &rc);
            int iRet = TrackPopupMenu(hMenu, TPM_RETURNCMD, rc.left + DPIS_CXTIME + GC.cyBT * 7, rc.top + GC.cyBT, 0, g_hMainWnd, NULL);
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
	return 0;
    case WM_DESTROY:
    {
        DestroyWindow(hToolTip);
        DeleteDC(hCDC);
        DeleteObject(hBitmap);
    }
    return 0;
    case WM_TIMER:
    {
        if (!g_hStream)
            return 0;

        if (BASS_ChannelIsActive(g_hStream) == BASS_ACTIVE_STOPPED && g_bPlaying)
        {
            SyncProc_End(NULL, 0, 0, NULL);
            return 0;
        }

        int iMin = g_fTime / 60,
            iMin2 = g_llLength / 1000 / 60;

        wsprintfW(szTime, L"%02d:%02d/%02d:%02d",
            iMin,
            (int)g_fTime - iMin * 60,
            iMin2,
            (int)(g_llLength / 1000) - iMin2 * 60);

        BitBlt(hCDC, 0, 0, rcTimeText.right, rcTimeText.bottom, m_hcdcLeftBK, m_xBtmBK, m_yBtmBK, SRCCOPY);
        DrawTextW(hCDC, szTime, -1, &rcTimeText, DT_RIGHT | DT_SINGLELINE | DT_VCENTER);
        HDC hDC = GetDC(hWnd);
        BitBlt(hDC, 0, 0, rcTimeText.right, rcTimeText.bottom, hCDC, rcTimeText.left, rcTimeText.top, SRCCOPY);
        ReleaseDC(hWnd, hDC);

        SendMessageW(g_hTBProgess, QKCTBM_SETPOS, TRUE, (LPARAM)g_fTime * 100);
        if (g_pITaskbarList)
            g_pITaskbarList->SetProgressValue(g_hMainWnd, (ULONGLONG)g_fTime * 100, (ULONGLONG)g_llLength / 10);

        m_TimeStru_VU[1].uTime += 500;
        if (m_TimeStru_VU[1].uTime >= 1000)
        {
            m_TimeStru_VU[1].bbool = TRUE;
            m_TimeStru_VU[1].uTime = 0;
        }
    }
    return 0;
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
    switch (message)
    {
    case WM_PAINT:
    {
        PAINTSTRUCT ps;
        HDC hDC = BeginPaint(hWnd, &ps);
        BitBlt(hDC, 0, 0, m_cxLeftBK, m_cyLeftBK, m_hcdcLeftBK2, 0, 0, SRCCOPY);// 简简单单贴个图
        EndPaint(hWnd, &ps);
    }
    return 0;
    case WM_SIZE:
    {
        m_cxLeftBK = LOWORD(lParam);
        m_cyLeftBK = HIWORD(lParam);

        if (m_bLrcShow)
        {
            SetRect(&m_rcLrcShow,
                DPIS_CXPIC + DPIS_EDGE * 2,
                DPIS_CYTOPBK + DPIS_EDGE,
                m_cxLeftBK - DPIS_EDGE,
                m_cyLeftBK - GC.cyBT - DPIS_CYPROGBAR);
            m_cxLrcShow = m_rcLrcShow.right - m_rcLrcShow.left;
            m_cyLrcShow = m_rcLrcShow.bottom - m_rcLrcShow.top;
            m_xSpe = DPIS_EDGE;
            m_ySpe = DPIS_CYTOPBK + DPIS_EDGE + DPIS_CXPIC + DPIS_EDGE;
            m_xWaves = DPIS_EDGE;
            m_yWaves = m_ySpe + DPIS_EDGE + DPIS_CYSPE;
            m_LrcHScrollInfo.iIndex = -1;
            m_LrcHScrollInfo.bWndSizeChangedFlag = TRUE;
        }
        else
        {
            int cxSpe = (m_cxLeftBK - DPIS_EDGE * 2) / (DPIS_CXSPEBAR + DPIS_CXSPEBARDIV) * (DPIS_CXSPEBAR + DPIS_CXSPEBARDIV);
            if (cxSpe > DPIS_CXSPE)
                cxSpe = DPIS_CXSPE;
            SetRect(&m_rcLrcShow, 0, 0, 0, 0);
            m_cxLrcShow = 0;
            m_cyLrcShow = 0;
            m_xSpe = (m_cxLeftBK - (cxSpe * 2 + DPIS_EDGE)) / 2;
            m_ySpe = m_cyLeftBK - GC.cyBT - DPIS_CYPROGBAR - DPIS_CYSPE;
            m_xWaves = m_xSpe + DPIS_EDGE + DPIS_CXSPE;
            m_yWaves = m_ySpe;
        }

        SetWindowPos(g_hTBProgess, 0,
            0,
            m_cyLeftBK - GC.cyBT - DPIS_CYPROGBAR,
            m_cxLeftBK,
            DPIS_CYPROGBAR,
            SWP_NOZORDER);//进度条
        m_xBtmBK = (m_cxLeftBK - GC.cxBKBtm) / 2;
        m_yBtmBK = m_cyLeftBK - GC.cyBT;
        SetWindowPos(g_hBKBtm, 0,
            m_xBtmBK,
            m_yBtmBK,
            0, 0, SWP_NOZORDER | SWP_NOSIZE);//底部按钮容器

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

        GDIObj_LeftBK();
        m_IsDraw[0] = m_IsDraw[1] = m_IsDraw[2] = TRUE;
        UI_UpdateLeftBK();
    }
    return 0;
    case WM_HSCROLL:// 进度滑块条
    {
        switch (LOWORD(wParam))
        {
        case TB_THUMBTRACK:
            KillTimer(g_hMainWnd, IDT_PGS);
            return 0;
        case TB_THUMBPOSITION:
            BASS_ChannelSetPosition(
                g_hStream,
                BASS_ChannelSeconds2Bytes(
                    g_hStream,
                    SendMessageW(g_hTBProgess, QKCTBM_GETPOS, 0, 0) / 100),
                BASS_POS_BYTE
            );// 设置位置
            SetTimer(g_hMainWnd, IDT_PGS, TIMERELAPSE_PGS, TimerProc);
            return 0;
        }
    }
    return 0;
    case WM_LBUTTONDOWN:// 左键按下得焦点
    {
        iDelayTime = 5;
        POINT pt = { GET_X_LPARAM(lParam), GET_Y_LPARAM(lParam) };
        if (PtInRect(&m_rcLrcShow, pt))
            SetFocus(hWnd);
        else if (PtInRect(&rcLrcSB, pt))
        {
            if (PtInRect(&rcLrcSBThumb, pt))
            {
                bSBLBtnDown = TRUE;
                iCursorOffest = pt.y - m_rcLrcShow.top - m_iLrcSBPos * (rcLrcSB.bottom - rcLrcSB.top - iThumbSize) / iSBMax;
                SetCapture(hWnd);
            }
        }
    }
    return 0;
    case WM_LBUTTONUP:
    {
        bSBLBtnDown = FALSE;
        ReleaseCapture();
    }
    return 0;
    case WM_MOUSEWHEEL:// 鼠标滚轮滚动
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
        WndProc_LeftBK(hWnd, LEFTBKM_REDRAWSB, TRUE, 0);
        m_IsDraw[2] = TRUE;
        TimerProc(NULL, 0, IDT_DRAWING_LRC, 0);
        SetTimer(hWnd, IDT_LRCSCROLL, TIMERELAPSE_LRCSCROLL, NULL);
    }
    return 0;
    case WM_TIMER:// 延时隐藏滑块条
    {
        --iDelayTime;
        if (iDelayTime <= 0)
        {
            KillTimer(hWnd, IDT_LRCSCROLL);
            m_iLrcSBPos = -1;
            WndProc_LeftBK(hWnd, LEFTBKM_REDRAWSB, TRUE, 0);
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
                WndProc_LeftBK(hWnd, LEFTBKM_REDRAWSB, TRUE, 0);
                m_IsDraw[2] = TRUE;
                TimerProc(NULL, 0, IDT_DRAWING_LRC, 0);
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
    }
    return 0;
    case WM_MOUSELEAVE:
    {
        if (m_iLrcFixedIndex != -1)
            return 0;
        iLastIndex = m_iLrcMouseHover = -1;
        m_IsDraw[2] = TRUE;
        TimerProc(NULL, 0, IDT_DRAWING_LRC, 0);
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
        }
        if (wParam)
            BitBlt(m_hcdcLeftBK2, rcLrcSB.left, rcLrcSB.top, rcLrcSB.right - rcLrcSB.left, rcLrcSB.bottom - rcLrcSB.top,
                m_hcdcLeftBK, rcLrcSB.left, rcLrcSB.top, SRCCOPY);

        if (m_iLrcSBPos != -1)
            FillRect(m_hcdcLeftBK2, &rcLrcSBThumb, GC.hbrMyBule);

        HDC hDC = GetDC(hWnd);
        BitBlt(hDC, rcLrcSB.left, rcLrcSB.top, rcLrcSB.right - rcLrcSB.left, rcLrcSB.bottom - rcLrcSB.top,
            m_hcdcLeftBK2, rcLrcSB.left, rcLrcSB.top, SRCCOPY);
        ReleaseDC(hWnd, hDC);
    }
    return 0;
    case LEFTBKM_SETMAX:
    {
        iSBMax = wParam;
        WndProc_LeftBK(hWnd, LEFTBKM_REDRAWSB, TRUE, 0);
    }
    return 0;
    case WM_KEYDOWN:
    {
        if (wParam == VK_ESCAPE)
        {
            KillTimer(hWnd, IDT_LRCSCROLL);
            m_iLrcSBPos = -1;
            WndProc_LeftBK(hWnd, LEFTBKM_REDRAWSB, TRUE, 0);
            return 0;
        }
    }
    break;
    }
    return DefWindowProcW(hWnd, message, wParam, lParam);
}
INT_PTR CALLBACK DlgProc_About(HWND hDlg, UINT message, WPARAM wParam, LPARAM lParam)
{
    static HBITMAP hBitmap;
    switch (message)
    {
    case WM_INITDIALOG:
    {
        hBitmap = (HBITMAP)LoadImageW(g_hInst, MAKEINTRESOURCEW(IDB_ABOUT), IMAGE_BITMAP, 0, 0, 0);
        BITMAP bmp;
        GetObjectW(hBitmap, sizeof(bmp), &bmp);

        RECT rcClient, rcWnd;
        GetWindowRect(hDlg, &rcWnd);
        GetClientRect(hDlg, &rcClient);
        int cx = bmp.bmWidth + rcWnd.right - rcWnd.left - rcClient.right,
            cy = bmp.bmHeight + rcWnd.bottom - rcWnd.top - rcClient.bottom + 50;

        SetWindowPos(hDlg, NULL,
            (GetSystemMetrics(SM_CXSCREEN) - cx) / 2,
            (GetSystemMetrics(SM_CYSCREEN) - cy) / 2,
            cx,
            cy,
            SWP_NOZORDER);
        HWND hStatic = GetDlgItem(hDlg, IDC_ST_ABOUT);
        SetWindowPos(hStatic, NULL, 0, 0, bmp.bmWidth, bmp.bmHeight, SWP_NOZORDER);

        SetWindowLongPtrW(hStatic, GWL_STYLE, GetWindowLongPtrW(hStatic, GWL_STYLE) | SS_BITMAP);
        SendMessageW(hStatic, STM_SETIMAGE, IMAGE_BITMAP, (LPARAM)hBitmap);

        GetClientRect(GetDlgItem(hDlg, IDC_BT_OK), &rcClient);
        SetWindowPos(GetDlgItem(hDlg, IDC_BT_OK), NULL,
            (bmp.bmWidth - rcClient.right) / 2,
            bmp.bmHeight,
            rcClient.right, 50, SWP_NOZORDER | SWP_NOSIZE);
    }
    return FALSE;
    case WM_CLOSE:
    {
        DeleteObject(hBitmap);
        EndDialog(hDlg, NULL);
    }
    return TRUE;
    case WM_COMMAND:
    {
        if (LOWORD(wParam) == IDC_BT_OK)
        {
            EndDialog(hDlg, NULL);
            return TRUE;
        }
    }
    return FALSE;
    case WM_PAINT:
    {
        PAINTSTRUCT ps;
        HDC hDC = BeginPaint(hDlg, &ps);
        FillRect(hDC, &(ps.rcPaint), (HBRUSH)GetStockObject(WHITE_BRUSH));
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
    case IDT_DRAWING_WAVES:
    {
        g_fTime = BASS_ChannelBytes2Seconds(
            g_hStream,
            BASS_ChannelGetPosition(g_hStream, BASS_POS_BYTE)
        );

        if (!m_IsDraw[0])
            return;

        RECT rc = { m_xWaves,m_yWaves,m_xWaves + DPIS_CXSPE,m_yWaves + DPIS_CYSPE };

        HDC hDC = GetDC(g_hBKLeft);
        BitBlt(m_hcdcLeftBK2, m_xWaves, m_yWaves, DPIS_CXSPE, DPIS_CYSPE, m_hcdcLeftBK, m_xWaves, m_yWaves, SRCCOPY);
        LPCWSTR pszText = NULL;
        if (m_uThreadFlagWaves == THREADFLAG_WORKING)// 正在加载
            pszText = L"正在加载...";
        else if (!g_hStream)// 已停止
            pszText = L"未播放";
        else if (m_uThreadFlagWaves == THREADFLAG_ERROR)// 出错
            pszText = L"错误！";

        if (pszText)
        {
            m_IsDraw[0] = FALSE;
            SetTextColor(m_hcdcLeftBK2, QKCOLOR_CYANDEEPER);// 蓝色
            SelectObject(m_hcdcLeftBK2, g_hFontDrawing);
            DrawTextW(m_hcdcLeftBK2, pszText, -1, &rc, DT_CENTER | DT_VCENTER | DT_SINGLELINE);// 画错误提示
            BitBlt(hDC, m_xWaves, m_yWaves, DPIS_CXSPE, DPIS_CYSPE, m_hcdcLeftBK2, m_xWaves, m_yWaves, SRCCOPY);// 显示
            ReleaseDC(g_hBKLeft, hDC);
            return;
        }

        if (m_uThreadFlagWaves == THREADFLAG_STOPED)
        {
            int iCurrIndex = (int)(g_fTime * 1000.0 / 20.0);// 算数组索引    20ms一单位
            if (iCurrIndex < 0 || iCurrIndex > m_dwWavesDataCount - 1)
            {
                ReleaseDC(g_hBKLeft, hDC);
                return;
            }

            int i = iCurrIndex;
            int x = m_xWaves + DPIS_CXSPEHALF,
                y = m_yWaves + DPIS_CYSPEHALF;
            HGDIOBJ hOldPen = SelectObject(m_hcdcLeftBK2, CreatePen(PS_SOLID, DPIS_CXWAVESLINE, 0xDA9E46));

            HRGN hRgn = CreateRectRgnIndirect(&rc);
            SelectClipRgn(m_hcdcLeftBK2, hRgn);// 不剪辑的话画笔会越界...应该是设置了画笔宽度的原因
            DeleteObject(hRgn);

            // 上面是右声道，下面是左声道
            while (true)// 向右画
            {
                MoveToEx(m_hcdcLeftBK2, x, y, NULL);
                LineTo(m_hcdcLeftBK2, x, y - HIWORD(m_dwWavesData[i]) * DPIS_CYSPEHALF / 32768);
                MoveToEx(m_hcdcLeftBK2, x, y, NULL);
                LineTo(m_hcdcLeftBK2, x, y + LOWORD(m_dwWavesData[i]) * DPIS_CYSPEHALF / 32768);
                x += DPIS_CXWAVESLINE;
                i++;
                if (i > m_dwWavesDataCount - 1 || x >= m_xWaves + DPIS_CXSPE)
                    break;
            }
            i = iCurrIndex;
            x = m_xWaves + DPIS_CXSPEHALF;
            while (true)// 向左画
            {
                MoveToEx(m_hcdcLeftBK2, x, y, NULL);
                LineTo(m_hcdcLeftBK2, x, y - (int)((float)HIWORD(m_dwWavesData[i]) / 32768.0f * (float)DPIS_CYSPEHALF));
                MoveToEx(m_hcdcLeftBK2, x, y, NULL);
                LineTo(m_hcdcLeftBK2, x, y + (int)((float)LOWORD(m_dwWavesData[i]) / 32768.0f * (float)DPIS_CYSPEHALF));
                x -= DPIS_CXWAVESLINE;
                i--;
                if (i < 0 || x < m_xWaves)
                    break;
            }
            x = m_xWaves + DPIS_CXSPEHALF;

            DeleteObject(SelectObject(m_hcdcLeftBK2, CreatePen(PS_SOLID, DPIS_CXWAVESLINE, QKCOLOR_RED)));
            MoveToEx(m_hcdcLeftBK2, x, m_yWaves, NULL);
            LineTo(m_hcdcLeftBK2, x, m_yWaves + DPIS_CYSPE);
            DeleteObject(SelectObject(m_hcdcLeftBK2, hOldPen));
            SelectClipRgn(m_hcdcLeftBK2, NULL);
        }
        BitBlt(hDC, m_xWaves, m_yWaves, DPIS_CXSPE, DPIS_CYSPE, m_hcdcLeftBK2, m_xWaves, m_yWaves, SRCCOPY);//显示
        ReleaseDC(g_hBKLeft, hDC);


    }
    return;
    case IDT_DRAWING_LRC:
    {
        RECT rc;
        HDC hDC;
        BOOL bSwitchLrc;
        if (m_IsDraw[2])
        {
            PCWSTR pszText = NULL;
            if (!g_hStream)
            {
                pszText = L"☆★晴空的音乐播放器★☆";
                g_iLrcState = LRCSTATE_STOP;
                g_iCurrLrcIndex = -2;
            }
            else if (!g_Lrc->iCount)//无歌词
            {
                pszText = L"无歌词(─.─|||";
                g_iLrcState = LRCSTATE_NOLRC;
                g_iCurrLrcIndex = -2;
            }

            if (pszText)
            {
                hDC = GetDC(g_hBKLeft);
                BitBlt(m_hcdcLeftBK2, m_rcLrcShow.left, m_rcLrcShow.top, m_rcLrcShow.right - m_rcLrcShow.left, m_rcLrcShow.bottom - m_rcLrcShow.top,
                    m_hcdcLeftBK, m_rcLrcShow.left, m_rcLrcShow.top, SRCCOPY);//清除
                m_IsDraw[2] = FALSE;
                SelectObject(m_hcdcLeftBK2, g_hFontDrawing);
                SetTextColor(m_hcdcLeftBK2, QKCOLOR_CYANDEEPER);
                DrawTextW(m_hcdcLeftBK2, pszText, -1, &m_rcLrcShow, DT_CENTER);
                LrcWnd_DrawLrc();
                BitBlt(hDC, m_rcLrcShow.left, m_rcLrcShow.top, m_rcLrcShow.right - m_rcLrcShow.left, m_rcLrcShow.bottom - m_rcLrcShow.top,
                    m_hcdcLeftBK2, m_rcLrcShow.left, m_rcLrcShow.top, SRCCOPY);// 显示
                ReleaseDC(g_hBKLeft, hDC);
                return;
            }
        }

        m_iLrcCenter = -1;
        g_iCurrLrcIndex = -2;
        if (g_Lrc->iCount)
        {
            int iArrayCount = g_Lrc->iCount;
            for (int i = 0; i < iArrayCount; i++)//遍历歌词数组
            {
                float fTempTime = ((LRCDATA*)QKArray_Get(g_Lrc, i))->fTime;
                if (g_fTime < ((LRCDATA*)QKArray_Get(g_Lrc, 0))->fTime)//还没播到第一句
                {
                    m_iLrcCenter = 0;
                    g_iCurrLrcIndex = -1;
                    break;
                }
                else if (i == iArrayCount - 1)//最后一句
                {
                    if (g_fTime >= fTempTime)
                    {
                        m_iLrcCenter = g_iCurrLrcIndex = i;
                        break;
                    }
                }
                else if (g_fTime >= fTempTime && g_fTime < ((LRCDATA*)QKArray_Get(g_Lrc, i + 1))->fTime)//左闭右开，判断歌词区间
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
            if (bSwitchLrc && GS.bLrcAnimation && m_iLrcCenter != m_iLastLrcIndex[0] && m_iLrcSBPos == -1 && m_iLastLrcIndex[0] != -1)
            {
                UINT uFlags = DT_NOPREFIX | DT_CALCRECT | (GS.bForceTwoLines ? 0 : DT_CENTER | DT_WORDBREAK);
                if (m_iLastLrcIndex[0] > g_Lrc->iCount - 1)
                    m_iLastLrcIndex[0] = g_Lrc->iCount - 1;;
                LRCDATA* p1 = (LRCDATA*)QKArray_Get(g_Lrc, m_iLrcCenter), * p2 = (LRCDATA*)QKArray_Get(g_Lrc, m_iLastLrcIndex[0]);
                int iHeight = DrawTextW(m_hcdcLeftBK2, p1->pszLrc, -1, &rc, uFlags);
                int iHeight2 = DrawTextW(m_hcdcLeftBK2, p2->pszLrc, -1, &rc, uFlags);
                int iTop = m_rcLrcShow.top + (m_cyLrcShow - iHeight2) / 2;
                m_LrcVScrollInfo.iDestTop = m_rcLrcShow.top + (m_cyLrcShow - iHeight) / 2;;
                m_LrcVScrollInfo.fDelay = 0.1f;
                m_LrcVScrollInfo.fTime = g_fTime;
                float ff;
                if (m_iLrcCenter > m_iLastLrcIndex[0])
                {
                    m_LrcVScrollInfo.bDirection = TRUE;
                    m_LrcVScrollInfo.iSrcTop = iTop + iHeight2 + DPIS_LRCSHOWGAP;
                    m_LrcVScrollInfo.iDistance = m_LrcVScrollInfo.iSrcTop - m_LrcVScrollInfo.iDestTop;
                    ff = p1->fTime - p2->fTime;
                    if (m_LrcVScrollInfo.fDelay > ff)
                        m_LrcVScrollInfo.fDelay = ff / 2;
                }
                else
                {
                    m_LrcVScrollInfo.bDirection = FALSE;
                    m_LrcVScrollInfo.iSrcTop = iTop - DPIS_LRCSHOWGAP - iHeight;
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
                return;
            }

            BitBlt(m_hcdcLeftBK2, m_rcLrcShow.left, m_rcLrcShow.top, m_rcLrcShow.right - m_rcLrcShow.left, m_rcLrcShow.bottom - m_rcLrcShow.top,
                m_hcdcLeftBK, m_rcLrcShow.left, m_rcLrcShow.top, SRCCOPY);//清除
            HRGN hLrcShowRgn = CreateRectRgnIndirect(&m_rcLrcShow);
            SelectClipRgn(m_hcdcLeftBK2, hLrcShowRgn);//设置剪辑区
            DeleteObject(hLrcShowRgn);
            m_iLastLrcIndex[0] = m_iLrcCenter;
            m_iLastLrcIndex[1] = g_iCurrLrcIndex;
            m_IsDraw[2] = FALSE;//如果歌词没有变化，下一次周期事件就不要再画了
            ////////////////////////////////////////////////画中间文本
            LRCDATA* p = (LRCDATA*)QKArray_Get(g_Lrc, m_iLrcCenter);
            rc = m_rcLrcShow;
            int iHeight = DrawTextW(m_hcdcLeftBK2, p->pszLrc, -1, &rc,
                DT_NOPREFIX | DT_CALCRECT | (GS.bForceTwoLines ? 0 : DT_CENTER | DT_WORDBREAK));

            int iTop = m_rcLrcShow.top + (m_cyLrcShow - iHeight) / 2;
            iHeight = Lrc_DrawItem(m_iLrcCenter, iTop, TRUE, FALSE, FALSE);
            int iBottom = iTop - DPIS_LRCSHOWGAP;
            iTop += (DPIS_LRCSHOWGAP + iHeight);
            LrcWnd_DrawLrc();

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
            SelectClipRgn(m_hcdcLeftBK2, NULL);//清除剪辑区
            hDC = GetDC(g_hBKLeft);
            BitBlt(hDC, m_rcLrcShow.left, m_rcLrcShow.top, m_rcLrcShow.right - m_rcLrcShow.left, m_rcLrcShow.bottom - m_rcLrcShow.top,
                m_hcdcLeftBK2, m_rcLrcShow.left, m_rcLrcShow.top, SRCCOPY);// 显示
            ReleaseDC(g_hBKLeft, hDC);
        }
    }
    return;
    case IDT_DRAWING_SPE:
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
        BitBlt(m_hcdcLeftBK2, m_xSpe, m_ySpe, DPIS_CXSPE, DPIS_CYSPE, m_hcdcLeftBK, m_xSpe, m_ySpe, SRCCOPY);
        if (BASS_ChannelGetData(g_hStream, fData, BASS_DATA_FFT256) == -1)
        {
            ZeroMemory(cySpeOld, sizeof(cySpeOld));
            ZeroMemory(cySpe, sizeof(cySpe));
        }

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
            RECT rc;
            rc.left = m_xSpe + (DPIS_CXSPEBAR + 1) * i;
            rc.top = m_ySpe + DPIS_CYSPE - cySpeOld[i];
            rc.right = rc.left + DPIS_CXSPEBAR;
            rc.bottom = m_ySpe + DPIS_CYSPE;
            FillRect(m_hcdcLeftBK2, &rc, GC.hbrMyBule);
            //////////制峰值指示矩形
            rc.top = m_ySpe + DPIS_CYSPE - yMaxMark[i];
            rc.bottom = rc.top + 3;
            FillRect(m_hcdcLeftBK2, &rc, GC.hbrMyBule);
        }
        HDC hDC = GetDC(g_hBKLeft);
        BitBlt(hDC, m_xSpe, m_ySpe, DPIS_CXSPE, DPIS_CYSPE, m_hcdcLeftBK2, m_xSpe, m_ySpe, SRCCOPY);// 显示
        ReleaseDC(g_hBKLeft, hDC);
    }
    return;
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
            TimerProc(NULL, 0, IDT_DRAWING_LRC, 0);
            KillTimer(g_hMainWnd, IDT_ANIMATION2);
            return;
        }
        ////////////////////////////////////////////////画中间文本
        int iTop;
        static int iLastTop = 0x80000000;
        float fLastTime = g_fTime - m_LrcVScrollInfo.fTime;
        if (m_LrcVScrollInfo.bDirection)
        {
            iTop = m_LrcVScrollInfo.iSrcTop - fLastTime * m_LrcVScrollInfo.iDistance / m_LrcVScrollInfo.fDelay;
            if (iTop <= m_LrcVScrollInfo.iDestTop)
            {
                m_IsDraw[2] = TRUE;
                TimerProc(NULL, 0, IDT_DRAWING_LRC, 0);
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
                TimerProc(NULL, 0, IDT_DRAWING_LRC, 0);
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

        BitBlt(m_hcdcLeftBK2, m_rcLrcShow.left, m_rcLrcShow.top, m_rcLrcShow.right - m_rcLrcShow.left, m_rcLrcShow.bottom - m_rcLrcShow.top,
            m_hcdcLeftBK, m_rcLrcShow.left, m_rcLrcShow.top, SRCCOPY);// 清除
        HRGN hLrcShowRgn = CreateRectRgnIndirect(&m_rcLrcShow);
        SelectClipRgn(m_hcdcLeftBK2, hLrcShowRgn);//设置剪辑区
        DeleteObject(hLrcShowRgn);
        int iHeight = Lrc_DrawItem(m_iLrcCenter, iTop, TRUE, FALSE, FALSE);
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
        SelectClipRgn(m_hcdcLeftBK2, NULL);//清除剪辑区
        HDC hDC = GetDC(g_hBKLeft);
        hLrcShowRgn = CreateRectRgnIndirect(&m_rcLrcShow);
        SelectClipRgn(hDC, hLrcShowRgn);//设置剪辑区
        DeleteObject(hLrcShowRgn);
        BitBlt(hDC, m_rcLrcShow.left, m_rcLrcShow.top, m_rcLrcShow.right - m_rcLrcShow.left, m_rcLrcShow.bottom - m_rcLrcShow.top,
            m_hcdcLeftBK2, m_rcLrcShow.left, m_rcLrcShow.top, SRCCOPY);// 显示
        ReleaseDC(g_hBKLeft, hDC);
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
LRESULT CALLBACK QKCProc_TBPaint(HWND hWnd, UINT iMsg, WPARAM wParam, LPARAM lParam)//滑块条绘制过程
{
    if (iMsg == QKCTBN_PAINT)
    {
        QKCTBPAINT* Pnt = (QKCTBPAINT*)lParam;
        RECT rc;
        int cxClient, cyClient;
        GetWindowRect(hWnd, &rc);
        cxClient = rc.right - rc.left;
        cyClient = rc.bottom - rc.top;
        ScreenToClient(g_hBKLeft, (LPPOINT)&rc);
        ScreenToClient(g_hBKLeft, (LPPOINT)&rc + 1);
        BitBlt(Pnt->hDC, 0, 0, cxClient, cyClient, m_hcdcLeftBK, rc.left, rc.top, SRCCOPY);//画背景
        rc.left = 6;
        rc.top = Pnt->rcClient->bottom / 2 - 2;
        rc.right = Pnt->rcClient->right - 6;
        rc.bottom = rc.top + 4;
        HBRUSH hBrush = CreateSolidBrush(MYCLR_TBTRACK);
        FillRect(Pnt->hDC, &rc, hBrush);
        DeleteObject(hBrush);

        if (Pnt->byPressed == 1)
            hBrush = CreateSolidBrush(MYCLR_BTPUSHED);
        else
            hBrush = CreateSolidBrush(MYCLR_BTHOT);
        FillRect(Pnt->hDC, Pnt->rc, hBrush);
        DeleteObject(hBrush);
    }
    return 0;
}
void GDIObj_LeftBK(DWORD dwOpe)
{
    if (m_hbmpLeftBK)
        DeleteObject(m_hbmpLeftBK);
    if (m_hbmpLeftBK2)
        DeleteObject(m_hbmpLeftBK2);
    if (dwOpe == GDIOBJOPE_REFRESH)
    {
        RECT rc;
        GetClientRect(g_hBKLeft, &rc);
        HDC hDC = GetDC(g_hBKLeft);
        m_hbmpLeftBK = CreateCompatibleBitmap(hDC, rc.right, rc.bottom);
        m_hbmpLeftBK2 = CreateCompatibleBitmap(hDC, rc.right, rc.bottom);
        SelectObject(m_hcdcLeftBK, m_hbmpLeftBK);
        SelectObject(m_hcdcLeftBK2, m_hbmpLeftBK2);
        ReleaseDC(g_hBKLeft, hDC);
    }
}
int HitTest_BtmBK(int x, int y)
{
    if (x < DPIS_CXTIME || y < 0 || x > GC.cxBKBtm || y > GC.cyBT)
        return -1;

	for (int i = 0; i < BTMBKBTNCOUNT; i++)
    {
        if (x > DPIS_CXTIME + i * GC.cyBT && x < DPIS_CXTIME + (i + 1) * GC.cyBT)
            return i;
    }
    return -1;
}
int HitTest_LrcShow(POINT pt)
{
    if (!g_Lrc->iCount || !g_hStream)
        return -1;

    RECT rc = { 0,0,m_rcLrcShow.right ,0 };
    int y, cy, cyCenter, yHalfLrc = (m_rcLrcShow.top + m_rcLrcShow.bottom) / 2;

    cyCenter = ((LRCDATA*)QKArray_Get(g_Lrc, m_iLrcCenter))->cy;

    rc.top = yHalfLrc - cyCenter / 2;
    rc.bottom = rc.top + cyCenter;
    if (PtInRect(&rc, pt))
    {
        if (cyCenter > 0)
            return m_iLrcCenter;
    }
    else if (pt.y < yHalfLrc)// 落在上半部分
    {
        y = yHalfLrc - cyCenter / 2 - DPIS_LRCSHOWGAP;
        for (int i = m_iLrcCenter - 1; i >= 0; --i)
        {
            cy = ((LRCDATA*)QKArray_Get(g_Lrc, i))->cy;
            if (cy < 0)
                break;
            else if (cy == 0)
                continue;

            rc.bottom = y;
            y -= cy;
            rc.top = y;
            y -= DPIS_LRCSHOWGAP;
            if (PtInRect(&rc, pt))
                return i;
        }
    }
    else// 落在下半部分
    {
        y = yHalfLrc + cyCenter / 2 + DPIS_LRCSHOWGAP;
        for (int i = m_iLrcCenter + 1; i < g_Lrc->iCount; ++i)
        {
            cy = ((LRCDATA*)QKArray_Get(g_Lrc, i))->cy;
            if (cy < 0)
                break;
            else if (cy == 0)
                continue;

            rc.top = y;
            y += cy;
            rc.bottom = y;
            y += DPIS_LRCSHOWGAP;
            if (PtInRect(&rc, pt))
                return i;
        }
    }
    return -1;
}