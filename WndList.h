/*
* WndList.h
* �����б�����������ش��ڹ��̺���غ����Ķ���
*/
#pragma once
#include <Windows.h>
#include "MyProject.h"

struct PLAYERLISTUNIT// �ڴ沥���б���Ŀ
{
	DWORD dwFlags;				// ��־��QKLIF_���������£�
	PWSTR pszName;				// ����
	PWSTR pszFile;				// �ļ���
	PWSTR pszTime;				// ʱ��
	PWSTR pszBookMark;			// ��ǩ��
	PWSTR pszBookMarkComment;	// ��ǩ��ע
	COLORREF crBookMark;		// ��ǩ��ɫ
	int iMappingIndexSearch;	// ����ʱӳ�䵽������
	int iMappingIndexSort;		// ����ʱӳ�䵽������
};
#define QKLIF_INVALID			0x00000001// ��Ŀ��Ч
#define QKLIF_IGNORED			0x00000002// ����
#define QKLIF_BOOKMARK			0x00000004// ����ǩ
#define QKLIF_DRAGMARK_CURRFILE	0x00000008// �����Ϸ�ʱ��Ч�����в��ű�־����ԭ������
#define QKLIF_TIME				0x00000010// ���ڴ浵���ȡ�ļ�ʱ��Ч������ʱ���ַ���
#define QKLIF_DRAGMARK_PLLATER	0x00000020// �����Ϸ�ʱ��Ч���Ժ󲥷ű�־����ԭ������

struct LISTFILEHEADER	// �����б��ļ�ͷ
{
	CHAR cHeader[4];	// �ļ���ʼ��ǣ�ASCII�ַ�"QKPL"
	int iCount;			// ��Ŀ��
	DWORD dwVer;		// �浵�ļ��汾��QKLFVER_���������£�
	DWORD dwReserve;	// ����������Ϊ0
};
#define QKLFVER_1				0// ����汾���б��ļ���û�м�¼ʱ��Ĺ��ܣ����������Ѿ����Լ����б�������ˣ���Ϊ�������ţ��������Լ�������汾���ƣ��������ֶι�Ȼ�����ǵ�ѡ��2333��
#define QKLFVER_2				1

struct LISTFILEITEM		// �����б��ļ���Ŀͷ
{
	UINT uFlags;		// ��Ŀ��־
	DWORD dwReserve1;	// ����������Ϊ0
	DWORD dwReserve2;	// ����������Ϊ0
};

#define DLGTYPE_LOADLIST		1
#define DLGTYPE_SAVELIST		2
#define LISTWND_REDRAWBOOKMARK	WM_USER + 1

#define PROP_OLEDROPLASTINDEX	L"QKProp.OLEDrop.LastIndex"
#define PROP_OLEDROPTARGETINDEX L"QKProp.OLEDrop.TargetInex"
#define PROP_OLEDROPINVALID		L"QKProp.OLEDrop.Invalid"
#define PROP_BOOKMARKCLRDLGBUF	L"QKProp.BookMark.DlgBuffer"

#define CLIPBOARDFMT_MYDRAGDROP L"QKClipboardFormat.MyDragDrop"

#define QKOLEDRAGVER_1			0

#define UI_RedrawBookMarkPos() SendMessageW(g_hBKList, LISTWND_REDRAWBOOKMARK, 0, 0)
void UI_SetRitCtrlPos();
/*
 * Ŀ�꣺���б���ɾ������
 *
 * ������
 * iItem ��ɾ������Ŀ���ڴ���Ŀ����������Ϊ-1��ɾ��������Ŀ
 * bRedraw �Ƿ��ػ�
 *
 * ����ֵ��
 * ����������
 * ��ע�����뱣֤λ�úϷ�
 */
void List_Delete(int iItem, BOOL bRedraw);
/*
 * Ŀ�꣺���������뵽�б�
 *
 * ������
 * pszFile �ļ���
 * pszName ���ƣ�����ʹ���ļ���
 * iPos ����λ��
 *
 * ����ֵ���ڴ���Ŀ����
 * ����������
 * ��ע��
 */
int List_Add(PWSTR pszFile, PWSTR pszName, int iPos, BOOL bRedraw, DWORD dwFlags = 0, COLORREF crBookMark = 0,
	PWSTR pszBookMark = NULL, PWSTR pszBookMarkComment = NULL, PWSTR pszTime = NULL);
void List_Redraw();
/*
 * Ŀ�꣺��LV�����õ��ڴ���Ŀ���������Զ���������ӳ��
 *
 * ������
 * iLVIndex LV��Ŀ����
 *
 * ����ֵ���ڴ���Ŀ����
 * ����������
 * ��ע���������������Ϸ�
 */
int List_GetArrayItemIndex(int iLVIndex);
/*
 * Ŀ�꣺��LV�����õ��ڴ���Ŀ��ָ�룬�Զ���������ӳ��
 *
 * ������
 * iLVIndex LV��Ŀ����
 *
 * ����ֵ���ڴ���Ŀָ��
 * ����������
 * ��ע���������������Ϸ�
 */
PLAYERLISTUNIT* List_GetArrayItem(int iLVIndex);
void List_ResetLV();
void List_SetRedraw(BOOL b);
/*
 * Ŀ�꣺����һ���̣߳���̨����б��ʱ����
 *
 * ������
 * bJudgeItem �Ƿ������Ŀ�ı�־���������Ƿ�ǿ�Ƹ���
 *
 * ����ֵ��
 * ����������
 * ��ע��
 */
void List_FillMusicTimeColumn(BOOL bJudgeItem);
void StopThread_MusicTime();



LRESULT CALLBACK WndProc_PlayList(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam);
LRESULT CALLBACK WndProc_RitBK(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam);
LRESULT CALLBACK WndProc_Edit(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam);
LRESULT CALLBACK WndProc_ListView(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam);
INT_PTR CALLBACK DlgProc_List(HWND hDlg, UINT message, WPARAM wParam, LPARAM lParam);
INT_PTR CALLBACK DlgProc_BookMark(HWND hDlg, UINT message, WPARAM wParam, LPARAM lParam);
HRESULT CALLBACK OLEDrag_GiveFeedBack(DWORD dwEffect);
HRESULT CALLBACK OLEDrop_OnEnter(IDataObject* pDataObj, DWORD grfKeyState, POINTL pt, DWORD* pdwEffect);
HRESULT CALLBACK OLEDrop_OnOver(DWORD grfKeyState, POINTL pt, DWORD* pdwEffect);
HRESULT CALLBACK OLEDrop_OnLeave();
HRESULT CALLBACK OLEDrop_OnDrop(IDataObject* pDataObj, DWORD grfKeyState, POINTL pt, DWORD* pdwEffect);








