#pragma once
#include "Windows.h"
#include "Function.h"
#include "UxTheme.h"
#include "MyProject.h"

BOOL QKCtrlInit();
typedef LRESULT(CALLBACK* QKCPROC)(HWND, UINT, WPARAM, LPARAM);

struct QKCTBCONTEXT// [�ڲ�]  ������������
{
	int iMax;
	int iPos;
	QKCPROC pCallBack;
	RECT rc;// ��������
	BYTE byHot;
	BYTE byPressed;
	int iDragPos;
};
struct QKCTBPAINT// ���������ƽṹ
{
	HDC hDC;
	RECT* rc;
	RECT* rcClient;
	UINT iCtrlID;
	BYTE byHot;
	BYTE byPressed;
};

struct QKCSEBCONTEXT
{
	HWND hChild;
	HDC hDC;
	HBITMAP hBitmap;
	COLORREF cr1;
	ARGB cr2;
	QKCPROC pProc;
	BOOL bLBTDown;
	BOOL bDragging;
	int iMin;
	int iMax;
	int y;
	int iPos;
	int iOffest;
};
#define QKCCN_TRACKBAR				L"QKWndClass.TrackBar"
#define QKCCN_SEPARATEBAR			L"QKWndClass.SeparateBar"
#define QKCCN_SEPARATEBAR2			L"QKWndClass.SeparateBar2"
//////////////////////////������
#define QKCTBM_SETRANGE				WM_USER + 1
#define QKCTBM_GETRANGE				WM_USER + 2
#define QKCTBM_SETPOS				WM_USER + 3
#define QKCTBM_GETPOS				WM_USER + 4
#define QKCTBM_SETPROC				WM_USER + 5
#define QKCTBM_GETTRACKPOS			WM_USER + 6

#define QKCTBN_PAINT				100
#define QKCSEPN_DRAGEND				101// (x, 0) �������
//////////////////////////�ָ���
#define QKCSEBM_SETCOLOR			WM_USER + 1// (crColor, bRedraw) ����ɫ
#define QKCSEBM_SETPROC				WM_USER + 2// (pProc, 0) �ûص�
#define QKCSEBM_SETCOLOR2			WM_USER + 3// (crColor, bRedraw) ���϶�ʱ��ɫ��ARGB
#define QKCSEBM_SETRANGE			WM_USER + 4// (iMin, iMax) �÷�Χ���϶�ʱ�Զ����ָ����޶��ڴ˷�Χ��
#define QKCSEBM_GETRANGE			WM_USER + 5// (0, 0) ȡ��Χ��ret = MAKELONG(iMin, iMax)
//#define QKCSEBM_			WM_USER + 6