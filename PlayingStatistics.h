#pragma once
#include <Windows.h>

#include "Function.h"
/*
*****�������ļ���
�ļ�ͷ
{
	��Ŀͷ
	���������ֳ���
	����������
}
������Ŀͷ


*****�����ļ���
�ļ�ͷ
{
	��Ŀͷ
	����������
	������
	�����Ҹ���
	{
		���������ֳ���
		����������
	}
}
������Ŀͷ
*/

#define QKAFVER_1					0
struct ARTISTSFILEHEADER// �����Ҵ浵�ļ�ͷ
{
	CHAR cHeader[4];	// �ļ���ʼ��ǣ�ASCII�ַ���"QKAL"
	DWORD dwVer;		// �浵�ļ��汾��QKAFVER_����
	int iCount;			// ��Ŀ��
	SYSTEMTIME st;
	DWORD dwReserved;	// ����������Ϊ0
};

#define QKALF_END					(1u << 1)
struct ARTISTSLISTUNIT
{
	UINT uFlags;		// ��־��QKALF_����
	UINT uSecond;
	UINT uPlayingCount;
};

#define FILEMAGIC_SONGSFILEHEDAER	"QKSL"
#define QKSFVER_1					0
struct SONGSSTATFILEHEADER// ����ͳ����Ϣ�浵�ļ�ͷ
{
	CHAR cHeader[4];	// �ļ���ʼ��ǣ�ASCII�ַ���"QKSL"
	DWORD dwVer;		// �浵�ļ��汾��QKSFVER_����
	int iCount;
	SYSTEMTIME st;		// ʱ��
	DWORD dwReserved;	// ����������Ϊ0
};

#define QKSLF_END					(1u << 1)
struct SONGSSTATUNIT
{
	UINT uFlags;		// ��־��QKSLF_����
	UINT uSecond;
	UINT uPlayingCount;
	UINT uSingleLoopCount;
	SYSTEMTIME stSingleLoop;
	BOOL bNeedFreeArray;
	QKARRAY aArtists;
};

#define FILEPREFIX_STATINFO				L"StatInfo-"	// ͳ���ļ�ǰ׺��������ʽ���ơ�StatInfo-2023-1.qkpsi��
#define SLEN_FILEPREFIX_STATINFO		(sizeof(FILEPREFIX_STATINFO) / sizeof(WCHAR))		// ͳ���ļ�ǰ׺���ȣ���WCHAR�ƣ�������βNULL
#define FILEEXT_STATINFOARTISTS			L".qkpasi"		// ������ͳ���ļ���׺
#define SLEN_FILEEXT_STATINFOARTISTS	(sizeof(FILEEXT_STATINFOARTISTS) / sizeof(WCHAR))	// ������ͳ���ļ���׺���ȣ���WCHAR�ƣ�������βNULL
#define FILEEXT_STATINFOSONGS			L".qkpssi"		// ����ͳ���ļ���׺
#define SLEN_FILEEXT_STATINFOSONGS		(sizeof(FILEEXT_STATINFOSONGS) / sizeof(WCHAR))		// ����ͳ���ļ���׺���ȣ���WCHAR�ƣ�������βNULL
#define MAX_DATA_BUFSIZE				9				// �����ַ�����󻺳�����С����WCHAR�ƣ�������βNULL

typedef void(CALLBACK* STATARTISTSFINDPROC)(void* p, BOOL bNewItem, LPARAM lParam);

/// <summary>
/// �ָ��������ַ���
/// </summary>
/// <param name="pszArtist">�������ַ���</param>
/// <param name="pszDiv">�ָ���</param>
/// <param name="ppArray">���ؽ����ʹ��ADF_DELETEARRAY��־ɾ��</param>
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