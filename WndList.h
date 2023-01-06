/*
* WndList.h
* 包含列表容器窗口相关窗口过程和相关函数的定义
*/
#pragma once
#include <Windows.h>
#include "MyProject.h"
#include "Function.h"

struct PLAYERLISTUNIT// 内存播放列表项目
{
	DWORD dwFlags;				// 标志，QKLIF_常量（见下）
	PWSTR pszName;				// 名称
	PWSTR pszFile;				// 文件名
	PWSTR pszTime;				// 时长
	PWSTR pszBookMark;			// 书签名
	PWSTR pszBookMarkComment;	// 书签备注
	COLORREF crBookMark;		// 书签颜色
	int iMappingIndexSearch;	// 搜索时映射到的索引
	int iMappingIndexSort;		// 排序时映射到的索引
	QKARRAY Artists;			// 缓存艺术家列表
};
#define QKLIF_INVALID			0x00000001// 项目无效
#define QKLIF_IGNORED			0x00000002// 忽略
#define QKLIF_BOOKMARK			0x00000004// 有书签
#define QKLIF_DRAGMARK_CURRFILE	0x00000008// 仅在拖放时有效，现行播放标志，还原索引用
#define QKLIF_TIME				0x00000010// 仅在存档或读取文件时有效，已有时长字符串
#define QKLIF_DRAGMARK_PLLATER	0x00000020// 仅在拖放时有效，稍后播放标志，还原索引用

struct LISTFILEHEADER	// 播放列表文件头
{
	CHAR cHeader[4];	// 文件起始标记，ASCII字符串"QKPL"
	int iCount;			// 项目数
	DWORD dwVer;		// 存档文件版本，QKLFVER_常量（见下）
	DWORD dwReserved;	// 保留，必须为0
};

#define QKLFVER_1				0// 这个版本的列表文件还没有记录时间的功能，但是作者已经把自己的列表排序好了，因为不想重排（懒）所以加了这个版本控制（留保留字段果然是明智的选择2333）
#define QKLFVER_2				1

struct LISTFILEITEM		// 播放列表文件项目头
{
	UINT uFlags;		// 项目标志
	DWORD dwReserved1;	// 保留，必须为0
	DWORD dwReserved2;	// 保留，必须为0
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
 * 目标：从列表中删除歌曲
 *
 * 参数：
 * iItem 欲删除的项目，内存项目索引；设置为-1以删除所有项目
 * bRedraw 是否重画
 *
 * 返回值：
 * 操作简述：
 * 备注：必须保证位置合法
 */
void List_Delete(int iItem, BOOL bRedraw);
/*
 * 目标：将歌曲加入到列表
 *
 * 参数：
 * pszFile 文件名
 * pszName 名称，空则使用文件名
 * iPos 插入位置
 *
 * 返回值：内存项目索引
 * 操作简述：
 * 备注：
 */
int List_Add(PWSTR pszFile, PWSTR pszName, int iPos, BOOL bRedraw, DWORD dwFlags = 0, COLORREF crBookMark = 0,
	PWSTR pszBookMark = NULL, PWSTR pszBookMarkComment = NULL, PWSTR pszTime = NULL);
void List_Redraw();
/*
 * 目标：由LV索引得到内存项目的索引，自动处理索引映射
 *
 * 参数：
 * iLVIndex LV项目索引
 *
 * 返回值：内存项目索引
 * 操作简述：
 * 备注：传入的索引必须合法
 */
int List_GetArrayItemIndex(int iLVIndex);
/*
 * 目标：由LV索引得到内存项目的指针，自动处理索引映射
 *
 * 参数：
 * iLVIndex LV项目索引
 *
 * 返回值：内存项目指针
 * 操作简述：
 * 备注：传入的索引必须合法
 */
PLAYERLISTUNIT* List_GetArrayItem(int iLVIndex);
void List_ResetLV();
void List_SetRedraw(BOOL b);
/*
 * 目标：启动一个线程，后台填充列表的时间列
 *
 * 参数：
 * bJudgeItem 是否根据项目的标志工作，即是否强制更新
 *
 * 返回值：
 * 操作简述：
 * 备注：
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








