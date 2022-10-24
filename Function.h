/*
* Function.h
* 包含帮助性函数、结构和常量的定义
*/
#pragma once
#include <Windows.h>
#include <wincodec.h>

#include "WndMain.h"
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
    BYTE Flags;			// 标志
    BYTE Size[4];		// 标签大小，28位数据，每个字节最高位不使用，包括标签头的10个字节和所有的标签帧
};

struct ID3v2_ExtHeader
{
    BYTE ExtHeaderSize[4];
    BYTE Flags[2];
    BYTE PaddingSize[4];
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

// COLORREF，0x0BGR（红绿蓝从低位到高位排列）
#define QKCOLOR_GREEN                     0x00FF00
#define QKCOLOR_BLUE                      0xFF0000
#define QKCOLOR_RED                       0x0000FF
#define QKCOLOR_YELLOW                    0x00FFFF
#define QKCOLOR_CYAN                      0xFFFF00// cyan [ˈsaɪən] 青色
#define QKCOLOR_FUCHSIA                   0xFF00FF// fuchsia [ˈfjuːʃə] 紫红色，品红
#define QKCOLOR_WHITE                     0xFFFFFF
#define QKCOLOR_BLACK                     0x000000
#define QKCOLOR_CYANDEEPER                0xC06000// 易语言青蓝
#define QKCOLOR_GRAYDEEPER2               0x808080// 易语言灰色
#define QKCOLOR_GRAYDEEPER1               0xA0A0A0
#define QKCOLOR_GRAY                      0xC0C0C0// 易语言浅灰

#define PROP_INPUTBOXCONTEXT              L"QKProp.InputBox.Context"

//typedef LPVOID QKARRAY;
////////////////////////////////////
/*
* 目标：创建GDI字体
*
* 参数：
* pszFontName 字体名
* nPoint 点数
* nWeight 粗细
* IsItalic 是否倾斜
* IsUnderline 是否下划线
* IsStrikeOut 是否删除线
*
* 返回值：字体句柄
* 备注：封装了CreateFont便于使用
*/
HFONT QKCreateFont(PCWSTR pszFontName, int nPoint = 9, int nWeight = 400, BOOL IsItalic = FALSE, BOOL IsUnderline = FALSE, BOOL IsStrikeOut = FALSE);
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
    PCWSTR pszMainInstruction,
    PCWSTR pszContent = NULL,
    HICON hIcon = NULL,
    PCWSTR pszWndTitle = NULL,
    HWND hParent = NULL,
    PCWSTR pszChackBoxTitle = NULL,// 设为Null则不显示复选框
    UINT iButtonCount = 1,
    PCWSTR pszButton1Title = NULL,
    PCWSTR pszButton2Title = NULL,
    PCWSTR pszButton3Title = NULL,
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
/*
* 目标：大端序32位整数转小端序
*
* 参数：
* p 字节流指针
*
* 返回值：小端序整数
* 备注：
*/
UINT32 QKBEUINT32ToUINT32(BYTE* p);
/*
* 目标：同步安全整数转32位整数
*
* 参数：
* p 字节流指针
*
* 返回值：转换结果
* 备注：
*/
UINT32 QKSynchsafeUINT32ToUINT32(BYTE* p);
int QKStrInStrRev(PCWSTR pszOrg, PCWSTR pszSubStr, int iStartPos=1);
/*
* 目标：从文件解析音乐信息
*
* 参数：
* pszFile 文件名
* pmi 结果，稍后应调用MusicInfo_Release释放
*
* 返回值：
* 备注：
*/
void MusicInfo_Get(PCWSTR pszFile, MUSICINFO* pmi);
/*
* 目标：安全释放音乐信息数据
*
* 参数：
* mi 音乐信息结构
*
* 返回值：
* 备注：
*/
void MusicInfo_Release(MUSICINFO* mi);
/*
 * 目标：安全释放歌词数据
 *
 * 参数：
 * x 歌词数据数组
 *
 * 返回值：
 * 备注：
 */
void Lrc_ClearArray(QKARRAY x);
/*
 * 目标：从文件或字节流解析歌词数据
 *
 * 参数：
 * pStream 文件名字符串指针或LRC文件数据字节流。当作为文件字节流输入时，函数复制内存并使用副本
 * iSize 若输入LRC字节流，则该参数指示字节流长度
 * bFileName 指示pStream是否为文件名
 * Result 结果数组，调用函数前数组应初始化完毕
 * iDefTextCode 默认文本编码，0 自动；1 GB2312；2 UTF-8；3 UTF-16LE；4 UTF-16BE
 *
 * 返回值：
 * 备注：我写的这算法有点哈人
 */
void Lrc_ParseLrcData(void* pStream, int iSize, BOOL bFileName, QKARRAY* IDResult, QKARRAY* Result, int iDefTextCode);
/*
* 目标：粗略判断字节流是否为UTF8文本
*
* 参数：
* str 字节流指针
* length 字节流长度
*
* 返回值：是否为UTF8文本
* 备注：网上抄的
*/
BOOL QKIsTextUTF8(char* str, ULONGLONG length);
/*
* 目标：从文件创建WIC位图
*
* 参数：
* pszFile 文件名
* ppWICBitmap 返回WIC位图对象
*
* 返回值：
* 备注：
*/
HRESULT WICCreateBitmap(PWSTR pszFile, IWICBitmap** ppWICBitmap);
/*
* 目标：从字节流创建WIC位图
*
* 参数：
* pStream 字节流指针
* ppWICBitmap 返回WIC位图对象
*
* 返回值：
* 备注：
*/
HRESULT WICCreateBitmap(IStream* pStream, IWICBitmap** ppWICBitmap);