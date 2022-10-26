/*
* WndList.cpp
* 包含列表容器窗口相关窗口过程和相关函数的实现
*/
#include "WndList.h"
#include <Windows.h>
#include <UxTheme.h>
#include <vsstyle.h>
#include <shlobj_core.h>

#include <stdio.h>

#include "MyProject.h"
#include "GlobalVar.h"
#include "resource.h"
#include "OLEDragDrop.h"
#include "WndMain.h"

HANDLE          m_htdMusicTime          = NULL;
UINT            m_uThreadFlagMusicTime  = THREADFLAG_STOP;

CDropTarget*    m_pDropTarget           = NULL;

int             m_iDragTimerElapse      = 0;
int             m_iDragDirection        = 0;// 0 无效   1 向上   2 向下
void UI_SetRitCtrlPos()
{
    // 列表名称 
	SetWindowPos(GetDlgItem(g_hBKRight, IDC_ST_LISTNAME), NULL, 0, 0,
		g_cxBKList - DPIS_GAP,
		DPIS_CYSTLISTNAME,
		SWP_NOZORDER | SWP_NOMOVE);
    // 搜索框
	HWND hEdit = GetDlgItem(g_hBKRight, IDC_ED_SEARCH);
	SetWindowPos(hEdit, NULL, 0, 0,
		g_cxBKList - GC.cyBT - DPIS_GAP,
        GC.cyBT,
		SWP_NOZORDER | SWP_NOMOVE);
    // 置居中对齐
    RECT rc;
    GetClientRect(hEdit, &rc);
    TEXTMETRICW tm;
	HDC hDC = GetDC(hEdit);
	GetTextMetricsW(hDC, &tm);
	ReleaseDC(hEdit, hDC);
	rc.left = DPI(1);
	rc.right--;
	rc.top = (rc.bottom - rc.top - DPI(tm.tmHeight)) / 2 + DPI(1);
	rc.bottom = rc.top + DPI(tm.tmHeight);
	SendMessageW(hEdit, EM_SETRECT, 0, (LPARAM)&rc);
    // 搜索按钮
	SetWindowPos(GetDlgItem(g_hBKRight, IDC_BT_SEARCH), NULL,
		g_cxBKList - GC.cyBT - DPIS_GAP,
		DPIS_CYSTLISTNAME + DPIS_GAP * 2,
		0, 0, SWP_NOZORDER | SWP_NOSIZE);
}
/*
 * 目标：填充列表时间列
 *
 * 参数：
 * p 工作参数
 *
 * 返回值：
 * 操作简述：
 * 备注：线程
 */
DWORD WINAPI Thread_FillTimeColumn(void* p)
{
    PLAYERLISTUNIT* pI;
    TPARAM_FILLTIMECLM* pp = (TPARAM_FILLTIMECLM*)p;
    for (int i = 0; i < g_ItemData->iCount; ++i)
    {
        if (m_uThreadFlagMusicTime == THREADFLAG_STOP)
            break;

        pI = (PLAYERLISTUNIT*)QKArray_Get(g_ItemData, i);
        if (!pI->pszTime || !pp->bJudgeItem)
        {
            HSTREAM hStream = BASS_OpenMusic(pI->pszFile, BASS_STREAM_DECODE, BASS_MUSIC_DECODE | BASS_MUSIC_PRESCAN);
            int iTime = (int)BASS_ChannelBytes2Seconds(hStream, BASS_ChannelGetLength(hStream, BASS_POS_BYTE));
            BASS_FreeMusic(hStream);
            int iMin = iTime / 60,
                iSec = iTime - iMin * 60;

            delete[] pI->pszTime;

            pI->pszTime = new WCHAR[_snwprintf(NULL, 0, L"%02d:%02d", iMin, iSec) + 1];
            wsprintfW(pI->pszTime, L"%02d:%02d", iMin, iSec);
        }
    }
    List_Redraw();
    delete pp;
    return 0;
}
void List_FillMusicTimeColumn(BOOL bJudgeItem)
{
    StopThread_MusicTime();
    TPARAM_FILLTIMECLM* p = new TPARAM_FILLTIMECLM;
    p->bJudgeItem = bJudgeItem;
    m_uThreadFlagMusicTime = THREADFLAG_WORKING;
    m_htdMusicTime = CreateThread(NULL, 0, Thread_FillTimeColumn, p, 0, NULL);
}
void List_SetRedraw(BOOL b)
{
    SendMessageW(g_hLV, WM_SETREDRAW, b, 0);
}
void List_ResetLV()
{
    if (g_iSearchResult == -1)
        SendMessageW(g_hLV, LVM_SETITEMCOUNT, g_ItemData->iCount, 0);
    else
        SendMessageW(g_hLV, LVM_SETITEMCOUNT, g_iSearchResult, 0);

}
PLAYERLISTUNIT* List_GetArrayItem(int iLVIndex)
{
    if (iLVIndex < 0)
        return NULL;

	int i = iLVIndex;
	if (g_iSearchResult != -1)// 应执行搜索时索引映射
		i = ((PLAYERLISTUNIT*)QKArray_Get(g_ItemData, i))->iMappingIndexSearch;

	if (g_bSort)// 应执行排序索引映射
		i = ((PLAYERLISTUNIT*)QKArray_Get(g_ItemData, i))->iMappingIndexSort;

	if (i == -1)
		return (PLAYERLISTUNIT*)QKArray_Get(g_ItemData, iLVIndex);
	else
		return (PLAYERLISTUNIT*)QKArray_Get(g_ItemData, i);
}
int List_GetArrayItemIndex(int iLVIndex)
{
	if (g_iSearchResult != -1)// 应执行搜索时索引映射
		iLVIndex = ((PLAYERLISTUNIT*)QKArray_Get(g_ItemData, iLVIndex))->iMappingIndexSearch;

	if (g_bSort)// 应执行排序索引映射
		iLVIndex = ((PLAYERLISTUNIT*)QKArray_Get(g_ItemData, iLVIndex))->iMappingIndexSort;

	return iLVIndex;
}
void List_Redraw()
{
	InvalidateRect(g_hLV, NULL, FALSE);
	UpdateWindow(g_hLV);
}
int List_Add(PWSTR pszFile, PWSTR pszName, int iPos, BOOL bRedraw, DWORD dwFlags, COLORREF crBookMark,
	PWSTR pszBookMark, PWSTR pszBookMarkComment, PWSTR pszTime)
{
	PLAYERLISTUNIT* pListUnit = new PLAYERLISTUNIT;
	ZeroMemory(pListUnit, sizeof(PLAYERLISTUNIT));// ZEROINIT
	pListUnit->dwFlags = dwFlags;
	pListUnit->iMappingIndexSearch = -1;
	pListUnit->iMappingIndexSort = -1;
	pListUnit->pszFile = new WCHAR[lstrlenW(pszFile) + 1];
	lstrcpyW(pListUnit->pszFile, pszFile);

	if (pszName)
	{
		pListUnit->pszName = new WCHAR[lstrlenW(pszName) + 1];
		lstrcpyW(pListUnit->pszName, pszName);
	}
	else
	{
		PWSTR pszNameTemp = new WCHAR[lstrlenW(pszFile) + 1];
		lstrcpyW(pszNameTemp, pszFile);
		PathStripPathW(pszNameTemp);//去掉路径
		PathRemoveExtensionW(pszNameTemp);//去掉扩展名

		pListUnit->pszName = new WCHAR[lstrlenW(pszNameTemp) + 1];
		lstrcpyW(pListUnit->pszName, pszNameTemp);

		delete[] pszNameTemp;
	}

	if (dwFlags & QKLIF_BOOKMARK)// 有书签
	{
		pListUnit->crBookMark = crBookMark;
		if (pszBookMark)
		{
			pListUnit->pszBookMark = new WCHAR[lstrlenW(pszBookMark) + 1];
			lstrcpyW(pListUnit->pszBookMark, pszBookMark);
		}
		if (pszBookMarkComment)
		{
			pListUnit->pszBookMarkComment = new WCHAR[lstrlenW(pszBookMarkComment) + 1];
			lstrcpyW(pListUnit->pszBookMarkComment, pszBookMarkComment);
		}
	}

	if (pszTime)
	{
		pListUnit->pszTime = new WCHAR[lstrlenW(pszTime) + 1];
		lstrcpyW(pListUnit->pszTime, pszTime);
	}


	int iIndex;
	if (iPos == -1)
		iIndex = QKArray_Add(&g_ItemData, pListUnit);
	else
		iIndex = QKArray_Insert(&g_ItemData, pListUnit, iPos);

	if (bRedraw)
		List_ResetLV();

	return iIndex;
}
void List_Delete(int iItem, BOOL bRedraw)
{
    DWORD dwExitCode;
    if (GetExitCodeThread(m_htdMusicTime, &dwExitCode))
        StopThread_MusicTime();
    else
        dwExitCode = 0;

    PLAYERLISTUNIT* p;
    if (iItem == -1)
    {
        for (int i = 0; i < g_ItemData->iCount; ++i)
        {
            p = (PLAYERLISTUNIT*)QKArray_Get(g_ItemData, i);
            // 对空指针delete是安全的
            delete[] p->pszFile;
            delete[] p->pszName;
            delete[] p->pszTime;
            delete[] p->pszBookMark;
            delete[] p->pszBookMarkComment;
        }
        QKArray_Delete(g_ItemData, QKADF_DELETE);
        g_ItemData = QKArray_Create(0);
    }
    else
    {
        p = (PLAYERLISTUNIT*)QKArray_Get(g_ItemData, iItem);
        delete[] p->pszFile;
        delete[] p->pszName;
        delete[] p->pszTime;
        delete[] p->pszBookMark;
        delete[] p->pszBookMarkComment;
        QKArray_DeleteMember(&g_ItemData, iItem, QKADF_DELETE);
    }
    if (bRedraw)
        List_ResetLV();
    if (dwExitCode == STILL_ACTIVE)
        List_FillMusicTimeColumn(TRUE);
}
void StopThread_MusicTime()
{
    if (!m_htdMusicTime)
        return;

    DWORD dwExitCode;
    BOOL bResult = GetExitCodeThread(m_htdMusicTime, &dwExitCode);
    if (bResult && dwExitCode == STILL_ACTIVE)
    {
        m_uThreadFlagMusicTime = THREADFLAG_STOP;
        WaitForSingleObject(m_htdMusicTime, INFINITE);//等待线程退出
        m_uThreadFlagMusicTime = THREADFLAG_STOPED;
    }

    CloseHandle(m_htdMusicTime);
    g_iLrcState = LRCSTATE_NOLRC;
    m_htdMusicTime = NULL;//清空句柄
}
void Sort_End()
{
    if (g_iCurrFileIndex != -1)
    {
        int j;
        BOOL b[2] = { 0 };
        for (int i = 0; i < g_ItemData->iCount; ++i)
        {
            j = ((PLAYERLISTUNIT*)QKArray_Get(g_ItemData, i))->iMappingIndexSort;
            if (j == g_iCurrFileIndex)
            {
                g_iCurrFileIndex = i;// 转换现行播放索引
                b[0] = TRUE;
            }
            if (j == g_iLaterPlay)// 这俩中间不能加else，因为现行播放和稍后播放可能是同一项
            {
                g_iLaterPlay = i;// 转换稍后播放索引
                b[1] = TRUE;
            }
            if (b[0] && b[1])
                break;
        }
    }
    List_Redraw();
    UI_RedrawBookMarkPos();
}

LRESULT CALLBACK WndProc_PlayList(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam)
{
	static HTHEME hLVTheme = NULL;
	static int cxClient, cyClient;
	static int cyVSBArrow = GetSystemMetrics(SM_CYVSCROLL);
	static int cySBTrack;
	static COLORREF CustClr[16] = { 0 };
	switch (message)
	{
	case WM_CREATE:// 狠狠地创建窗口
	{
		SetPropW(hWnd, PROP_BOOKMARKCLRDLGBUF, (HANDLE)CustClr);
		HWND hCtrl, hCtrl2;
		///////////////////////////右部工具栏
		hCtrl2 = CreateWindowExW(0, BKWNDCLASS, NULL, WS_CHILD | WS_VISIBLE,
			0, 0, g_cxBKList, DPIS_CYRITBK,
			hWnd, (HMENU)IDC_BK_RIGHTBTBK, g_hInst, NULL);
		g_hBKRight = hCtrl2;
		SetWindowLongPtrW(hCtrl2, GWLP_WNDPROC, (LONG_PTR)WndProc_RitBK);
		///////////////////////////列表名称静态
		hCtrl = CreateWindowExW(0, WC_STATICW, NULL, WS_CHILD | WS_VISIBLE | SS_CENTERIMAGE | SS_ELLIPSISMASK,
			0, DPIS_GAP, 0, 0,
			hCtrl2, (HMENU)IDC_ST_LISTNAME, g_hInst, NULL);
		SendMessageW(hCtrl, WM_SETFONT, (WPARAM)g_hFontDrawing, FALSE);
		SetWindowTextW(hCtrl, L"/*当前无播放列表*/");
		///////////////////////////搜索编辑框
		hCtrl = CreateWindowExW(WS_EX_CLIENTEDGE, WC_EDITW, NULL, WS_CHILD | WS_VISIBLE| ES_MULTILINE | ES_WANTRETURN,
			0, DPIS_CYSTLISTNAME + DPIS_GAP * 2, 0, 0,
			hCtrl2, (HMENU)IDC_ED_SEARCH, g_hInst, NULL);
		SendMessageW(hCtrl, WM_SETFONT, (WPARAM)g_hFont, FALSE);
		SetPropW(hCtrl, PROP_WNDPROC,
			(HANDLE)SetWindowLongPtrW(hCtrl, GWLP_WNDPROC, (LONG_PTR)WndProc_Edit));
		///////////////////////////搜索按钮
		hCtrl = CreateWindowExW(0, WC_BUTTONW, NULL, WS_CHILD | WS_VISIBLE | BS_ICON,
			0, 0, GC.cyBT, GC.cyBT,
			hCtrl2, (HMENU)IDC_BT_SEARCH, g_hInst, NULL);
		SendMessageW(hCtrl, BM_SETIMAGE, IMAGE_ICON, (LPARAM)GR.hiSearch);
		UI_SetRitCtrlPos();
		///////////////////////////
        int iLeft = 0, iTop = DPIS_CYSTLISTNAME + DPIS_GAP * 3 + GC.cyBT;
		///////////////////////////
		hCtrl = CreateWindowExW(0, WC_BUTTONW, L"定位", WS_CHILD | WS_VISIBLE,
			iLeft, iTop, DPIS_CXRITBT, GC.cyBT,
			hCtrl2, (HMENU)IDC_BT_JUMP, g_hInst, NULL);
		SendMessageW(hCtrl, WM_SETFONT, (WPARAM)g_hFont, FALSE);
		SendMessageW(hCtrl, BM_SETIMAGE, IMAGE_ICON, (LPARAM)GR.hiLocate);
		iLeft += (DPIS_CXRITBT + DPIS_GAP);
		///////////////////////////
		hCtrl = CreateWindowExW(0, WC_BUTTONW, L"添加", WS_CHILD | WS_VISIBLE,
			iLeft, iTop, DPIS_CXRITBT, GC.cyBT,
			hCtrl2, (HMENU)IDC_BT_OPEN, g_hInst, NULL);
		SendMessageW(hCtrl, WM_SETFONT, (WPARAM)g_hFont, FALSE);
		SendMessageW(hCtrl, BM_SETIMAGE, IMAGE_ICON, (LPARAM)GR.hiPlus);
		iLeft += (DPIS_CXRITBT + DPIS_GAP);
		///////////////////////////
		hCtrl = CreateWindowExW(0, WC_BUTTONW, L"读取", WS_CHILD | WS_VISIBLE,
			iLeft, iTop, DPIS_CXRITBT, GC.cyBT,
			hCtrl2, (HMENU)IDC_BT_LOADLIST, g_hInst, NULL);
		SendMessageW(hCtrl, WM_SETFONT, (WPARAM)g_hFont, FALSE);
		SendMessageW(hCtrl, BM_SETIMAGE, IMAGE_ICON, (LPARAM)GR.hiReadFile);
		iLeft += (DPIS_CXRITBT + DPIS_GAP);
		///////////////////////////
		hCtrl = CreateWindowExW(0, WC_BUTTONW, L"保存", WS_CHILD | WS_VISIBLE,
			iLeft, iTop, DPIS_CXRITBT, GC.cyBT,
			hCtrl2, (HMENU)IDC_BT_SAVE, g_hInst, NULL);
		SendMessageW(hCtrl, WM_SETFONT, (WPARAM)g_hFont, FALSE);
		SendMessageW(hCtrl, BM_SETIMAGE, IMAGE_ICON, (LPARAM)GR.hiSaveFile);
		iLeft += (DPIS_CXRITBT + DPIS_GAP);
		///////////////////////////
		hCtrl = CreateWindowExW(0, WC_BUTTONW, L"清空", WS_CHILD | WS_VISIBLE,
			iLeft, iTop, DPIS_CXRITBT, GC.cyBT,
			hCtrl2, (HMENU)IDC_BT_EMPTY, g_hInst, NULL);
		SendMessageW(hCtrl, WM_SETFONT, (WPARAM)g_hFont, FALSE);
		SendMessageW(hCtrl, BM_SETIMAGE, IMAGE_ICON, (LPARAM)GR.hiCross);
		iLeft += (DPIS_CXRITBT + DPIS_GAP);
		///////////////////////////
		hCtrl = CreateWindowExW(0, WC_BUTTONW, L"管理", WS_CHILD | WS_VISIBLE,
			iLeft, iTop, DPIS_CXRITBT, GC.cyBT,
			hCtrl2, (HMENU)IDC_BT_MANAGING, g_hInst, NULL);
		SendMessageW(hCtrl, WM_SETFONT, (WPARAM)g_hFont, FALSE);
		SendMessageW(hCtrl, BM_SETIMAGE, IMAGE_ICON, (LPARAM)GR.hiListManaging);
		///////////////////////////播放列表
		hCtrl = CreateWindowExW(0, WC_LISTVIEWW, NULL, WS_CHILD | WS_VISIBLE | LVS_OWNERDATA | LVS_SHOWSELALWAYS,
			0, 0, 0, 0, hWnd, (HMENU)IDC_LV_PLAY, g_hInst, NULL);
		g_hLV = hCtrl;
		SendMessageW(hCtrl, WM_SETFONT, (WPARAM)g_hFont, FALSE);
		SendMessageW(hCtrl, LVM_SETVIEW, LV_VIEW_DETAILS, 0);
		UINT uExStyle = LVS_EX_FULLROWSELECT | LVS_EX_DOUBLEBUFFER;
		SendMessageW(hCtrl, LVM_SETEXTENDEDLISTVIEWSTYLE, uExStyle, uExStyle);
		LVCOLUMNW lc;
		lc.mask = LVCF_TEXT | LVCF_WIDTH;
		lc.pszText = (PWSTR)L"名称";
		lc.cx = DPI(295);
		SendMessageW(hCtrl, LVM_INSERTCOLUMNW, 0, (LPARAM)&lc);
		lc.pszText = (PWSTR)L"时长";
		lc.cx = DPI(50);
		SendMessageW(hCtrl, LVM_INSERTCOLUMNW, 1, (LPARAM)&lc);
		SetWindowTheme(hCtrl, L"Explorer", NULL);
		hLVTheme = OpenThemeData(hCtrl, L"ListView");
		SetPropW(hCtrl, PROP_WNDPROC,
			(HANDLE)SetWindowLongPtrW(hCtrl, GWLP_WNDPROC, (LONG_PTR)WndProc_ListView));
		SendMessageW(hCtrl, LVM_SETIMAGELIST, LVSIL_SMALL, (LPARAM)ImageList_Create(1, DPIS_CYLVITEM, 0, 1, 0));// ILC_COLOR缺省

        m_pDropTarget = new CDropTarget(OLEDrop_OnEnter, OLEDrop_OnOver, OLEDrop_OnLeave, OLEDrop_OnDrop);
        RegisterDragDrop(hWnd, m_pDropTarget);// 注册拖放目标
	}
	return 0;
	case WM_SIZE:
	{
		cxClient = LOWORD(lParam);
		cyClient = HIWORD(lParam);
		int iLeft;
		if (g_bListSeped)
			iLeft = DPIS_GAP;
		else
			iLeft = 0;
		SetWindowPos(g_hBKRight, NULL,
			iLeft,
			0,
			cxClient - DPIS_GAP,
			DPIS_CYRITBK,
			SWP_NOZORDER);//右部按钮容器

		SetWindowPos(g_hLV, NULL,
			iLeft,
			DPIS_CYRITBK,
			cxClient - DPIS_GAP - iLeft,
			cyClient - DPIS_CYRITBK,
			SWP_NOZORDER);//列表

		cySBTrack = cyClient - DPIS_CYRITBK - 6 - cyVSBArrow * 2;
	}
	return 0;
	case WM_NOTIFY:
	{
		if (((NMHDR*)lParam)->idFrom == IDC_LV_PLAY)
		{
			switch (((NMHDR*)lParam)->code)
			{
			case NM_DBLCLK:// 双击
			{
				NMITEMACTIVATE* p = (NMITEMACTIVATE*)lParam;
				if (p->iItem != -1)
					Playing_PlayFile(p->iItem);
			}
			break;// 该通知不使用返回值
			case NM_CUSTOMDRAW://自定义绘制
			{
				NMLVCUSTOMDRAW* p = (NMLVCUSTOMDRAW*)lParam;
				switch (p->nmcd.dwDrawStage)
				{
				case CDDS_PREPAINT:
					return CDRF_NOTIFYITEMDRAW;
				case CDDS_ITEMPREPAINT:
				{
					int iState;
					/*对于具有LVS_SHOWSELALWAYS样式的所有者绘制的列表视图控件，此标志(指CDIS_SELECTED)不能正常工作
					对于这些控件，您可以通过使用LVM_GETITEMSTATE并检查LVIS_SELECTED标志来确定是否选择了项目(MSDN)*/
					if (SendMessageW(g_hLV, LVM_GETITEMSTATE, p->nmcd.dwItemSpec, LVIS_SELECTED) == LVIS_SELECTED)//选中
					{
						if (p->nmcd.uItemState & CDIS_HOT)
							iState = LISS_HOTSELECTED;
						else
							iState = LISS_SELECTED;
					}
					else if (p->nmcd.uItemState & CDIS_HOT)
						iState = LISS_HOT;
					else
						iState = 0;

					HBRUSH hBrush;

					if (p->nmcd.dwItemSpec % 2)//交替行色
					{
						hBrush = CreateSolidBrush(MYCLR_LISTGRAY);
						FillRect(p->nmcd.hdc, &p->nmcd.rc, hBrush);
						DeleteObject(hBrush);
					}
					if (p->nmcd.dwItemSpec == g_iCurrFileIndex)//标记现行播放项
					{
						hBrush = CreateSolidBrush(MYCLR_LISTPLAYING);
						FillRect(p->nmcd.hdc, &p->nmcd.rc, hBrush);
						DeleteObject(hBrush);
					}
					if (iState)//画表项框
						DrawThemeBackground(hLVTheme, p->nmcd.hdc, LVP_LISTITEM, iState, &p->nmcd.rc, NULL);

					PLAYERLISTUNIT* pI = List_GetArrayItem(p->nmcd.dwItemSpec);

					if (pI->dwFlags & QKLIF_IGNORED)
						SetTextColor(p->nmcd.hdc, QKCOLOR_GRAY);
					else
						SetTextColor(p->nmcd.hdc, QKCOLOR_BLACK);

					RECT rc = p->nmcd.rc;
					rc.left = DPIS_LVTEXTSPACE;
					rc.right = SendMessageW(g_hLV, LVM_GETCOLUMNWIDTH, 0, 0);
					DrawTextW(p->nmcd.hdc, pI->pszName, -1, &rc, DT_SINGLELINE | DT_VCENTER | DT_END_ELLIPSIS | DT_NOPREFIX);
					rc.left = rc.right;
					rc.right = p->nmcd.rc.right;
					DrawTextW(p->nmcd.hdc, pI->pszTime, -1, &rc, DT_SINGLELINE | DT_VCENTER | DT_CENTER | DT_END_ELLIPSIS);

					if (pI->dwFlags & QKLIF_BOOKMARK)
					{
						HGDIOBJ hOldPen = SelectObject(p->nmcd.hdc, CreatePen(PS_SOLID, 1, pI->crBookMark));
						MoveToEx(p->nmcd.hdc, p->nmcd.rc.left, p->nmcd.rc.top, NULL);
						LineTo(p->nmcd.hdc, p->nmcd.rc.right, p->nmcd.rc.top);
						DeleteObject(SelectObject(p->nmcd.hdc, hOldPen));
					}
					if (p->nmcd.dwItemSpec == g_iLaterPlay)
						FrameRect(p->nmcd.hdc, &p->nmcd.rc, GC.hbrCyanDeeper);
				}
				return CDRF_SKIPDEFAULT;
				}
			}
			break;
			case NM_RCLICK://右键单击
			{
				int iItem = ((NMITEMACTIVATE*)lParam)->iItem;
				UINT uFlags = (iItem == -1) ? MF_GRAYED : 0,
					uFlags2 = g_bSort || g_iSearchResult != -1 ? MF_GRAYED : uFlags;
				HMENU hMenu = CreatePopupMenu();
				AppendMenuW(hMenu, uFlags, IDMI_TL_PLAY, L"播放");
				AppendMenuW(hMenu, uFlags, IDMI_TL_PLAYLATER, L"稍后播放");
				SetMenuDefaultItem(hMenu, IDMI_TL_PLAY, FALSE);
				AppendMenuW(hMenu, MF_SEPARATOR, 0, NULL);
				AppendMenuW(hMenu, uFlags, IDMI_TL_OPEN_IN_EXPLORER, L"打开文件位置");
				AppendMenuW(hMenu, uFlags2, IDMI_TL_DELETE_FROM_LIST, L"从播放列表中删除");
				AppendMenuW(hMenu, uFlags2, IDMI_TL_DELETE, L"从磁盘中删除");
				AppendMenuW(hMenu, MF_SEPARATOR, 0, NULL);
				AppendMenuW(hMenu, uFlags, IDMI_TL_IGNORE, L"忽略/取消忽略此项目");
				AppendMenuW(hMenu, uFlags, IDMI_TL_RENAME, L"重命名");
				AppendMenuW(hMenu, uFlags, IDMI_TL_INFO, L"详细信息");
				AppendMenuW(hMenu, MF_SEPARATOR, 0, NULL);
				AppendMenuW(hMenu, uFlags, IDMI_TL_LASTBOOKMARK, L"跳到上一书签");
				AppendMenuW(hMenu, uFlags, IDMI_TL_NEXTBOOKMARK, L"跳到下一书签");
				AppendMenuW(hMenu, uFlags, IDMI_TL_ADDBOOKMARK, L"添加书签");
				AppendMenuW(hMenu, uFlags, IDMI_TL_REMOVEBOOKMARK, L"删除书签");
				//SetMenuItemBitmaps()

				POINT pt;
				GetCursorPos(&pt);
				int iRet = TrackPopupMenu(hMenu, TPM_RETURNCMD, pt.x, pt.y, 0, hWnd, NULL);
				DestroyMenu(hMenu);
				switch (iRet)
				{
				case IDMI_TL_PLAY:
				{
					if (iItem != -1)
						Playing_PlayFile(iItem);
				}
				break;
				case IDMI_TL_OPEN_IN_EXPLORER:
				{
					int iCount = QKArray_GetCount(g_ItemData);
					QKARRAY pArray = QKArray_Create(0);
					for (int i = 0; i < iCount; ++i)
					{
						if (SendMessageW(g_hLV, LVM_GETITEMSTATE, i, LVIS_SELECTED) == LVIS_SELECTED)
							QKArray_Add(&pArray, List_GetArrayItem(i)->pszFile);
					}
					iCount = QKArray_GetCount(pArray);
					if (!iCount)
						return 0;
					WCHAR pszPath[MAX_PATH];
					lstrcpyW(pszPath, (PCWSTR)QKArray_Get(pArray, 0));
					PathRemoveFileSpecW(pszPath);
					LPITEMIDLIST pPathIDL;
					SHParseDisplayName(pszPath, NULL, &pPathIDL, 0, 0);

					LPITEMIDLIST* pFileIDL = new LPITEMIDLIST[iCount];

					for (int i = 0; i < iCount; i++)
					{
						SHParseDisplayName((PCWSTR)QKArray_Get(pArray, i), NULL, pFileIDL + i, 0, 0);
					}

					HRESULT hr = SHOpenFolderAndSelectItems(pPathIDL, iCount, (LPCITEMIDLIST*)pFileIDL, 0);

					for (int i = 0; i < iCount; i++)
					{
						CoTaskMemFree(pFileIDL[i]);
					}
					CoTaskMemFree(pPathIDL);
					delete[] pFileIDL;
					QKArray_Delete(pArray);
				}
				break;
				case IDMI_TL_ADDBOOKMARK:
				{
					CHOOSECOLORW cc = { sizeof(CHOOSECOLORW) };
					cc.hwndOwner = hWnd;
					cc.lpCustColors = CustClr;
					cc.Flags = CC_FULLOPEN;
					PLAYERLISTUNIT* p;
					if (ChooseColorW(&cc))
					{
						for (int i = 0; i < g_ItemData->iCount; ++i)
						{
							if (SendMessageW(g_hLV, LVM_GETITEMSTATE, i, LVIS_SELECTED) == LVIS_SELECTED)
							{
								p = List_GetArrayItem(i);
								p->dwFlags |= QKLIF_BOOKMARK;
								p->crBookMark = cc.rgbResult;
								if (SendMessageW(g_hLV, LVM_GETITEMSTATE, i, LVIS_SELECTED) == LVIS_SELECTED)
									SendMessageW(g_hLV, LVM_REDRAWITEMS, i, i);
							}
						}
						UI_RedrawBookMarkPos();
					}
				}
				break;
				case IDMI_TL_REMOVEBOOKMARK:
				{
					for (int i = 0; i < g_ItemData->iCount; ++i)
					{
						if (SendMessageW(g_hLV, LVM_GETITEMSTATE, i, LVIS_SELECTED) == LVIS_SELECTED)
						{
							((PLAYERLISTUNIT*)QKArray_Get(g_ItemData, i))->dwFlags &= ~QKLIF_BOOKMARK;
							if (SendMessageW(g_hLV, LVM_GETITEMSTATE, i, LVIS_SELECTED) == LVIS_SELECTED)
								SendMessageW(g_hLV, LVM_REDRAWITEMS, i, i);
						}
					}
					UI_RedrawBookMarkPos();
				}
				break;
				case IDMI_TL_LASTBOOKMARK:
				{
					LVITEMW li;
					li.stateMask = li.state = LVIS_SELECTED;
					int iIndex;
					for (int i = iItem; i >= 0; --i)
					{
						if (List_GetArrayItem(i)->dwFlags & QKLIF_BOOKMARK)
						{
							SendMessageW(hWnd, LVM_SETITEMSTATE, i, (LPARAM)&li);
							iIndex = i - SendMessageW(g_hLV, LVM_GETCOUNTPERPAGE, 0, 0);// 把书签顶到最下面
							if (iIndex < 0)
								iIndex = 0;

							SendMessageW(g_hLV, LVM_ENSUREVISIBLE, iIndex, FALSE);
							break;
						}
					}
				}
				return 0;
				case IDMI_TL_NEXTBOOKMARK:
				{
					LVITEMW li;
					li.stateMask = li.state = LVIS_SELECTED;
					int iIndex;
					for (int i = iItem; i < g_ItemData->iCount; ++i)
					{
						if (List_GetArrayItem(i)->dwFlags & QKLIF_BOOKMARK)
						{
							SendMessageW(hWnd, LVM_SETITEMSTATE, i, (LPARAM)&li);
							iIndex = i + SendMessageW(g_hLV, LVM_GETCOUNTPERPAGE, 0, 0) - 1;// 把书签顶到最上面
							if (iIndex >= g_ItemData->iCount)
								iIndex = g_ItemData->iCount - 1;
							SendMessageW(g_hLV, LVM_ENSUREVISIBLE, iIndex, FALSE);
							break;
						}
					}
				}
				return 0;
				case IDMI_TL_DELETE:
				{
					if (QKMessageBox(L"确定要删除选中的文件吗", L"删除后不可恢复，请确认是否要继续删除", (HICON)TD_INFORMATION_ICON, L"询问", hWnd, NULL, 2) == QKMSGBOX_BTID_2)
						return 0;
					for (int i = g_ItemData->iCount; i >= 0; --i)
					{
						if (SendMessageW(g_hLV, LVM_GETITEMSTATE, i, LVIS_SELECTED) == LVIS_SELECTED)
						{
							DeleteFileW(((PLAYERLISTUNIT*)QKArray_Get(g_ItemData, i))->pszFile);
							List_Delete(i, FALSE);
						}
					}
					List_ResetLV();
				}
				return 0;
				case IDMI_TL_DELETE_FROM_LIST:
				{
					for (int i = g_ItemData->iCount; i >= 0; --i)
					{
						if (SendMessageW(g_hLV, LVM_GETITEMSTATE, i, LVIS_SELECTED) == LVIS_SELECTED)
							List_Delete(i, FALSE);
					}
					List_ResetLV();
				}
				return 0;
				case IDMI_TL_RENAME:
				{
					PWSTR pBuf;
					PLAYERLISTUNIT* p;
					if (QKInputBox(L"重命名", L"输入新名称：", &pBuf, hWnd))
					{
						p = List_GetArrayItem(iItem);
						delete[] p->pszName;
						p->pszName = pBuf;
						SendMessageW(g_hLV, LVM_REDRAWITEMS, iItem, iItem);
					}
				}
				return 0;
				case IDMI_TL_INFO:
				{

				}
				return 0;
				case IDMI_TL_IGNORE:
				{
					PLAYERLISTUNIT* p;
					for (int i = 0; i < g_ItemData->iCount; ++i)
					{
						if (SendMessageW(g_hLV, LVM_GETITEMSTATE, i, LVIS_SELECTED) == LVIS_SELECTED)
						{
							p = List_GetArrayItem(i);
							if (p->dwFlags & QKLIF_IGNORED)
								p->dwFlags &= (~QKLIF_IGNORED);
							else
								p->dwFlags |= QKLIF_IGNORED;
							SendMessageW(g_hLV, LVM_REDRAWITEMS, i, i);
						}
					}
				}
				return 0;
				case IDMI_TL_PLAYLATER:
				{
					if (g_iLaterPlay != -1)
						SendMessageW(g_hLV, LVM_REDRAWITEMS, g_iLaterPlay, g_iLaterPlay);

					if (g_iLaterPlay == iItem)
						g_iLaterPlay = -1;
					else
						g_iLaterPlay = iItem;
					SendMessageW(g_hLV, LVM_REDRAWITEMS, iItem, iItem);
				}
				return 0;
				}
			}
			return TRUE;// 返回非零以不允许默认处理，或返回零以允许默认处理
			case LVN_BEGINDRAG:// 开始拖放
			{
				// 这才是LV拖放正统触发方式，拖放不应该由WM_LBUTTONDOWN触发
				if (((NMLISTVIEW*)lParam)->iItem != -1)
				{
                    if (g_iSearchResult != -1 || g_bSort)
                    {
                        QKMessageBox(L"现在不可重排项目", L"可逆式排序或搜索状态下不可拖动", (HICON)TD_ERROR_ICON, L"错误");
                        return 0;
                    }
					if (g_iCurrFileIndex != -1)
						((PLAYERLISTUNIT*)QKArray_Get(g_ItemData, g_iCurrFileIndex))->dwFlags |= QKLIF_DRAGMARK_CURRFILE;
                    if (g_iLaterPlay != -1)
                        ((PLAYERLISTUNIT*)QKArray_Get(g_ItemData, g_iLaterPlay))->dwFlags |= QKLIF_DRAGMARK_PLLATER;
					//////////////////取选中项目
					QKARRAY Files = QKArray_Create(0);// 文件名数组
					QKARRAY Items = QKArray_Create(0);// 索引数组

					for (int i = 0; i < g_ItemData->iCount; ++i)
					{
						if (SendMessageW(g_hLV, LVM_GETITEMSTATE, i, LVIS_SELECTED) == LVIS_SELECTED)
						{
							QKArray_Add(&Files, List_GetArrayItem(i)->pszFile);
							QKArray_AddValue(&Items, &i);
						}
					}
                    //////////////////制拖放源
					CDataObject* pDataObject;
					CDropSource* pDropSource;
					QKMakeDropSource(Files, OLEDrag_GiveFeedBack, &pDataObject, &pDropSource, TRUE);
					QKArray_Delete(Files);// 清理
                    //////////////////制自定义拖放信息，用来保存索引以便自身程序拖放
					FORMATETC fe =
					{
						g_uMyClipBoardFmt,
						NULL,
						DVASPECT_CONTENT,
						-1,
						TYMED_HGLOBAL
					};
					STGMEDIUM sm;
					sm.tymed = TYMED_HGLOBAL;
					sm.pUnkForRelease = NULL;
                    sm.hGlobal = GlobalAlloc(GMEM_MOVEABLE | GMEM_DDESHARE, (Items->iCount + 2) * sizeof(int) + sizeof(DWORD));

                    // (int)        版本信息
                    // (DWORD)      进程ID
                    // (int)        项目数
                    // (void*)      项目数据
                    // ...

					int* p = (int*)GlobalLock(sm.hGlobal);
                    *p = QKOLEDRAGVER_1;// 拖放信息版本号
                    ++p;

                    *(DWORD*)p = GetCurrentProcessId();// 进程ID
                    p = (int*)((DWORD*)p + 1);

					*p = Items->iCount;// 项目数
					++p;

					for (int i = 0; i < Items->iCount; ++i)
					{
						memcpy(p, QKArray_GetValue(Items, i), sizeof(int));
						++p;
					}
					GlobalUnlock(sm.hGlobal);
					pDataObject->SetData(&fe, &sm, TRUE);
                    //////////////////执行拖放
					DWORD dwEffect = DROPEFFECT_NONE;
					HRESULT hr = SHDoDragDrop(hWnd, pDataObject, pDropSource, DROPEFFECT_COPY, &dwEffect);
                    //////////////////清理，数据为数据对象所有，数据对象释放时会将其一并释放
					delete pDataObject;
					delete pDropSource;
					return 0;
				}
			}
			break;// 没有返回值
			}
			break;
		}
	}
	break;
	case WM_PAINT:
	{
		PAINTSTRUCT ps;
		HDC hDC = BeginPaint(hWnd, &ps);
		FillRect(hDC, &ps.rcPaint, (HBRUSH)GetStockObject(WHITE_BRUSH));
		if (!GS.bNoBookMarkWhenSort || !g_bSort)
		{
			if (g_iSearchResult == -1)
			{
				HGDIOBJ hOldPen = SelectObject(hDC, CreatePen(PS_SOLID, DPI(3), MYCLR_LISTPLAYING));
				int y;
				for (int i = 0; i < g_ItemData->iCount; ++i)
				{
					if (List_GetArrayItem(i)->dwFlags & QKLIF_BOOKMARK)
					{
						y = cyVSBArrow + cySBTrack * i / g_ItemData->iCount + DPIS_CYRITBK;
						MoveToEx(hDC, cxClient - DPIS_GAP, y, NULL);
						LineTo(hDC, cxClient, y);
					}
				}
				DeleteObject(SelectObject(hDC, hOldPen));
			}
		}
		EndPaint(hWnd, &ps);
	}
	return 0;
	case WM_DESTROY:
        delete m_pDropTarget;
		CloseThemeData(hLVTheme);
		return 0;
	case WM_THEMECHANGED:
	{
		CloseThemeData(hLVTheme);
		hLVTheme = OpenThemeData(g_hLV, L"ListView");
		cyVSBArrow = GetSystemMetrics(SM_CYVSCROLL);
	}
	return 0;
    case LISTWND_REDRAWBOOKMARK:
    {
        RECT rc = { cxClient - DPIS_GAP,DPIS_CYRITBK,cxClient,cyClient };
        InvalidateRect(hWnd, &rc, FALSE);
        UpdateWindow(hWnd);
    }
    return 0;
    case WM_TIMER:
    {
        int iIndex = SendMessageW(g_hLV, LVM_GETTOPINDEX, 0, 0);
        if (m_iDragDirection == 1)// 向上
        {
            iIndex -= 1;
            SendMessageW(g_hLV, LVM_ENSUREVISIBLE, iIndex, TRUE);
        }
        else if (m_iDragDirection == 2)// 向下
        {
            iIndex += (SendMessageW(g_hLV, LVM_GETCOUNTPERPAGE, 0, 0) + 1);
            SendMessageW(g_hLV, LVM_ENSUREVISIBLE, iIndex, TRUE);
        }
    }
    return 0;
	}
	return DefWindowProcW(hWnd, message, wParam, lParam);
}
LRESULT CALLBACK WndProc_RitBK(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam)//右上容器窗口过程
{
    static WCHAR szListName[MAX_PATH];
    switch (message)
    {
    case WM_COMMAND:
    {
        if (lParam)
        {
            switch (LOWORD(wParam))
            {
            case IDC_BT_JUMP:
            {
                if (g_iCurrFileIndex != -1)
                {
                    SendMessageW(g_hLV, LVM_ENSUREVISIBLE, g_iCurrFileIndex, FALSE);
                    SetFocus(g_hLV);
                }
            }
            return 0;
            case IDC_BT_OPEN:
            {
                HMENU hMenu = CreatePopupMenu();
                UINT uFlags = g_bSort || g_iSearchResult != -1 ? MF_GRAYED : 0;
                AppendMenuW(hMenu, uFlags, IDMI_OPEN_FILE, L"打开文件");
                AppendMenuW(hMenu, uFlags, IDMI_OPEN_FOLDER, L"打开文件夹");

                RECT rc;
                GetWindowRect((HWND)lParam, &rc);
                int iRet = TrackPopupMenu(hMenu, TPM_RETURNCMD, rc.left, rc.bottom, 0, hWnd, NULL);
                DestroyMenu(hMenu);
                int iIndex = -1;
                for (int i = 0; i < g_ItemData->iCount; ++i)
                {
                    if (SendMessageW(g_hLV, LVM_GETITEMSTATE, i, LVIS_SELECTED) == LVIS_SELECTED)
                    {
                        iIndex = i;
                        break;
                    }
                }

                if (iIndex < 0)
                    iIndex = g_ItemData->iCount;
                switch (iRet)
                {
                case IDMI_OPEN_FILE:
                {
                    IFileOpenDialog* pfod;
                    HRESULT hr = CoCreateInstance(CLSID_FileOpenDialog,
                        NULL,
                        CLSCTX_INPROC_SERVER,
                        IID_PPV_ARGS(&pfod));
                    if (!SUCCEEDED(hr))
                        return 0;
                    pfod->SetTitle(L"打开音频文件");
                    pfod->SetOptions(FOS_ALLOWMULTISELECT | FOS_FORCEFILESYSTEM | FOS_FILEMUSTEXIST);
                    COMDLG_FILTERSPEC cf[] =
                    {
                        {L"音频文件(*.mp1;*.mp2;*.xm;*.mp3;*.flac;*.wma;*.wav;*.m4a;*.ogg;*.acc;*.ape;*.aiff)",
                        L"*.mp1;*.mp2;*.xm;*.mp3;*.flac;*.wma;*.wav;*.m4a;*.ogg;*.acc;*.ape;*.aiff"},
                        {L"所有文件",L"*.*"}
                    };
                    pfod->SetFileTypes(2, cf);
                    pfod->Show(hWnd);
                    IShellItemArray* psia;
                    hr = pfod->GetResults(&psia);
                    if (!SUCCEEDED(hr))
                    {
                        pfod->Release();
                        return 0;
                    }
                    DWORD cItem;
                    hr = psia->GetCount(&cItem);
                    if (!SUCCEEDED(hr))
                    {
                        psia->Release();
                        pfod->Release();
                        return 0;
                    }
                    IShellItem* psi;
                    LPWSTR pszFile;
                    List_SetRedraw(FALSE);
                    DWORD dwExitCode;
                    if (GetExitCodeThread(m_htdMusicTime, &dwExitCode))
                        StopThread_MusicTime();
                    else
                        dwExitCode = 0;

					if (iIndex <= g_iCurrFileIndex && g_iCurrFileIndex != -1 && cItem)
                        g_iCurrFileIndex += cItem;

                    for (DWORD i = 0; i < cItem; i++)
                    {
                        psia->GetItemAt(i, &psi);
                        psi->GetDisplayName(SIGDN_DESKTOPABSOLUTEPARSING, &pszFile);
                        List_Add(pszFile, NULL, iIndex, FALSE);
                        ++iIndex;
                        CoTaskMemFree(pszFile);
                        psi->Release();
                    }

                    psia->Release();
                    pfod->Release();
                    List_SetRedraw(TRUE);
                    List_ResetLV();
                    List_FillMusicTimeColumn(TRUE);
                }
                return 0;
                case IDMI_OPEN_FOLDER:
                {
                    IFileOpenDialog* pfod;
                    HRESULT hr = CoCreateInstance(CLSID_FileOpenDialog,
                        NULL,
                        CLSCTX_INPROC_SERVER,
                        IID_PPV_ARGS(&pfod));
                    if (!SUCCEEDED(hr))
                        return 0;
                    pfod->SetOptions(FOS_PICKFOLDERS | FOS_FORCEFILESYSTEM | FOS_PATHMUSTEXIST);
                    pfod->Show(hWnd);
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
                    WIN32_FIND_DATAW wfd;
                    WCHAR szFile[MAX_PATH];
                    lstrcpyW(szFile, pszPath);
                    lstrcatW(szFile, L"\\*.mp3");
                    HANDLE hFind = FindFirstFileW(szFile, &wfd);//开始枚举
                    if (hFind == INVALID_HANDLE_VALUE)
                        return 0;
                    List_SetRedraw(FALSE);
                    DWORD dwExitCode;
                    if (GetExitCodeThread(m_htdMusicTime, &dwExitCode))
                        StopThread_MusicTime();
                    else
                        dwExitCode = 0;

                    int cItem = 0;
                    do
                    {
                        lstrcpyW(szFile, pszPath);
                        lstrcatW(szFile, L"\\");
                        lstrcatW(szFile, wfd.cFileName);
                        List_Add(szFile, NULL, iIndex, FALSE);
                        ++cItem;
                        ++iIndex;
                    } while (FindNextFileW(hFind, &wfd));//继续枚举
                    if (cItem)
                    {
                        if (iIndex - cItem <= g_iCurrFileIndex && g_iCurrFileIndex != -1)
                            g_iCurrFileIndex += cItem;
                    }

                    FindClose(hFind);//释放
                    CoTaskMemFree(pszPath);
                    List_SetRedraw(TRUE);
                    List_ResetLV();
                    List_FillMusicTimeColumn(TRUE);
                }
                return 0;
                }
            }
            return 0;
            case IDC_BT_LOADLIST:
            {
                INT_PTR pResult = DialogBoxParamW(g_hInst, MAKEINTRESOURCEW(IDD_LIST), g_hBKList, DlgProc_List, DLGTYPE_LOADLIST);
                if (pResult == NULL)
                    return 0;

                lstrcpyW(szListName, ((DLGRESULT_LIST*)pResult)->szFileName);

                HANDLE hFile = CreateFileW(
                    ((DLGRESULT_LIST*)pResult)->szFileName,
                    GENERIC_READ,
                    0,
                    NULL,
                    OPEN_EXISTING,
                    FILE_ATTRIBUTE_NORMAL,
                    NULL);
                if (hFile == INVALID_HANDLE_VALUE)
                {
                    delete (DLGRESULT_LIST*)pResult;
                    return 0;
                }

                BYTE byFileMark[4];
                DWORD cbRead;
                ReadFile(hFile, byFileMark, 4, &cbRead, NULL);
                if (memcmp(byFileMark, "QKPL", 4))
                {
                    CloseHandle(hFile);
                    delete (DLGRESULT_LIST*)pResult;
                    return 0;
                }

                DWORD dwExitCode;
                if (GetExitCodeThread(m_htdMusicTime, &dwExitCode))
                    StopThread_MusicTime();
                else
                    dwExitCode = 0;

                DWORD dwSize = GetFileSize(hFile, NULL);
                HANDLE hMapping = CreateFileMappingW(hFile, NULL, PAGE_READONLY, 0, dwSize, NULL);
                BYTE* pFile = (BYTE*)MapViewOfFile(hMapping, FILE_MAP_READ, 0, 0, dwSize);
                BYTE* pCurr = pFile;

                LISTFILEHEADER* pListHeader = (LISTFILEHEADER*)pCurr;
                LISTFILEITEM* pListItem;
                PLAYERLISTUNIT t = { 0 };
                int iLenght;

                pCurr += sizeof(LISTFILEHEADER);//跳过文件头
                List_SetRedraw(FALSE);
                for (int i = 0; i < pListHeader->iCount; ++i)
                {
                    //读项目头
                    pListItem = (LISTFILEITEM*)pCurr;
                    pCurr += sizeof(LISTFILEITEM);
                    //读名称
                    t.pszName = (PWSTR)pCurr;
                    pCurr += (lstrlenW(t.pszName) + 1) * sizeof(WCHAR);
                    //读文件名
                    t.pszFile = (PWSTR)pCurr;
                    pCurr += (lstrlenW(t.pszFile) + 1) * sizeof(WCHAR);
                    if (pListItem->uFlags & QKLIF_BOOKMARK)
                    {
                        memcpy(&t.crBookMark, pCurr, sizeof(COLORREF));// 读书签颜色
                        pCurr += sizeof(COLORREF);

                        iLenght = lstrlenW((PWSTR)pCurr);
                        if (iLenght)
                            t.pszBookMark = (PWSTR)pCurr;// 读书签名称
                        else
                            t.pszBookMark = NULL;
                        pCurr += (iLenght + 1) * sizeof(WCHAR);

                        iLenght = lstrlenW((PWSTR)pCurr);
                        if (iLenght)
                            t.pszBookMarkComment = (PWSTR)pCurr;// 读书签备注
                        else
                            t.pszBookMarkComment = NULL;
                        pCurr += (iLenght + 1) * sizeof(WCHAR);
                    }
                    else
                        pCurr += sizeof(COLORREF) + sizeof(WCHAR) * 2;

                    if (pListHeader->dwVer >= QKLFVER_2)// 版本2
                    {
                        if (pListItem->uFlags & QKLIF_TIME)
                        {
                            t.pszTime = (PWSTR)pCurr;
                            pCurr += (lstrlenW(t.pszTime) + 1) * sizeof(WCHAR);
                        }
                        else
                        {
                            t.pszTime = NULL;
                            pCurr += sizeof(WCHAR);
                        }
                    }
                    else
                        t.pszTime = NULL;

                    List_Add(t.pszFile, t.pszName, -1, FALSE, pListItem->uFlags, t.crBookMark, t.pszBookMark, t.pszBookMarkComment, t.pszTime);
                }
                List_SetRedraw(TRUE);
                List_ResetLV();
                UI_RedrawBookMarkPos();

                UnmapViewOfFile(pFile);
                CloseHandle(hMapping);
                CloseHandle(hFile);
                delete (DLGRESULT_LIST*)pResult;

                PathStripPathW(szListName);
                PathRemoveExtensionW(szListName);
                SetWindowTextW(GetDlgItem(g_hBKRight, IDC_ST_LISTNAME), szListName);
                g_iLaterPlay = -1;
                List_FillMusicTimeColumn(TRUE);
            }
            return 0;
            case IDC_BT_SAVE:
            {
                if (!QKArray_GetCount(g_ItemData))
                    return 0;

                INT_PTR pResult = DialogBoxParamW(g_hInst, MAKEINTRESOURCEW(IDD_LIST), g_hBKList, DlgProc_List, DLGTYPE_SAVELIST);
                if (!pResult)
                    return 0;

                lstrcpyW(szListName, ((DLGRESULT_LIST*)pResult)->szFileName);

                HANDLE hFile = CreateFileW(
                    ((DLGRESULT_LIST*)pResult)->szFileName,
                    GENERIC_WRITE | GENERIC_READ,
                    0,
                    NULL,
                    CREATE_ALWAYS,
                    FILE_ATTRIBUTE_NORMAL,
                    NULL);// 打开文件
                if (hFile == INVALID_HANDLE_VALUE)
                {
                    delete (DLGRESULT_LIST*)pResult;
                    return 0;
                }

                DWORD cbWritten;
                LISTFILEHEADER ListHeader = { 0 };
                memcpy(&(ListHeader.cHeader), "QKPL", 4);// 文件头
                ListHeader.iCount = g_ItemData->iCount;
                ListHeader.dwVer = QKLFVER_2;

                WriteFile(hFile, &ListHeader, sizeof(LISTFILEHEADER), &cbWritten, NULL);//写文件头

                int iIndex = 0;

                LISTFILEITEM ListItem = { 0 };
                PLAYERLISTUNIT* p;
                WCHAR cNULL = 0;
                for (int i = 0; i < ListHeader.iCount; ++i)
                {
                    p = (PLAYERLISTUNIT*)QKArray_Get(g_ItemData, i);
                    ListItem.uFlags = p->dwFlags | (p->pszTime ? QKLIF_TIME : 0);
                    WriteFile(hFile, &ListItem, sizeof(LISTFILEITEM), &cbWritten, NULL);// 项目头
                    WriteFile(hFile, p->pszName, (lstrlenW(p->pszName) + 1) * sizeof(WCHAR), &cbWritten, NULL);// 名称
                    WriteFile(hFile, p->pszFile, (lstrlenW(p->pszFile) + 1) * sizeof(WCHAR), &cbWritten, NULL);// 文件名
                    WriteFile(hFile, &p->crBookMark, sizeof(COLORREF), &cbWritten, NULL);// 书签颜色
                    if (p->pszBookMark)
                        WriteFile(hFile, p->pszBookMark, (lstrlenW(p->pszBookMark) + 1) * sizeof(WCHAR), &cbWritten, NULL);// 书签名称
                    else
                        WriteFile(hFile, &cNULL, sizeof(WCHAR), &cbWritten, NULL);// 没有则写NULL
                    if (p->pszBookMarkComment)
                        WriteFile(hFile, p->pszBookMarkComment, (lstrlenW(p->pszBookMarkComment) + 1) * sizeof(WCHAR), &cbWritten, NULL);// 书签备注
                    else
                        WriteFile(hFile, &cNULL, sizeof(WCHAR), &cbWritten, NULL);// 没有则写NULL
                    if (p->pszTime)
                        WriteFile(hFile, p->pszTime, (lstrlenW(p->pszTime) + 1) * sizeof(WCHAR), &cbWritten, NULL);// 书签备注
                    else
                        WriteFile(hFile, &cNULL, sizeof(WCHAR), &cbWritten, NULL);// 没有则写NULL
                }
                CloseHandle(hFile);
                delete (DLGRESULT_LIST*)pResult;

                PathStripPathW(szListName);
                PathRemoveExtensionW(szListName);
                SetWindowTextW(GetDlgItem(g_hBKRight, IDC_ST_LISTNAME), szListName);
            }
            return 0;
            case IDC_BT_EMPTY:
            {
				if (QKMessageBox(L"确定要清空播放列表吗？", NULL, (HICON)TD_WARNING_ICON, L"询问", g_hBKList, NULL, 2) == QKMSGBOX_BTID_1)
                {
                    Playing_Stop();
                    List_Delete(-1, TRUE);
                    SetWindowTextW(GetDlgItem(g_hBKRight, IDC_ST_LISTNAME), L"/*当前无播放列表*/");
                    UI_RedrawBookMarkPos();
                    g_iLaterPlay = -1;
                }
            }
            return 0;
            case IDC_BT_SEARCH:
            {
                PWSTR pszEdit;
                int iLength = GetWindowTextLengthW(GetDlgItem(g_hBKRight, IDC_ED_SEARCH));
                static int iTopIndex = -1, iVisibleItemCount = 0;
                static int iTempCurr = -1, iTempLater = -1;
                if (!iLength)// 没有文本
                {
                    if (g_iSearchResult != -1)// 做现行播放项LV索引转换
                    {
                        ///////还原现行播放索引和稍后播放索引
                        if (g_iCurrFileIndex < g_iSearchResult && g_iCurrFileIndex != -1)// 索引落在结果数之内，说明搜索时现行索引被转换，现在要转换回来
                            g_iCurrFileIndex = List_GetArrayItemIndex(g_iCurrFileIndex);
                        if (iTempCurr != -1)
                            g_iCurrFileIndex = iTempCurr;

                        // 转换稍后播放，跟上面同理
                        if (g_iLaterPlay < g_iSearchResult && g_iLaterPlay != -1)
                            g_iLaterPlay = List_GetArrayItemIndex(g_iLaterPlay);
                        if (iTempLater != -1)
                            g_iLaterPlay = iTempLater;
                        ///////还原位置
                        g_iSearchResult = -1;
                        List_ResetLV();// 数目变了，不是List_Redraw而是List_ResetLV
                        SendMessageW(g_hLV, LVM_ENSUREVISIBLE, iTopIndex + iVisibleItemCount - 1, TRUE);// 回到原来的位置（话说微软就不能搞个设置LV滚动条位置的消息？）
                        UI_RedrawBookMarkPos();
                    }
                }
                else
                {
                    iTopIndex = SendMessageW(g_hLV, LVM_GETTOPINDEX, 0, 0);
                    iVisibleItemCount = SendMessageW(g_hLV, LVM_GETCOUNTPERPAGE, 0, 0);

                    pszEdit = new WCHAR[iLength + 1];
                    GetWindowTextW(GetDlgItem(g_hBKRight, IDC_ED_SEARCH), pszEdit, iLength + 1);// 取编辑框文本
                    g_iSearchResult = 0;// 搜索结果置0
                    int iIndex;
                    BOOL b1 = (g_iCurrFileIndex != -1);// 是否要比对并转换现行索引
                    BOOL b2 = (g_iLaterPlay != -1);
                    BOOL bTransed1 = FALSE, bTransed2 = FALSE;
                    for (int i = 0; i < g_ItemData->iCount; ++i)
                    {
                        iIndex = ((PLAYERLISTUNIT*)QKArray_Get(g_ItemData, i))->iMappingIndexSort;
                        if (iIndex == -1)
                            iIndex = i;
                        // 上面是处理索引映射
                        if (QKStrInStr(((PLAYERLISTUNIT*)QKArray_Get(g_ItemData, iIndex))->pszName, pszEdit))// 比对文本
                        {
                            ((PLAYERLISTUNIT*)QKArray_Get(g_ItemData, g_iSearchResult))->iMappingIndexSearch = iIndex;// 映射，这个要从列表的开头开始映射，顺次往下，使用g_iSearchResult做到这一点
                            if (b1)
                            {
                                if (i == g_iCurrFileIndex)
                                {
                                    g_iCurrFileIndex = g_iSearchResult;// 转换现行播放索引
                                    b1 = FALSE;
                                    bTransed1 = TRUE;
                                }
                            }
                            if (b2)
                            {
                                if (i == g_iLaterPlay)
                                {
                                    g_iLaterPlay = g_iSearchResult;// 转换稍后播放索引
                                    b2 = FALSE;
                                    bTransed2 = TRUE;
                                }
                            }
                            ++g_iSearchResult;// 结果数递增
                        }
                    }

                    if (!bTransed1)
                    {
                        iTempCurr = g_iCurrFileIndex;
                        g_iCurrFileIndex = -1;
                    }
                    if (!bTransed2)
                    {
                        iTempLater = g_iLaterPlay;
                        g_iLaterPlay = -1;
                    }

                    List_ResetLV();
                    UI_RedrawBookMarkPos();
                    SetScrollPos(g_hLV, SB_VERT, 0, TRUE);
                    SendMessageW(g_hLV, WM_VSCROLL, MAKEWPARAM(SB_THUMBPOSITION, 0), NULL);
                    SendMessageW(g_hLV, WM_MOUSEWHEEL, 0, 0);
                }
            }
            return 0;
            case IDC_BT_MANAGING:
            {
                static BOOL bAscending = TRUE;// 升序：从小到大；降序：从大到小
                HMENU hMenu = CreatePopupMenu();
				UINT uFlags = ((g_iSearchResult != -1) || !g_ItemData->iCount) ? MF_GRAYED : 0;
                AppendMenuW(hMenu, g_bSort ? uFlags : MF_GRAYED, IDMI_LM_SORT_DEF, L"默认排序");
                AppendMenuW(hMenu, uFlags, IDMI_LM_SORT_FILENAME, L"按文件名排序");
                AppendMenuW(hMenu, uFlags, IDMI_LM_SORT_NAME, L"按名称排序");
                AppendMenuW(hMenu, uFlags, IDMI_LM_SORT_CTIME, L"按创建时间排序");
                AppendMenuW(hMenu, uFlags, IDMI_LM_SORT_MTIME, L"按修改时间排序");
                AppendMenuW(hMenu, uFlags, IDMI_LM_SORT_REVERSE, L"倒置排序");
                AppendMenuW(hMenu, MF_SEPARATOR, 0, NULL);
                AppendMenuW(hMenu, 0, IDMI_LM_SORT_ASCENDING, L"升序");
                AppendMenuW(hMenu, 0, IDMI_LM_SORT_DESCENDING, L"降序");
                CheckMenuRadioItem(hMenu, IDMI_LM_SORT_ASCENDING, IDMI_LM_SORT_DESCENDING,
                    GS.bAscending ? IDMI_LM_SORT_ASCENDING : IDMI_LM_SORT_DESCENDING, MF_BYCOMMAND);// 处理单选项

                AppendMenuW(hMenu, GS.bNoBookMarkWhenSort ? MF_CHECKED : 0, IDMI_LM_NOBOOKMARK, L"排序时不显示书签标记");
                AppendMenuW(hMenu, g_bSort ? uFlags : MF_GRAYED, IDMI_LM_FIXSORT, L"固定为默认排序");
                AppendMenuW(hMenu, MF_SEPARATOR, 0, NULL);
                AppendMenuW(hMenu, uFlags, IDMI_LM_BOOKMARK, L"书签...");
                AppendMenuW(hMenu, 0, IDMI_LM_DETAIL, L"详细信息...");
                AppendMenuW(hMenu, 0, IDMI_LM_SETLVDEFWIDTH, L"将列表调至默认宽度");
                AppendMenuW(hMenu, 0, IDMI_LM_UPDATETIME, L"强制更新列表时间信息");
                RECT rc;
                GetWindowRect((HWND)lParam, &rc);
                int iRet = TrackPopupMenu(hMenu, TPM_RETURNCMD, rc.left, rc.bottom, 0, hWnd, NULL);
                DestroyMenu(hMenu);
                switch (iRet)
                {
                case IDMI_LM_SORT_DEF:// 返回默认排序
                {
                    if (g_iCurrFileIndex != -1)
                        g_iCurrFileIndex = ((PLAYERLISTUNIT*)QKArray_Get(g_ItemData, g_iCurrFileIndex))->iMappingIndexSort;

                    if (g_iLaterPlay != -1)
                        g_iLaterPlay = ((PLAYERLISTUNIT*)QKArray_Get(g_ItemData, g_iLaterPlay))->iMappingIndexSort;

                    for (int i = 0; i < g_ItemData->iCount; ++i)
                        ((PLAYERLISTUNIT*)QKArray_Get(g_ItemData, i))->iMappingIndexSort = -1;

                    g_bSort = FALSE;
                    List_Redraw();
                    UI_RedrawBookMarkPos();
                }
                return 0;
                case IDMI_LM_SORT_FILENAME:// 按文件名排序
                {
                    g_bSort = TRUE;
                    int iCount = g_ItemData->iCount;
                    PLAYERLISTUNIT* p1, * p2;
                    int m, n;
                    int iResult = GS.bAscending ? 1 : -1;
                    // StrCmpLogicalW：字符串相同返回零；psz1大于psz2返回1；psz1小于psz2返回-1
                    // 冒泡.........
                    for (int i = 0; i < iCount - 1; ++i)
                    {
                        for (int j = 0; j < iCount - i - 1; ++j)
                        {
                            p1 = (PLAYERLISTUNIT*)QKArray_Get(g_ItemData, j);
                            p2 = (PLAYERLISTUNIT*)QKArray_Get(g_ItemData, j + 1);
                            m = p1->iMappingIndexSort == -1 ? j : p1->iMappingIndexSort;
                            n = p2->iMappingIndexSort == -1 ? j + 1 : p2->iMappingIndexSort;
                            if (StrCmpLogicalW(
                                ((PLAYERLISTUNIT*)QKArray_Get(g_ItemData, m))->pszFile,
                                ((PLAYERLISTUNIT*)QKArray_Get(g_ItemData, n))->pszFile) == iResult)
                            {
                                p1->iMappingIndexSort = n;
                                p2->iMappingIndexSort = m;
                            }
                        }
                    }
                    Sort_End();
                }
                return 0;
                case IDMI_LM_SORT_NAME:// 按列表名称排序
                {
                    g_bSort = TRUE;
                    int iCount = g_ItemData->iCount;
                    PLAYERLISTUNIT* p1, * p2;
                    int m, n;
                    int iResult = GS.bAscending ? 1 : -1;
                    // StrCmpLogicalW：字符串相同返回零；psz1大于psz2返回1；psz1小于psz2返回-1
                    for (int i = 0; i < iCount - 1; ++i)
                    {
                        for (int j = 0; j < iCount - i - 1; ++j)
                        {
                            p1 = (PLAYERLISTUNIT*)QKArray_Get(g_ItemData, j);
                            p2 = (PLAYERLISTUNIT*)QKArray_Get(g_ItemData, j + 1);
                            m = p1->iMappingIndexSort == -1 ? j : p1->iMappingIndexSort;
                            n = p2->iMappingIndexSort == -1 ? j + 1 : p2->iMappingIndexSort;
                            if (StrCmpLogicalW(
                                ((PLAYERLISTUNIT*)QKArray_Get(g_ItemData, m))->pszName,
                                ((PLAYERLISTUNIT*)QKArray_Get(g_ItemData, n))->pszName) == iResult)
                            {
                                p1->iMappingIndexSort = n;
                                p2->iMappingIndexSort = m;
                            }
                        }
                    }
                    Sort_End();
                }
                return 0;
                case IDMI_LM_SORT_CTIME:// 按创建时间排序
                case IDMI_LM_SORT_MTIME:// 按修改时间排序
                {
                    g_bSort = TRUE;
                    int iCount = g_ItemData->iCount;
                    PLAYERLISTUNIT* p1, * p2;
                    int m, n;
                    int iResult = GS.bAscending ? 1 : -1;
                    // CompareFileTime：第一个文件时间早于第二个返回-1；第一个等于第二个返回0；第一个晚于第二个返回1
                    WIN32_FIND_DATAW wfd;
                    HANDLE hFind;
                    FILETIME* ft = new FILETIME[iCount];
                    if (iRet == IDMI_LM_SORT_CTIME)
                    {
                        for (int i = 0; i < iCount; ++i)
                        {
                            hFind = FindFirstFileW(((PLAYERLISTUNIT*)QKArray_Get(g_ItemData, i))->pszFile, &wfd);// 这样比先CreateFile再GetFileTime快
                            memcpy(ft + i, &wfd.ftCreationTime, sizeof(FILETIME));
                            FindClose(hFind);
                        }
                    }
                    else
                    {
                        for (int i = 0; i < iCount; ++i)
                        {
                            hFind = FindFirstFileW(((PLAYERLISTUNIT*)QKArray_Get(g_ItemData, i))->pszFile, &wfd);// 这样比先CreateFile再GetFileTime快
                            memcpy(ft + i, &wfd.ftLastWriteTime, sizeof(FILETIME));
                            FindClose(hFind);
                        }
                    }

                    for (int i = 0; i < iCount - 1; ++i)
                    {
                        for (int j = 0; j < iCount - i - 1; ++j)
                        {
                            p1 = (PLAYERLISTUNIT*)QKArray_Get(g_ItemData, j);
                            p2 = (PLAYERLISTUNIT*)QKArray_Get(g_ItemData, j + 1);
                            m = p1->iMappingIndexSort == -1 ? j : p1->iMappingIndexSort;
                            n = p2->iMappingIndexSort == -1 ? j + 1 : p2->iMappingIndexSort;
                            if (CompareFileTime(ft + m, ft + n) == iResult)
                            {
                                p1->iMappingIndexSort = n;
                                p2->iMappingIndexSort = m;
                            }
                        }
                    }
                    Sort_End();
                }
                return 0;
                case IDMI_LM_SORT_ASCENDING:// 升序
                    GS.bAscending = TRUE;
                    return 0;
                case IDMI_LM_SORT_DESCENDING:// 降序
                    GS.bAscending = FALSE;
                    return 0;
                case IDMI_LM_FIXSORT:// 固定为默认排序
                {
                    g_bSort = FALSE;
                    PLAYERLISTUNIT** p;
                    int iCount = g_ItemData->iCount;
                    p = new PLAYERLISTUNIT * [iCount];
                    for (int i = 0; i < iCount; ++i)
                    {
                        p[i] = (PLAYERLISTUNIT*)QKArray_Get(g_ItemData, ((PLAYERLISTUNIT*)QKArray_Get(g_ItemData, i))->iMappingIndexSort);
                    }

                    for (int i = 0; i < iCount; ++i)
                    {
                        ((PLAYERLISTUNIT*)QKArray_Get(g_ItemData, i))->iMappingIndexSort = -1;
                        QKArray_Set(g_ItemData, i, p[i]);
                    }
                    delete[] p;
                    List_Redraw();
                }
                return 0;
                case IDMI_LM_DETAIL:// 查看列表详细信息
                {

                }
                return 0;
                case IDMI_LM_BOOKMARK:// 书签管理
                    DialogBoxParamW(g_hInst, MAKEINTRESOURCEW(IDD_BOOKMARK), g_hBKList, DlgProc_BookMark, 0);
                    return 0;
                case IDMI_LM_SORT_REVERSE:// 倒置排序
                {
                    g_bSort = TRUE;
                    int m;
                    PLAYERLISTUNIT* p1, * p2;
                    int iCount = g_ItemData->iCount / 2;
                    int i;
                    for (i = 0; i < iCount; ++i)
                    {
                        p1 = (PLAYERLISTUNIT*)QKArray_Get(g_ItemData, i);
                        p2 = (PLAYERLISTUNIT*)QKArray_Get(g_ItemData, g_ItemData->iCount - 1 - i);

                        m = p1->iMappingIndexSort == -1 ? i : p1->iMappingIndexSort;
                        p1->iMappingIndexSort = p2->iMappingIndexSort == -1 ? g_ItemData->iCount - 1 - i : p2->iMappingIndexSort;
                        p2->iMappingIndexSort = m;
                    }
                    p1 = (PLAYERLISTUNIT*)QKArray_Get(g_ItemData, i);
                    if (p1->iMappingIndexSort == -1)
                        p1->iMappingIndexSort = i;

                    if (g_iCurrFileIndex != -1)
                    {
                        for (int i = 0; i < iCount; ++i)
                        {
                            if (((PLAYERLISTUNIT*)QKArray_Get(g_ItemData, i))->iMappingIndexSort == g_iCurrFileIndex)
                            {
                                g_iCurrFileIndex = i;
                                break;
                            }
                        }
                    }
                    List_Redraw();
                    UI_RedrawBookMarkPos();
                }
                return 0;
                case IDMI_LM_SETLVDEFWIDTH:// 置默认列表宽
				{
					if (g_bListSeped)
					{
						RECT rcWnd, rcClient;
						GetWindowRect(g_hBKList, &rcWnd);
						GetClientRect(g_hBKList, &rcClient);
						g_cxBKList = DPIS_DEFCXLV + DPIS_GAP;
						SetWindowPos(g_hBKList, NULL, 0, 0,
							g_cxBKList + (rcWnd.right - rcWnd.left - rcClient.right),
							rcWnd.bottom - rcWnd.top,
							SWP_NOZORDER | SWP_NOMOVE);
                    }
                    else
                    {
                        g_cxBKList = DPIS_DEFCXLV;
                        UI_SetRitCtrlPos();
                        RECT rc;
                        GetClientRect(g_hMainWnd, &rc);
                        SendMessageW(g_hMainWnd, WM_SIZE, 0, MAKELPARAM(rc.right, rc.bottom));
                    }
                }
                return 0;
                case IDMI_LM_UPDATETIME:
                    List_FillMusicTimeColumn(FALSE);
                    return 0;
                case IDMI_LM_NOBOOKMARK:
                    if (GS.bNoBookMarkWhenSort)
                        GS.bNoBookMarkWhenSort = FALSE;
                    else
                        GS.bNoBookMarkWhenSort = TRUE;
                    UI_RedrawBookMarkPos();
                    return 0;
                }
            }
            }
        }
    }
    break;
    case WM_PAINT:
    {
        PAINTSTRUCT ps;
        HDC hDC = BeginPaint(hWnd, &ps);
        FillRect(hDC, &ps.rcPaint, (HBRUSH)GetStockObject(WHITE_BRUSH));
        EndPaint(hWnd, &ps);
    }
    return 0;
    case WM_CTLCOLORSTATIC:// 静态着色
        SetTextColor((HDC)wParam, QKCOLOR_CYANDEEPER);
        return (LRESULT)GetStockObject(WHITE_BRUSH);
    }
    return DefWindowProcW(hWnd, message, wParam, lParam);
}
LRESULT CALLBACK WndProc_Edit(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam)
{
    switch (message)
    {
    case WM_KEYDOWN:
    {
        if (wParam == VK_RETURN)//按回车键
            SendMessageW(g_hBKRight, WM_COMMAND, MAKELONG(IDC_BT_SEARCH, 0), (LPARAM)GetDlgItem(g_hBKRight, IDC_BT_SEARCH));
    }
    }

    return CallWindowProcW((WNDPROC)GetPropW(hWnd, PROP_WNDPROC), hWnd, message, wParam, lParam);
}
LRESULT CALLBACK WndProc_ListView(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam)
{
    static int iLastHoverItem = -1;
    int iCurrItem = -1;
    LRESULT lRet = CallWindowProcW((WNDPROC)GetPropW(hWnd, PROP_WNDPROC), hWnd, message, wParam, lParam);

    switch (message)
    {
    case WM_LBUTTONDOWN:// 啊啊啊我没办法了，不这么写从别的控件点过来的时候LV不会获得焦点，WM_KEYDOWN也就不会发给LV......
        SetFocus(hWnd);
        break;
    case WM_KEYDOWN:
    {
        if (wParam == VK_RETURN)// 按回车键播放曲目
        {
            int iCount = QKArray_GetCount(g_ItemData);
            int i;
            for (i = 0; i < iCount; ++i)
            {
                if (SendMessageW(hWnd, LVM_GETITEMSTATE, i, LVIS_SELECTED) == LVIS_SELECTED)
                    break;
            }

            if (i != iCount)
                Playing_PlayFile(i);
        }
        else if (wParam == 0x41)// A键按下
        {
            if (GetKeyState(VK_CONTROL) & 0x80000000)// Ctrl + A 全选
            {
                LVITEMW li;
                li.stateMask = LVIS_SELECTED;
                li.state = LVIS_SELECTED;
                SendMessageW(hWnd, LVM_SETITEMSTATE, -1, (LPARAM)&li);// wParam = -1：应用于所有项目
            }
        }
    }
    break;
    }
    return lRet;
}
INT_PTR CALLBACK DlgProc_List(HWND hDlg, UINT message, WPARAM wParam, LPARAM lParam)
{
    static DWORD dwDlgType;
    switch (message)
    {
    case WM_INITDIALOG:
    {
        // 设置提示文本
        if (lParam == DLGTYPE_SAVELIST)
        {
            SetDlgItemTextW(hDlg, IDC_ST_TIP, L"选择一个文件覆盖，或新建一个新文件");
            SetWindowTextW(hDlg, L"保存歌曲列表");
        }
        else if (lParam == DLGTYPE_LOADLIST)
        {
            SetDlgItemTextW(hDlg, IDC_ST_TIP, L"选择一个文件载入");
            SetWindowTextW(hDlg, L"载入歌曲列表");
        }
        else
            EndDialog(hDlg, NULL);
        dwDlgType = lParam;
        HWND hLV = GetDlgItem(hDlg, IDC_LV_LISTFILE);
        // 插入列
        LVCOLUMNW lc = { 0 };
        WCHAR szTitle[5];
        lc.mask = LVCF_TEXT | LVCF_WIDTH;

        lstrcpyW(szTitle, L"列表文件");
        lc.pszText = szTitle;
        lc.cx = DPI(300);
        SendMessageW(hLV, LVM_INSERTCOLUMNW, 0, (LPARAM)&lc);

        lstrcpyW(szTitle, L"修改时间");
        lc.pszText = szTitle;
        lc.cx = DPI(150);
        SendMessageW(hLV, LVM_INSERTCOLUMNW, 1, (LPARAM)&lc);
        // 设置风格
        SetWindowLongPtrW(hLV, GWL_STYLE, GetWindowLongW(hLV, GWL_STYLE) | LVS_SINGLESEL);// 单一选择
        DWORD dwStyle = LVS_EX_FULLROWSELECT | LVS_EX_DOUBLEBUFFER;
        SendMessageW(hLV, LVM_SETEXTENDEDLISTVIEWSTYLE, dwStyle, dwStyle);// 整行选择，双缓冲
        SetWindowTheme(hLV, L"Explorer", NULL);// 可视风格
        // 枚举文件
        WIN32_FIND_DATAW wfd;
        WCHAR szListFile[MAX_PATH];
        lstrcpyW(szListFile, g_pszListDir);
        lstrcatW(szListFile, L"*.QKList");// 转换QKList扩展名
        HANDLE hFind = FindFirstFileW(szListFile, &wfd);// 开始枚举
        if (hFind == INVALID_HANDLE_VALUE)
            return FALSE;
        int iIndex;
        FILETIME ft;
        SYSTEMTIME st;
        WCHAR szTime[20];
        WCHAR szBuffer[3];
        LVITEMW li;
        do
        {
            li.mask = LVIF_TEXT;
            li.iItem = SendMessageW(hLV, LVM_GETITEMCOUNT, 0, 0);
            li.iSubItem = 0;
            PathRemoveExtensionW(wfd.cFileName);
            li.pszText = wfd.cFileName;
            iIndex = SendMessageW(hLV, LVM_INSERTITEMW, 0, (LPARAM)&li);// 插入表项

            FileTimeToLocalFileTime(&wfd.ftLastWriteTime, &ft);
            FileTimeToSystemTime(&ft, &st);

            wsprintfW(szTime, L"%d-%02d-%02d %02d:%02d:%02d",
                st.wYear, st.wMonth, st.wDay, st.wHour, st.wMinute, st.wSecond);

            li.iSubItem = 1;
            li.pszText = szTime;
            SendMessageW(hLV, LVM_SETITEMTEXTW, iIndex, (LPARAM)&li);// 置标题
        } while (FindNextFileW(hFind, &wfd));// 继续枚举
        FindClose(hFind);// 释放
    }
    return FALSE;// 返回TRUE将设定具有WS_TABSTOP风格的第一个控件（wParam为窗口句柄），返回FALSE则不设定焦点
    case WM_CLOSE:
        EndDialog(hDlg, NULL);
        return TRUE;
    case WM_COMMAND:
    {
        switch (LOWORD(wParam))
        {
        case IDC_BT_OK:
        {
            DLGRESULT_LIST* pResult = new DLGRESULT_LIST;
            lstrcpyW(pResult->szFileName, g_pszListDir);
            WCHAR szFile[MAX_PATH];
            if (!GetWindowTextW(GetDlgItem(hDlg, IDC_ED_FILE), szFile, MAX_PATH))
            {
                ZeroMemory(pResult, 1);
            }
            lstrcatW(pResult->szFileName, szFile);
            lstrcatW(pResult->szFileName, L".QKList");
            if (dwDlgType == DLGTYPE_LOADLIST)
            {
                if (PathFileExistsW(pResult->szFileName))
                    EndDialog(hDlg, (INT_PTR)pResult);
                else
					QKMessageBox(L"列表文件无效", L"选定的列表文件不存在", (HICON)TD_ERROR_ICON, L"错误", hDlg);
            }
            else
                EndDialog(hDlg, (INT_PTR)pResult);
        }
        return TRUE;
        case IDC_BT_CANCEL:
            EndDialog(hDlg, NULL);
            return TRUE;
        }
    }
    return FALSE;
    case WM_NOTIFY:
    {
        if (wParam == IDC_LV_LISTFILE)
        {
            if (((NMHDR*)lParam)->code == LVN_ITEMCHANGED)
            {
                int iIndex = ((NMLISTVIEW*)lParam)->iItem;
                if (iIndex >= 0)
                {
                    LVITEMW li = { 0 };
                    li.mask = LVIF_TEXT;
                    li.iItem = iIndex;
                    li.iSubItem = 0;
                    li.cchTextMax = MAX_PATH;
                    li.pszText = new WCHAR[MAX_PATH];
                    SendMessageW(((NMHDR*)lParam)->hwndFrom, LVM_GETITEMW, 0, (LPARAM)&li);
                    SetDlgItemTextW(hDlg, IDC_ED_FILE, li.pszText);
                    delete[] li.pszText;
                    return TRUE;
                }
            }
        }
    }
    return FALSE;
    }
    return FALSE;// 处理一条消息时会返回TRUE，不处理一条消息时返回FALSE
}
INT_PTR CALLBACK DlgProc_BookMark(HWND hDlg, UINT message, WPARAM wParam, LPARAM lParam)
{
    static HWND hLV,
        hStatic;
    static HBRUSH hbrStatic = NULL;
    switch (message)
    {
    case WM_INITDIALOG:
    {
        hLV = GetDlgItem(hDlg, IDC_LV_BOOKMARK);
        hStatic = GetDlgItem(hDlg, IDC_ST_BMCLR);
        ///////////////设置风格
        DWORD dwStyle = LVS_EX_FULLROWSELECT | LVS_EX_DOUBLEBUFFER;
        SendMessageW(hLV, LVM_SETEXTENDEDLISTVIEWSTYLE, dwStyle, dwStyle);
        SetWindowTheme(hLV, L"Explorer", NULL);
        ///////////////插入列
        LVCOLUMNW lc;
        lc.mask = LVCF_TEXT | LVCF_WIDTH;
        lc.pszText = (PWSTR)L"索引";
        lc.cx = DPI(34);
        SendMessageW(hLV, LVM_INSERTCOLUMNW, 0, (LPARAM)&lc);

        lc.pszText = (PWSTR)L"名称";
        lc.cx = DPI(110);
        SendMessageW(hLV, LVM_INSERTCOLUMNW, 1, (LPARAM)&lc);

        lc.pszText = (PWSTR)L"颜色";
        lc.cx = DPI(70);
        SendMessageW(hLV, LVM_INSERTCOLUMNW, 2, (LPARAM)&lc);

        lc.pszText = (PWSTR)L"备注";
        lc.cx = DPI(170);
        SendMessageW(hLV, LVM_INSERTCOLUMNW, 3, (LPARAM)&lc);
        LVITEMW li;
        ///////////////插入表项
        SendMessageW(hLV, WM_SETREDRAW, FALSE, 0);
        li.mask = LVIF_TEXT | LVIF_PARAM;
        PLAYERLISTUNIT* p;
        int j = 0;
        WCHAR szBuf[20];
        int iIndex;
        for (int i = 0; i < g_ItemData->iCount; ++i)
        {
            p = List_GetArrayItem(i);
            if (p->dwFlags & QKLIF_BOOKMARK)
            {
                iIndex = List_GetArrayItemIndex(i);
                li.iItem = j;
                li.iSubItem = 0;
                wsprintfW(szBuf, L"%d", iIndex);
                li.pszText = szBuf;
                li.lParam = iIndex;
                SendMessageW(hLV, LVM_INSERTITEMW, 0, (LPARAM)&li);// 索引

                li.iSubItem = 1;
                li.pszText = p->pszBookMark;
                SendMessageW(hLV, LVM_SETITEMTEXTW, j, (LPARAM)&li);// 名称
                li.iSubItem = 2;
                wsprintfW(szBuf, L"0x%06X", QKGDIClrToCommonClr(p->crBookMark));// 如果在格式化文本里加#会把0x里的x一起大写，那样不太好看...
                li.pszText = szBuf;
                SendMessageW(hLV, LVM_SETITEMTEXTW, j, (LPARAM)&li);// 颜色
                li.iSubItem = 3;
                li.pszText = p->pszBookMarkComment;
                SendMessageW(hLV, LVM_SETITEMTEXTW, j, (LPARAM)&li);// 备注
                ++j;
            }
        }
        SendMessageW(hLV, WM_SETREDRAW, TRUE, 0);
        InvalidateRect(hLV, NULL, FALSE);
    }
    return TRUE;
    case WM_COMMAND:
    {
        switch (LOWORD(wParam))
        {
        case IDC_BT_OK:
            EndDialog(hDlg, 0);
            return TRUE;
        case IDC_BT_BMDEL:
        {
            LVITEMW li;
            for (int i = SendMessageW(hLV, LVM_GETITEMCOUNT, 0, 0); i >= 0; --i)
            {
                if (SendMessageW(hLV, LVM_GETITEMSTATE, i, LVIS_SELECTED) == LVIS_SELECTED)
                {
                    li.iItem = i;
                    li.iSubItem = 0;
                    li.mask = LVIF_PARAM;
                    SendMessageW(hLV, LVM_GETITEMW, 0, (LPARAM)&li);
                    List_GetArrayItem(li.lParam)->dwFlags &= ~QKLIF_BOOKMARK;
                    SendMessageW(g_hLV, LVM_REDRAWITEMS, li.lParam, li.lParam);
                    SendMessageW(hLV, LVM_DELETEITEM, i, 0);
                }
            }
            ///////////////清除现行信息
            SetWindowTextW(GetDlgItem(hDlg, IDC_ED_BMNAME), NULL);
            SetWindowTextW(GetDlgItem(hDlg, IDC_ED_BMCOMMENT), NULL);
            SetWindowTextW(GetDlgItem(hDlg, IDC_ED_BMCLR), NULL);
            SetWindowTextW(GetDlgItem(hDlg, IDC_ED_BMINDEX), NULL);
            if (hbrStatic)
                DeleteObject(hbrStatic);
            hbrStatic = NULL;
        }
        return TRUE;
        case IDC_BT_BMSAVE:
        {
            ///////////////取首选中项
            int i;
            int iCount = SendMessageW(hLV, LVM_GETITEMCOUNT, 0, 0);
            for (i = 0; i < iCount; ++i)
            {
                if (SendMessageW(hLV, LVM_GETITEMSTATE, i, LVIS_SELECTED) == LVIS_SELECTED)
                    break;
            }
            if (i == iCount)
                return TRUE;
            LVITEMW li;
            li.iItem = i;
            li.iSubItem = 0;
            li.mask = LVIF_PARAM;
            SendMessageW(hLV, LVM_GETITEMW, 0, (LPARAM)&li);
            PLAYERLISTUNIT* p = List_GetArrayItem(li.lParam);
            int iLength;
            ///////////////名称
            iLength = GetWindowTextLengthW(GetDlgItem(hDlg, IDC_ED_BMNAME));
            delete[] p->pszBookMark;
            if (!iLength)
                p->pszBookMark = NULL;
            else
            {
                p->pszBookMark = new WCHAR[iLength + 1];
                GetWindowTextW(GetDlgItem(hDlg, IDC_ED_BMNAME), p->pszBookMark, iLength + 1);
            }
            li.iSubItem = 1;
            li.pszText = p->pszBookMark;
            SendMessageW(hLV, LVM_SETITEMTEXTW, i, (LPARAM)&li);
            ///////////////备注
            iLength = GetWindowTextLengthW(GetDlgItem(hDlg, IDC_ED_BMCOMMENT));
            delete[] p->pszBookMarkComment;
            if (!iLength)
                p->pszBookMarkComment = NULL;
            else
            {
                p->pszBookMarkComment = new WCHAR[iLength + 1];
                GetWindowTextW(GetDlgItem(hDlg, IDC_ED_BMCOMMENT), p->pszBookMarkComment, iLength + 1);
            }
            li.iSubItem = 3;
            li.pszText = p->pszBookMarkComment;
            SendMessageW(hLV, LVM_SETITEMTEXTW, i, (LPARAM)&li);
            ///////////////颜色
            PWSTR pszBuf;
            iLength = GetWindowTextLengthW(GetDlgItem(hDlg, IDC_ED_BMCLR));
            pszBuf = new WCHAR[iLength + 1];
            GetWindowTextW(GetDlgItem(hDlg, IDC_ED_BMCLR), pszBuf, iLength + 1);
            StrToIntExW(pszBuf, STIF_SUPPORT_HEX, (int*)&p->crBookMark);
            p->crBookMark = QKCommonClrToGDIClr(p->crBookMark);
            li.iSubItem = 2;
            li.pszText = pszBuf;
            SendMessageW(hLV, LVM_SETITEMTEXTW, i, (LPARAM)&li);
            delete[] pszBuf;
        }
        return TRUE;
        case IDC_BT_BMJUMP:// 跳转且不关闭对话框
        {
            int i;
            int iCount = SendMessageW(hLV, LVM_GETITEMCOUNT, 0, 0);
            for (i = 0; i < iCount; ++i)
            {
                if (SendMessageW(hLV, LVM_GETITEMSTATE, i, LVIS_SELECTED) == LVIS_SELECTED)
                    break;
            }
            if (i == iCount)
                return TRUE;
            LVITEMW li;
            li.iItem = i;
            li.iSubItem = 0;
            li.mask = LVIF_PARAM;
            SendMessageW(hLV, LVM_GETITEMW, 0, (LPARAM)&li);
            SendMessageW(g_hLV, LVM_ENSUREVISIBLE, List_GetArrayItemIndex(li.lParam), FALSE);
        }
        return TRUE;
        case IDC_ST_BMCLR:
        {
            if (HIWORD(wParam) == STN_DBLCLK)// 要求SS_NOTIFY样式；双击静态重新选择颜色
            {
                CHOOSECOLORW cc = { sizeof(CHOOSECOLORW) };
                cc.hwndOwner = hDlg;
                cc.lpCustColors = (COLORREF*)GetPropW(g_hBKList, PROP_BOOKMARKCLRDLGBUF);
                cc.Flags = CC_FULLOPEN;
                if (ChooseColorW(&cc))
                {
                    if (hbrStatic)
                        DeleteObject(hbrStatic);
                    hbrStatic = CreateSolidBrush(cc.rgbResult);// 更新静态画刷
                    InvalidateRect(hStatic, NULL, TRUE);
                    WCHAR szBuf[20];
                    wsprintfW(szBuf, L"0x%06X", QKGDIClrToCommonClr(cc.rgbResult));
                    SetWindowTextW(GetDlgItem(hDlg, IDC_ED_BMCLR), szBuf);// 重设编辑框
                }
            }
        }
        }
    }
    break;
    case WM_CLOSE:
        EndDialog(hDlg, 0);
        return TRUE;
    case WM_DESTROY:
        if (hbrStatic)
            DeleteObject(hbrStatic);
        hbrStatic = NULL;
        return TRUE;
    case WM_NOTIFY:
    {
        if (wParam == IDC_LV_BOOKMARK)
        {
            switch (((NMHDR*)lParam)->code)
            {
            case LVN_ITEMCHANGED:
            {
                NMLISTVIEW* p = (NMLISTVIEW*)lParam;
                if (p->uNewState & LVIS_SELECTED && p->iItem != -1)
                {
                    LVITEMW li;
                    li.iItem = p->iItem;
                    li.mask = LVIF_PARAM;
                    SendMessageW(hLV, LVM_GETITEMW, 0, (LPARAM)&li);
                    PLAYERLISTUNIT* p = List_GetArrayItem(li.lParam);
                    SetWindowTextW(GetDlgItem(hDlg, IDC_ED_BMNAME), p->pszBookMark);
                    SetWindowTextW(GetDlgItem(hDlg, IDC_ED_BMCOMMENT), p->pszBookMarkComment);
                    if (hbrStatic)
                        DeleteObject(hbrStatic);
                    hbrStatic = CreateSolidBrush(p->crBookMark);
                    InvalidateRect(hStatic, NULL, TRUE);
                    WCHAR szBuf[20];
                    wsprintfW(szBuf, L"0x%06X", QKGDIClrToCommonClr(p->crBookMark));
                    SetWindowTextW(GetDlgItem(hDlg, IDC_ED_BMCLR), szBuf);
                    wsprintfW(szBuf, L"%d", li.lParam);
                    SetWindowTextW(GetDlgItem(hDlg, IDC_ED_BMINDEX), szBuf);
                }
                else
                {
                    SetWindowTextW(GetDlgItem(hDlg, IDC_ED_BMNAME), NULL);
                    SetWindowTextW(GetDlgItem(hDlg, IDC_ED_BMCOMMENT), NULL);
                    SetWindowTextW(GetDlgItem(hDlg, IDC_ED_BMCLR), NULL);
                    SetWindowTextW(GetDlgItem(hDlg, IDC_ED_BMINDEX), NULL);
                    if (hbrStatic)
                        DeleteObject(hbrStatic);
                    hbrStatic = NULL;
                    InvalidateRect(hStatic, NULL, TRUE);
                }
            }
            return TRUE;
            case NM_DBLCLK:// 跳转且关闭对话框
                DlgProc_BookMark(hDlg, WM_COMMAND, MAKELONG(IDC_BT_BMJUMP, 0), 0);
                SetFocus(g_hLV);
                EndDialog(hDlg, 0);
                return TRUE;
            }
        }
    }
    break;
    case WM_CTLCOLORSTATIC:// 静态着色
        if (lParam == (LPARAM)hStatic)
            return (INT_PTR)hbrStatic;// 没有画刷的时候画刷句柄为NULL，这时候相当于返回FALSE，也就是使用默认处理
        else
            break;
    }
    return FALSE;
}
HRESULT CALLBACK OLEDrag_GiveFeedBack(DWORD dwEffect)
{
    return DRAGDROP_S_USEDEFAULTCURSORS;// 使用默认光标
}
HRESULT CALLBACK OLEDrop_OnEnter(IDataObject* pDataObj, DWORD grfKeyState, POINTL pt, DWORD* pdwEffect)
{
    SetPropW(g_hLV, PROP_OLEDROPLASTINDEX, (HANDLE)-1);
    SetPropW(g_hLV, PROP_OLEDROPTARGETINDEX, (HANDLE)g_ItemData->iCount);

    FORMATETC fe =
    {
        CF_HDROP,
        NULL,
        DVASPECT_CONTENT,
        -1,
        TYMED_HGLOBAL
    };

    if (SUCCEEDED(pDataObj->QueryGetData(&fe)))
    {
        SetPropW(g_hLV, PROP_OLEDROPINVALID, (HANDLE)FALSE);
        *pdwEffect = DROPEFFECT_COPY;
    }
    else
    {
        SetPropW(g_hLV, PROP_OLEDROPINVALID, (HANDLE)TRUE);
        *pdwEffect = DROPEFFECT_NONE;
    }
    return S_OK;
}
HRESULT CALLBACK OLEDrop_OnOver(DWORD grfKeyState, POINTL pt, DWORD* pdwEffect)
{
    if (GetPropW(g_hLV, PROP_OLEDROPINVALID))
        return S_OK;

    RECT rcLV;
    GetClientRect(g_hLV, &rcLV);
    POINT pti = { pt.x,pt.y };
    ScreenToClient(g_hLV, &pti);
    if (PtInRect(&rcLV, pti))
    {
        LVHITTESTINFO lhti;
        lhti.pt = { 0,pti.y };
        int iIndex = SendMessageW(g_hLV, LVM_HITTEST, 0, (LPARAM)&lhti);
        RECT rcItem;
        LVINSERTMARK lim = { 0 };
        if (iIndex != -1)
        {
            rcItem.left = LVIR_BOUNDS;
            SendMessageW(g_hLV, LVM_GETITEMRECT, iIndex, (LPARAM)&rcItem);
            if (pti.y > rcItem.top + (rcItem.bottom - rcItem.top) / 2)
            {
                ++iIndex;
                if (iIndex >= g_ItemData->iCount)
                {
                    iIndex = g_ItemData->iCount;
                    lim.dwFlags = LVIM_AFTER;
                }
            }
        }
        else if (lhti.flags & LVHT_NOWHERE)
        {
            iIndex = g_ItemData->iCount;
            lim.dwFlags = LVIM_AFTER;
        }

        if ((int)GetPropW(g_hLV, PROP_OLEDROPLASTINDEX) != iIndex)
        {
            SetPropW(g_hLV, PROP_OLEDROPLASTINDEX, (HANDLE)iIndex);
            SetPropW(g_hLV, PROP_OLEDROPTARGETINDEX, (HANDLE)iIndex);
            lim.cbSize = sizeof(LVINSERTMARK);
            if (lim.dwFlags == LVIM_AFTER)
                --iIndex;
            lim.iItem = iIndex;
            SendMessageW(g_hLV, LVM_SETINSERTMARK, 0, (LPARAM)&lim);
        }
    }
    else
    {
        if ((int)GetPropW(g_hLV, PROP_OLEDROPLASTINDEX) != -1)
        {
            SetPropW(g_hLV, PROP_OLEDROPLASTINDEX, (HANDLE)-1);
            SetPropW(g_hLV, PROP_OLEDROPTARGETINDEX, (HANDLE)g_ItemData->iCount);
            LVINSERTMARK lim = { 0 };
            lim.cbSize = sizeof(LVINSERTMARK);
            lim.iItem = -1;
            SendMessageW(g_hLV, LVM_SETINSERTMARK, 0, (LPARAM)&lim);
        }
	}

	if (pti.y < GC.DS_LVDRAGEDGE)
	{
        int iElapse = TIMERELAPSE_LISTBKDRAG - (GC.DS_LVDRAGEDGE - pti.y) * 10;
        if (iElapse < 10)
            iElapse = 10;

        if (m_iDragTimerElapse != iElapse)
        {
            m_iDragDirection = 1;
            m_iDragTimerElapse = iElapse;
            SetTimer(g_hBKList, IDT_LISTBKDRAG, m_iDragTimerElapse, NULL);
        }
	}
	else if (pti.y > (rcLV.bottom - GC.DS_LVDRAGEDGE))
	{
		int iElapse = TIMERELAPSE_LISTBKDRAG - (pti.y - (rcLV.bottom - GC.DS_LVDRAGEDGE)) * 10;
        if (iElapse < 10)
            iElapse = 10;

        if (m_iDragTimerElapse != iElapse)
        {
            m_iDragDirection = 2;
            m_iDragTimerElapse = iElapse;
            SetTimer(g_hBKList, IDT_LISTBKDRAG, m_iDragTimerElapse, NULL);
        }
	}
    else
    {
        if (m_iDragTimerElapse)
        {
            m_iDragTimerElapse = 0;
            m_iDragDirection = 0;
            KillTimer(g_hBKList, IDT_LISTBKDRAG);
        }
    }

    return S_OK;
}
HRESULT CALLBACK OLEDrop_OnLeave()
{
    RemovePropW(g_hLV, PROP_OLEDROPLASTINDEX);
    RemovePropW(g_hLV, PROP_OLEDROPTARGETINDEX);
    RemovePropW(g_hLV, PROP_OLEDROPINVALID);
    LVINSERTMARK lim = { 0 };
    lim.cbSize = sizeof(LVINSERTMARK);
    lim.iItem = -1;
    SendMessageW(g_hLV, LVM_SETINSERTMARK, 0, (LPARAM)&lim);
    m_iDragTimerElapse = 0;
    m_iDragDirection = 0;
    KillTimer(g_hBKList, IDT_LISTBKDRAG);
    return S_OK;
}
HRESULT CALLBACK OLEDrop_OnDrop(IDataObject* pDataObj, DWORD grfKeyState, POINTL pt, DWORD* pdwEffect)
{
    BOOL b = (BOOL)GetPropW(g_hLV, PROP_OLEDROPINVALID);

    int iTargetIndex = (int)GetPropW(g_hLV, PROP_OLEDROPTARGETINDEX);
    RemovePropW(g_hLV, PROP_OLEDROPLASTINDEX);
    RemovePropW(g_hLV, PROP_OLEDROPTARGETINDEX);
    RemovePropW(g_hLV, PROP_OLEDROPINVALID);

    m_iDragTimerElapse = 0;
    m_iDragDirection = 0;
    KillTimer(g_hBKList, IDT_LISTBKDRAG);

    if (b)
        return S_OK;

    LVINSERTMARK lim = { 0 };
    lim.cbSize = sizeof(LVINSERTMARK);
    lim.iItem = -1;
    SendMessageW(g_hLV, LVM_SETINSERTMARK, 0, (LPARAM)&lim);

    FORMATETC fe =
    {
        g_uMyClipBoardFmt,
        NULL,
        DVASPECT_CONTENT,
        -1,
        TYMED_HGLOBAL
    };

    STGMEDIUM sm = { 0 };
    sm.tymed = TYMED_HGLOBAL;
    sm.pUnkForRelease = NULL;

    if (SUCCEEDED(pDataObj->GetData(&fe, &sm)))// 判断有没有自定义拖放信息
    {
        int* p = (int*)GlobalLock(sm.hGlobal);
        if (!p)
            return 0;

        int iVer = *p;// 读版本号
        if (iVer == QKOLEDRAGVER_1)
        {
            ++p;
            DWORD dwProcID = *(DWORD*)p;// 读进程ID
            if (dwProcID != GetCurrentProcessId())// 判断是不是自己向自己拖放
            {
                GlobalUnlock(sm.hGlobal);
                goto CommonDragFile;
            }
            p = (int*)((DWORD*)p + 1);

            int iCount = *p;
            ++p;
            int m = 0, n = 0;
            int iIndex;
            for (int i = 0; i < iCount; ++i)
            {
                iIndex = *p;
                if (iIndex >= iTargetIndex)
                {
                    QKArray_Insert(&g_ItemData, QKArray_Get(g_ItemData, iIndex), iTargetIndex + m);
                    QKArray_DeleteMember(&g_ItemData, iIndex + 1);
                    ++m;
                }
                else
                {
                    iIndex -= n;
                    QKArray_Insert(&g_ItemData, QKArray_Get(g_ItemData, iIndex), iTargetIndex);
                    QKArray_DeleteMember(&g_ItemData, iIndex);
                    ++n;
                }
                ++p;
            }
            LVITEMW li;
            li.stateMask = LVIS_SELECTED;
            li.state = 0;
            SendMessageW(g_hLV, LVM_SETITEMSTATE, -1, (LPARAM)&li);// 清除选中；wParam = -1：应用于所有项目
            li.state = LVIS_SELECTED;

            iTargetIndex -= n;
            for (int i = 0; i < iCount; ++i)
            {
                SendMessageW(g_hLV, LVM_SETITEMSTATE, iTargetIndex + i, (LPARAM)&li);// 选中新移动的项
            }

            PLAYERLISTUNIT* pi;
            BOOL b[2] = { 0 };
            for (int i = 0; i < g_ItemData->iCount; ++i)// 捏麻麻地算这算那的劳资脑子不够用了，直接用个简单粗暴的方法还原索引
            {
                pi = (PLAYERLISTUNIT*)QKArray_Get(g_ItemData, i);
                if (pi->dwFlags & QKLIF_DRAGMARK_CURRFILE)
                {
                    g_iCurrFileIndex = i;
                    pi->dwFlags &= ~QKLIF_DRAGMARK_CURRFILE;
                    b[0] = TRUE;
                }

                if (pi->dwFlags & QKLIF_DRAGMARK_PLLATER)
                {
                    g_iLaterPlay = i;
                    pi->dwFlags &= ~QKLIF_DRAGMARK_PLLATER;
                    b[1] = TRUE;
                }

                if (b[0] && b[1])
                    break;
            }
            GlobalUnlock(sm.hGlobal);
            List_Redraw();
            ReleaseStgMedium(&sm);
        }
        else
        {
            GlobalUnlock(sm.hGlobal);
        }
    }
    else// 仅拖放文件
    {
    CommonDragFile:// 进程ID与自身不相等时跳过来
        fe.cfFormat = CF_HDROP;
        if (FAILED(pDataObj->GetData(&fe, &sm)))// 取回HDROP
            return S_OK;
        HDROP hDrop = (HDROP)sm.hGlobal;
        int iFileCount = DragQueryFileW(hDrop, 0xFFFFFFFF, NULL, 0);// 取拖放文件总数，(int)0xFFFFFFFF == -1
        UINT iBufferSize;
        LPWSTR pszFile;
        for (int i = 0; i < iFileCount; i++)
        {
            iBufferSize = DragQueryFileW(hDrop, i, NULL, 0);// 取缓冲区大小
            pszFile = new WCHAR[iBufferSize + 1];
            DragQueryFileW(hDrop, i, pszFile, iBufferSize + 1);// 查询文件名
            List_Add(pszFile, NULL, iTargetIndex, FALSE);
            delete[] pszFile;
            ++iTargetIndex;
        }
        List_ResetLV();
        ReleaseStgMedium(&sm);
    }
    return S_OK;
}