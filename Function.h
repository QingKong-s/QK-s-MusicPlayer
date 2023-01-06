/*
* Function.h
* 包含帮助性函数、结构和常量的定义
*/
#pragma once
#include <Windows.h>
#include <wincodec.h>

#include <assert.h>

#include "WndMain.h"
typedef LPCVOID PCVOID;// 什么东西都带个远近指针跟老缠一样
/////////////////数组
#define QKADF_NO                0// 无动作
#define QKADF_DELETE            1// delete运算符
#define QKADF_DELETEARRAY       2// delete[]运算符
#define QKADF_HEAPFREE          3// HeapFree，使用数组所使用的堆
#define QKADF_DELPROC           4// 调用删除回调
typedef void(CALLBACK* QKARYDELPROC)(void*);// 数组删除回调
typedef struct QKARRAYHEADER                // 数组上下文
{
    int iCount;         // 项数
    int iCountAllocated;// 已分配的项数
    int iGrow;          // 每次重分配的增长数
    HANDLE hHeap;       // 堆
    QKARYDELPROC pProc; // 成员删除回调
    BYTE* pData;        // 实际数据
}*QKARRAY;
/////////////////输入框
struct QKINPUTBOXCOMTEXT// 输入框上下文
{
    PCWSTR pszTitle;
    PCWSTR pszTip;
    PWSTR* ppszBuffer;
    int iButton;
};
/////////////////哈希表
typedef void(CALLBACK* QKHTDELPROC)(void*); // 哈希表删除回调

typedef struct QKHASHTABLEHEADER            // 哈希表上下文
{
    int iCount;             // 表长
    int iP;                 // 除留取余的p
    HANDLE hHeap;           // 堆
    QKARRAY ary;            // 链表入口数组
    BOOL bKeyInt;           // 键是否为数值
    BOOL bValueInt;         // 值是否为数值
    QKHTDELPROC pKeyProc;   // 键释放回调
    QKHTDELPROC pValueProc; // 值释放回调
}*QKHASHTABLE;

struct QKHASHTABLELINKNODE                  // 哈希表内部链表节点
{
    UINT uHashCode;
    union// 键
    {
        struct
        {
            void* pKey;
            int iKeyLen;
        };
        UINT uKey;
    };

    union// 值
    {
        struct
        {
            void* pValue;
            int iValueLen;
        };
        UINT uValue;
    };
    QKHASHTABLELINKNODE* pNext;// 下一节点
};
typedef BOOL(CALLBACK* QKHTPUTKEYEXISTPROC)(QKHASHTABLELINKNODE*); // 哈希表添加键冲突回调
typedef BOOL(CALLBACK* QKHTENUMPROC)(QKHASHTABLELINKNODE*, LPARAM);
/////////////////音乐文件信息
struct ID3v2_Header		//ID3v2标签头
{
    CHAR Header[3];		// "ID3"
    BYTE Ver;			// 版本号
    BYTE Revision;		// 副版本号
    BYTE Flags;			// 标志
    BYTE Size[4];		// 标签大小，28位数据，每个字节最高位不使用，包括标签头的10个字节和所有的标签帧
};

struct ID3v2_ExtHeader  // ID3v2扩展头
{
    BYTE ExtHeaderSize[4];  // 扩展头大小
    BYTE Flags[2];          // 标志
    BYTE PaddingSize[4];    // 空白大小
};

struct ID3v2_UnitHeader	//ID3v2帧头
{
    CHAR ID[4];			// 帧标识
    BYTE Size[4];		// 帧内容的大小，32位数据，不包括帧头
    BYTE Flags[2];		// 存放标志
};

struct FLAC_Header      // Flac头
{
    BYTE by;
    BYTE bySize[3];
};
/////////////////INI解析
typedef struct QKINIDESC
{
    int iType;
    PWSTR pszContent;
    HANDLE hFile;
    DWORD dwSize;
}*HQKINI;
////////////////////////////////////
#define QKMSGBOX_BTID_1                   100
#define QKMSGBOX_BTID_2                   101
#define QKMSGBOX_BTID_3                   102

// COLORREF，0x0BGR（红绿蓝从低位到高位排列）
#define QKCOLOR_GREEN                       0x00FF00
#define QKCOLOR_BLUE                        0xFF0000
#define QKCOLOR_RED                         0x0000FF
#define QKCOLOR_YELLOW                      0x00FFFF
#define QKCOLOR_CYAN                        0xFFFF00// cyan [ˈsaɪən] 青色
#define QKCOLOR_FUCHSIA                     0xFF00FF// fuchsia [ˈfjuːʃə] 紫红色，品红
#define QKCOLOR_WHITE                       0xFFFFFF
#define QKCOLOR_BLACK                       0x000000
#define QKCOLOR_CYANDEEPER                  0xC06000// 易语言青蓝
#define QKCOLOR_GRAYDEEPER2                 0x808080// 易语言灰色
#define QKCOLOR_GRAYDEEPER1                 0xA0A0A0
#define QKCOLOR_GRAY                        0xC0C0C0// 易语言浅灰

#define PROP_INPUTBOXCONTEXT                L"QKProp.InputBox.Context"

#define QKINI_ERR                           0
#define QKINI_NEWFILE                       1
#define QKINI_NORMAL                        2

#define PTRSIZE sizeof(void*)
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

/// <summary>
/// 数组_创建
/// </summary>
/// <param name="iCount">初始成员数</param>
/// <param name="iGrow">扩容成员数</param>
/// <param name="hHeap">堆，若为NULL则使用进程堆</param>
/// <param name="pDelProc">成员释放回调，若为NULL则不使用回调</param>
/// <returns>成功返回数组句柄，失败返回NULL</returns>
QKARRAY QKACreate(int iCount, int iGrow = 10, HANDLE hHeap = NULL, QKARYDELPROC pDelProc = NULL);

/// <summary>
/// 数组_销毁
/// </summary>
/// <param name="pArray">数组句柄</param>
/// <param name="uDeleteFlag">删除标志，QKADF_常量</param>
void QKADelete(QKARRAY pArray, UINT uDeleteFlag = QKADF_NO);

/// <summary>
/// 数组_清除
/// </summary>
/// <param name="pArray">数组句柄</param>
/// <param name="uDeleteFlag">删除标志，QKADF_常量</param>
void QKAClear(QKARRAY pArray, UINT uDeleteFlag = QKADF_NO);

/// <summary>
/// 数组_加入，在数组尾部附加成员
/// </summary>
/// <param name="pArray">数组句柄</param>
/// <param name="pData">数据</param>
/// <returns>成功返回插入新成员的索引，失败返回-1</returns>
int QKAAdd(QKARRAY pArray, void* pData);

/// <summary>
/// 数组_加入数值
/// </summary>
/// <param name="pArray">数组句柄</param>
/// <param name="uValue">数值，限制为32位</param>
/// <returns>成功返回插入新成员的索引，失败返回-1</returns>
int QKAAddValue(QKARRAY pArray, UINT uValue);

/// <summary>
/// 数组_插入，在iIndex处插入一个成员
/// </summary>
/// <param name="pArray">数组句柄</param>
/// <param name="pData">数据</param>
/// <param name="iIndex">插入位置</param>
/// <returns>成功返回插入新成员的索引，失败返回-1</returns>
int QKAInsert(QKARRAY pArray, void* pData, int iIndex);

/// <summary>
/// 数组_取成员
/// </summary>
/// <param name="pArray">数组句柄</param>
/// <param name="iIndex">索引</param>
/// <returns>返回数据指针</returns>
__forceinline void* QKAGet(QKARRAY pArray, int iIndex)
{
    assert(pArray);
    return *(void**)(pArray->pData + PTRSIZE * iIndex);
}

/// <summary>
/// 数组_取数值
/// </summary>
/// <param name="pArray">数组句柄</param>
/// <param name="iIndex">索引</param>
/// <returns>返回数值</returns>
__forceinline UINT QKAGetValue(QKARRAY pArray, int iIndex)
{
    assert(pArray);
    return *(UINT*)(pArray->pData + PTRSIZE * iIndex);
}

/// <summary>
/// 数组_置数据
/// </summary>
/// <param name="pArray">数组句柄</param>
/// <param name="iIndex">索引</param>
/// <param name="pData">数据</param>
/// <param name="uDeleteFlag">删除标志，QKADF_常量</param>
void QKASet(QKARRAY pArray, int iIndex, void* pData, UINT uDeleteFlag = QKADF_NO);

/// <summary>
/// 数组_取成员数
/// </summary>
/// <param name="pArray">数组句柄</param>
/// <returns>返回成员数</returns>
__forceinline int QKAGetCount(QKARRAY pArray)
{
    assert(pArray);
    return pArray->iCount;
}

/// <summary>
/// 数组_删除成员
/// </summary>
/// <param name="pArray">数组句柄</param>
/// <param name="iIndex">索引</param>
/// <param name="uDeleteFlag">删除标志，QKADF_常量</param>
/// <param name="iDelCount">要删除的个数</param>
/// <return>成功返回TRUE，否则返回FALSE</return>
BOOL QKADeleteMember(QKARRAY pArray, int iIndex, UINT uDeleteFlag = QKADF_NO, int iDelCount = 1);

/// <summary>
/// 数组_取数据指针，注意：该指针在数组生命周期中可能发生变化
/// </summary>
/// <param name="pArray">数组句柄</param>
/// <returns>数据指针</returns>
__forceinline void* QKAGetDataPtr(QKARRAY pArray)
{
    assert(pArray);
    return pArray->pData;
}

/// <summary>
/// 数组_去除多余空间
/// </summary>
/// <param name="pArray">数组句柄</param>
/// <param name="bReserveGrowSpace">是否保留出增长空间</param>
/// <returns>若尺寸已改变，返回TRUE，若尺寸未改变或发生错误则返回FALSE</returns>
BOOL QKATrimSize(QKARRAY pArray, BOOL bReserveGrowSpace = TRUE);


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
int QKStrInStrRev(PCWSTR pszOrg, PCWSTR pszSubStr, int iStartPos = 1);
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
/*
* 目标：客户区坐标矩形转屏幕坐标矩形
*
* 参数：
* hWnd 窗口句柄
* prc 要转换的矩形
*
* 返回值：
* 备注：
*/
void QKRcClientToScreen(HWND hWnd, RECT* prc);
/*
* 目标：屏幕坐标矩形转客户区坐标矩形
*
* 参数：
* hWnd 窗口句柄
* prc 要转换的矩形
*
* 返回值：
* 备注：
*/
void QKRcScreenToClient(HWND hWnd, RECT* prc);
/*
* 目标：COLORREF颜色转D2D1_COLOR_F
*
* 参数：
* cr COLORREF颜色
* D2DCr 结果
* iAlpha 透明度，默认不透明
*
* 返回值：
* 备注：
*/
void QKGDIColorToD2DColor(COLORREF cr, D2D1_COLOR_F* D2DCr, int iAlpha = 0xFF);

// （WinAPI提供的INI读写有点闹弹，自己写个吧）
/// <summary>
/// 读文本配置项
/// </summary>
/// /// <param name="hINI">INI句柄</param>
/// <param name="pszSectionName">节名</param>
/// <param name="pszKeyName">键名</param>
/// <param name="pszDefStr">默认字符串</param>
/// <param name="pszRetStr">结果缓冲区</param>
/// <param name="iMaxBufSize">缓冲区大小，以WCHAR计，包括结尾NULL</param>
/// <returns>返回复制到缓冲区的字节数，不包括结尾NULL；若缓冲区尺寸太小无法完全接收，则字符串将截断并添加结尾NULL，此时返回值为接收完整字符串所需的缓冲区大小的负值，不包括结尾NULL，失败返回0</returns>
int QKINIReadString(HQKINI hINI, PCWSTR pszSectionName, PCWSTR pszKeyName, PCWSTR pszDefStr, PWSTR pszRetStr, int iMaxBufSize);
/// <summary>
/// 解析INI文件
/// </summary>
/// <param name="pszFile">INI文件名</param>
/// <returns>返回INI句柄，失败返回NULL</returns>
HQKINI QKINIParse(PCWSTR pszFile);
/// <summary>
/// 释放INI句柄
/// </summary>
/// <param name="hINI">INI句柄</param>
void QKINIClose(HQKINI hINI);
/// <summary>
/// 保存INI文件
/// </summary>
/// <param name="hINI">INI句柄</param>
/// <returns>成功返回TRUE，失败返回FALSE</returns>
BOOL QKINISave(HQKINI hINI);
/// <summary>
/// 写文本配置项
/// </summary>
/// <param name="hINI">INI句柄</param>
/// <param name="pszSectionName">节名</param>
/// <param name="pszKeyName">键名</param>
/// <param name="pszString">写入字符串</param>
void QKINIWriteString(HQKINI hINI, PCWSTR pszSectionName, PCWSTR pszKeyName, PCWSTR pszString);
/// <summary>
/// 读整数配置项
/// </summary>
/// <param name="hINI">INI句柄</param>
/// <param name="pszSectionName">节名</param>
/// <param name="pszKeyName">键名</param>
/// <param name="iDefValue">默认数值</param>
/// <returns>返回读取结果</returns>
int QKINIReadInt(HQKINI hINI, PCWSTR pszSectionName, PCWSTR pszKeyName, int iDefValue);
/// <summary>
/// 写整数配置项
/// </summary>
/// <param name="hINI">INI句柄</param>
/// <param name="pszSectionName">节名</param>
/// <param name="pszKeyName">键名</param>
/// <param name="iValue">写入值</param>
void QKINIWriteInt(HQKINI hINI, PCWSTR pszSectionName, PCWSTR pszKeyName, int iValue);
/// <summary>
/// 读文本配置项
/// </summary>
/// <param name="hINI">INI句柄</param>
/// <param name="pszSectionName">节名</param>
/// <param name="pszKeyName">键名</param>
/// <param name="pszDefStr">默认字符串</param>
/// <returns>返回获取结果文本指针，使用完毕后必须调用delete[]释放</returns>
PWSTR QKINIReadString2(HQKINI hINI, PCWSTR pszSectionName, PCWSTR pszKeyName, PCWSTR pszDefStr);



/// <summary>
/// 哈希表_创建
/// </summary>
/// <param name="iCount">表长</param>
/// <param name="bKeyInt">键是否为数值</param>
/// <param name="bValueInt">值是否为数值</param>
/// <param name="pKeyProc">键释放回调，可为NULL</param>
/// <param name="pValueProc">值释放回调，可为NULL</param>
/// <param name="hHeap">要在其中分配内存的堆句柄，可为NULL，为NULL则使用进程堆</param>
/// <returns></returns>
QKHASHTABLE QKHTCreate(int iCount, BOOL bKeyInt, BOOL bValueInt, QKHTDELPROC pKeyProc = NULL, QKHTDELPROC pValueProc = NULL, 
    HANDLE hHeap = NULL);

/// <summary>
/// 哈希表_取哈希值
/// </summary>
/// <param name="pData">数据指针</param>
/// <param name="iLen">数据长度，以字节为单位，当此参数指向Unicode字符串时可传递负数，函数将自动推断长度</param>
/// <returns>返回哈希值</returns>
UINT QKHTGetHashCode(PCVOID pData, int iLen);

/// <summary>
/// 哈希表_取哈希值
/// </summary>
/// <param name="uValue">数值型数据</param>
/// <returns>返回哈希值</returns>
__forceinline UINT QKHTGetHashCode(UINT uValue)
{
    return uValue;
}

/// <summary>
/// 哈希表_置键值，若键已存在，则覆盖原值，否则添加此键
/// </summary>
/// <param name="hHT">哈希表句柄</param>
/// <param name="pKey">键</param>
/// <param name="iKeyLen">键长度，以字节为单位，当此参数指向Unicode字符串时可传递负数，函数将自动推断长度</param>
/// <param name="pValue">值</param>
/// <param name="iValueLen">值长度，以字节为单位，当此参数指向Unicode字符串时可传递负数，函数将自动推断长度</param>
/// <param name="uValue">数值型的值</param>
/// <param name="bCover">是否覆盖原来的值</param>
/// <param name="puHashCode">返回哈希值，可为NULL</param>
/// <returns></returns>
BOOL QKHTPut(QKHASHTABLE hHT, PCVOID pKey, int iKeyLen, PCVOID pValue, int iValueLen, UINT uValue, 
	BOOL bCover = TRUE, UINT* puHashCode = NULL, QKHTPUTKEYEXISTPROC pKeyExistProc = NULL);

/// <summary>
/// 哈希表_键取值
/// </summary>
/// <param name="hHT">哈希表句柄</param>
/// <param name="pKey">键</param>
/// <param name="iKeyLen">键长度，以字节为单位，当此参数指向Unicode字符串时可传递负数，函数将自动推断长度</param>
/// <param name="pValue">指向接收值的变量的指针。若值类型为数值，则该参数指向UINT变量，否则该参数指向指针变量，且接收到的值只读</param>
/// <param name="bHasHashCode">是否已有哈希值，若有，则uHashCode参数必须为一个有效的哈希值</param>
/// <param name="uHashCode">哈希值，bHasHashCode==TRUE时有效</param>
/// <returns>查找到返回TRUE，否则返回FALSE</returns>
BOOL QKHTGet(QKHASHTABLE hHT, PCVOID pKey, int iKeyLen, void* ppValue, BOOL bHasHashCode = FALSE, UINT uHashCode = 0);

/// <summary>
/// 哈希表_删除键
/// </summary>
/// <param name="hHT">哈希表句柄</param>
/// <param name="pKey">键</param>
/// <param name="iKeyLen">键长度，以字节为单位，当此参数指向Unicode字符串时可传递负数，函数将自动推断长度</param>
/// <param name="bHasHashCode">是否已有哈希值，若有，则uHashCode参数必须为一个有效的哈希值</param>
/// <param name="uHashCode">哈希值，bHasHashCode==TRUE时有效</param>
/// <returns>成功返回TRUE，否则返回FALSE</returns>
BOOL QKHTDelKey(QKHASHTABLE hHT, PCVOID pKey, int iKeyLen, BOOL bHasHashCode = FALSE, UINT uHashCode = 0);

/// <summary>
/// 哈希表_删除，释放整个哈希表
/// </summary>
/// <param name="hHT">哈希表句柄，可以为NULL</param>
void QKHTDelete(QKHASHTABLE hHT);

void QKHTEnum(QKHASHTABLE hHT, QKHTENUMPROC pProc, LPARAM lParam);

#define QKReInterpretNumber(num, type) IntReInterpretNumber((type)0, num)
template<typename T1,typename T2>
__forceinline T1 IntReInterpretNumber(T1 nNoUsed,T2 n)
{
    return *(T1*)&n;
}