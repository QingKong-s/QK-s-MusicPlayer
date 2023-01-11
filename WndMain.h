#pragma once
#include <Windows.h>
#include <dwrite.h>

#include "MyProject.h"

//*****************************�ṹ*****************************//
struct LRCDATA			// ��ǰ�����ĸ����Ϣ����g_Lrc�д洢
{
	float fTime;		// ��Ӧʱ��
	float fDelay;		// ��ʱ����һ���ʵ���һ���ʵ�ʱ��
	PWSTR pszLrc;		// ���
	int iOrgLength;		// ԭ�ĵ��ַ��������ں������
	int cy;				// ����ܸ߶ȣ��������в���
	int iLastTop;		// �ϴλ���ʱ��ʾ��εĶ��ߣ����ڹ�����ʻ���
	int iDrawID;		// ����ID������������Ŀ�Ƿ���ʾ
	RECT rcItem;		// ��ʾ��Σ��������в���
};

struct LRCHSCROLLINFO				// ��ʺ��������Ϣ����Ա�к������1/2�ı�ʾΪ��һ��͵ڶ��� 
{
	int iIndex;						// ��ǰ��
	int cx1;						// ���1
	int cx2;						// ���2
	int x1;							// ��������1
	int x2;							// ��������2
	float fNoScrollingTime1;		// ͣ��ʱ��1
	float fNoScrollingTime2;		// ͣ��ʱ��2
	BOOL bWndSizeChangedFlag;		// �����ѱ��ı��־
	BOOL bShowCurr;					// ָʾ������Ŀ�Ƿ�ɼ�
	DWRITE_TEXT_METRICS Metrics1;	// �ϴβ�߽��1
	DWRITE_TEXT_METRICS Metrics2;	// �ϴβ�߽��2
};

struct LRCVSCROLLINFO	// ��ʴ�ֱ������Ϣ
{
	int iSrcTop;		// ��ʼ����
	int iDestTop;		// Ŀ�궥��
	int iDistance;		// ����
	BOOL bDirection;	// ����TRUE=���£�FALSE=����
	float fDelay;		// ���������ӳ�
	float fTime;		// ������ʼʱ�Ĳ��Ž���
};

//*****************************����*****************************//

//************������������ʾ�ַ���
const PCWSTR c_szBtmTip[] =
{
	L"��һ��",
	L"����/��ͣ",
	L"ֹͣ",
	L"��һ��",
	L"���",
	L"ѭ����ʽ",
	L"��������",
	L"��ʾ/���ز����б�",
	L"����",
	L"����",
	L"ѭ����ʽ������ѭ��",
	L"ѭ����ʽ���������",
	L"ѭ����ʽ����������",
	L"ѭ����ʽ������ѭ��",
	L"ѭ����ʽ�����岥��"
};
//************�ؼ�ID
#define IDC_BK_LEFT					100// �󱳾�
#define IDC_BK_RIGHTBTBK			101// �Ұ�ť����
#define IDC_BK_LIST					102// ���б�����

#define IDC_ST_LISTNAME				110// �б����ƾ�̬

#define IDC_LV_PLAY					120// �б�ListView

#define IDC_BT_JUMP					200
#define IDC_BT_OPEN					201
#define IDC_BT_LOADLIST				202
#define IDC_BT_SAVELIST				203
#define IDC_BT_EMPTY				204
#define IDC_BT_MORE					205
#define IDC_BT_SEARCH				206
#define IDC_BT_MANAGING				207

#define IDC_ED_SEARCH				130

#define IDC_SEB						140// �ָ���
//************��������������ť
#define IDTBB_LAST					1000
#define IDTBB_PLAY					1001
#define IDTBB_NEXT					1002
//************�˵���ID
//****** "���"
#define IDMI_OPEN_FILE				101
#define IDMI_OPEN_FOLDER			102
//****** "����"
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
//******��������ʰ�ť
#define IDMI_LRC_SHOW				120
#define IDMI_LRC_LRCSHOW			121
#define IDMI_LRC_FORCETWOLINES		122
//******��������Ҽ��˵�
#define IDMI_LS_PLAY				140
#define IDMI_LS_COPY				141
//****** "����"
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
//******�������б�ť
#define IDMI_PL_SEPARATE			170
#define IDMI_PL_SHOW				171
//************����

//******Ƶ�׻�ͼ
#define SPESTEP_BAR					7
#define SPESTEP_MAX					11
//******�󱳾���Ϣ
#define LEFTBKM_LRCSHOW_SETMAX			WM_USER + 1// (Max, 0)
#define LEFTBKM_LRCSHOW_REDRAWSB		WM_USER + 2// (bImmdShow, bIndependlyDrawing)
#define LEFTBKM_TOOLBAR_REDRAW			WM_USER + 3// (bImmdShow, bIndependlyDrawing)
#define LEFTBKM_TOOLBAR_INIT			WM_USER + 4// (0, 0)
#define LEFTBKM_TOOLBAR_GETREPEATMODE	WM_USER + 5// (0, 0)
#define LEFTBKM_TOOLBAR_DOBTOPE			WM_USER + 6// (0, 0)
#define LEFTBKM_PROGBAR_REDRAW			WM_USER + 7// (bImmdShow, bIndependlyDrawing)
#define LEFTBKM_PROGBAR_SETPOS			WM_USER + 8// (uPos, bRedraw)
#define LEFTBKM_PROGBAR_GETPOS			WM_USER + 9// (0, 0)
#define LEFTBKM_PROGBAR_SETMAX			WM_USER + 10// (uMax, bRedraw)
#define LEFTBKM_PROGBAR_GETMAX			WM_USER + 11// (0, 0)
//******��������Ϣ
#define MAINWNDM_AUTONEXT				WM_USER + 1// (0, 0)
//******ѭ��ģʽ
#define REPEATMODE_TOTALLOOP	0// ����ѭ��
#define REPEATMODE_RADOM		1// �������
#define REPEATMODE_SINGLE		2// ��������
#define REPEATMODE_SINGLELOOP	3// ����ѭ��
#define REPEATMODE_TOTAL		4// ���岥��
//******��������ť״̬
#define BTMBT_NORMAL			0
#define BTMBT_HOT				1
#define BTMBT_PUSHED			2
//******��������ť��Ŀ
#define BTMBKBTNCOUNT			10
//******����/��ͣ��־
#define POPF_PLAYICON			1// Ӧ��ʾ����ͼ��
#define POPF_PAUSEICON			2// Ӧ��ʾ��ͣͼ��
#define POPF_AUTOSWITCH			3// �Զ�ת��״̬
//*****************************����*****************************//
/*
* Ŀ�꣺�����ǰ��Ϣ
*
* ������
*
* ����ֵ��
* ��ע��
*/
void MainWnd_ReleaseCurrInfo();
/*
* Ŀ�꣺�������ͬ������
*
* ������
*
* ����ֵ��
* ��ע���ص�
*/
void CALLBACK SyncProc_End(HSYNC handle, DWORD channel, DWORD data, void* user);
/*
 * Ŀ�꣺��һ�и��
 *
 * ������
 * iIndex �������
 * y ��ʼy���꣨���߻�ױߣ�����Ϊ-1��ʹ���ϴλ滭ʱ�Ķ��ߣ���ʱbTop������Ч
 * bTop �Ƿ�Ϊ����
 * bClearBK �Ƿ��������
 * bImmdShow ������ʾ����ΪFALSE���������⽫��̨λͼ������ǰ̨����ΪTRUE�����Զ������������
 *
 * ����ֵ�������ѻ��Ƶĸ�ʸ߶�
 * ��ע�����뱣֤�����Ϸ����Զ�����ǿ��˫�����ã��Զ������ȵ��ѡ����Ŀ����������ʱ��������ʾ���
 */
int UI_DrawLrcItem(int iIndex, int y, BOOL bTop, BOOL bClearBK, BOOL bImmdShow, BOOL bCenterLine = FALSE, int* yOut = NULL);
/*
 * Ŀ�꣺�����������б��е��ļ�
 *
 * ������
 * iIndex LV����
 *
 * ����ֵ��
 * ����������
 * ��ע��
 */
void Playing_PlayFile(int iIndex);
void Playing_Stop(BOOL bNoGap = FALSE);
void Playing_PlayOrPause(UINT uFlag = POPF_AUTOSWITCH);
void Playing_PlayNext(BOOL bReverse = FALSE);
void Playing_AutoNext();
void StopThread_Waves();
UINT __stdcall Thread_GetWavesData(void* p);
/*
 * Ŀ�꣺��������ڴ�λͼ����ʾ
 *
 * ������
 *
 * ����ֵ��
 * ����������
 * ��ע��
 */
void UI_UpdateLeftBK();
void UI_SeparateListWnd(BOOL b);
void UI_ShowList(BOOL b);
LRESULT CALLBACK WndProc_TBGhost(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam);
LRESULT CALLBACK WndProc(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam);
LRESULT CALLBACK WndProc_LeftBK(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam);
INT_PTR CALLBACK DlgProc_About(HWND hDlg, UINT uMsg, WPARAM wParam, LPARAM lParam);
void CALLBACK TimerProc(HWND hWnd, UINT uMsg, UINT_PTR idEvent, DWORD dwTime);
LRESULT CALLBACK QKCProc_SEB(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam);
int HitTest_LrcShow(POINT pt);
int HitTest_BtmBK(POINT pt);
void UI_VEDrawAlbum(BOOL bImmdShow = TRUE, BOOL bIndependlyDrawing = TRUE);
BOOL UI_VEDrawWaves(BOOL bImmdShow = TRUE, BOOL bIndependlyDrawing = TRUE);
void UI_VEDrawSpe(BOOL bImmdShow = TRUE, BOOL bIndependlyDrawing = TRUE);
void UI_VEDrawLrc(int yCenter, BOOL bImmdShow = TRUE, BOOL bIndependlyDrawing = TRUE);
BOOL UI_VEProcLrcShowing(BOOL bImmdShow = TRUE, BOOL bIndependlyDrawing = TRUE, BOOL bOnlyDrawDTLrc = FALSE);
void UI_RefreshBmpBrush();
void SettingsUpd_WndMain();