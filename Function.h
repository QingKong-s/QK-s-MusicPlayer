#pragma once
#include <Windows.h>

#include "MainWnd.h"
////////////////////////////////////
typedef struct QKARRAYHEADER
{
	int iCount;
}*QKARRAY;

struct QKINPUTBOXCOMTEXT
{
    LPCWSTR pszTitle;
    LPCWSTR pszTip;
    LPWSTR* ppszBuffer;
    int iButton;
};

struct ID3v2_Header		//ID3v2标签头
{
    CHAR Header[3];		// "ID3"
    BYTE Ver;			// 版本号
    BYTE Revision;		// 副版本号
    BYTE Flag;			// 标志
    BYTE Size[4];		// 标签大小，28位数据，每个字节最高位不使用，包括标签头的10个字节和所有的标签帧
};

struct ID3v2_UnitHeader	//ID3v2帧头
{
    CHAR ID[4];			// 帧标识
    BYTE Size[4];		// 帧内容的大小，32位数据，不包括帧头
    BYTE Flags[2];		// 存放标志
};

struct FLAC_Header
{
    BYTE by;
    BYTE bySize[3];
};
////////////////////////////////////
#define QKADF_NO             0
#define QKADF_DELETE         1
#define QKADF_DELETEARRAY    2
#define QKADF_HEAPFREE       3

#define QKMSGBOX_BTID_1                   100
#define QKMSGBOX_BTID_2                   101
#define QKMSGBOX_BTID_3                   102

//0x0BGR（红绿蓝从低位到高位排列）
#define QKCOLOR_GREEN                     0x00FF00
#define QKCOLOR_BLUE                      0xFF0000
#define QKCOLOR_RED                       0x0000FF
#define QKCOLOR_YELLOW                    0x00FFFF
#define QKCOLOR_CYAN                      0xFFFF00//cyan [ˈsaɪən] 青色
#define QKCOLOR_FUCHSIA                   0xFF00FF//fuchsia [ˈfjuːʃə] 紫红色，品红
#define QKCOLOR_WHITE                     0xFFFFFF
#define QKCOLOR_BLACK                     0x000000
#define QKCOLOR_CYANDEEPER                0xC06000//易语言青蓝
#define QKCOLOR_GRAYDEEPER2               0x808080//易语言灰色
#define QKCOLOR_GRAYDEEPER1               0xA0A0A0
#define QKCOLOR_GRAY                      0xC0C0C0//易语言浅灰

#define PROP_INPUTBOXCONTEXT              L"QKProp.InputBox.Context"

//typedef LPVOID QKARRAY;
////////////////////////////////////
HFONT QKCreateFont(LPCWSTR szFontName, int nPoint = 9, int nWeight = 400, bool IsItalic = false, bool IsUnderline = false, bool IsStrikeOut = false);
/*
 * 目标：在一个字符串中查找另一字符串出现的位置
 *
 * 参数：
 * pszOrg 被搜寻的字符串
 * pszSubStr 要查找的字符串
 * iStartPos 起始位置，缺省1
 *
 * 返回值：返回位置，查找失败返回0
 * 操作简述：
 * 备注：！！不区分大小写，位置从1开始
 */
int QKStrInStr(PCWSTR pszOrg, PCWSTR pszSubStr, int iStartPos = 1);
/*
 * 目标：在一个字符串中查找另一字符串出现的位置
 *
 * 参数：
 * pszOrg 被搜寻的字符串
 * pszSubStr 要查找的字符串
 * iStartPos 起始位置，缺省1
 *
 * 返回值：返回位置，查找失败返回0
 * 操作简述：
 * 备注：！！区分大小写，位置从1开始
 */
int QKStrInStrCS(PCWSTR pszOrg, PCWSTR pszSubStr, int iStartPos = 1);
/*
 * 目标：删首尾空
 *
 * 参数：
 * pszOrg 被操作的字符串
 *
 * 返回值：
 * 操作简述：删除字符串首尾全、半角空格及制表符
 * 备注：原字符串将被破坏，欲留副本，请复制内存
 */
void QKStrTrim(PWSTR pszOrg);

LPVOID QKArray_Get(QKARRAY pArray, DWORD dwIndex);
LPVOID QKArray_GetValue(QKARRAY pArray, DWORD dwIndex);
void QKArray_Delete(QKARRAY pArray, DWORD dwDeleteFlag = QKADF_NO);
void QKArray_Clear(QKARRAY pArray, DWORD dwDeleteFlag = QKADF_NO);
int QKArray_GetCount(QKARRAY pArray);
QKARRAY QKArray_Create(int iCount);
int QKArray_Add(QKARRAY* ppArray, LPVOID pMember);
int QKArray_AddValue(QKARRAY* ppArray, LPVOID pMember);
int QKArray_Insert(QKARRAY* ppArray, LPVOID pMember, int iIndex);
void QKArray_Set(QKARRAY pArray, DWORD dwIndex, LPVOID pMember, DWORD dwDeleteFlag = QKADF_NO);
void QKArray_DeleteMember(QKARRAY* ppArray, DWORD dwIndex, DWORD dwDeleteFlag = QKADF_NO);
BOOL QKInputBox(PCWSTR pszTitle, PCWSTR pszTip, PWSTR* ppszBuffer, HWND hParent);
UINT QKMessageBox(
    LPCWSTR pszMainInstruction,
    LPCWSTR pszContent = NULL,
    HICON hIcon = NULL,
    LPCWSTR pszWndTitle = NULL,
    LPCWSTR pszChackBoxTitle = NULL,//设为Null则不显示复选框
    UINT iButtonCount = 1,
    LPCWSTR pszButton1Title = NULL,
    LPCWSTR pszButton2Title = NULL,
    LPCWSTR pszButton3Title = NULL,
    HWND hParent = NULL,
    UINT iDefButton = 1,
    BOOL IsCenterPos = FALSE,
    BOOL* IsCheck = NULL
);
BOOL QKGradientFill(HDC hDC, RECT* Rect, COLORREF Color1, COLORREF Color2, ULONG Mode = GRADIENT_FILL_RECT_H);
void QKOutputLastErrorCode();
void QKOutputDebugInt(int i);
UINT QKGDIClrToCommonClr(COLORREF cr);
COLORREF QKCommonClrToGDIClr(UINT u);
UINT QKGetDPIForWindow(HWND hWnd);
BOOL QKStrToBool(PCWSTR psz);
UINT32 QKByteStreamToBEUINT32(BYTE* p);
int QKStrInStrRev(PCWSTR pszOrg, PCWSTR pszSubStr, int iStartPos=1);
void MusicInfo_Get(PCWSTR pszFile, MUSICINFO* pmi);
void MusicInfo_Release(MUSICINFO* mi);
void Lrc_ClearArray(QKARRAY x);
void Lrc_ParseLrcData(void* pStream, int iSize, BOOL bFileName, QKARRAY* IDResult, QKARRAY* Result, int iDefTextCode);
int QKIsTextUTF8(char* str, ULONGLONG length);