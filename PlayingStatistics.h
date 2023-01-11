#pragma once
#include <Windows.h>

#include "Function.h"
/*
*****艺术家文件：
文件头
{
	项目头
	艺术家名字长度
	艺术家名字
}
结束项目头


*****歌曲文件：
文件头
{
	项目头
	歌曲名长度
	歌曲名
	艺术家个数
	{
		艺术家名字长度
		艺术家名字
	}
}
结束项目头
*/

#define QKAFVER_1					0
struct ARTISTSFILEHEADER// 艺术家存档文件头
{
	CHAR cHeader[4];	// 文件起始标记，ASCII字符串"QKAL"
	DWORD dwVer;		// 存档文件版本，QKAFVER_常量
	int iCount;			// 项目数
	SYSTEMTIME st;
	DWORD dwReserved;	// 保留，必须为0
};

#define QKALF_END					(1u << 1)
struct ARTISTSLISTUNIT
{
	UINT uFlags;		// 标志，QKALF_常量
	UINT uSecond;
	UINT uPlayingCount;
};

#define FILEMAGIC_SONGSFILEHEDAER	"QKSL"
#define QKSFVER_1					0
struct SONGSSTATFILEHEADER// 歌曲统计信息存档文件头
{
	CHAR cHeader[4];	// 文件起始标记，ASCII字符串"QKSL"
	DWORD dwVer;		// 存档文件版本，QKSFVER_常量
	int iCount;
	SYSTEMTIME st;		// 时间
	DWORD dwReserved;	// 保留，必须为0
};

#define QKSLF_END					(1u << 1)
struct SONGSSTATUNIT
{
	UINT uFlags;		// 标志，QKSLF_常量
	UINT uSecond;
	UINT uPlayingCount;
	UINT uSingleLoopCount;
	SYSTEMTIME stSingleLoop;
	BOOL bNeedFreeArray;
	QKARRAY aArtists;
};

#define FILEPREFIX_STATINFO				L"StatInfo-"	// 统计文件前缀，最终样式类似“StatInfo-2023-1.qkpsi”
#define SLEN_FILEPREFIX_STATINFO		(sizeof(FILEPREFIX_STATINFO) / sizeof(WCHAR))		// 统计文件前缀长度，以WCHAR计，包含结尾NULL
#define FILEEXT_STATINFOARTISTS			L".qkpasi"		// 艺术家统计文件后缀
#define SLEN_FILEEXT_STATINFOARTISTS	(sizeof(FILEEXT_STATINFOARTISTS) / sizeof(WCHAR))	// 艺术家统计文件后缀长度，以WCHAR计，包含结尾NULL
#define FILEEXT_STATINFOSONGS			L".qkpssi"		// 歌曲统计文件后缀
#define SLEN_FILEEXT_STATINFOSONGS		(sizeof(FILEEXT_STATINFOSONGS) / sizeof(WCHAR))		// 歌曲统计文件后缀长度，以WCHAR计，包含结尾NULL
#define MAX_DATA_BUFSIZE				9				// 日期字符串最大缓冲区大小，以WCHAR计，包含结尾NULL

typedef void(CALLBACK* STATARTISTSFINDPROC)(void* p, BOOL bNewItem, LPARAM lParam);

/// <summary>
/// 分割艺术家字符串
/// </summary>
/// <param name="pszArtist">艺术家字符串</param>
/// <param name="pszDiv">分隔符</param>
/// <param name="ppArray">返回结果，使用ADF_DELETEARRAY标志删除</param>
void PS_SplitArtist(PCWSTR pszArtist, PCWSTR pszDiv, QKARRAY* pQKArray);

void PS_AddArtistPlayingCount(PCWSTR pszArtist, BOOL bHasHashCode = FALSE, UINT uHashCode = 0, UINT* puHashCode = NULL);
void PS_AddArtistPlayingTime(PCWSTR pszArtist, UINT uSecond, BOOL bHasHashCode = FALSE, UINT uHashCode = 0, UINT* puHashCode = NULL);
BOOL PS_StatArtistFind(PCWSTR pszArtist, BOOL bHasHashCode, UINT uHashCode, UINT* puHashCode, STATARTISTSFINDPROC pProc, LPARAM lParam = 0);
void PS_AddSongPlayingCount(PCWSTR pszSongName, QKARRAY aArtists, BOOL bHasHashCode = FALSE, UINT uHashCode = 0, UINT* puHashCode = NULL);
void PS_AddSongPlayingTime(PCWSTR pszSongName, UINT uSecond, BOOL bHasHashCode = FALSE, UINT uHashCode = 0, UINT* puHashCode = NULL);
void PS_AddSongSingleLoop(PCWSTR pszSongName, SYSTEMTIME* pst, BOOL bHasHashCode = FALSE, UINT uHashCode = 0, UINT* puHashCode = NULL);
BOOL PS_StatSongFind(PCWSTR pszSongName, BOOL bHasHashCode, UINT uHashCode, UINT* puHashCode, STATARTISTSFINDPROC pProc, LPARAM lParam = 0);
BOOL PS_LoadStatFile(PCWSTR pszDir);
void PS_SaveStatFile(PCWSTR pszDir = NULL);
INT_PTR CALLBACK DlgProc_PlayingStat(HWND hDlg, UINT uMsg, WPARAM wParam, LPARAM lParam);