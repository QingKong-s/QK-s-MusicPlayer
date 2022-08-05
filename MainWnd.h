#pragma once
#include <Windows.h>
#include "MyProject.h"

struct LRCDATA
{
	float fTime;		// ��Ӧʱ��
	float fDelay;		// ��ʱ����һ���ʵ���һ���ʵ�ʱ��
	PWSTR pszLrc;		// ���
	int iOrgLength;		// ԭ�ĵ��ַ���
	int cy;				// �������в��ԣ�����ܸ߶�
	int iLastTop;		// ���ڹ�����ʻ��ƣ��ϴλ���ʱ�Ķ���
	int iDrawID;
};

struct LRCHSCROLLINFO
{
	int iIndex;					// ��ǰ��
	int cx1;					// ���1
	int cx2;					// ���2
	int x1;						// ��������1
	int x2;						// ��������2
	float fNoScrollingTime1;	// ͣ��ʱ��1
	float fNoScrollingTime2;	// ͣ��ʱ��2
	BOOL bWndSizeChangedFlag;	// �����ѱ��ı��־
	BOOL bShowCurr;				// ָʾ������Ŀ�Ƿ�ɼ�
};

struct LRCVSCROLLINFO
{
	int iSrcTop;				// ��ʼ����
	int iDestTop;				// Ŀ�궥��
	int iDistance;				// ����
	BOOL bDirection;			// TRUE ����
	float fDelay;				// ���������ӳ�
	float fTime;				// 
};

const PCWSTR c_szBtmTip[15] =
{
	L"��һ��",
	L"����/��ͣ",
	L"ֹͣ",
	L"��һ��",
	L"���",
	L"ѭ����ʽ",
	L"��������",
	L"��ʾ/���ز����б�",
	L"���ڷ����ڲ���",
	L"����",
	L"ѭ����ʽ������ѭ��",
	L"ѭ����ʽ���������",
	L"ѭ����ʽ����������",
	L"ѭ����ʽ������ѭ��",
	L"ѭ����ʽ�����岥��"
};

#define IDC_BK_LEFT					800
#define IDC_BK_PIC					801
#define IDC_BK_SPE					802
#define IDC_BK_BOTTOMBTBK			803
#define IDC_BK_RIGHT				804
#define IDC_BK_SEARCH				805
#define IDC_BK_RIGHTBTBK			806
#define IDC_BK_TLSB					807
#define IDC_BK_WAVES				808
#define IDC_BK_LIST					809

#define IDC_ST_WAVES				302

#define IDC_ST_TOP					305
#define IDC_ST_LRC					306
#define IDC_ST_BTBACK				307
#define IDC_ST_BOTTOM				308
#define IDC_ST_BOTTOMCTRLBK			309
#define IDC_ST_LISTNAME				315

#define IDC_LV_PLAY					401
#define IDC_TL_PLAY					402

#define IDC_TB_PGS					501
#define IDC_TB_LRCSCROLL			502

#define IDC_BT_LAST					602
#define IDC_BT_PLAY					603
#define IDC_BT_STOP					604
#define IDC_BT_NEXT					605
#define IDC_BT_LRC					606
#define IDC_BT_JUMP					607
#define IDC_BT_OPEN					608
#define IDC_BT_LOADLIST				609
#define IDC_BT_SAVE					610
#define IDC_BT_EMPTY				611
#define IDTBB_LAST					612
#define IDTBB_PLAY					613
#define IDTBB_NEXT					614
#define IDC_BT_MORE					615
#define IDC_BT_SEARCH				616
#define IDC_BT_MANAGING				617

#define IDC_CB_REPEATMODE			700

#define IDC_ED_SEARCH				800

#define IDC_SEB						900

#define IDMI_NULL					99

#define IDMI_OPEN_FILE				101
#define IDMI_OPEN_FOLDER			102

#define IDMI_TL_PLAY				110
#define IDMI_TL_DELETE_FROM_LIST	111
#define IDMI_TL_DELETE				112
#define IDMI_TL_OPEN_IN_EXPLORER	113
#define IDMI_TL_RENAME				114
#define IDMI_TL_INFO				115
#define IDMI_TL_ADDBOOKMARK			116
#define IDMI_TL_REMOVEBOOKMARK		117
#define IDMI_TL_NEXTBOOKMARK		118
#define IDMI_TL_LASTBOOKMARK		119
#define IDMI_TL_IGNORE				120
#define IDMI_TL_PLAYLATER			121

#define IDMI_LRC_SHOW				120
#define IDMI_LRC_LRCSHOW			121
#define IDMI_LRC_FORCETWOLINES		122

#define IDMI_MORE_ABOUT				130
#define IDMI_MORE_SETTING			131

#define IDMI_LS_PLAY				140
#define IDMI_LS_COPY				141

#define IDMI_LM_SORT_DEF			150
#define IDMI_LM_SORT_FILENAME		151
#define IDMI_LM_SORT_NAME			152
#define IDMI_LM_SORT_CTIME			153
#define IDMI_LM_SORT_MTIME			154
#define IDMI_LM_SORT_ASCENDING		155
#define IDMI_LM_SORT_DESCENDING		156
#define IDMI_LM_FIXSORT				157
#define IDMI_LM_DETAIL				158
#define IDMI_LM_BOOKMARK			159
#define IDMI_LM_SORT_REVERSE		160
#define IDMI_LM_SETLVDEFWIDTH		161
#define IDMI_LM_UPDATETIME			162
#define IDMI_LM_NOBOOKMARK			163

#define IDMI_PL_SEPARATE			170
#define IDMI_PL_SHOW				171



#define SPESTEP_BAR				7
#define SPESTEP_MAX				11

#define LEFTBKM_SETMAX			WM_USER + 1// (Max, 0)
#define LEFTBKM_REDRAWSB		WM_USER + 2// (�Ƿ��������, 0)

#define BTMBKM_INIT				WM_USER + 1
#define BTMBKM_GETREPEATMODE	WM_USER + 2
#define BTMBKM_SETPLAYBTICON	WM_USER + 3
#define BTMBKM_DOBTOPE			WM_USER + 4

#define REPEATMODE_TOTALLOOP	0// ����ѭ��
#define REPEATMODE_RADOM		1// �������
#define REPEATMODE_SINGLE		2// ��������
#define REPEATMODE_SINGLELOOP	3// ����ѭ��
#define REPEATMODE_TOTAL		4// ���岥��

#define BTMBT_NORMAL			0
#define BTMBT_HOT				1
#define BTMBT_PUSHED			2

void MainWnd_ReleaseCurrInfo();
void CALLBACK SyncProc_End(HSYNC handle, DWORD channel, DWORD data, void* user);
int Lrc_DrawItem(int iIndex, int y, BOOL bTop, BOOL bClearBK, BOOL bImmdShow);
void Playing_PlayFile(int iIndex);
void Playing_Stop(BOOL bNoGap = FALSE);
void Playing_PlayNext(BOOL bReverse = FALSE);
void Playing_AutoNext();
void StopThread_Waves();
DWORD WINAPI Thread_GetWavesData(void* p);
void UI_UpdateLeftBK();
void UI_SeparateListWnd(BOOL b);
void UI_ShowList(BOOL b);
LRESULT CALLBACK WndProc_TBGhost(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam);
LRESULT CALLBACK WndProc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam);
LRESULT CALLBACK WndProc_BtmBK(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam);
LRESULT CALLBACK WndProc_LeftBK(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam);
INT_PTR CALLBACK DlgProc_About(HWND hDlg, UINT message, WPARAM wParam, LPARAM lParam);
void CALLBACK TimerProc(HWND hWnd, UINT uMsg, UINT_PTR idEvent, DWORD dwTime);
LRESULT CALLBACK QKCProc_SEB(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam);
LRESULT CALLBACK QKCProc_TBPaint(HWND hWnd, UINT iMsg, WPARAM wParam, LPARAM lParam);
void GDIObj_LeftBK(DWORD dwOpe = GDIOBJOPE_REFRESH);
int HitTest_LrcShow(POINT pt);
int HitTest_BtmBK(int x, int y);