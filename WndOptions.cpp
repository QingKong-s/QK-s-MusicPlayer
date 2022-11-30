/*
* WndOptions.cpp
* 包含设置对话框窗口过程和函数的实现
*/
#include "WndOptions.h"

#include <Windows.h>
#include <strsafe.h>

#include "bassmidi.h"

#include "MyProject.h"
#include "GlobalVar.h"
#include "resource.h"
#include "WndMain.h"
#include "WndLrc.h"

BOOL m_bSoundFontChanged = FALSE;

void Settings_Read()
{
    HQKINI hINI = QKINIParse(g_pszProfie);
    // 缺省编码
    GS.iDefTextCode = QKINIReadInt(hINI, PPF_SECTIONLRC, PPF_KEY_DEFTEXTCODE, 0);
    // 歌词目录
    delete[] GS.pszLrcDir;
	GS.pszLrcDir = QKINIReadString2(hINI, PPF_SECTIONLRC, PPF_KEY_LRCDIR, NULL);
	// 禁用过渡动画
	GS.bLrcAnimation = !QKINIReadInt(hINI, PPF_SECTIONLRC, PPF_KEY_DISABLEVANIMATION, FALSE);
	// 禁止换行
	GS.bForceTwoLines = QKINIReadInt(hINI, PPF_SECTIONLRC, PPF_KEY_DISABLEWORDBREAK, TRUE);
	// 禁用阴影
	GS.bDTLrcShandow = !QKINIReadInt(hINI, PPF_SECTIONLRC, PPF_KEY_DISABLEDTLRCSHANDOW, FALSE);
	// DT字体
	delete[] GS.pszDTLrcFontName;
	GS.pszDTLrcFontName = QKINIReadString2(hINI, PPF_SECTIONLRC, PPF_KEY_DTFONTNAME, L"微软雅黑");

	GS.iDTLrcFontSize = QKINIReadInt(hINI, PPF_SECTIONLRC, PPF_KEY_DTFONTSIZE, 40);
	GS.iDTLrcFontWeight = QKINIReadInt(hINI, PPF_SECTIONLRC, PPF_KEY_DTFONTWEIGHT, 400);
	// DT颜色
	GS.crDTLrc1 = QKCommonClrToGDIClr(QKINIReadInt(hINI, PPF_SECTIONLRC, PPF_KEY_DTLRCCLR1, QKGDIClrToCommonClr(QKCOLOR_GREEN)));
	GS.crDTLrc2 = QKCommonClrToGDIClr(QKINIReadInt(hINI, PPF_SECTIONLRC, PPF_KEY_DTLRCCLR2, QKGDIClrToCommonClr(QKCOLOR_BLUE)));
	// DT透明度
	GS.iDTLrcTransparent = QKINIReadInt(hINI, PPF_SECTIONLRC, PPF_KEY_DTLRCTRANSPARENT, 0xFF);
	// 空行替代
	delete[] GS.pszDTLrcSpaceLine;
	GS.pszDTLrcSpaceLine = QKINIReadString2(hINI, PPF_SECTIONLRC, PPF_KEY_DTLRCSPACELINE, L"...");
	// SC字体
	delete[] GS.pszSCLrcFontName;
	GS.pszSCLrcFontName = QKINIReadString2(hINI, PPF_SECTIONLRC, PPF_KEY_SCFONTNAME, L"微软雅黑");

	GS.iSCLrcFontSize = QKINIReadInt(hINI, PPF_SECTIONLRC, PPF_KEY_SCFONTSIZE, 18);
	GS.iSCLrcFontWeight = QKINIReadInt(hINI, PPF_SECTIONLRC, PPF_KEY_SCFONTWEIGHT, 400);
	// SC颜色
	GS.crSCLrc1 = QKCommonClrToGDIClr(QKINIReadInt(hINI, PPF_SECTIONLRC, PPF_KEY_SCLRCCLR1, QKGDIClrToCommonClr(QKCOLOR_CYANDEEPER)));
	GS.crSCLrc2 = QKCommonClrToGDIClr(QKINIReadInt(hINI, PPF_SECTIONLRC, PPF_KEY_SCLRCCLR2, QKGDIClrToCommonClr(QKCOLOR_RED)));
	// SC对齐
	GS.iSCLrcAlign = QKINIReadInt(hINI, PPF_SECTIONLRC, PPF_KEY_SCALIGN, 0);
	// SC行距
	GS.iSCLrcLineGap = QKINIReadInt(hINI, PPF_SECTIONLRC, PPF_KEY_SCLINEGAP, 10);
	// SC对齐偏移
	GS.iSCLrcOffset = QKINIReadInt(hINI, PPF_SECTIONLRC, PPF_KEY_SCOFFSET, 0);
	// 呈现方式
	GS.iVisualMode = QKINIReadInt(hINI, PPF_SECTIONVISUAL, PPF_KEY_VISUALMODE, 0);
	// 水平滚动文本
	GS.bHScrollText = QKINIReadInt(hINI, PPF_SECTIONVISUAL, PPF_KEY_HSCROLLTEXT, 0);
	// 封面尺寸1
	GS.iAlbumPicSize1 = QKINIReadInt(hINI, PPF_SECTIONVISUAL, PPF_KEY_ALBUMPICSIZE1, 210);
	// 封面尺寸2
	GS.iAlbumPicSize2 = QKINIReadInt(hINI, PPF_SECTIONVISUAL, PPF_KEY_ALBUMPICSIZE2, 310);
	// DT边框
	GS.crDTLrcBorder = QKINIReadInt(hINI, PPF_SECTIONLRC, PPF_KEY_DTLRCBORDERCLR, QKCOLOR_BLACK);
	GS.bEnableDTLrcBorder = QKINIReadInt(hINI, PPF_SECTIONLRC, PPF_KEY_ENABLEDTLRCBORDER, TRUE);
	// 音色库
	delete[] GS.pszSoundFont;
	GS.pszSoundFont = QKINIReadString2(hINI, PPF_SECTIONPLAYING, PPF_KEY_SOUNDFONT, NULL);
	if (m_bSoundFontChanged)
		BASS_UpdateSoundFont();

    QKINIClose(hINI);
}
void Settings_Save()
{
	HQKINI hINI = QKINIParse(g_pszProfie);
	WCHAR sz[MAXPROFILEBUFFER];
	wsprintfW(sz, L"%u", GS.iDefTextCode);
	QKINIWriteInt(hINI, PPF_SECTIONLRC, PPF_KEY_DEFTEXTCODE, GS.iDefTextCode);

	QKINIWriteString(hINI, PPF_SECTIONLRC, PPF_KEY_LRCDIR, GS.pszLrcDir);

	QKINIWriteInt(hINI, PPF_SECTIONLRC, PPF_KEY_DISABLEVANIMATION, !GS.bLrcAnimation);

	QKINIWriteInt(hINI, PPF_SECTIONLRC, PPF_KEY_DISABLEWORDBREAK, GS.bForceTwoLines);

	QKINIWriteInt(hINI, PPF_SECTIONLRC, PPF_KEY_DISABLEDTLRCSHANDOW, !GS.bDTLrcShandow);

	QKINIWriteString(hINI, PPF_SECTIONLRC, PPF_KEY_DTFONTNAME, GS.pszDTLrcFontName);

	QKINIWriteInt(hINI, PPF_SECTIONLRC, PPF_KEY_DTFONTSIZE, GS.iDTLrcFontSize);

	QKINIWriteInt(hINI, PPF_SECTIONLRC, PPF_KEY_DTFONTWEIGHT, GS.iDTLrcFontWeight);

	QKINIWriteInt(hINI, PPF_SECTIONLRC, PPF_KEY_DTLRCCLR1, QKGDIClrToCommonClr(GS.crDTLrc1));
	QKINIWriteInt(hINI, PPF_SECTIONLRC, PPF_KEY_DTLRCCLR2, QKGDIClrToCommonClr(GS.crDTLrc2));

	QKINIWriteInt(hINI, PPF_SECTIONLRC, PPF_KEY_DTLRCTRANSPARENT, GS.iDTLrcTransparent);

	QKINIWriteString(hINI, PPF_SECTIONLRC, PPF_KEY_DTLRCSPACELINE, GS.pszDTLrcSpaceLine);

	QKINIWriteString(hINI, PPF_SECTIONLRC, PPF_KEY_SCFONTNAME, GS.pszSCLrcFontName);

	QKINIWriteInt(hINI, PPF_SECTIONLRC, PPF_KEY_SCFONTSIZE, GS.iSCLrcFontSize);

	QKINIWriteInt(hINI, PPF_SECTIONLRC, PPF_KEY_SCFONTWEIGHT, GS.iSCLrcFontWeight);

	QKINIWriteInt(hINI, PPF_SECTIONLRC, PPF_KEY_SCLRCCLR1, QKGDIClrToCommonClr(GS.crSCLrc1));
	QKINIWriteInt(hINI, PPF_SECTIONLRC, PPF_KEY_SCLRCCLR2, QKGDIClrToCommonClr(GS.crSCLrc2));

	QKINIWriteInt(hINI, PPF_SECTIONLRC, PPF_KEY_SCLINEGAP, GS.iSCLrcLineGap);

	QKINIWriteInt(hINI, PPF_SECTIONLRC, PPF_KEY_SCALIGN, GS.iSCLrcAlign);

	QKINIWriteInt(hINI, PPF_SECTIONLRC, PPF_KEY_SCOFFSET, GS.iSCLrcOffset);

	QKINIWriteInt(hINI, PPF_SECTIONVISUAL, PPF_KEY_VISUALMODE, GS.iVisualMode);

	QKINIWriteInt(hINI, PPF_SECTIONVISUAL, PPF_KEY_HSCROLLTEXT, GS.bHScrollText);

	QKINIWriteInt(hINI, PPF_SECTIONVISUAL, PPF_KEY_ALBUMPICSIZE1, GS.iAlbumPicSize1);

	QKINIWriteInt(hINI, PPF_SECTIONVISUAL, PPF_KEY_ALBUMPICSIZE2, GS.iAlbumPicSize2);

	QKINIWriteInt(hINI, PPF_SECTIONLRC, PPF_KEY_DTLRCBORDERCLR, GS.crDTLrcBorder);

	QKINIWriteInt(hINI, PPF_SECTIONLRC, PPF_KEY_ENABLEDTLRCBORDER, GS.bEnableDTLrcBorder);

    QKINIWriteString(hINI, PPF_SECTIONPLAYING, PPF_KEY_SOUNDFONT, GS.pszSoundFont);

	QKINISave(hINI);
	QKINIClose(hINI);
}
void Settings_GetFromCtrl(HWND* hChild)
{
    WCHAR sz[MAXPROFILEBUFFER];
    int iPos;
    HWND hWnd;
    int iLength;
    ///////////////////显示
    hWnd = hChild[0];
    GS.iVisualMode = SendDlgItemMessageW(hWnd, IDC_CB_VISUAL, CB_GETCURSEL, 0, 0);

    GS.bHScrollText = (SendDlgItemMessageW(hWnd, IDC_CB_HSCROLLTEXT, BM_GETCHECK, 0, 0) == BST_CHECKED);

    GetDlgItemTextW(hWnd, IDC_ED_ALBUMPICSIZE1, sz, MAXPROFILEBUFFER);
    GS.iAlbumPicSize1 = StrToIntW(sz);

    GetDlgItemTextW(hWnd, IDC_ED_ALBUMPICSIZE2, sz, MAXPROFILEBUFFER);
    GS.iAlbumPicSize2 = StrToIntW(sz);
    ///////////////////播放
    hWnd = hChild[1];
    delete[] GS.pszSoundFont;
    iLength = GetWindowTextLengthW(GetDlgItem(hWnd, IDC_ED_SOUNDFONT));
    GS.pszSoundFont = new WCHAR[iLength + 1];
    GetDlgItemTextW(hWnd, IDC_ED_SOUNDFONT, GS.pszSoundFont, iLength + 1);
    ///////////////////歌词
    hWnd = hChild[2];
    GS.iDefTextCode = SendDlgItemMessageW(hWnd, IDC_CB_DEFTEXTCODE, CB_GETCURSEL, 0, 0);

    delete[] GS.pszLrcDir;
    iLength = GetWindowTextLengthW(GetDlgItem(hWnd, IDC_ED_LRCDIR));
    GS.pszLrcDir = new WCHAR[iLength + 1];
    GetDlgItemTextW(hWnd, IDC_ED_LRCDIR, GS.pszLrcDir, iLength + 1);

    GS.bLrcAnimation = !(SendDlgItemMessageW(hWnd, IDC_CB_DISABLEVANIMATION, BM_GETCHECK, 0, 0) == BST_CHECKED);
    GS.bForceTwoLines = (SendDlgItemMessageW(hWnd, IDC_CB_DISABLEWORDBREAK, BM_GETCHECK, 0, 0) == BST_CHECKED);
    GS.bDTLrcShandow = !(SendDlgItemMessageW(hWnd, IDC_CB_DISABLEDTLRCSHANDOW, BM_GETCHECK, 0, 0) == BST_CHECKED);

    delete[] GS.pszDTLrcFontName;
    iLength = GetWindowTextLengthW(GetDlgItem(hWnd, IDC_ED_DTLRCFONTINFO));
    GS.pszDTLrcFontName = new WCHAR[iLength + 1];
    GetDlgItemTextW(hWnd, IDC_ED_DTLRCFONTINFO, GS.pszDTLrcFontName, iLength + 1);

    iPos = QKStrInStr(GS.pszDTLrcFontName, L",");
    *(GS.pszDTLrcFontName + iPos - 1) = L'\0';
    GS.iDTLrcFontSize = (UINT)GetPropW(GetDlgItem(hWnd, IDC_ED_DTLRCFONTINFO), PROP_DTLRCFONTSIZE);
    GS.iDTLrcFontWeight = (UINT)GetPropW(GetDlgItem(hWnd, IDC_ED_DTLRCFONTINFO), PROP_DTLRCFONTWEIGHT);

    GS.crDTLrc1 = (COLORREF)GetPropW(GetDlgItem(hWnd, IDC_ST_DTLRCCLR1), PROP_DTLRCCLR1);
    GS.crDTLrc2 = (COLORREF)GetPropW(GetDlgItem(hWnd, IDC_ST_DTLRCCLR2), PROP_DTLRCCLR2);

    GS.iDTLrcTransparent = SendDlgItemMessageW(hWnd, IDC_TB_DTLRCTRANSPARENT, TBM_GETPOS, 0, 0);
    delete[] GS.pszDTLrcSpaceLine;
    iLength = GetWindowTextLengthW(GetDlgItem(hWnd, IDC_ED_DTLRCSPACELINE));
    GS.pszDTLrcSpaceLine = new WCHAR[iLength + 1];
    GetDlgItemTextW(hWnd, IDC_ED_DTLRCSPACELINE, GS.pszDTLrcSpaceLine, iLength + 1);

    delete[] GS.pszSCLrcFontName;
    iLength = GetWindowTextLengthW(GetDlgItem(hWnd, IDC_ED_SCLRCFONTINFO));
    GS.pszSCLrcFontName = new WCHAR[iLength + 1];
    GetDlgItemTextW(hWnd, IDC_ED_SCLRCFONTINFO, GS.pszSCLrcFontName, iLength + 1);

    iPos = QKStrInStr(GS.pszSCLrcFontName, L",");
    *(GS.pszSCLrcFontName + iPos - 1) = L'\0';
    GS.iSCLrcFontSize = (UINT)GetPropW(GetDlgItem(hWnd, IDC_ED_SCLRCFONTINFO), PROP_SCLRCFONTSIZE);
    GS.iSCLrcFontWeight = (UINT)GetPropW(GetDlgItem(hWnd, IDC_ED_SCLRCFONTINFO), PROP_SCLRCFONTWEIGHT);

    GS.crSCLrc1 = (COLORREF)GetPropW(GetDlgItem(hWnd, IDC_ST_SCLRCCLR1), PROP_SCLRCCLR1);
    GS.crSCLrc2 = (COLORREF)GetPropW(GetDlgItem(hWnd, IDC_ST_SCLRCCLR2), PROP_SCLRCCLR2);

    GetDlgItemTextW(hWnd, IDC_ED_SCLRCLINEGAP, sz, MAXPROFILEBUFFER);
    GS.iSCLrcLineGap = StrToIntW(sz);

    GS.iSCLrcAlign = SendDlgItemMessageW(hWnd, IDC_CB_SCLRCALIGN, CB_GETCURSEL, 0, 0);

    GetDlgItemTextW(hWnd, IDC_ED_SCLRCALIGNOFFSET, sz, MAXPROFILEBUFFER);
    GS.iSCLrcOffset = StrToIntW(sz);

    GS.crDTLrcBorder = (COLORREF)GetPropW(GetDlgItem(hWnd, IDC_ST_DTLRCBORDERCLR), PROP_DTLRCBORDERCLR);
    GS.bEnableDTLrcBorder = (SendDlgItemMessageW(hWnd, IDC_CB_ENABLEDTLRCBORDER, BM_GETCHECK, 0, 0) == BST_CHECKED);
    ///////////////////快捷键
    hWnd = hChild[3];
}
INT_PTR CALLBACK DlgProc_OptVisual(HWND hDlg, UINT message, WPARAM wParam, LPARAM lParam)
{
    switch (message)
    {
    case WM_INITDIALOG:
    {
        HWND hCtrl;
        WCHAR sz[MAXPROFILEBUFFER];
        ////////////////呈现方式
        hCtrl = GetDlgItem(hDlg, IDC_CB_VISUAL);
        SendMessageW(hCtrl, CB_INSERTSTRING, -1, (LPARAM)L"封面、波形、频谱、歌词");
        SendMessageW(hCtrl, CB_INSERTSTRING, -1, (LPARAM)L"封面、波形、频谱、歌词（尺寸自适应）");
        SendMessageW(hCtrl, CB_INSERTSTRING, -1, (LPARAM)L"旋转封面、频谱、歌词");
        SendMessageW(hCtrl, CB_INSERTSTRING, -1, (LPARAM)L"旋转封面、频谱、歌词（尺寸自适应）");
        SendMessageW(hCtrl, CB_SETCURSEL, GS.iVisualMode, 0);
        ////////////////横向滚动溢出文本
        SendDlgItemMessageW(hDlg, IDC_CB_HSCROLLTEXT, BM_SETCHECK, GS.bHScrollText ? BST_CHECKED : BST_UNCHECKED, 0);
        ////////////////封面图尺寸
        wsprintfW(sz, L"%u", GS.iAlbumPicSize1);
        SetDlgItemTextW(hDlg, IDC_ED_ALBUMPICSIZE1, sz);
        wsprintfW(sz, L"%u", GS.iAlbumPicSize2);
        SetDlgItemTextW(hDlg, IDC_ED_ALBUMPICSIZE2, sz);
    }
    return FALSE;
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

        SetDlgItemTextW(hDlg, IDC_ED_SOUNDFONT, GS.pszSoundFont);
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
            case IDC_BT_CHANGESOUNDFONT:
            {
                IFileOpenDialog* pfod;
                HRESULT hr = CoCreateInstance(CLSID_FileOpenDialog,
                    NULL,
                    CLSCTX_INPROC_SERVER,
                    IID_PPV_ARGS(&pfod));
                if (!SUCCEEDED(hr))
                    return 0;
                pfod->SetOptions(FOS_FORCEFILESYSTEM | FOS_PATHMUSTEXIST);
                COMDLG_FILTERSPEC cf[] =
                {
                    {L"音色库文件(*.sf2)",L"*.sf2"},
                    {L"所有文件",L"*.*"}
                };
                pfod->SetFileTypes(2, cf);
                pfod->Show(hDlg);
                IShellItem* psi;
                hr = pfod->GetResult(&psi);
                if (!SUCCEEDED(hr))
                {
                    pfod->Release();
                    return 0;
                }
                PWSTR pszPath;
                hr = psi->GetDisplayName(SIGDN_DESKTOPABSOLUTEPARSING, &pszPath);
                psi->Release();
                pfod->Release();
                if (!SUCCEEDED(hr))
                    return 0;
                SetDlgItemTextW(hDlg, IDC_ED_SOUNDFONT, pszPath);
                CoTaskMemFree(pszPath);

                m_bSoundFontChanged = TRUE;
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
    static HBRUSH hbr1 = NULL/*DT左*/, hbr2 = NULL/*DT右*/, hbr3 = NULL/*SC左*/, hbr4 = NULL/*SC右*/, hbr5 = NULL/*边框*/;
    static HWND hST1 = NULL/*DT左*/, hST2 = NULL/*DT右*/, hST3 = NULL/*SC左*/, hST4 = NULL/*SC右*/, hST5 = NULL/*边框*/;
	static COLORREF CustClr[16] = { 0 };
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
        hST5 = GetDlgItem(hDlg, IDC_ST_DTLRCBORDERCLR);
		////////////////////////////////////////////////全局
		////////////////缺省编码
		hCtrl = GetDlgItem(hDlg, IDC_CB_DEFTEXTCODE);
		SendMessageW(hCtrl, CB_INSERTSTRING, -1, (LPARAM)L"自动判断（不一定准确）");
		SendMessageW(hCtrl, CB_INSERTSTRING, -1, (LPARAM)L"GBK");
		SendMessageW(hCtrl, CB_INSERTSTRING, -1, (LPARAM)L"UTF-8");
		SendMessageW(hCtrl, CB_INSERTSTRING, -1, (LPARAM)L"UTF-16LE");
		SendMessageW(hCtrl, CB_INSERTSTRING, -1, (LPARAM)L"UTF-16BE");
        SendMessageW(hCtrl, CB_SETCURSEL, GS.iDefTextCode, 0);
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

        wsprintfW(sz, L"%u,", GS.iSCLrcFontSize);
        SetPropW(GetDlgItem(hDlg, IDC_ED_SCLRCFONTINFO), PROP_SCLRCFONTSIZE, (HANDLE)GS.iSCLrcFontSize);
        SendDlgItemMessageW(hDlg, IDC_ED_SCLRCFONTINFO, EM_SETSEL, -2, -1);
        SendDlgItemMessageW(hDlg, IDC_ED_SCLRCFONTINFO, EM_REPLACESEL, FALSE, (LPARAM)sz);

        wsprintfW(sz, L"%u", GS.iSCLrcFontWeight);
        SetPropW(GetDlgItem(hDlg, IDC_ED_SCLRCFONTINFO), PROP_SCLRCFONTWEIGHT, (HANDLE)GS.iSCLrcFontWeight);
        SendDlgItemMessageW(hDlg, IDC_ED_SCLRCFONTINFO, EM_SETSEL, -2, -1);
        SendDlgItemMessageW(hDlg, IDC_ED_SCLRCFONTINFO, EM_REPLACESEL, FALSE, (LPARAM)sz);// 加入文本
        ////////////////滚动歌词颜色
        hbr3 = CreateSolidBrush(GS.crSCLrc1);
        hbr4 = CreateSolidBrush(GS.crSCLrc2);
        SetPropW(hST3, PROP_SCLRCCLR1, (HANDLE)GS.crSCLrc1);
        SetPropW(hST4, PROP_SCLRCCLR2, (HANDLE)GS.crSCLrc2);
        ////////////////行间距
        wsprintfW(sz, L"%u", GS.iSCLrcLineGap);
        SetDlgItemTextW(hDlg, IDC_ED_SCLRCLINEGAP, sz);
        ////////////////对齐方式
        hCtrl = GetDlgItem(hDlg, IDC_CB_SCLRCALIGN);
        SendMessageW(hCtrl, CB_INSERTSTRING, -1, (LPARAM)L"居中对齐");
        SendMessageW(hCtrl, CB_INSERTSTRING, -1, (LPARAM)L"左对齐");
        SendMessageW(hCtrl, CB_INSERTSTRING, -1, (LPARAM)L"右对齐");
        SendMessageW(hCtrl, CB_SETCURSEL, GS.iSCLrcAlign, 0);
        ////////////////对齐偏移
        wsprintfW(sz, L"%u", GS.iSCLrcOffset);
        SetDlgItemTextW(hDlg, IDC_ED_SCLRCALIGNOFFSET, sz);


        ////////////////////////////////////////////////桌面歌词
        ////////////////禁用桌面歌词阴影
        SendDlgItemMessageW(hDlg, IDC_CB_DISABLEDTLRCSHANDOW, BM_SETCHECK, GS.bDTLrcShandow ? BST_UNCHECKED : BST_CHECKED, 0);
        ////////////////桌面歌词字体
        SetDlgItemTextW(hDlg, IDC_ED_DTLRCFONTINFO, GS.pszDTLrcFontName);

        SendDlgItemMessageW(hDlg, IDC_ED_DTLRCFONTINFO, EM_SETSEL, -2, -1);
        SendDlgItemMessageW(hDlg, IDC_ED_DTLRCFONTINFO, EM_REPLACESEL, FALSE, (LPARAM)L",");// 加入文本

        wsprintfW(sz, L"%u,", GS.iDTLrcFontSize);
        SetPropW(GetDlgItem(hDlg, IDC_ED_DTLRCFONTINFO), PROP_DTLRCFONTSIZE, (HANDLE)GS.iDTLrcFontSize);
        SendDlgItemMessageW(hDlg, IDC_ED_DTLRCFONTINFO, EM_SETSEL, -2, -1);
        SendDlgItemMessageW(hDlg, IDC_ED_DTLRCFONTINFO, EM_REPLACESEL, FALSE, (LPARAM)sz);

        wsprintfW(sz, L"%u", GS.iDTLrcFontWeight);
        SetPropW(GetDlgItem(hDlg, IDC_ED_DTLRCFONTINFO), PROP_DTLRCFONTWEIGHT, (HANDLE)GS.iDTLrcFontWeight);
        SendDlgItemMessageW(hDlg, IDC_ED_DTLRCFONTINFO, EM_SETSEL, -2, -1);
        SendDlgItemMessageW(hDlg, IDC_ED_DTLRCFONTINFO, EM_REPLACESEL, FALSE, (LPARAM)sz);// 加入文本
        ////////////////桌面歌词颜色
        hbr1 = CreateSolidBrush(GS.crDTLrc1);
        hbr2 = CreateSolidBrush(GS.crDTLrc2);
        SetPropW(hST1, PROP_DTLRCCLR1, (HANDLE)GS.crDTLrc1);
        SetPropW(hST2, PROP_DTLRCCLR2, (HANDLE)GS.crDTLrc2);
        ////////////////桌面歌词透明度
        SendDlgItemMessageW(hDlg, IDC_TB_DTLRCTRANSPARENT, TBM_SETRANGE, FALSE, MAKELPARAM(0, 0xFF));
        SendDlgItemMessageW(hDlg, IDC_TB_DTLRCTRANSPARENT, TBM_SETPOS, TRUE, GS.iDTLrcTransparent);
        ////////////////空行替代
        SetDlgItemTextW(hDlg, IDC_ED_DTLRCSPACELINE, GS.pszDTLrcSpaceLine);
        ////////////////边框
        hbr5 = CreateSolidBrush(GS.crDTLrcBorder);
        SetPropW(hST5, PROP_DTLRCBORDERCLR, (HANDLE)GS.crDTLrcBorder);
        SendDlgItemMessageW(hDlg, IDC_CB_ENABLEDTLRCBORDER, BM_SETCHECK, GS.bEnableDTLrcBorder ? BST_CHECKED : BST_UNCHECKED, 0);
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
                PWSTR pszPath;
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

                int iLength = GetWindowTextLengthW(GetDlgItem(hDlg, IDC_ED_DTLRCFONTINFO));
                PWSTR psz = new WCHAR[iLength + 1];
                GetDlgItemTextW(hDlg, IDC_ED_DTLRCFONTINFO, psz, iLength + 1);
                *(psz + QKStrInStr(psz, L",") - 1) = L'\0';
                StringCchPrintfW(lf.lfFaceName, LF_FACESIZE, L"%s", psz);
                delete[] psz;

                HDC hDC = GetDC(NULL);
				lf.lfHeight = -MulDiv(
                    (int)GetPropW(GetDlgItem(hDlg, IDC_ED_DTLRCFONTINFO), PROP_DTLRCFONTSIZE), 
                    GetDeviceCaps(hDC, LOGPIXELSY), 
                    72);
                ReleaseDC(NULL, hDC);

                CHOOSEFONTW cf = { 0 };
                cf.lStructSize = sizeof(CHOOSEFONTW);
                cf.hwndOwner = hDlg;
                cf.lpLogFont = &lf;
                cf.Flags = CF_INITTOLOGFONTSTRUCT | CF_SCREENFONTS | CF_FORCEFONTEXIST | CF_NOVERTFONTS;

                if (ChooseFontW(&cf))
                {
					UINT uFontSize = cf.iPointSize / 10;
                    UINT uFontWeight = lf.lfWeight;

                    WCHAR sz[MAXPROFILEBUFFER];
                    SetDlgItemTextW(hDlg, IDC_ED_DTLRCFONTINFO, lf.lfFaceName);

                    SendDlgItemMessageW(hDlg, IDC_ED_DTLRCFONTINFO, EM_SETSEL, -2, -1);
                    SendDlgItemMessageW(hDlg, IDC_ED_DTLRCFONTINFO, EM_REPLACESEL, FALSE, (LPARAM)L",");// 加入文本

                    wsprintfW(sz, L"%u,", uFontSize);
                    SetPropW(GetDlgItem(hDlg, IDC_ED_DTLRCFONTINFO), PROP_DTLRCFONTSIZE, (HANDLE)uFontSize);
                    SendDlgItemMessageW(hDlg, IDC_ED_DTLRCFONTINFO, EM_SETSEL, -2, -1);
                    SendDlgItemMessageW(hDlg, IDC_ED_DTLRCFONTINFO, EM_REPLACESEL, FALSE, (LPARAM)sz);

                    wsprintfW(sz, L"%u", uFontWeight);
                    SetPropW(GetDlgItem(hDlg, IDC_ED_DTLRCFONTINFO), PROP_DTLRCFONTWEIGHT, (HANDLE)uFontWeight);
                    SendDlgItemMessageW(hDlg, IDC_ED_DTLRCFONTINFO, EM_SETSEL, -2, -1);
                    SendDlgItemMessageW(hDlg, IDC_ED_DTLRCFONTINFO, EM_REPLACESEL, FALSE, (LPARAM)sz);// 加入文本
                }
            }
            return TRUE;
            case IDC_BT_CHANGEFONT2:
            {
                LOGFONTW lf = { 0 };

                int iLength = GetWindowTextLengthW(GetDlgItem(hDlg, IDC_ED_SCLRCFONTINFO));
                PWSTR psz = new WCHAR[iLength + 1];
                GetDlgItemTextW(hDlg, IDC_ED_SCLRCFONTINFO, psz, iLength + 1);
                *(psz + QKStrInStr(psz, L",") - 1) = L'\0';
                StringCchPrintfW(lf.lfFaceName, LF_FACESIZE, L"%s", psz);
                delete[] psz;

                HDC hDC = GetDC(NULL);
                lf.lfHeight = -MulDiv(
                    (int)GetPropW(GetDlgItem(hDlg, IDC_ED_SCLRCFONTINFO), PROP_SCLRCFONTSIZE),
                    GetDeviceCaps(hDC, LOGPIXELSY),
                    72);
                ReleaseDC(NULL, hDC);

				CHOOSEFONTW cf = { 0 };
				cf.lStructSize = sizeof(CHOOSEFONTW);
				cf.hwndOwner = hDlg;
				cf.lpLogFont = &lf;
				cf.Flags = CF_INITTOLOGFONTSTRUCT | CF_SCREENFONTS | CF_FORCEFONTEXIST | CF_NOVERTFONTS;

				if (ChooseFontW(&cf))
				{
					UINT uFontSize = cf.iPointSize / 10;
					UINT uFontWeight = lf.lfWeight;

					WCHAR sz[MAXPROFILEBUFFER];
					SetDlgItemTextW(hDlg, IDC_ED_SCLRCFONTINFO, lf.lfFaceName);

                    SendDlgItemMessageW(hDlg, IDC_ED_SCLRCFONTINFO, EM_SETSEL, -2, -1);
                    SendDlgItemMessageW(hDlg, IDC_ED_SCLRCFONTINFO, EM_REPLACESEL, FALSE, (LPARAM)L",");// 加入文本

                    wsprintfW(sz, L"%u,", uFontSize);
                    SetPropW(GetDlgItem(hDlg, IDC_ED_SCLRCFONTINFO), PROP_SCLRCFONTSIZE, (HANDLE)uFontSize);
                    SendDlgItemMessageW(hDlg, IDC_ED_SCLRCFONTINFO, EM_SETSEL, -2, -1);
                    SendDlgItemMessageW(hDlg, IDC_ED_SCLRCFONTINFO, EM_REPLACESEL, FALSE, (LPARAM)sz);

                    wsprintfW(sz, L"%u", uFontWeight);
                    SetPropW(GetDlgItem(hDlg, IDC_ED_SCLRCFONTINFO), PROP_SCLRCFONTWEIGHT, (HANDLE)uFontWeight);
                    SendDlgItemMessageW(hDlg, IDC_ED_SCLRCFONTINFO, EM_SETSEL, -2, -1);
                    SendDlgItemMessageW(hDlg, IDC_ED_SCLRCFONTINFO, EM_REPLACESEL, FALSE, (LPARAM)sz);// 加入文本
                }
            }
            return TRUE;
            }
        }
        return TRUE;
        case STN_DBLCLK:
        {
            switch (LOWORD(wParam))
            {
            case IDC_ST_DTLRCCLR1:
            case IDC_ST_DTLRCCLR2:
            case IDC_ST_SCLRCCLR1:
            case IDC_ST_SCLRCCLR2:
            case IDC_ST_DTLRCBORDERCLR:
            {
                CHOOSECOLORW cc = { sizeof(CHOOSECOLORW) };
                cc.hwndOwner = hDlg;
                cc.lpCustColors = CustClr;
                cc.Flags = CC_FULLOPEN;
                if (ChooseColorW(&cc))
                {
                    switch (LOWORD(wParam))
                    {
                    case IDC_ST_DTLRCCLR1:
                        DeleteObject(hbr1);
                        hbr1 = CreateSolidBrush(cc.rgbResult);
                        SetPropW(hST1, PROP_DTLRCCLR1, (HANDLE)cc.rgbResult);
                        InvalidateRect(hST1, NULL, FALSE);
                        break;
                    case IDC_ST_DTLRCCLR2:
                        DeleteObject(hbr2);
                        hbr2 = CreateSolidBrush(cc.rgbResult);
                        SetPropW(hST2, PROP_DTLRCCLR2, (HANDLE)cc.rgbResult);
                        InvalidateRect(hST2, NULL, FALSE);
                        break;
                    case IDC_ST_SCLRCCLR1:
                        DeleteObject(hbr3);
                        hbr3 = CreateSolidBrush(cc.rgbResult);
                        SetPropW(hST3, PROP_SCLRCCLR1, (HANDLE)cc.rgbResult);
                        InvalidateRect(hST3, NULL, FALSE);
                        break;
                    case IDC_ST_SCLRCCLR2:
                        DeleteObject(hbr4);
                        hbr4 = CreateSolidBrush(cc.rgbResult);
                        SetPropW(hST4, PROP_SCLRCCLR2, (HANDLE)cc.rgbResult);
                        InvalidateRect(hST4, NULL, FALSE);
                        break;
                    case IDC_ST_DTLRCBORDERCLR:
                        DeleteObject(hbr5);
                        hbr5 = CreateSolidBrush(cc.rgbResult);
                        SetPropW(hST5, PROP_DTLRCBORDERCLR, (HANDLE)cc.rgbResult);
                        InvalidateRect(hST5, NULL, FALSE);
                        break;
                    }
                }
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
        else if (lParam == (LPARAM)hST5)
            return (LRESULT)hbr5;
    }
    return NULL;
    case WM_DESTROY:
	{
		DeleteObject(hbr1);
		DeleteObject(hbr2);
		DeleteObject(hbr3);
		DeleteObject(hbr4);
        DeleteObject(hbr5);
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
        m_bSoundFontChanged = FALSE;

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
            hChild[i] = CreateDialogParamW(g_hInst, DialogID[i], NULL, DialogProc[i], 0);
            GetClientRect(hChild[i], &rc);
            cyChild[i] = rc.bottom;

            SetWindowLongPtrW(hChild[i], GWL_STYLE, WS_CHILD);
            SetParent(hChild[i], hDlg);
            SetWindowPos(hChild[i], NULL,
                rcLB.right + rcLB.left,
                rcLB.top,
                rcClient.right - (rcLB.right + rcLB.left) - iSBSize,
                rcLB.bottom - rcLB.top,
                SWP_NOZORDER);
        }

        SetWindowPos(GetDlgItem(hDlg, IDC_SB_SETTINGS), NULL, rcClient.right - iSBSize, rcLB.top, iSBSize, cyLB, SWP_NOZORDER);

        iOptionIndex = 0;
        SendDlgItemMessageW(hDlg, IDC_LB_OPTIONS, LB_SETCURSEL, iOptionIndex, 0);
        goto SwitchTab;
    }
    return FALSE;
    case WM_MEASUREITEM:
    {
        MEASUREITEMSTRUCT* p = (MEASUREITEMSTRUCT*)lParam;
        if (p->CtlID == IDC_LB_OPTIONS && p->CtlType == ODT_LISTBOX)
        {
            p->itemHeight = DPI(30);// 项目高度
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
                rcText.left += DPI(10);// 左边空一点
                if (p->itemData)
                    DrawTextW(p->hDC, (PCWSTR)p->itemData, -1, &rcText, DT_SINGLELINE | DT_VCENTER);
                // 还原
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
        case IDOK2:
		{
            Settings_GetFromCtrl(hChild);
			Settings_Save();

			SettingsUpd_WndMain();
			if (IsWindow(g_hLrcWnd))
				SettingsUpd_WndLrc();

            if (m_bSoundFontChanged)
                BASS_UpdateSoundFont();
            m_bSoundFontChanged = FALSE;
			if (LOWORD(wParam) == IDOK)
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
        else if (si.nPos > (int)(si.nMax - si.nPage))
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