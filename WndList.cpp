/*
* WndList.cpp
* �����б�����������ش��ڹ��̺���غ�����ʵ��
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
int             m_iDragDirection        = 0;// 0 ��Ч   1 ����   2 ����
void UI_SetRitCtrlPos()
{
    // �б����� 
	SetWindowPos(GetDlgItem(g_hBKRight, IDC_ST_LISTNAME), NULL, 0, 0,
		g_cxBKList - DPIS_GAP,
		DPIS_CYSTLISTNAME,
		SWP_NOZORDER | SWP_NOMOVE);
    // ������
	HWND hEdit = GetDlgItem(g_hBKRight, IDC_ED_SEARCH);
	SetWindowPos(hEdit, NULL, 0, 0,
		g_cxBKList - GC.cyBT - DPIS_GAP,
        GC.cyBT,
		SWP_NOZORDER | SWP_NOMOVE);
    // �þ��ж���
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
    // ������ť
	SetWindowPos(GetDlgItem(g_hBKRight, IDC_BT_SEARCH), NULL,
		g_cxBKList - GC.cyBT - DPIS_GAP,
		DPIS_CYSTLISTNAME + DPIS_GAP * 2,
		0, 0, SWP_NOZORDER | SWP_NOSIZE);
}
/*
 * Ŀ�꣺����б�ʱ����
 *
 * ������
 * p ��������
 *
 * ����ֵ��
 * ����������
 * ��ע���߳�
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
	if (g_iSearchResult != -1)// Ӧִ������ʱ����ӳ��
		i = ((PLAYERLISTUNIT*)QKArray_Get(g_ItemData, i))->iMappingIndexSearch;

	if (g_bSort)// Ӧִ����������ӳ��
		i = ((PLAYERLISTUNIT*)QKArray_Get(g_ItemData, i))->iMappingIndexSort;

	if (i == -1)
		return (PLAYERLISTUNIT*)QKArray_Get(g_ItemData, iLVIndex);
	else
		return (PLAYERLISTUNIT*)QKArray_Get(g_ItemData, i);
}
int List_GetArrayItemIndex(int iLVIndex)
{
	if (g_iSearchResult != -1)// Ӧִ������ʱ����ӳ��
		iLVIndex = ((PLAYERLISTUNIT*)QKArray_Get(g_ItemData, iLVIndex))->iMappingIndexSearch;

	if (g_bSort)// Ӧִ����������ӳ��
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
		PathStripPathW(pszNameTemp);//ȥ��·��
		PathRemoveExtensionW(pszNameTemp);//ȥ����չ��

		pListUnit->pszName = new WCHAR[lstrlenW(pszNameTemp) + 1];
		lstrcpyW(pListUnit->pszName, pszNameTemp);

		delete[] pszNameTemp;
	}

	if (dwFlags & QKLIF_BOOKMARK)// ����ǩ
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
            // �Կ�ָ��delete�ǰ�ȫ��
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
        WaitForSingleObject(m_htdMusicTime, INFINITE);//�ȴ��߳��˳�
        m_uThreadFlagMusicTime = THREADFLAG_STOPED;
    }

    CloseHandle(m_htdMusicTime);
    g_iLrcState = LRCSTATE_NOLRC;
    m_htdMusicTime = NULL;//��վ��
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
                g_iCurrFileIndex = i;// ת�����в�������
                b[0] = TRUE;
            }
            if (j == g_iLaterPlay)// �����м䲻�ܼ�else����Ϊ���в��ź��Ժ󲥷ſ�����ͬһ��
            {
                g_iLaterPlay = i;// ת���Ժ󲥷�����
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
	case WM_CREATE:// �ݺݵش�������
	{
		SetPropW(hWnd, PROP_BOOKMARKCLRDLGBUF, (HANDLE)CustClr);
		HWND hCtrl, hCtrl2;
		///////////////////////////�Ҳ�������
		hCtrl2 = CreateWindowExW(0, BKWNDCLASS, NULL, WS_CHILD | WS_VISIBLE,
			0, 0, g_cxBKList, DPIS_CYRITBK,
			hWnd, (HMENU)IDC_BK_RIGHTBTBK, g_hInst, NULL);
		g_hBKRight = hCtrl2;
		SetWindowLongPtrW(hCtrl2, GWLP_WNDPROC, (LONG_PTR)WndProc_RitBK);
		///////////////////////////�б����ƾ�̬
		hCtrl = CreateWindowExW(0, WC_STATICW, NULL, WS_CHILD | WS_VISIBLE | SS_CENTERIMAGE | SS_ELLIPSISMASK,
			0, DPIS_GAP, 0, 0,
			hCtrl2, (HMENU)IDC_ST_LISTNAME, g_hInst, NULL);
		SendMessageW(hCtrl, WM_SETFONT, (WPARAM)g_hFontDrawing, FALSE);
		SetWindowTextW(hCtrl, L"/*��ǰ�޲����б�*/");
		///////////////////////////�����༭��
		hCtrl = CreateWindowExW(WS_EX_CLIENTEDGE, WC_EDITW, NULL, WS_CHILD | WS_VISIBLE| ES_MULTILINE | ES_WANTRETURN,
			0, DPIS_CYSTLISTNAME + DPIS_GAP * 2, 0, 0,
			hCtrl2, (HMENU)IDC_ED_SEARCH, g_hInst, NULL);
		SendMessageW(hCtrl, WM_SETFONT, (WPARAM)g_hFont, FALSE);
		SetPropW(hCtrl, PROP_WNDPROC,
			(HANDLE)SetWindowLongPtrW(hCtrl, GWLP_WNDPROC, (LONG_PTR)WndProc_Edit));
		///////////////////////////������ť
		hCtrl = CreateWindowExW(0, WC_BUTTONW, NULL, WS_CHILD | WS_VISIBLE | BS_ICON,
			0, 0, GC.cyBT, GC.cyBT,
			hCtrl2, (HMENU)IDC_BT_SEARCH, g_hInst, NULL);
		SendMessageW(hCtrl, BM_SETIMAGE, IMAGE_ICON, (LPARAM)GR.hiSearch);
		UI_SetRitCtrlPos();
		///////////////////////////
        int iLeft = 0, iTop = DPIS_CYSTLISTNAME + DPIS_GAP * 3 + GC.cyBT;
		///////////////////////////
		hCtrl = CreateWindowExW(0, WC_BUTTONW, L"��λ", WS_CHILD | WS_VISIBLE,
			iLeft, iTop, DPIS_CXRITBT, GC.cyBT,
			hCtrl2, (HMENU)IDC_BT_JUMP, g_hInst, NULL);
		SendMessageW(hCtrl, WM_SETFONT, (WPARAM)g_hFont, FALSE);
		SendMessageW(hCtrl, BM_SETIMAGE, IMAGE_ICON, (LPARAM)GR.hiLocate);
		iLeft += (DPIS_CXRITBT + DPIS_GAP);
		///////////////////////////
		hCtrl = CreateWindowExW(0, WC_BUTTONW, L"���", WS_CHILD | WS_VISIBLE,
			iLeft, iTop, DPIS_CXRITBT, GC.cyBT,
			hCtrl2, (HMENU)IDC_BT_OPEN, g_hInst, NULL);
		SendMessageW(hCtrl, WM_SETFONT, (WPARAM)g_hFont, FALSE);
		SendMessageW(hCtrl, BM_SETIMAGE, IMAGE_ICON, (LPARAM)GR.hiPlus);
		iLeft += (DPIS_CXRITBT + DPIS_GAP);
		///////////////////////////
		hCtrl = CreateWindowExW(0, WC_BUTTONW, L"��ȡ", WS_CHILD | WS_VISIBLE,
			iLeft, iTop, DPIS_CXRITBT, GC.cyBT,
			hCtrl2, (HMENU)IDC_BT_LOADLIST, g_hInst, NULL);
		SendMessageW(hCtrl, WM_SETFONT, (WPARAM)g_hFont, FALSE);
		SendMessageW(hCtrl, BM_SETIMAGE, IMAGE_ICON, (LPARAM)GR.hiReadFile);
		iLeft += (DPIS_CXRITBT + DPIS_GAP);
		///////////////////////////
		hCtrl = CreateWindowExW(0, WC_BUTTONW, L"����", WS_CHILD | WS_VISIBLE,
			iLeft, iTop, DPIS_CXRITBT, GC.cyBT,
			hCtrl2, (HMENU)IDC_BT_SAVE, g_hInst, NULL);
		SendMessageW(hCtrl, WM_SETFONT, (WPARAM)g_hFont, FALSE);
		SendMessageW(hCtrl, BM_SETIMAGE, IMAGE_ICON, (LPARAM)GR.hiSaveFile);
		iLeft += (DPIS_CXRITBT + DPIS_GAP);
		///////////////////////////
		hCtrl = CreateWindowExW(0, WC_BUTTONW, L"���", WS_CHILD | WS_VISIBLE,
			iLeft, iTop, DPIS_CXRITBT, GC.cyBT,
			hCtrl2, (HMENU)IDC_BT_EMPTY, g_hInst, NULL);
		SendMessageW(hCtrl, WM_SETFONT, (WPARAM)g_hFont, FALSE);
		SendMessageW(hCtrl, BM_SETIMAGE, IMAGE_ICON, (LPARAM)GR.hiCross);
		iLeft += (DPIS_CXRITBT + DPIS_GAP);
		///////////////////////////
		hCtrl = CreateWindowExW(0, WC_BUTTONW, L"����", WS_CHILD | WS_VISIBLE,
			iLeft, iTop, DPIS_CXRITBT, GC.cyBT,
			hCtrl2, (HMENU)IDC_BT_MANAGING, g_hInst, NULL);
		SendMessageW(hCtrl, WM_SETFONT, (WPARAM)g_hFont, FALSE);
		SendMessageW(hCtrl, BM_SETIMAGE, IMAGE_ICON, (LPARAM)GR.hiListManaging);
		///////////////////////////�����б�
		hCtrl = CreateWindowExW(0, WC_LISTVIEWW, NULL, WS_CHILD | WS_VISIBLE | LVS_OWNERDATA | LVS_SHOWSELALWAYS,
			0, 0, 0, 0, hWnd, (HMENU)IDC_LV_PLAY, g_hInst, NULL);
		g_hLV = hCtrl;
		SendMessageW(hCtrl, WM_SETFONT, (WPARAM)g_hFont, FALSE);
		SendMessageW(hCtrl, LVM_SETVIEW, LV_VIEW_DETAILS, 0);
		UINT uExStyle = LVS_EX_FULLROWSELECT | LVS_EX_DOUBLEBUFFER;
		SendMessageW(hCtrl, LVM_SETEXTENDEDLISTVIEWSTYLE, uExStyle, uExStyle);
		LVCOLUMNW lc;
		lc.mask = LVCF_TEXT | LVCF_WIDTH;
		lc.pszText = (PWSTR)L"����";
		lc.cx = DPI(295);
		SendMessageW(hCtrl, LVM_INSERTCOLUMNW, 0, (LPARAM)&lc);
		lc.pszText = (PWSTR)L"ʱ��";
		lc.cx = DPI(50);
		SendMessageW(hCtrl, LVM_INSERTCOLUMNW, 1, (LPARAM)&lc);
		SetWindowTheme(hCtrl, L"Explorer", NULL);
		hLVTheme = OpenThemeData(hCtrl, L"ListView");
		SetPropW(hCtrl, PROP_WNDPROC,
			(HANDLE)SetWindowLongPtrW(hCtrl, GWLP_WNDPROC, (LONG_PTR)WndProc_ListView));
		SendMessageW(hCtrl, LVM_SETIMAGELIST, LVSIL_SMALL, (LPARAM)ImageList_Create(1, DPIS_CYLVITEM, 0, 1, 0));// ILC_COLORȱʡ

        m_pDropTarget = new CDropTarget(OLEDrop_OnEnter, OLEDrop_OnOver, OLEDrop_OnLeave, OLEDrop_OnDrop);
        RegisterDragDrop(hWnd, m_pDropTarget);// ע���Ϸ�Ŀ��
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
			SWP_NOZORDER);//�Ҳ���ť����

		SetWindowPos(g_hLV, NULL,
			iLeft,
			DPIS_CYRITBK,
			cxClient - DPIS_GAP - iLeft,
			cyClient - DPIS_CYRITBK,
			SWP_NOZORDER);//�б�

		cySBTrack = cyClient - DPIS_CYRITBK - 6 - cyVSBArrow * 2;
	}
	return 0;
	case WM_NOTIFY:
	{
		if (((NMHDR*)lParam)->idFrom == IDC_LV_PLAY)
		{
			switch (((NMHDR*)lParam)->code)
			{
			case NM_DBLCLK:// ˫��
			{
				NMITEMACTIVATE* p = (NMITEMACTIVATE*)lParam;
				if (p->iItem != -1)
					Playing_PlayFile(p->iItem);
			}
			break;// ��֪ͨ��ʹ�÷���ֵ
			case NM_CUSTOMDRAW://�Զ������
			{
				NMLVCUSTOMDRAW* p = (NMLVCUSTOMDRAW*)lParam;
				switch (p->nmcd.dwDrawStage)
				{
				case CDDS_PREPAINT:
					return CDRF_NOTIFYITEMDRAW;
				case CDDS_ITEMPREPAINT:
				{
					int iState;
					/*���ھ���LVS_SHOWSELALWAYS��ʽ�������߻��Ƶ��б���ͼ�ؼ����˱�־(ָCDIS_SELECTED)������������
					������Щ�ؼ���������ͨ��ʹ��LVM_GETITEMSTATE�����LVIS_SELECTED��־��ȷ���Ƿ�ѡ������Ŀ(MSDN)*/
					if (SendMessageW(g_hLV, LVM_GETITEMSTATE, p->nmcd.dwItemSpec, LVIS_SELECTED) == LVIS_SELECTED)//ѡ��
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

					if (p->nmcd.dwItemSpec % 2)//������ɫ
					{
						hBrush = CreateSolidBrush(MYCLR_LISTGRAY);
						FillRect(p->nmcd.hdc, &p->nmcd.rc, hBrush);
						DeleteObject(hBrush);
					}
					if (p->nmcd.dwItemSpec == g_iCurrFileIndex)//������в�����
					{
						hBrush = CreateSolidBrush(MYCLR_LISTPLAYING);
						FillRect(p->nmcd.hdc, &p->nmcd.rc, hBrush);
						DeleteObject(hBrush);
					}
					if (iState)//�������
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
			case NM_RCLICK://�Ҽ�����
			{
				int iItem = ((NMITEMACTIVATE*)lParam)->iItem;
				UINT uFlags = (iItem == -1) ? MF_GRAYED : 0,
					uFlags2 = g_bSort || g_iSearchResult != -1 ? MF_GRAYED : uFlags;
				HMENU hMenu = CreatePopupMenu();
				AppendMenuW(hMenu, uFlags, IDMI_TL_PLAY, L"����");
				AppendMenuW(hMenu, uFlags, IDMI_TL_PLAYLATER, L"�Ժ󲥷�");
				SetMenuDefaultItem(hMenu, IDMI_TL_PLAY, FALSE);
				AppendMenuW(hMenu, MF_SEPARATOR, 0, NULL);
				AppendMenuW(hMenu, uFlags, IDMI_TL_OPEN_IN_EXPLORER, L"���ļ�λ��");
				AppendMenuW(hMenu, uFlags2, IDMI_TL_DELETE_FROM_LIST, L"�Ӳ����б���ɾ��");
				AppendMenuW(hMenu, uFlags2, IDMI_TL_DELETE, L"�Ӵ�����ɾ��");
				AppendMenuW(hMenu, MF_SEPARATOR, 0, NULL);
				AppendMenuW(hMenu, uFlags, IDMI_TL_IGNORE, L"����/ȡ�����Դ���Ŀ");
				AppendMenuW(hMenu, uFlags, IDMI_TL_RENAME, L"������");
				AppendMenuW(hMenu, uFlags, IDMI_TL_INFO, L"��ϸ��Ϣ");
				AppendMenuW(hMenu, MF_SEPARATOR, 0, NULL);
				AppendMenuW(hMenu, uFlags, IDMI_TL_LASTBOOKMARK, L"������һ��ǩ");
				AppendMenuW(hMenu, uFlags, IDMI_TL_NEXTBOOKMARK, L"������һ��ǩ");
				AppendMenuW(hMenu, uFlags, IDMI_TL_ADDBOOKMARK, L"�����ǩ");
				AppendMenuW(hMenu, uFlags, IDMI_TL_REMOVEBOOKMARK, L"ɾ����ǩ");
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
							iIndex = i - SendMessageW(g_hLV, LVM_GETCOUNTPERPAGE, 0, 0);// ����ǩ����������
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
							iIndex = i + SendMessageW(g_hLV, LVM_GETCOUNTPERPAGE, 0, 0) - 1;// ����ǩ����������
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
					if (QKMessageBox(L"ȷ��Ҫɾ��ѡ�е��ļ���", L"ɾ���󲻿ɻָ�����ȷ���Ƿ�Ҫ����ɾ��", (HICON)TD_INFORMATION_ICON, L"ѯ��", hWnd, NULL, 2) == QKMSGBOX_BTID_2)
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
					if (QKInputBox(L"������", L"���������ƣ�", &pBuf, hWnd))
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
			return TRUE;// ���ط����Բ�����Ĭ�ϴ����򷵻���������Ĭ�ϴ���
			case LVN_BEGINDRAG:// ��ʼ�Ϸ�
			{
				// �����LV�Ϸ���ͳ������ʽ���ϷŲ�Ӧ����WM_LBUTTONDOWN����
				if (((NMLISTVIEW*)lParam)->iItem != -1)
				{
                    if (g_iSearchResult != -1 || g_bSort)
                    {
                        QKMessageBox(L"���ڲ���������Ŀ", L"����ʽ���������״̬�²����϶�", (HICON)TD_ERROR_ICON, L"����");
                        return 0;
                    }
					if (g_iCurrFileIndex != -1)
						((PLAYERLISTUNIT*)QKArray_Get(g_ItemData, g_iCurrFileIndex))->dwFlags |= QKLIF_DRAGMARK_CURRFILE;
                    if (g_iLaterPlay != -1)
                        ((PLAYERLISTUNIT*)QKArray_Get(g_ItemData, g_iLaterPlay))->dwFlags |= QKLIF_DRAGMARK_PLLATER;
					//////////////////ȡѡ����Ŀ
					QKARRAY Files = QKArray_Create(0);// �ļ�������
					QKARRAY Items = QKArray_Create(0);// ��������

					for (int i = 0; i < g_ItemData->iCount; ++i)
					{
						if (SendMessageW(g_hLV, LVM_GETITEMSTATE, i, LVIS_SELECTED) == LVIS_SELECTED)
						{
							QKArray_Add(&Files, List_GetArrayItem(i)->pszFile);
							QKArray_AddValue(&Items, &i);
						}
					}
                    //////////////////���Ϸ�Դ
					CDataObject* pDataObject;
					CDropSource* pDropSource;
					QKMakeDropSource(Files, OLEDrag_GiveFeedBack, &pDataObject, &pDropSource, TRUE);
					QKArray_Delete(Files);// ����
                    //////////////////���Զ����Ϸ���Ϣ���������������Ա���������Ϸ�
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

                    // (int)        �汾��Ϣ
                    // (DWORD)      ����ID
                    // (int)        ��Ŀ��
                    // (void*)      ��Ŀ����
                    // ...

					int* p = (int*)GlobalLock(sm.hGlobal);
                    *p = QKOLEDRAGVER_1;// �Ϸ���Ϣ�汾��
                    ++p;

                    *(DWORD*)p = GetCurrentProcessId();// ����ID
                    p = (int*)((DWORD*)p + 1);

					*p = Items->iCount;// ��Ŀ��
					++p;

					for (int i = 0; i < Items->iCount; ++i)
					{
						memcpy(p, QKArray_GetValue(Items, i), sizeof(int));
						++p;
					}
					GlobalUnlock(sm.hGlobal);
					pDataObject->SetData(&fe, &sm, TRUE);
                    //////////////////ִ���Ϸ�
					DWORD dwEffect = DROPEFFECT_NONE;
					HRESULT hr = SHDoDragDrop(hWnd, pDataObject, pDropSource, DROPEFFECT_COPY, &dwEffect);
                    //////////////////��������Ϊ���ݶ������У����ݶ����ͷ�ʱ�Ὣ��һ���ͷ�
					delete pDataObject;
					delete pDropSource;
					return 0;
				}
			}
			break;// û�з���ֵ
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
        if (m_iDragDirection == 1)// ����
        {
            iIndex -= 1;
            SendMessageW(g_hLV, LVM_ENSUREVISIBLE, iIndex, TRUE);
        }
        else if (m_iDragDirection == 2)// ����
        {
            iIndex += (SendMessageW(g_hLV, LVM_GETCOUNTPERPAGE, 0, 0) + 1);
            SendMessageW(g_hLV, LVM_ENSUREVISIBLE, iIndex, TRUE);
        }
    }
    return 0;
	}
	return DefWindowProcW(hWnd, message, wParam, lParam);
}
LRESULT CALLBACK WndProc_RitBK(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam)//�����������ڹ���
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
                AppendMenuW(hMenu, uFlags, IDMI_OPEN_FILE, L"���ļ�");
                AppendMenuW(hMenu, uFlags, IDMI_OPEN_FOLDER, L"���ļ���");

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
                    pfod->SetTitle(L"����Ƶ�ļ�");
                    pfod->SetOptions(FOS_ALLOWMULTISELECT | FOS_FORCEFILESYSTEM | FOS_FILEMUSTEXIST);
                    COMDLG_FILTERSPEC cf[] =
                    {
                        {L"��Ƶ�ļ�(*.mp1;*.mp2;*.xm;*.mp3;*.flac;*.wma;*.wav;*.m4a;*.ogg;*.acc;*.ape;*.aiff)",
                        L"*.mp1;*.mp2;*.xm;*.mp3;*.flac;*.wma;*.wav;*.m4a;*.ogg;*.acc;*.ape;*.aiff"},
                        {L"�����ļ�",L"*.*"}
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
                    HANDLE hFind = FindFirstFileW(szFile, &wfd);//��ʼö��
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
                    } while (FindNextFileW(hFind, &wfd));//����ö��
                    if (cItem)
                    {
                        if (iIndex - cItem <= g_iCurrFileIndex && g_iCurrFileIndex != -1)
                            g_iCurrFileIndex += cItem;
                    }

                    FindClose(hFind);//�ͷ�
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

                pCurr += sizeof(LISTFILEHEADER);//�����ļ�ͷ
                List_SetRedraw(FALSE);
                for (int i = 0; i < pListHeader->iCount; ++i)
                {
                    //����Ŀͷ
                    pListItem = (LISTFILEITEM*)pCurr;
                    pCurr += sizeof(LISTFILEITEM);
                    //������
                    t.pszName = (PWSTR)pCurr;
                    pCurr += (lstrlenW(t.pszName) + 1) * sizeof(WCHAR);
                    //���ļ���
                    t.pszFile = (PWSTR)pCurr;
                    pCurr += (lstrlenW(t.pszFile) + 1) * sizeof(WCHAR);
                    if (pListItem->uFlags & QKLIF_BOOKMARK)
                    {
                        memcpy(&t.crBookMark, pCurr, sizeof(COLORREF));// ����ǩ��ɫ
                        pCurr += sizeof(COLORREF);

                        iLenght = lstrlenW((PWSTR)pCurr);
                        if (iLenght)
                            t.pszBookMark = (PWSTR)pCurr;// ����ǩ����
                        else
                            t.pszBookMark = NULL;
                        pCurr += (iLenght + 1) * sizeof(WCHAR);

                        iLenght = lstrlenW((PWSTR)pCurr);
                        if (iLenght)
                            t.pszBookMarkComment = (PWSTR)pCurr;// ����ǩ��ע
                        else
                            t.pszBookMarkComment = NULL;
                        pCurr += (iLenght + 1) * sizeof(WCHAR);
                    }
                    else
                        pCurr += sizeof(COLORREF) + sizeof(WCHAR) * 2;

                    if (pListHeader->dwVer >= QKLFVER_2)// �汾2
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
                    NULL);// ���ļ�
                if (hFile == INVALID_HANDLE_VALUE)
                {
                    delete (DLGRESULT_LIST*)pResult;
                    return 0;
                }

                DWORD cbWritten;
                LISTFILEHEADER ListHeader = { 0 };
                memcpy(&(ListHeader.cHeader), "QKPL", 4);// �ļ�ͷ
                ListHeader.iCount = g_ItemData->iCount;
                ListHeader.dwVer = QKLFVER_2;

                WriteFile(hFile, &ListHeader, sizeof(LISTFILEHEADER), &cbWritten, NULL);//д�ļ�ͷ

                int iIndex = 0;

                LISTFILEITEM ListItem = { 0 };
                PLAYERLISTUNIT* p;
                WCHAR cNULL = 0;
                for (int i = 0; i < ListHeader.iCount; ++i)
                {
                    p = (PLAYERLISTUNIT*)QKArray_Get(g_ItemData, i);
                    ListItem.uFlags = p->dwFlags | (p->pszTime ? QKLIF_TIME : 0);
                    WriteFile(hFile, &ListItem, sizeof(LISTFILEITEM), &cbWritten, NULL);// ��Ŀͷ
                    WriteFile(hFile, p->pszName, (lstrlenW(p->pszName) + 1) * sizeof(WCHAR), &cbWritten, NULL);// ����
                    WriteFile(hFile, p->pszFile, (lstrlenW(p->pszFile) + 1) * sizeof(WCHAR), &cbWritten, NULL);// �ļ���
                    WriteFile(hFile, &p->crBookMark, sizeof(COLORREF), &cbWritten, NULL);// ��ǩ��ɫ
                    if (p->pszBookMark)
                        WriteFile(hFile, p->pszBookMark, (lstrlenW(p->pszBookMark) + 1) * sizeof(WCHAR), &cbWritten, NULL);// ��ǩ����
                    else
                        WriteFile(hFile, &cNULL, sizeof(WCHAR), &cbWritten, NULL);// û����дNULL
                    if (p->pszBookMarkComment)
                        WriteFile(hFile, p->pszBookMarkComment, (lstrlenW(p->pszBookMarkComment) + 1) * sizeof(WCHAR), &cbWritten, NULL);// ��ǩ��ע
                    else
                        WriteFile(hFile, &cNULL, sizeof(WCHAR), &cbWritten, NULL);// û����дNULL
                    if (p->pszTime)
                        WriteFile(hFile, p->pszTime, (lstrlenW(p->pszTime) + 1) * sizeof(WCHAR), &cbWritten, NULL);// ��ǩ��ע
                    else
                        WriteFile(hFile, &cNULL, sizeof(WCHAR), &cbWritten, NULL);// û����дNULL
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
				if (QKMessageBox(L"ȷ��Ҫ��ղ����б���", NULL, (HICON)TD_WARNING_ICON, L"ѯ��", g_hBKList, NULL, 2) == QKMSGBOX_BTID_1)
                {
                    Playing_Stop();
                    List_Delete(-1, TRUE);
                    SetWindowTextW(GetDlgItem(g_hBKRight, IDC_ST_LISTNAME), L"/*��ǰ�޲����б�*/");
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
                if (!iLength)// û���ı�
                {
                    if (g_iSearchResult != -1)// �����в�����LV����ת��
                    {
                        ///////��ԭ���в����������Ժ󲥷�����
                        if (g_iCurrFileIndex < g_iSearchResult && g_iCurrFileIndex != -1)// �������ڽ����֮�ڣ�˵������ʱ����������ת��������Ҫת������
                            g_iCurrFileIndex = List_GetArrayItemIndex(g_iCurrFileIndex);
                        if (iTempCurr != -1)
                            g_iCurrFileIndex = iTempCurr;

                        // ת���Ժ󲥷ţ�������ͬ��
                        if (g_iLaterPlay < g_iSearchResult && g_iLaterPlay != -1)
                            g_iLaterPlay = List_GetArrayItemIndex(g_iLaterPlay);
                        if (iTempLater != -1)
                            g_iLaterPlay = iTempLater;
                        ///////��ԭλ��
                        g_iSearchResult = -1;
                        List_ResetLV();// ��Ŀ���ˣ�����List_Redraw����List_ResetLV
                        SendMessageW(g_hLV, LVM_ENSUREVISIBLE, iTopIndex + iVisibleItemCount - 1, TRUE);// �ص�ԭ����λ�ã���˵΢��Ͳ��ܸ������LV������λ�õ���Ϣ����
                        UI_RedrawBookMarkPos();
                    }
                }
                else
                {
                    iTopIndex = SendMessageW(g_hLV, LVM_GETTOPINDEX, 0, 0);
                    iVisibleItemCount = SendMessageW(g_hLV, LVM_GETCOUNTPERPAGE, 0, 0);

                    pszEdit = new WCHAR[iLength + 1];
                    GetWindowTextW(GetDlgItem(g_hBKRight, IDC_ED_SEARCH), pszEdit, iLength + 1);// ȡ�༭���ı�
                    g_iSearchResult = 0;// ���������0
                    int iIndex;
                    BOOL b1 = (g_iCurrFileIndex != -1);// �Ƿ�Ҫ�ȶԲ�ת����������
                    BOOL b2 = (g_iLaterPlay != -1);
                    BOOL bTransed1 = FALSE, bTransed2 = FALSE;
                    for (int i = 0; i < g_ItemData->iCount; ++i)
                    {
                        iIndex = ((PLAYERLISTUNIT*)QKArray_Get(g_ItemData, i))->iMappingIndexSort;
                        if (iIndex == -1)
                            iIndex = i;
                        // �����Ǵ�������ӳ��
                        if (QKStrInStr(((PLAYERLISTUNIT*)QKArray_Get(g_ItemData, iIndex))->pszName, pszEdit))// �ȶ��ı�
                        {
                            ((PLAYERLISTUNIT*)QKArray_Get(g_ItemData, g_iSearchResult))->iMappingIndexSearch = iIndex;// ӳ�䣬���Ҫ���б�Ŀ�ͷ��ʼӳ�䣬˳�����£�ʹ��g_iSearchResult������һ��
                            if (b1)
                            {
                                if (i == g_iCurrFileIndex)
                                {
                                    g_iCurrFileIndex = g_iSearchResult;// ת�����в�������
                                    b1 = FALSE;
                                    bTransed1 = TRUE;
                                }
                            }
                            if (b2)
                            {
                                if (i == g_iLaterPlay)
                                {
                                    g_iLaterPlay = g_iSearchResult;// ת���Ժ󲥷�����
                                    b2 = FALSE;
                                    bTransed2 = TRUE;
                                }
                            }
                            ++g_iSearchResult;// ���������
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
                static BOOL bAscending = TRUE;// ���򣺴�С���󣻽��򣺴Ӵ�С
                HMENU hMenu = CreatePopupMenu();
				UINT uFlags = ((g_iSearchResult != -1) || !g_ItemData->iCount) ? MF_GRAYED : 0;
                AppendMenuW(hMenu, g_bSort ? uFlags : MF_GRAYED, IDMI_LM_SORT_DEF, L"Ĭ������");
                AppendMenuW(hMenu, uFlags, IDMI_LM_SORT_FILENAME, L"���ļ�������");
                AppendMenuW(hMenu, uFlags, IDMI_LM_SORT_NAME, L"����������");
                AppendMenuW(hMenu, uFlags, IDMI_LM_SORT_CTIME, L"������ʱ������");
                AppendMenuW(hMenu, uFlags, IDMI_LM_SORT_MTIME, L"���޸�ʱ������");
                AppendMenuW(hMenu, uFlags, IDMI_LM_SORT_REVERSE, L"��������");
                AppendMenuW(hMenu, MF_SEPARATOR, 0, NULL);
                AppendMenuW(hMenu, 0, IDMI_LM_SORT_ASCENDING, L"����");
                AppendMenuW(hMenu, 0, IDMI_LM_SORT_DESCENDING, L"����");
                CheckMenuRadioItem(hMenu, IDMI_LM_SORT_ASCENDING, IDMI_LM_SORT_DESCENDING,
                    GS.bAscending ? IDMI_LM_SORT_ASCENDING : IDMI_LM_SORT_DESCENDING, MF_BYCOMMAND);// ����ѡ��

                AppendMenuW(hMenu, GS.bNoBookMarkWhenSort ? MF_CHECKED : 0, IDMI_LM_NOBOOKMARK, L"����ʱ����ʾ��ǩ���");
                AppendMenuW(hMenu, g_bSort ? uFlags : MF_GRAYED, IDMI_LM_FIXSORT, L"�̶�ΪĬ������");
                AppendMenuW(hMenu, MF_SEPARATOR, 0, NULL);
                AppendMenuW(hMenu, uFlags, IDMI_LM_BOOKMARK, L"��ǩ...");
                AppendMenuW(hMenu, 0, IDMI_LM_DETAIL, L"��ϸ��Ϣ...");
                AppendMenuW(hMenu, 0, IDMI_LM_SETLVDEFWIDTH, L"���б����Ĭ�Ͽ��");
                AppendMenuW(hMenu, 0, IDMI_LM_UPDATETIME, L"ǿ�Ƹ����б�ʱ����Ϣ");
                RECT rc;
                GetWindowRect((HWND)lParam, &rc);
                int iRet = TrackPopupMenu(hMenu, TPM_RETURNCMD, rc.left, rc.bottom, 0, hWnd, NULL);
                DestroyMenu(hMenu);
                switch (iRet)
                {
                case IDMI_LM_SORT_DEF:// ����Ĭ������
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
                case IDMI_LM_SORT_FILENAME:// ���ļ�������
                {
                    g_bSort = TRUE;
                    int iCount = g_ItemData->iCount;
                    PLAYERLISTUNIT* p1, * p2;
                    int m, n;
                    int iResult = GS.bAscending ? 1 : -1;
                    // StrCmpLogicalW���ַ�����ͬ�����㣻psz1����psz2����1��psz1С��psz2����-1
                    // ð��.........
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
                case IDMI_LM_SORT_NAME:// ���б���������
                {
                    g_bSort = TRUE;
                    int iCount = g_ItemData->iCount;
                    PLAYERLISTUNIT* p1, * p2;
                    int m, n;
                    int iResult = GS.bAscending ? 1 : -1;
                    // StrCmpLogicalW���ַ�����ͬ�����㣻psz1����psz2����1��psz1С��psz2����-1
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
                case IDMI_LM_SORT_CTIME:// ������ʱ������
                case IDMI_LM_SORT_MTIME:// ���޸�ʱ������
                {
                    g_bSort = TRUE;
                    int iCount = g_ItemData->iCount;
                    PLAYERLISTUNIT* p1, * p2;
                    int m, n;
                    int iResult = GS.bAscending ? 1 : -1;
                    // CompareFileTime����һ���ļ�ʱ�����ڵڶ�������-1����һ�����ڵڶ�������0����һ�����ڵڶ�������1
                    WIN32_FIND_DATAW wfd;
                    HANDLE hFind;
                    FILETIME* ft = new FILETIME[iCount];
                    if (iRet == IDMI_LM_SORT_CTIME)
                    {
                        for (int i = 0; i < iCount; ++i)
                        {
                            hFind = FindFirstFileW(((PLAYERLISTUNIT*)QKArray_Get(g_ItemData, i))->pszFile, &wfd);// ��������CreateFile��GetFileTime��
                            memcpy(ft + i, &wfd.ftCreationTime, sizeof(FILETIME));
                            FindClose(hFind);
                        }
                    }
                    else
                    {
                        for (int i = 0; i < iCount; ++i)
                        {
                            hFind = FindFirstFileW(((PLAYERLISTUNIT*)QKArray_Get(g_ItemData, i))->pszFile, &wfd);// ��������CreateFile��GetFileTime��
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
                case IDMI_LM_SORT_ASCENDING:// ����
                    GS.bAscending = TRUE;
                    return 0;
                case IDMI_LM_SORT_DESCENDING:// ����
                    GS.bAscending = FALSE;
                    return 0;
                case IDMI_LM_FIXSORT:// �̶�ΪĬ������
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
                case IDMI_LM_DETAIL:// �鿴�б���ϸ��Ϣ
                {

                }
                return 0;
                case IDMI_LM_BOOKMARK:// ��ǩ����
                    DialogBoxParamW(g_hInst, MAKEINTRESOURCEW(IDD_BOOKMARK), g_hBKList, DlgProc_BookMark, 0);
                    return 0;
                case IDMI_LM_SORT_REVERSE:// ��������
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
                case IDMI_LM_SETLVDEFWIDTH:// ��Ĭ���б��
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
    case WM_CTLCOLORSTATIC:// ��̬��ɫ
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
        if (wParam == VK_RETURN)//���س���
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
    case WM_LBUTTONDOWN:// ��������û�취�ˣ�����ôд�ӱ�Ŀؼ��������ʱ��LV�����ý��㣬WM_KEYDOWNҲ�Ͳ��ᷢ��LV......
        SetFocus(hWnd);
        break;
    case WM_KEYDOWN:
    {
        if (wParam == VK_RETURN)// ���س���������Ŀ
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
        else if (wParam == 0x41)// A������
        {
            if (GetKeyState(VK_CONTROL) & 0x80000000)// Ctrl + A ȫѡ
            {
                LVITEMW li;
                li.stateMask = LVIS_SELECTED;
                li.state = LVIS_SELECTED;
                SendMessageW(hWnd, LVM_SETITEMSTATE, -1, (LPARAM)&li);// wParam = -1��Ӧ����������Ŀ
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
        // ������ʾ�ı�
        if (lParam == DLGTYPE_SAVELIST)
        {
            SetDlgItemTextW(hDlg, IDC_ST_TIP, L"ѡ��һ���ļ����ǣ����½�һ�����ļ�");
            SetWindowTextW(hDlg, L"��������б�");
        }
        else if (lParam == DLGTYPE_LOADLIST)
        {
            SetDlgItemTextW(hDlg, IDC_ST_TIP, L"ѡ��һ���ļ�����");
            SetWindowTextW(hDlg, L"��������б�");
        }
        else
            EndDialog(hDlg, NULL);
        dwDlgType = lParam;
        HWND hLV = GetDlgItem(hDlg, IDC_LV_LISTFILE);
        // ������
        LVCOLUMNW lc = { 0 };
        WCHAR szTitle[5];
        lc.mask = LVCF_TEXT | LVCF_WIDTH;

        lstrcpyW(szTitle, L"�б��ļ�");
        lc.pszText = szTitle;
        lc.cx = DPI(300);
        SendMessageW(hLV, LVM_INSERTCOLUMNW, 0, (LPARAM)&lc);

        lstrcpyW(szTitle, L"�޸�ʱ��");
        lc.pszText = szTitle;
        lc.cx = DPI(150);
        SendMessageW(hLV, LVM_INSERTCOLUMNW, 1, (LPARAM)&lc);
        // ���÷��
        SetWindowLongPtrW(hLV, GWL_STYLE, GetWindowLongW(hLV, GWL_STYLE) | LVS_SINGLESEL);// ��һѡ��
        DWORD dwStyle = LVS_EX_FULLROWSELECT | LVS_EX_DOUBLEBUFFER;
        SendMessageW(hLV, LVM_SETEXTENDEDLISTVIEWSTYLE, dwStyle, dwStyle);// ����ѡ��˫����
        SetWindowTheme(hLV, L"Explorer", NULL);// ���ӷ��
        // ö���ļ�
        WIN32_FIND_DATAW wfd;
        WCHAR szListFile[MAX_PATH];
        lstrcpyW(szListFile, g_pszListDir);
        lstrcatW(szListFile, L"*.QKList");// ת��QKList��չ��
        HANDLE hFind = FindFirstFileW(szListFile, &wfd);// ��ʼö��
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
            iIndex = SendMessageW(hLV, LVM_INSERTITEMW, 0, (LPARAM)&li);// �������

            FileTimeToLocalFileTime(&wfd.ftLastWriteTime, &ft);
            FileTimeToSystemTime(&ft, &st);

            wsprintfW(szTime, L"%d-%02d-%02d %02d:%02d:%02d",
                st.wYear, st.wMonth, st.wDay, st.wHour, st.wMinute, st.wSecond);

            li.iSubItem = 1;
            li.pszText = szTime;
            SendMessageW(hLV, LVM_SETITEMTEXTW, iIndex, (LPARAM)&li);// �ñ���
        } while (FindNextFileW(hFind, &wfd));// ����ö��
        FindClose(hFind);// �ͷ�
    }
    return FALSE;// ����TRUE���趨����WS_TABSTOP���ĵ�һ���ؼ���wParamΪ���ھ����������FALSE���趨����
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
					QKMessageBox(L"�б��ļ���Ч", L"ѡ�����б��ļ�������", (HICON)TD_ERROR_ICON, L"����", hDlg);
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
    return FALSE;// ����һ����Ϣʱ�᷵��TRUE��������һ����Ϣʱ����FALSE
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
        ///////////////���÷��
        DWORD dwStyle = LVS_EX_FULLROWSELECT | LVS_EX_DOUBLEBUFFER;
        SendMessageW(hLV, LVM_SETEXTENDEDLISTVIEWSTYLE, dwStyle, dwStyle);
        SetWindowTheme(hLV, L"Explorer", NULL);
        ///////////////������
        LVCOLUMNW lc;
        lc.mask = LVCF_TEXT | LVCF_WIDTH;
        lc.pszText = (PWSTR)L"����";
        lc.cx = DPI(34);
        SendMessageW(hLV, LVM_INSERTCOLUMNW, 0, (LPARAM)&lc);

        lc.pszText = (PWSTR)L"����";
        lc.cx = DPI(110);
        SendMessageW(hLV, LVM_INSERTCOLUMNW, 1, (LPARAM)&lc);

        lc.pszText = (PWSTR)L"��ɫ";
        lc.cx = DPI(70);
        SendMessageW(hLV, LVM_INSERTCOLUMNW, 2, (LPARAM)&lc);

        lc.pszText = (PWSTR)L"��ע";
        lc.cx = DPI(170);
        SendMessageW(hLV, LVM_INSERTCOLUMNW, 3, (LPARAM)&lc);
        LVITEMW li;
        ///////////////�������
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
                SendMessageW(hLV, LVM_INSERTITEMW, 0, (LPARAM)&li);// ����

                li.iSubItem = 1;
                li.pszText = p->pszBookMark;
                SendMessageW(hLV, LVM_SETITEMTEXTW, j, (LPARAM)&li);// ����
                li.iSubItem = 2;
                wsprintfW(szBuf, L"0x%06X", QKGDIClrToCommonClr(p->crBookMark));// ����ڸ�ʽ���ı����#���0x���xһ���д��������̫�ÿ�...
                li.pszText = szBuf;
                SendMessageW(hLV, LVM_SETITEMTEXTW, j, (LPARAM)&li);// ��ɫ
                li.iSubItem = 3;
                li.pszText = p->pszBookMarkComment;
                SendMessageW(hLV, LVM_SETITEMTEXTW, j, (LPARAM)&li);// ��ע
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
            ///////////////���������Ϣ
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
            ///////////////ȡ��ѡ����
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
            ///////////////����
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
            ///////////////��ע
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
            ///////////////��ɫ
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
        case IDC_BT_BMJUMP:// ��ת�Ҳ��رնԻ���
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
            if (HIWORD(wParam) == STN_DBLCLK)// Ҫ��SS_NOTIFY��ʽ��˫����̬����ѡ����ɫ
            {
                CHOOSECOLORW cc = { sizeof(CHOOSECOLORW) };
                cc.hwndOwner = hDlg;
                cc.lpCustColors = (COLORREF*)GetPropW(g_hBKList, PROP_BOOKMARKCLRDLGBUF);
                cc.Flags = CC_FULLOPEN;
                if (ChooseColorW(&cc))
                {
                    if (hbrStatic)
                        DeleteObject(hbrStatic);
                    hbrStatic = CreateSolidBrush(cc.rgbResult);// ���¾�̬��ˢ
                    InvalidateRect(hStatic, NULL, TRUE);
                    WCHAR szBuf[20];
                    wsprintfW(szBuf, L"0x%06X", QKGDIClrToCommonClr(cc.rgbResult));
                    SetWindowTextW(GetDlgItem(hDlg, IDC_ED_BMCLR), szBuf);// ����༭��
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
            case NM_DBLCLK:// ��ת�ҹرնԻ���
                DlgProc_BookMark(hDlg, WM_COMMAND, MAKELONG(IDC_BT_BMJUMP, 0), 0);
                SetFocus(g_hLV);
                EndDialog(hDlg, 0);
                return TRUE;
            }
        }
    }
    break;
    case WM_CTLCOLORSTATIC:// ��̬��ɫ
        if (lParam == (LPARAM)hStatic)
            return (INT_PTR)hbrStatic;// û�л�ˢ��ʱ��ˢ���ΪNULL����ʱ���൱�ڷ���FALSE��Ҳ����ʹ��Ĭ�ϴ���
        else
            break;
    }
    return FALSE;
}
HRESULT CALLBACK OLEDrag_GiveFeedBack(DWORD dwEffect)
{
    return DRAGDROP_S_USEDEFAULTCURSORS;// ʹ��Ĭ�Ϲ��
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

    if (SUCCEEDED(pDataObj->GetData(&fe, &sm)))// �ж���û���Զ����Ϸ���Ϣ
    {
        int* p = (int*)GlobalLock(sm.hGlobal);
        if (!p)
            return 0;

        int iVer = *p;// ���汾��
        if (iVer == QKOLEDRAGVER_1)
        {
            ++p;
            DWORD dwProcID = *(DWORD*)p;// ������ID
            if (dwProcID != GetCurrentProcessId())// �ж��ǲ����Լ����Լ��Ϸ�
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
            SendMessageW(g_hLV, LVM_SETITEMSTATE, -1, (LPARAM)&li);// ���ѡ�У�wParam = -1��Ӧ����������Ŀ
            li.state = LVIS_SELECTED;

            iTargetIndex -= n;
            for (int i = 0; i < iCount; ++i)
            {
                SendMessageW(g_hLV, LVM_SETITEMSTATE, iTargetIndex + i, (LPARAM)&li);// ѡ�����ƶ�����
            }

            PLAYERLISTUNIT* pi;
            BOOL b[2] = { 0 };
            for (int i = 0; i < g_ItemData->iCount; ++i)// ��������������ǵ��������Ӳ������ˣ�ֱ���ø��򵥴ֱ��ķ�����ԭ����
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
    else// ���Ϸ��ļ�
    {
    CommonDragFile:// ����ID���������ʱ������
        fe.cfFormat = CF_HDROP;
        if (FAILED(pDataObj->GetData(&fe, &sm)))// ȡ��HDROP
            return S_OK;
        HDROP hDrop = (HDROP)sm.hGlobal;
        int iFileCount = DragQueryFileW(hDrop, 0xFFFFFFFF, NULL, 0);// ȡ�Ϸ��ļ�������(int)0xFFFFFFFF == -1
        UINT iBufferSize;
        LPWSTR pszFile;
        for (int i = 0; i < iFileCount; i++)
        {
            iBufferSize = DragQueryFileW(hDrop, i, NULL, 0);// ȡ��������С
            pszFile = new WCHAR[iBufferSize + 1];
            DragQueryFileW(hDrop, i, pszFile, iBufferSize + 1);// ��ѯ�ļ���
            List_Add(pszFile, NULL, iTargetIndex, FALSE);
            delete[] pszFile;
            ++iTargetIndex;
        }
        List_ResetLV();
        ReleaseStgMedium(&sm);
    }
    return S_OK;
}