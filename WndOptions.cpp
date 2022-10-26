#include "WndOptions.h"

#include <Windows.h>

#include "MyProject.h"
#include "GlobalVar.h"
#include "resource.h"

void Settings_Read()
{
    WCHAR szBuffer[MAXPROFILEBUFFER];
    UINT u;
    // 缺省编码
    GS.uDefTextCode = GetPrivateProfileIntW(PPF_SECTIONLRC, PPF_KEY_DEFTEXTCODE, 0, g_pszProfie);
    // 歌词目录
    GetPrivateProfileStringW(PPF_SECTIONLRC, PPF_KEY_LRCDIR, NULL, szBuffer, MAXPROFILEBUFFER, g_pszProfie);
    delete[] GS.pszLrcDir;
    GS.pszLrcDir = new WCHAR[lstrlenW(szBuffer) + 1];
    lstrcpyW(GS.pszLrcDir, szBuffer);
    // 禁用过渡动画
    GS.bLrcAnimation = !GetPrivateProfileIntW(PPF_SECTIONLRC, PPF_KEY_DISABLEVANIMATION, FALSE, g_pszProfie);
    // 禁止换行
    GS.bForceTwoLines = GetPrivateProfileIntW(PPF_SECTIONLRC, PPF_KEY_DISABLEWORDBREAK, TRUE, g_pszProfie);
    // 禁用阴影
    GS.bDTLrcShandow = !GetPrivateProfileIntW(PPF_SECTIONLRC, PPF_KEY_DISABLEDTLRCSHANDOW, FALSE, g_pszProfie);
    // DT字体
    GetPrivateProfileStringW(PPF_SECTIONLRC, PPF_KEY_DTFONTNAME, L"微软雅黑", szBuffer, MAXPROFILEBUFFER, g_pszProfie);
    delete[] GS.pszDTLrcFontName;
    GS.pszDTLrcFontName = new WCHAR[lstrlenW(szBuffer) + 1];
    lstrcpyW(GS.pszDTLrcFontName, szBuffer);

    GS.uDTLrcFontSize = GetPrivateProfileIntW(PPF_SECTIONLRC, PPF_KEY_DTFONTSIZE, 40, g_pszProfie);
    GS.uDTLrcFontWeight = GetPrivateProfileIntW(PPF_SECTIONLRC, PPF_KEY_DTFONTWEIGHT, 400, g_pszProfie);
    // DT颜色
    GS.crDTLrc1 = GetPrivateProfileIntW(PPF_SECTIONLRC, PPF_KEY_DTLRCCLR1, 0x00FF00, g_pszProfie);
    GS.crDTLrc2 = GetPrivateProfileIntW(PPF_SECTIONLRC, PPF_KEY_DTLRCCLR2, 0x0000FF, g_pszProfie);
    // DT透明度
    GS.uDTLrcTransparent = GetPrivateProfileIntW(PPF_SECTIONLRC, PPF_KEY_DTLRCTRANSPARENT, 0xFF, g_pszProfie);
    // 空行替代
    GetPrivateProfileStringW(PPF_SECTIONLRC, PPF_KEY_DTLRCSPACELINE, L"...", szBuffer, MAXPROFILEBUFFER, g_pszProfie);
    delete[] GS.pszDTLrcSpaceLine;
    GS.pszDTLrcSpaceLine = new WCHAR[lstrlenW(szBuffer) + 1];
    lstrcpyW(GS.pszDTLrcSpaceLine, szBuffer);
    // SC字体
    GetPrivateProfileStringW(PPF_SECTIONLRC, PPF_KEY_SCFONTNAME, L"微软雅黑", szBuffer, MAXPROFILEBUFFER, g_pszProfie);
    delete[] GS.pszSCLrcFontName;
    GS.pszSCLrcFontName = new WCHAR[lstrlenW(szBuffer) + 1];
    lstrcpyW(GS.pszSCLrcFontName, szBuffer);

    GS.uSCLrcFontSize = GetPrivateProfileIntW(PPF_SECTIONLRC, PPF_KEY_SCFONTSIZE, 40, g_pszProfie);
    GS.uSCLrcFontWeight = GetPrivateProfileIntW(PPF_SECTIONLRC, PPF_KEY_SCFONTWEIGHT, 400, g_pszProfie);
    // SC颜色
    GS.crSCLrc1 = GetPrivateProfileIntW(PPF_SECTIONLRC, PPF_KEY_SCLRCCLR1, 0x00FF00, g_pszProfie);
    GS.crSCLrc2 = GetPrivateProfileIntW(PPF_SECTIONLRC, PPF_KEY_SCLRCCLR2, 0x0000FF, g_pszProfie);
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

    WritePrivateProfileStringW(PPF_SECTIONLRC, PPF_KEY_DTFONTNAME, GS.pszDTLrcFontName, g_pszProfie);

    wsprintfW(sz, L"%u", GS.uDTLrcFontSize);
    WritePrivateProfileStringW(PPF_SECTIONLRC, PPF_KEY_DTFONTSIZE, sz, g_pszProfie);

    wsprintfW(sz, L"%u", GS.uDTLrcFontWeight);
    WritePrivateProfileStringW(PPF_SECTIONLRC, PPF_KEY_DTFONTWEIGHT, sz, g_pszProfie);

    wsprintfW(sz, L"%u", GS.crDTLrc1);
    WritePrivateProfileStringW(PPF_SECTIONLRC, PPF_KEY_DTLRCCLR1, sz, g_pszProfie);
    wsprintfW(sz, L"%u", GS.crDTLrc2);
    WritePrivateProfileStringW(PPF_SECTIONLRC, PPF_KEY_DTLRCCLR2, sz, g_pszProfie);

    wsprintfW(sz, L"%u", GS.uDTLrcTransparent);
    WritePrivateProfileStringW(PPF_SECTIONLRC, PPF_KEY_DTLRCTRANSPARENT, sz, g_pszProfie);

    WritePrivateProfileStringW(PPF_SECTIONLRC, PPF_KEY_DTLRCSPACELINE, GS.pszDTLrcSpaceLine, g_pszProfie);

    WritePrivateProfileStringW(PPF_SECTIONLRC, PPF_KEY_DTFONTNAME, GS.pszDTLrcFontName, g_pszProfie);

    wsprintfW(sz, L"%u", GS.uSCLrcFontSize);
    WritePrivateProfileStringW(PPF_SECTIONLRC, PPF_KEY_SCFONTSIZE, sz, g_pszProfie);

    wsprintfW(sz, L"%u", GS.uSCLrcFontWeight);
    WritePrivateProfileStringW(PPF_SECTIONLRC, PPF_KEY_SCFONTWEIGHT, sz, g_pszProfie);

    wsprintfW(sz, L"%u", GS.crSCLrc1);
    WritePrivateProfileStringW(PPF_SECTIONLRC, PPF_KEY_SCLRCCLR1, sz, g_pszProfie);
    wsprintfW(sz, L"%u", GS.crSCLrc2);
    WritePrivateProfileStringW(PPF_SECTIONLRC, PPF_KEY_SCLRCCLR2, sz, g_pszProfie);

    wsprintfW(sz, L"%u", GS.uSCLrcLineGap);
    WritePrivateProfileStringW(PPF_SECTIONLRC, PPF_KEY_SCLINEGAP, sz, g_pszProfie);

    wsprintfW(sz, L"%u", GS.uSCLrcAlign);
    WritePrivateProfileStringW(PPF_SECTIONLRC, PPF_KEY_SCALIGN, sz, g_pszProfie);

    wsprintfW(sz, L"%u", GS.uSCLrcOffset);
    WritePrivateProfileStringW(PPF_SECTIONLRC, PPF_KEY_SCOFFSET, sz, g_pszProfie);
}
INT_PTR CALLBACK DlgProc_OptVisual(HWND hDlg, UINT message, WPARAM wParam, LPARAM lParam)
{
    switch (message)
    {
    case WM_NCPAINT:
        return TRUE;
    }
    return FALSE;
}
INT_PTR CALLBACK DlgProc_OptPlaying(HWND hDlg, UINT message, WPARAM wParam, LPARAM lParam)
{
    switch (message)
    {
    case WM_INITDIALOG:
    {
        DlgProc_OptPlaying(hDlg, WM_COMMAND, MAKEWPARAM(IDC_BT_REFRESHDEV, BN_CLICKED), 0);
    }
    return FALSE;
    case WM_COMMAND:
    {
        switch (HIWORD(wParam))
        {
        case CBN_SELCHANGE:
            BASS_Init(SendMessageW((HWND)lParam, CB_GETCURSEL, 0, 0) + 1, 44100, BASS_DEVICE_REINIT, g_hMainWnd, NULL);
            return TRUE;
        case BN_CLICKED:
        {
            switch (LOWORD(wParam))
            {
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
        return TRUE;
        }
    }
    case WM_NCPAINT:
        return TRUE;
    }
    return FALSE;
}
INT_PTR CALLBACK DlgProc_OptLrc(HWND hDlg, UINT message, WPARAM wParam, LPARAM lParam)
{
    static HBRUSH hbr1/*DT左*/, hbr2/*DT右*/, hbr3/*SC左*/, hbr4/*SC右*/;
    static HWND hST1/*DT左*/, hST2/*DT右*/, hST3/*SC左*/, hST4/*SC右*/;
    switch (message)
    {
    case WM_INITDIALOG:
    {
        WCHAR sz[MAXPROFILEBUFFER];
        HWND hCtrl;
        hST1 = GetDlgItem(hDlg, IDC_ST_DTLRCCLR1);
        hST2 = GetDlgItem(hDlg, IDC_ST_DTLRCCLR2);
		hST3 = GetDlgItem(hDlg, IDC_ST_SCLRCCLR1);
		hST4 = GetDlgItem(hDlg, IDC_ST_SCLRCCLR2);
		////////////////////////////////////////////////全局
		////////////////缺省编码
		hCtrl = GetDlgItem(hDlg, IDC_CB_DEFTEXTCODE);
		SendMessageW(hCtrl, CB_INSERTSTRING, -1, (LPARAM)L"自动判断（不一定准确）");
		SendMessageW(hCtrl, CB_INSERTSTRING, -1, (LPARAM)L"GBK");
		SendMessageW(hCtrl, CB_INSERTSTRING, -1, (LPARAM)L"UTF-8");
		SendMessageW(hCtrl, CB_INSERTSTRING, -1, (LPARAM)L"UTF-16LE");
		SendMessageW(hCtrl, CB_INSERTSTRING, -1, (LPARAM)L"UTF-16BE");
        SendMessageW(hCtrl, CB_SETCURSEL, GS.uDefTextCode, 0);
        ////////////////禁止自动换行
        SendDlgItemMessageW(hDlg, IDC_CB_DISABLEWORDBREAK, BM_SETCHECK, GS.bForceTwoLines ? BST_CHECKED : BST_UNCHECKED, 0);
        ////////////////歌词搜索目录
        SetDlgItemTextW(hDlg, IDC_ED_LRCDIR, GS.pszLrcDir);


        ////////////////////////////////////////////////滚动歌词
        ////////////////禁用过渡动画
        SendDlgItemMessageW(hDlg, IDC_CB_DISABLEVANIMATION, BM_SETCHECK, GS.bLrcAnimation ? BST_UNCHECKED : BST_CHECKED, 0);
        ////////////////滚动歌词字体
        SetDlgItemTextW(hDlg, IDC_ED_SCLRCFONTINFO, GS.pszSCLrcFontName);

        SendDlgItemMessageW(hDlg, IDC_ED_SCLRCFONTINFO, EM_SETSEL, -2, -1);
        SendDlgItemMessageW(hDlg, IDC_ED_SCLRCFONTINFO, EM_REPLACESEL, FALSE, (LPARAM)L",");// 加入文本

        wsprintfW(sz, L"%u,", GS.uSCLrcFontSize);
        SetPropW(GetDlgItem(hDlg, IDC_ED_SCLRCFONTINFO), PROP_SCLRCFONTSIZE, (HANDLE)GS.uSCLrcFontSize);
        SendDlgItemMessageW(hDlg, IDC_ED_SCLRCFONTINFO, EM_SETSEL, -2, -1);
        SendDlgItemMessageW(hDlg, IDC_ED_SCLRCFONTINFO, EM_REPLACESEL, FALSE, (LPARAM)sz);

        wsprintfW(sz, L"%u", GS.uSCLrcFontWeight);
        SetPropW(GetDlgItem(hDlg, IDC_ED_SCLRCFONTINFO), PROP_SCLRCFONTWEIGHT, (HANDLE)GS.uSCLrcFontWeight);
        SendDlgItemMessageW(hDlg, IDC_ED_SCLRCFONTINFO, EM_SETSEL, -2, -1);
        SendDlgItemMessageW(hDlg, IDC_ED_SCLRCFONTINFO, EM_REPLACESEL, FALSE, (LPARAM)sz);// 加入文本
        ////////////////滚动歌词颜色
        hbr3 = CreateSolidBrush(QKCommonClrToGDIClr(GS.crSCLrc1));
        hbr4 = CreateSolidBrush(QKCommonClrToGDIClr(GS.crSCLrc2));
        SetPropW(GetDlgItem(hDlg, IDC_ST_SCLRCCLR1), PROP_SCLRCCLR1, (HANDLE)GS.crSCLrc1);
        SetPropW(GetDlgItem(hDlg, IDC_ST_SCLRCCLR2), PROP_SCLRCCLR2, (HANDLE)GS.crSCLrc2);
        ////////////////行间距
        wsprintfW(sz, L"%u", GS.uSCLrcLineGap);
        SetDlgItemTextW(hDlg, IDC_ED_SCLRCLINEGAP, sz);
        ////////////////对齐方式
        hCtrl = GetDlgItem(hDlg, IDC_CB_SCLRCALIGN);
        SendMessageW(hCtrl, CB_INSERTSTRING, -1, (LPARAM)L"居中对齐");
        SendMessageW(hCtrl, CB_INSERTSTRING, -1, (LPARAM)L"左对齐");
        SendMessageW(hCtrl, CB_INSERTSTRING, -1, (LPARAM)L"右对齐");
        SendMessageW(hCtrl, CB_SETCURSEL, GS.uSCLrcAlign, 0);
        ////////////////对齐偏移
        wsprintfW(sz, L"%u", GS.uSCLrcOffset);
        SetDlgItemTextW(hDlg, IDC_ED_SCLRCALIGNOFFSET, sz);


        ////////////////////////////////////////////////桌面歌词
        ////////////////禁用桌面歌词阴影
        SendDlgItemMessageW(hDlg, IDC_CB_DISABLEDTLRCSHANDOW, BM_SETCHECK, GS.bDTLrcShandow ? BST_UNCHECKED : BST_CHECKED, 0);
        ////////////////桌面歌词字体
        SetDlgItemTextW(hDlg, IDC_ED_DTLRCFONTINFO, GS.pszDTLrcFontName);

        SendDlgItemMessageW(hDlg, IDC_ED_DTLRCFONTINFO, EM_SETSEL, -2, -1);
        SendDlgItemMessageW(hDlg, IDC_ED_DTLRCFONTINFO, EM_REPLACESEL, FALSE, (LPARAM)L",");// 加入文本

        wsprintfW(sz, L"%u,", GS.uDTLrcFontSize);
        SetPropW(GetDlgItem(hDlg, IDC_ED_DTLRCFONTINFO), PROP_DTLRCFONTSIZE, (HANDLE)GS.uDTLrcFontSize);
        SendDlgItemMessageW(hDlg, IDC_ED_DTLRCFONTINFO, EM_SETSEL, -2, -1);
        SendDlgItemMessageW(hDlg, IDC_ED_DTLRCFONTINFO, EM_REPLACESEL, FALSE, (LPARAM)sz);

        wsprintfW(sz, L"%u", GS.uDTLrcFontWeight);
        SetPropW(GetDlgItem(hDlg, IDC_ED_DTLRCFONTINFO), PROP_DTLRCFONTWEIGHT, (HANDLE)GS.uDTLrcFontWeight);
        SendDlgItemMessageW(hDlg, IDC_ED_DTLRCFONTINFO, EM_SETSEL, -2, -1);
        SendDlgItemMessageW(hDlg, IDC_ED_DTLRCFONTINFO, EM_REPLACESEL, FALSE, (LPARAM)sz);// 加入文本
        ////////////////桌面歌词颜色
        hbr1 = CreateSolidBrush(QKCommonClrToGDIClr(GS.crDTLrc1));
        hbr2 = CreateSolidBrush(QKCommonClrToGDIClr(GS.crDTLrc2));
        SetPropW(GetDlgItem(hDlg, IDC_ST_DTLRCCLR1), PROP_DTLRCCLR1, (HANDLE)GS.crDTLrc1);
        SetPropW(GetDlgItem(hDlg, IDC_ST_DTLRCCLR2), PROP_DTLRCCLR2, (HANDLE)GS.crDTLrc2);
        ////////////////桌面歌词透明度
        SendDlgItemMessageW(hDlg, IDC_TB_DTLRCTRANSPARENT, TBM_SETRANGE, FALSE, MAKELPARAM(0, 0xFF));
        SendDlgItemMessageW(hDlg, IDC_TB_DTLRCTRANSPARENT, TBM_SETPOS, TRUE, GS.uDTLrcTransparent);
        ////////////////空行替代
        SetDlgItemTextW(hDlg, IDC_ED_DTLRCSPACELINE, GS.pszDTLrcSpaceLine);
    }
    return FALSE;
    case WM_NCPAINT:
        return TRUE;
    case WM_COMMAND:
    {
        switch (HIWORD(wParam))
        {
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
            }
        }
        return TRUE;
        }
    }
    return TRUE;
    case WM_CTLCOLORSTATIC:
    {
        if (lParam == (LPARAM)hST1)
            return (LRESULT)hbr1;
        else if (lParam == (LPARAM)hST2)
            return (LRESULT)hbr2;
        else if (lParam == (LPARAM)hST3)
            return (LRESULT)hbr3;
        else if (lParam == (LPARAM)hST4)
            return (LRESULT)hbr4;
    }
    return NULL;
    case WM_DESTROY:
    {
        DeleteObject((HBRUSH)GetPropW(GetDlgItem(hDlg, IDC_ST_DTLRCCLR1), PROP_DTLRCCLR1));
        DeleteObject((HBRUSH)GetPropW(GetDlgItem(hDlg, IDC_ST_DTLRCCLR2), PROP_DTLRCCLR2));
        DeleteObject((HBRUSH)GetPropW(GetDlgItem(hDlg, IDC_ST_SCLRCCLR1), PROP_SCLRCCLR1));
        DeleteObject((HBRUSH)GetPropW(GetDlgItem(hDlg, IDC_ST_SCLRCCLR2), PROP_SCLRCCLR2));
    }
    return TRUE;
    }
    return FALSE;
}
INT_PTR CALLBACK DlgProc_OptHotKey(HWND hDlg, UINT message, WPARAM wParam, LPARAM lParam)
{
    switch (message)
    {
    case WM_NCPAINT:
        return TRUE;
    }
    return FALSE;
}
INT_PTR CALLBACK DlgProc_Options(HWND hDlg, UINT message, WPARAM wParam, LPARAM lParam)
{
    static int iOptionIndex = -1;
    static HWND hChild[OPTIONSWNDTABCOUNT];
    static int cyChild[OPTIONSWNDTABCOUNT];
    static int iScrollOffset[OPTIONSWNDTABCOUNT] = { 0 };
    static int iSBSize;
    static int cyLB;
    static BOOL bSBShow = FALSE;
    switch (message)
    {
    case WM_INITDIALOG:
    {
        iSBSize = GetSystemMetrics(SM_CXVSCROLL);
        PCWSTR pszOptionsTip[] =
        {
            L"显示",
            L"播放",
            L"歌词",
            L"快捷键"
        };

        for (int i = 0; i < ARRAYSIZE(pszOptionsTip); ++i)
        {
            SendDlgItemMessageW(hDlg, IDC_LB_OPTIONS, LB_INSERTSTRING, -1, (LPARAM)pszOptionsTip[i]);// 填充列表框
        }

        PCWSTR DialogID[] =
        {
            MAKEINTRESOURCEW(IDD_OPTVISUAL),
            MAKEINTRESOURCEW(IDD_OPTPLAYING),
            MAKEINTRESOURCEW(IDD_OPTLRC),
            MAKEINTRESOURCEW(IDD_OPTHOTKEY)
        };
        DLGPROC DialogProc[] =
        {
            DlgProc_OptVisual,
            DlgProc_OptPlaying,
            DlgProc_OptLrc,
            DlgProc_OptHotKey
        };
        RECT rcLB, rcClient;
        GetClientRect(hDlg, &rcClient);
        GetWindowRect(GetDlgItem(hDlg, IDC_LB_OPTIONS), &rcLB);
        cyLB = rcLB.bottom - rcLB.top;
        QKRcScreenToClient(hDlg, &rcLB);

        RECT rc;
        for (int i = 0; i < OPTIONSWNDTABCOUNT; ++i)
        {
            hChild[i] = CreateDialogParamW(g_hInst, DialogID[i], hDlg, DialogProc[i], 0);
            GetClientRect(hChild[i], &rc);
            cyChild[i] = rc.bottom;

            SetParent(hChild[i], hDlg);
            SetWindowPos(hChild[i], NULL,
                rcLB.right + rcLB.left,
                rcLB.top,
                rcClient.right - (rcLB.right + rcLB.left) - iSBSize,
                rcLB.bottom - rcLB.top,
                SWP_NOZORDER);
            SetWindowLongPtrW(hChild[i], GWL_STYLE, WS_CHILD);
            ShowWindow(hChild[i], SW_HIDE);
        }
        ShowWindow(hChild[0], SW_SHOW);

        SetWindowPos(GetDlgItem(hDlg, IDC_SB_SETTINGS), NULL, rcClient.right - iSBSize, rcLB.top, iSBSize, cyLB, SWP_NOZORDER);

        iOptionIndex = 0;
        goto SwitchTab;
    }
    return FALSE;
    case WM_MEASUREITEM:
    {
        MEASUREITEMSTRUCT* p = (MEASUREITEMSTRUCT*)lParam;
        if (p->CtlID == IDC_LB_OPTIONS && p->CtlType == ODT_LISTBOX)
        {
            p->itemHeight = DPI(30);
        }
    }
    return TRUE;
    case WM_DRAWITEM:
    {
        DRAWITEMSTRUCT* p = (DRAWITEMSTRUCT*)lParam;
        if (p->CtlID == IDC_LB_OPTIONS && p->CtlType == ODT_LISTBOX)
        {
            if (p->itemID != -1)
            {
                COLORREF crOld;
                // 画背景
                if (p->itemID == iOptionIndex)
                {
                    FillRect(p->hDC, &p->rcItem, GC.hbrCyanDeeper);
                    crOld = SetTextColor(p->hDC, QKCOLOR_WHITE);
                }
                else
                {
                    FillRect(p->hDC, &p->rcItem, (HBRUSH)GetStockObject(WHITE_BRUSH));
                    crOld = SetTextColor(p->hDC, QKCOLOR_BLACK);
                }
                // 画文本
                int iBKModeOld = SetBkMode(p->hDC, TRANSPARENT);
                HGDIOBJ hFontOld = SelectObject(p->hDC, g_hFontDrawing);

                RECT rcText;
                CopyRect(&rcText, &p->rcItem);
                rcText.left += DPI(10);
                if (p->itemData)
                    DrawTextW(p->hDC, (PCWSTR)p->itemData, -1, &rcText, DT_SINGLELINE | DT_VCENTER);

                SelectObject(p->hDC, hFontOld);
                SetBkMode(p->hDC, iBKModeOld);
                SetTextColor(p->hDC, crOld);
            }
        }
    }
    return TRUE;
    case WM_COMMAND:
    {
        switch (LOWORD(wParam))
        {
        case IDOK:
        {
            WCHAR sz[MAXPROFILEBUFFER];
            int iPos;
            HWND hWnd;

            hWnd = hChild[2];
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

			iPos = QKStrInStr(GS.pszDTLrcFontName, L",");
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

            delete[] GS.pszSCLrcFontName;
            iLength = GetWindowTextLengthW(GetDlgItem(hWnd, IDC_ED_FONTINFO));
            GS.pszSCLrcFontName = new WCHAR[iLength + 1];
            GetDlgItemTextW(hWnd, IDC_ED_FONTINFO, GS.pszSCLrcFontName, iLength + 1);

            iPos = QKStrInStr(GS.pszSCLrcFontName, L",");
            *(GS.pszSCLrcFontName + iPos - 1) = L'\0';
            GS.uSCLrcFontSize = (UINT)GetPropW(GetDlgItem(hWnd, IDC_ED_SCLRCFONTINFO), PROP_SCLRCFONTSIZE);
            GS.uSCLrcFontWeight = (UINT)GetPropW(GetDlgItem(hWnd, IDC_ED_SCLRCFONTINFO), PROP_SCLRCFONTWEIGHT);

            GS.crSCLrc1 = (COLORREF)GetPropW(GetDlgItem(hWnd, IDC_ST_SCLRCCLR1), PROP_SCLRCCLR1);
            GS.crSCLrc2 = (COLORREF)GetPropW(GetDlgItem(hWnd, IDC_ST_SCLRCCLR2), PROP_SCLRCCLR2);

            GetDlgItemTextW(hWnd, IDC_ED_SCLRCLINEGAP, sz, MAXPROFILEBUFFER);
            GS.uSCLrcLineGap = StrToIntW(sz);

            GS.uSCLrcAlign = SendDlgItemMessageW(hWnd, IDC_CB_SCLRCALIGN, CB_GETCURSEL, 0, 0);

            GetDlgItemTextW(hWnd, IDC_ED_SCLRCLINEGAP, sz, MAXPROFILEBUFFER);
            GS.uSCLrcOffset = StrToIntW(sz);

            Settings_Save();
            EndDialog(hDlg, 0);
        }
        return TRUE;
        case IDCANCEL:
            EndDialog(hDlg, 0);
            return TRUE;
        case IDC_LB_OPTIONS:
        {
            if (HIWORD(wParam) == LBN_SELCHANGE)
            {
                iOptionIndex = SendMessageW((HWND)lParam, LB_GETCURSEL, 0, 0);
            SwitchTab:// 窗口创建后跳过来切换到第一页
                for (int i = 0; i < OPTIONSWNDTABCOUNT; ++i)
                {
                    if (i == iOptionIndex)
                    {
                        ShowWindow(hChild[i], SW_SHOW);
                        if (cyChild[i] > cyLB)
                        {
                            bSBShow = TRUE;
                            ShowWindow(GetDlgItem(hDlg, IDC_SB_SETTINGS), SW_SHOW);
                            SCROLLINFO si;
                            si.cbSize = sizeof(SCROLLINFO);
                            si.fMask = SIF_ALL;
                            si.nPos = iScrollOffset[i];
                            si.nMax = cyChild[i];
                            si.nMin = 0;
                            si.nPage = cyLB;
                            SetScrollInfo(GetDlgItem(hDlg, IDC_SB_SETTINGS), SB_CTL, &si, TRUE);
                        }
                        else
                        {
                            bSBShow = FALSE;
                            ShowWindow(GetDlgItem(hDlg, IDC_SB_SETTINGS), SW_HIDE);
                        }
                    }
                    else
                    {
                        ShowWindow(hChild[i], SW_HIDE);
                    }
                }
                InvalidateRect((HWND)lParam, NULL, TRUE);
            }
        }
        return TRUE;
        }
    }
    return TRUE;
    case WM_MOUSEWHEEL:
    {
		if (iOptionIndex < 0 || iOptionIndex >= OPTIONSWNDTABCOUNT || !bSBShow)
            return TRUE;

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

        iScrollOffset[iOptionIndex] = si.nPos;
        SetScrollPos(GetDlgItem(hDlg, IDC_SB_SETTINGS), SB_CTL, si.nPos, TRUE);
        ScrollWindow(hChild[iOptionIndex], 0, iPos - si.nPos, NULL, NULL);
    }
    return TRUE;
    case WM_VSCROLL:
    {
        if (iOptionIndex < 0 || iOptionIndex >= OPTIONSWNDTABCOUNT || !bSBShow)
            return TRUE;

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

        iScrollOffset[iOptionIndex] = si.nPos;
        SetScrollPos((HWND)lParam, SB_CTL, si.nPos, TRUE);
        ScrollWindow(hChild[iOptionIndex], 0, iPos - si.nPos, NULL, NULL);
    }
    return TRUE;
    case WM_CLOSE:
        EndDialog(hDlg, 0);
        return TRUE;
    case WM_DESTROY:
        ZeroMemory(iScrollOffset, sizeof(iScrollOffset));
        return TRUE;
    }
    return FALSE;
}