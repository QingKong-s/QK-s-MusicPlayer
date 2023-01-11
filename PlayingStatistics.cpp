#include <Windows.h>

#include "PlayingStatistics.h"
#include "Function.h"
#include "GlobalVar.h"
#include "resource.h"

QKHASHTABLE m_SongsStatWithArtists = NULL;// 便于进行艺术家->歌曲查找的哈希表，(键)艺术家->(值)歌曲统计信息LV索引

void PS_SplitArtist(PCWSTR pszArtist, PCWSTR pszDiv, QKARRAY* ppArray)
{
	if (!pszArtist)
		return;
	QKARRAY hA = QKACreate(0);
	*ppArray = hA;
	PCWSTR ptemp = pszArtist;
	PWSTR psz;
	int iPos = QKStrInStr(pszArtist, pszDiv), iLastPos = 0;
	int iLen = lstrlenW(pszArtist), iLenDiv = lstrlenW(pszDiv);
	int iCount;
	if (!iPos)
	{
		psz = new WCHAR[iLen + 1];
		lstrcpyW(psz, pszArtist);
		QKAAdd(hA, psz);
		return;
	}
	while (TRUE)
	{
		if (iPos)
		{
			iCount = iPos - iLastPos;
			psz = new WCHAR[iCount];
			
			lstrcpynW(psz, pszArtist + iLastPos, iCount);
			QKAAdd(hA, psz);

			iLastPos = iPos + iLenDiv - 1;
			iPos = QKStrInStr(pszArtist, pszDiv, iLastPos + 1);
		}
		else
		{
			iCount = iLen - iLastPos + 1;
			psz = new WCHAR[iCount];
			lstrcpynW(psz, pszArtist + iLastPos, iCount);
			QKAAdd(hA, psz);
			break;
		}
	}
}

BOOL PS_StatArtistFind(PCWSTR pszArtist, BOOL bHasHashCode, UINT uHashCode, UINT* puHashCode, STATARTISTSFINDPROC pProc, LPARAM lParam)
{
	if (!g_StatArtists)
		return FALSE;

	ARTISTSLISTUNIT* p;
	if (QKHTGet(g_StatArtists, pszArtist, -1, &p, bHasHashCode, uHashCode))
	{
		if (p)
		{
			pProc(p, FALSE, lParam);
			return TRUE;
		}
		else
			goto PutNewItem;
	}
	else
	{
	PutNewItem:
		p = new ARTISTSLISTUNIT;
		ZeroMemory(p, sizeof(ARTISTSLISTUNIT));
		PWSTR psz = new WCHAR[lstrlenW(pszArtist)];
		lstrcpyW(psz, pszArtist);
		QKHTPut(g_StatArtists, psz, -1, p, sizeof(ARTISTSLISTUNIT), 0, TRUE, puHashCode);
		pProc(p, TRUE, lParam);
	}
	return FALSE;
}

void PS_AddArtistPlayingCount(PCWSTR pszArtist, BOOL bHasHashCode, UINT uHashCode, UINT* puHashCode)
{
	PS_StatArtistFind(pszArtist, bHasHashCode, uHashCode, puHashCode,
		[](void* p, BOOL bNewItem, LPARAM lParam)
		{
			((ARTISTSLISTUNIT*)p)->uPlayingCount++;
		});
}

void PS_AddArtistPlayingTime(PCWSTR pszArtist, UINT uSecond, BOOL bHasHashCode, UINT uHashCode, UINT* puHashCode)
{
	PS_StatArtistFind(pszArtist, bHasHashCode, uHashCode, puHashCode,
		[](void* p, BOOL bNewItem, LPARAM lParam)
		{
			((ARTISTSLISTUNIT*)p)->uSecond += (UINT)lParam;
			if (bNewItem)
				((ARTISTSLISTUNIT*)p)->uPlayingCount = 1;
		},
		uSecond);
}

BOOL PS_StatSongFind(PCWSTR pszSongName, BOOL bHasHashCode, UINT uHashCode, UINT* puHashCode, STATARTISTSFINDPROC pProc, LPARAM lParam)
{
	if (!g_StatSongs)
		return FALSE;

	SONGSSTATUNIT* p;
	if (QKHTGet(g_StatSongs, pszSongName, -1, &p, bHasHashCode, uHashCode))
	{
		if (p)
		{
			pProc(p, FALSE, lParam);
			return TRUE;
		}
		else
			goto PutNewItem;
	}
	else
	{
	PutNewItem:
		p = new SONGSSTATUNIT;
		ZeroMemory(p, sizeof(SONGSSTATUNIT));
		p->uSingleLoopCount = 1;
		PWSTR psz = new WCHAR[lstrlenW(pszSongName)];
		lstrcpyW(psz, pszSongName);
		QKHTPut(g_StatSongs, psz, -1, p, sizeof(SONGSSTATUNIT), 0, TRUE, puHashCode);
		pProc(p, TRUE, lParam);
	}
	return FALSE;
}

void PS_AddSongPlayingCount(PCWSTR pszSongName, QKARRAY aArtists, BOOL bHasHashCode, UINT uHashCode, UINT* puHashCode)
{
	PS_StatSongFind(pszSongName, bHasHashCode, uHashCode, puHashCode,
		[](void* p, BOOL bNewItem, LPARAM lParam)
		{
			((SONGSSTATUNIT*)p)->uPlayingCount++;
			if (bNewItem)
			{
				((SONGSSTATUNIT*)p)->aArtists = (QKARRAY)lParam;
				((SONGSSTATUNIT*)p)->bNeedFreeArray = FALSE;
			}
		},
		(LPARAM)aArtists);
}

void PS_AddSongPlayingTime(PCWSTR pszSongName, UINT uSecond, BOOL bHasHashCode, UINT uHashCode, UINT* puHashCode)
{
	PS_StatSongFind(pszSongName, bHasHashCode, uHashCode, puHashCode,
		[](void* p, BOOL bNewItem, LPARAM lParam)
		{
			((SONGSSTATUNIT*)p)->uSecond += (UINT)lParam;
			if (bNewItem)
				((SONGSSTATUNIT*)p)->uPlayingCount = 1;
		},
		uSecond);
}

void PS_AddSongSingleLoop(PCWSTR pszSongName, SYSTEMTIME* pst, BOOL bHasHashCode, UINT uHashCode, UINT* puHashCode)
{
	PS_StatSongFind(pszSongName, bHasHashCode, uHashCode, puHashCode,
		[](void* p, BOOL bNewItem, LPARAM lParam)
		{
			((SONGSSTATUNIT*)p)->uSingleLoopCount++;
			((SONGSSTATUNIT*)p)->stSingleLoop = *(SYSTEMTIME*)lParam;
			if (bNewItem)
				((SONGSSTATUNIT*)p)->uPlayingCount = 1;
		},
		(LPARAM)pst);
}

BOOL PS_LoadStatFile(PCWSTR pszDir)
{
	QKHASHTABLE hHT;
	QKHTDelete(g_StatArtists);
 	g_StatArtists = QKHTCreate(200, FALSE, FALSE,
		[](void* p)
		{
			delete[] p;
		},
		[](void* p)
		{
			delete p;
		});
	hHT = g_StatArtists;

	PWSTR ptemp, pszFile = new WCHAR[MAX_PATH];
	if (!pszDir)
		pszDir = g_pszDataDir;
	int iLenDir = lstrlenW(pszDir);
	lstrcpyW(pszFile, pszDir);
	SYSTEMTIME st;
	GetLocalTime(&st);
	ptemp = pszFile + iLenDir;
	wsprintfW(ptemp, FILEPREFIX_STATINFO L"%04hu-%02hu", st.wYear, st.wMonth);
	ptemp += lstrlenW(ptemp);

	DWORD dwBytes;
	ARTISTSFILEHEADER FileHeader;
	ARTISTSLISTUNIT* pItemHeader;
	int iLen;
	PWSTR psz, psz2;
	HANDLE hFile;

	lstrcpyW(ptemp, FILEEXT_STATINFOARTISTS);
	hFile = CreateFileW(pszFile, GENERIC_READ, FILE_SHARE_READ, NULL, OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, NULL);
	if (hFile != INVALID_HANDLE_VALUE)
	{
		if (ReadFile(hFile, &FileHeader, sizeof(FileHeader), &dwBytes, NULL))
			if (memcmp(FileHeader.cHeader, "QKAL", sizeof(FileHeader.cHeader)) == 0)
				while (TRUE)
				{
					pItemHeader = new ARTISTSLISTUNIT;
					if (ReadFile(hFile, pItemHeader, sizeof(ARTISTSLISTUNIT), &dwBytes, NULL))
					{
						if (pItemHeader->uFlags & QKALF_END)
						{
							delete pItemHeader;
							break;
						}
						ReadFile(hFile, &iLen, sizeof(int), &dwBytes, NULL);
						psz = new WCHAR[iLen + 1];
						ReadFile(hFile, psz, (iLen + 1) * sizeof(WCHAR), &dwBytes, NULL);

						QKHTPut(hHT, psz, -1, pItemHeader, sizeof(ARTISTSLISTUNIT), 0);
					}
					else
					{
						delete pItemHeader;
						break;
					}
				}

		CloseHandle(hFile);
	}




	QKHTDelete(g_StatSongs);
	g_StatSongs = QKHTCreate(200, FALSE, FALSE,
		[](void* p)
		{
			delete[] p;
		},
		[](void* p)
		{
			if (((SONGSSTATUNIT*)p)->bNeedFreeArray)
				QKADelete(((SONGSSTATUNIT*)p)->aArtists, QKADF_DELETEARRAY);
			delete p;
		});
	hHT = g_StatSongs;
	SONGSSTATFILEHEADER FileHeader2;
	SONGSSTATUNIT* pItemHeader2;
	int iArtistsCount;
	lstrcpyW(ptemp, FILEEXT_STATINFOSONGS);
	hFile = CreateFileW(pszFile, GENERIC_READ, FILE_SHARE_READ, NULL, OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, NULL);
	if (hFile != INVALID_HANDLE_VALUE)
	{
		if (ReadFile(hFile, &FileHeader2, sizeof(FileHeader2), &dwBytes, NULL))
			if (memcmp(FileHeader2.cHeader, "QKSL", sizeof(FileHeader2.cHeader)) == 0)
				while (TRUE)
				{
					pItemHeader2 = new SONGSSTATUNIT;
					if (ReadFile(hFile, pItemHeader2, sizeof(SONGSSTATUNIT), &dwBytes, NULL))
					{
						if (pItemHeader2->uFlags & QKALF_END)
						{
							delete pItemHeader2;
							break;
						}
						ReadFile(hFile, &iLen, sizeof(int), &dwBytes, NULL);
						psz = new WCHAR[iLen + 1];
						ReadFile(hFile, psz, (iLen + 1) * sizeof(WCHAR), &dwBytes, NULL);

						ReadFile(hFile, &iArtistsCount, sizeof(int), &dwBytes, NULL);
						pItemHeader2->aArtists = QKACreate(iArtistsCount);
						pItemHeader2->bNeedFreeArray = TRUE;
						for (int i = 0; i < iArtistsCount; ++i)
						{
							ReadFile(hFile, &iLen, sizeof(int), &dwBytes, NULL);
							psz2 = new WCHAR[iLen + 1];
							ReadFile(hFile, psz2, (iLen + 1) * sizeof(WCHAR), &dwBytes, NULL);
							QKASet(pItemHeader2->aArtists, i, psz2);
						}

						QKHTPut(hHT, psz, -1, pItemHeader2, sizeof(SONGSSTATUNIT), 0);
					}
					else
					{
						delete pItemHeader2;
						break;
					}
				}

		CloseHandle(hFile);
	}

	delete[] pszFile;
	return TRUE;
}

void PS_SaveStatFile(PCWSTR pszDir)
{
	if (!pszDir)
		pszDir = g_pszDataDir;

	if (!g_StatArtists)
		return;


	PWSTR pszFile = new WCHAR[MAX_PATH];
	lstrcpyW(pszFile, pszDir);
	PWSTR ptemp = pszFile + lstrlenW(pszDir);
	ARTISTSFILEHEADER Header = { {'Q','K','A','L'},0,QKAFVER_1 };
	GetLocalTime(&Header.st);
	wsprintfW(ptemp, FILEPREFIX_STATINFO L"%04hd-%02hd", Header.st.wYear, Header.st.wMonth);
	ptemp += lstrlenW(ptemp);


	lstrcpyW(ptemp, FILEEXT_STATINFOARTISTS);
	HANDLE hFile = CreateFileW(pszFile, GENERIC_WRITE, FILE_SHARE_READ, NULL, CREATE_ALWAYS, FILE_ATTRIBUTE_NORMAL, NULL);
	DWORD dwBytes;

	WriteFile(hFile, &Header, sizeof(Header), &dwBytes, NULL);

	QKHTEnum(g_StatArtists,
		[](QKHASHTABLELINKNODE* p, LPARAM lParam)->BOOL
		{
			DWORD dwBytes;
			WriteFile((HANDLE)lParam, p->pValue, sizeof(ARTISTSLISTUNIT), &dwBytes, NULL);
			int iLen = lstrlenW((PCWSTR)p->pKey);
			WriteFile((HANDLE)lParam, &iLen, sizeof(int), &dwBytes, NULL);
			WriteFile((HANDLE)lParam, p->pKey, (iLen + 1) * sizeof(WCHAR), &dwBytes, NULL);
			return TRUE;
		},
		(LPARAM)hFile);

	ARTISTSLISTUNIT End = { QKALF_END };
	WriteFile(hFile, &End, sizeof(End), &dwBytes, NULL);
	CloseHandle(hFile);
	///////////////
	SONGSSTATFILEHEADER Header2 = { {'Q','K','S','L'},0,QKSFVER_1,Header.st };
	lstrcpyW(ptemp, FILEEXT_STATINFOSONGS);
	hFile = CreateFileW(pszFile, GENERIC_WRITE, FILE_SHARE_READ, NULL, CREATE_ALWAYS, FILE_ATTRIBUTE_NORMAL, NULL);

	WriteFile(hFile, &Header2, sizeof(Header2), &dwBytes, NULL);

	QKHTEnum(g_StatSongs,
		[](QKHASHTABLELINKNODE* p, LPARAM lParam)->BOOL
		{
			HANDLE hFile = (HANDLE)lParam;
			DWORD dwBytes;
			WriteFile(hFile, p->pValue, sizeof(SONGSSTATUNIT), &dwBytes, NULL);
			int iLen = lstrlenW((PCWSTR)p->pKey);
			WriteFile(hFile, &iLen, sizeof(int), &dwBytes, NULL);
			WriteFile(hFile, p->pKey, (iLen + 1) * sizeof(WCHAR), &dwBytes, NULL);

			auto pUnit = (SONGSSTATUNIT*)(p->pValue);
			WriteFile(hFile, &(pUnit->aArtists->iCount), sizeof(int), &dwBytes, NULL);

			PCWSTR pszArtist;
			for (int i = 0; i < pUnit->aArtists->iCount; ++i)
			{
				pszArtist = (PCWSTR)QKAGet(pUnit->aArtists, i);
				iLen = lstrlenW(pszArtist);
				WriteFile(hFile, &iLen, sizeof(int), &dwBytes, NULL);
				WriteFile(hFile, pszArtist, (iLen + 1) * sizeof(WCHAR), &dwBytes, NULL);
			}
			return TRUE;
		},
		(LPARAM)hFile);

	SONGSSTATUNIT End2 = { QKALF_END };
	WriteFile(hFile, &End2, sizeof(End2), &dwBytes, NULL);
	CloseHandle(hFile);

	delete[] pszFile;
}

void UI_FillStatDlg(HWND hDlg)
{
	/////////////////准备
	HWND hLV;

	int iLenDir = lstrlenW(g_pszDataDir);
	PWSTR pszFile = new WCHAR[MAX_PATH];
	PWSTR ptemp = pszFile + iLenDir;// 这个指针指到目录后面，方便替换文件名
	lstrcpyW(pszFile, g_pszDataDir);

	SYSTEMTIME st;
	GetLocalTime(&st);// 取现行时间

	LVINSERTGROUPSORTED lvigs;
	lvigs =
	{
		// 如果Group1_ID的数据小于Group2_ID的数据，则返回负值；如果大于，则返回正值；如果相同，则返回零。（MSDN）
		[](int Arg1, int Arg2, void* pData)->int
		{
			WORD wYear1 = LOWORD(Arg1),wYear2 = LOWORD(Arg2);
			WORD wMonth1 = HIWORD(Arg1), wMonth2 = HIWORD(Arg2);
			if (wYear1 == wYear2)
			{
				if (wMonth1 > wMonth2)
					return +1;
				else if (wMonth1 < wMonth2)
					return -1;
				else
					return 0;
			}
			else if (wYear1 > wYear2)
				return +1;
			else
				return -1;
		},
		NULL,// 回调lParam
		{ sizeof(LVGROUP) }// 初始化LVGROUP
	};
	lvigs.lvGroup.mask = LVGF_HEADER/*组标题*/ | LVGF_GROUPID | LVGF_STATE/*给组加可折叠用*/;
	lvigs.lvGroup.state = LVGS_COLLAPSIBLE;// 可折叠

	WCHAR szGroupTitleBuf[MAX_DATA_BUFSIZE + 36];// 组标题缓冲区
	WCHAR szValueBuf[36];// 数值缓冲区，包括播放时长

	LVITEMW li;
	li.mask = LVIF_TEXT;

	int iItemCount;// 记录已经插入的个数，这样就不用每次插入都获取项数了
	WIN32_FIND_DATAW wfd;
	HANDLE hFind;
	HANDLE hFile;
	DWORD dwBytes;
	/////////////////处理艺术家统计信息
	if (g_StatArtists)
	{
		hLV = GetDlgItem(hDlg, IDC_LV_STATARTISTS);
		SendMessageW(hLV, WM_SETREDRAW, FALSE, 0);
		SendMessageW(hLV, LVM_DELETEALLITEMS, 0, 0);
		SendMessageW(hLV, LVM_REMOVEALLGROUPS, 0, 0);
		iItemCount = 0;

		lstrcpyW(ptemp, L"*" FILEEXT_STATINFOARTISTS);// 枚举艺术家记录文件
		hFind = FindFirstFileW(pszFile, &wfd);
		if (hFind != INVALID_HANDLE_VALUE)
		{
			ARTISTSFILEHEADER FileHeader;
			ARTISTSLISTUNIT ItemHeader = { 0 };
			PWSTR pszArtist;
			int iLenArtist;

			do
			{
				lstrcpyW(ptemp, wfd.cFileName);
				hFile = CreateFileW(pszFile, GENERIC_READ, FILE_SHARE_READ, NULL, OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, NULL);
				if (hFile != INVALID_HANDLE_VALUE)
				{
					if (!ReadFile(hFile, &FileHeader, sizeof(FileHeader), &dwBytes, NULL))
					{
						CloseHandle(hFile);
						continue;
					}
					if (memcmp(FileHeader.cHeader, "QKAL", sizeof(FileHeader.cHeader)) == 0)
					{
						if (st.wYear != FileHeader.st.wYear &&
							st.wMonth != FileHeader.st.wMonth)// 当前月的不读取
						{
							wsprintfW(szGroupTitleBuf, L"%04hd-%02hd", FileHeader.st.wYear, FileHeader.st.wMonth);
							lvigs.lvGroup.pszHeader = szGroupTitleBuf;
							lvigs.lvGroup.iGroupId = MAKELONG(FileHeader.st.wYear, FileHeader.st.wMonth);
							SendMessageW(hLV, LVM_INSERTGROUPSORTED, (WPARAM)&lvigs, 0);
							li.iGroupId = lvigs.lvGroup.iGroupId;
							while (TRUE)
							{
								if (ReadFile(hFile, &ItemHeader, sizeof(ItemHeader), &dwBytes, NULL))
								{
									if (ItemHeader.uFlags & QKALF_END)
										break;
									li.mask |= LVIF_GROUPID;
									ReadFile(hFile, &iLenArtist, sizeof(int), &dwBytes, NULL);
									li.pszText = new WCHAR[iLenArtist + 1];
									ReadFile(hFile, li.pszText, (iLenArtist + 1) * sizeof(WCHAR), &dwBytes, NULL);
									li.iItem = iItemCount;
									li.iSubItem = 0;
									SendMessageW(hLV, LVM_INSERTITEMW, 0, (LPARAM)&li);
									li.mask &= (~LVIF_GROUPID);

									wsprintfW(szValueBuf, L"%u", ItemHeader.uPlayingCount);
									li.pszText = szValueBuf;
									li.iSubItem = 1;
									SendMessageW(hLV, LVM_SETITEM, 0, (LPARAM)&li);

									wsprintfW(szValueBuf, L"%02u:%02u:%02u",
										ItemHeader.uSecond / 3600,
										(ItemHeader.uSecond % 3600) / 60,
										(ItemHeader.uSecond % 3600) % 60);
									li.pszText = szValueBuf;
									li.iSubItem = 2;
									SendMessageW(hLV, LVM_SETITEM, 0, (LPARAM)&li);

									delete[] li.pszText;
									iItemCount++;
								}
								else
									break;
							}
						}
					}
					CloseHandle(hFile);
				}
			} while (FindNextFileW(hFind, &wfd));
			FindClose(hFind);

		}
		if (g_StatArtists)
		{
			wsprintfW(szGroupTitleBuf, L"%04hd-%02hd", st.wYear, st.wMonth);
			lvigs.lvGroup.pszHeader = szGroupTitleBuf;
			lvigs.lvGroup.iGroupId = MAKELONG(st.wYear, st.wMonth);
			SendMessageW(hLV, LVM_INSERTGROUPSORTED, (WPARAM)&lvigs, 0);
			li.iGroupId = lvigs.lvGroup.iGroupId;
			struct TEMP_READLISTCONTEXT
			{
				HWND hLV;
				int iItemCount;
				LVITEMW* pli;
			} Context =
			{ hLV,iItemCount,&li };

			QKHTEnum(g_StatArtists,
				[](QKHASHTABLELINKNODE* p, LPARAM lParam)->BOOL
				{
					auto pContext = (TEMP_READLISTCONTEXT*)lParam;
			pContext->pli->mask |= LVIF_GROUPID;
			pContext->pli->pszText = (PWSTR)p->pKey;
			pContext->pli->iItem = pContext->iItemCount;
			pContext->pli->iSubItem = 0;
			SendMessageW(pContext->hLV, LVM_INSERTITEMW, 0, (LPARAM)pContext->pli);
			pContext->iItemCount++;

			WCHAR szValueBuf[36];
			pContext->pli->mask &= (~LVIF_GROUPID);

			auto pUnit = (ARTISTSLISTUNIT*)p->pValue;

			wsprintfW(szValueBuf, L"%u", pUnit->uPlayingCount);
			pContext->pli->pszText = szValueBuf;
			pContext->pli->iSubItem = 1;
			SendMessageW(pContext->hLV, LVM_SETITEM, 0, (LPARAM)pContext->pli);

			wsprintfW(szValueBuf, L"%02u:%02u:%02u",
				pUnit->uSecond / 3600,
				(pUnit->uSecond % 3600) / 60,
				(pUnit->uSecond % 3600) % 60);
			pContext->pli->pszText = szValueBuf;
			pContext->pli->iSubItem = 2;
			SendMessageW(pContext->hLV, LVM_SETITEM, 0, (LPARAM)pContext->pli);
			return TRUE;
				},
				(LPARAM)&Context);
		}
		SendMessageW(hLV, WM_SETREDRAW, TRUE, 0);
	}
	/////////////////处理歌曲统计信息
	if (g_StatSongs)
	{
		hLV = GetDlgItem(hDlg, IDC_LV_STATSONGS);
		SendMessageW(hLV, WM_SETREDRAW, FALSE, 0);
		SendMessageW(hLV, LVM_DELETEALLITEMS, 0, 0);
		SendMessageW(hLV, LVM_REMOVEALLGROUPS, 0, 0);
		iItemCount = 0;

		QKHTDelete(m_SongsStatWithArtists);
		m_SongsStatWithArtists = QKHTCreate(200, FALSE, FALSE,
			[](void* p)
			{
				delete[] p;
			},
			[](void* p)
			{
				QKADelete((QKARRAY)p);
			});

		lstrcpyW(ptemp, L"*" FILEEXT_STATINFOSONGS);// 枚举艺术家记录文件
		hFind = FindFirstFileW(pszFile, &wfd);
		if (hFind != INVALID_HANDLE_VALUE)
		{
			SONGSSTATFILEHEADER FileHeader;
			SONGSSTATUNIT ItemHeader = { 0 };
			PWSTR pszArtist, pszSong;
			int iLenArtist, iLenSong;
			int iIndex;
			int iArtistsCount;
			QKARRAY hA;
			do
			{
				lstrcpyW(ptemp, wfd.cFileName);
				hFile = CreateFileW(pszFile, GENERIC_READ, FILE_SHARE_READ, NULL, OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, NULL);
				if (hFile != INVALID_HANDLE_VALUE)
				{
					if (!ReadFile(hFile, &FileHeader, sizeof(FileHeader), &dwBytes, NULL))
					{
						CloseHandle(hFile);
						continue;
					}
					if (memcmp(FileHeader.cHeader, FILEMAGIC_SONGSFILEHEDAER, sizeof(FileHeader.cHeader)) == 0)// 对比文件头
					{
						if (st.wYear != FileHeader.st.wYear &&
							st.wMonth != FileHeader.st.wMonth)// 当前月的不读取
						{
							wsprintfW(szGroupTitleBuf, L"%04hd-%02hd(%d)", FileHeader.st.wYear, FileHeader.st.wMonth, FileHeader.iCount);// 制组标题
							lvigs.lvGroup.pszHeader = szGroupTitleBuf;
							lvigs.lvGroup.iGroupId = MAKELONG(FileHeader.st.wYear, FileHeader.st.wMonth);// 高WORD=月，低WORD=年
							SendMessageW(hLV, LVM_INSERTGROUPSORTED, (WPARAM)&lvigs, 0);
							li.iGroupId = lvigs.lvGroup.iGroupId;
							while (TRUE)
							{
								if (ReadFile(hFile, &ItemHeader, sizeof(ItemHeader), &dwBytes, NULL))
								{
									if (ItemHeader.uFlags & QKSLF_END)
										break;
									li.mask |= LVIF_GROUPID;
									////////////////读名称
									ReadFile(hFile, &iLenSong, sizeof(int), &dwBytes, NULL);
									li.pszText = new WCHAR[iLenSong + 1];
									ReadFile(hFile, li.pszText, (iLenSong + 1) * sizeof(WCHAR), &dwBytes, NULL);

									li.iItem = iItemCount;
									li.iSubItem = 0;
									iIndex = SendMessageW(hLV, LVM_INSERTITEMW, 0, (LPARAM)&li);// 插入项
									li.mask &= (~LVIF_GROUPID);
									////////////////艺术家存入哈希表
									ReadFile(hFile, &iArtistsCount, sizeof(int), &dwBytes, NULL);
									for (int i = 0; i < iArtistsCount; ++i)
									{
										ReadFile(hFile, &iLenArtist, sizeof(int), &dwBytes, NULL);
										pszArtist = new WCHAR[iLenArtist + 1];
										ReadFile(hFile, pszArtist, (iLenArtist + 1) * sizeof(WCHAR), &dwBytes, NULL);

										if (QKHTGet(m_SongsStatWithArtists, pszArtist, -1, &hA))
										{
											QKAAddValue(hA, iIndex);
										}
										else
										{
											hA = QKACreate(0);
											QKAAddValue(hA, iIndex);
											QKHTPut(m_SongsStatWithArtists, pszArtist, -1, hA, sizeof(QKARRAY), 0);
										}
									}
									////////////////播放次数
									wsprintfW(szValueBuf, L"%u", ItemHeader.uPlayingCount);
									li.pszText = szValueBuf;
									li.iSubItem = 1;
									SendMessageW(hLV, LVM_SETITEM, 0, (LPARAM)&li);
									////////////////播放时长
									wsprintfW(szValueBuf, L"%02u:%02u:%02u",
										ItemHeader.uSecond / 3600,
										(ItemHeader.uSecond % 3600) / 60,
										(ItemHeader.uSecond % 3600) % 60);
									li.pszText = szValueBuf;
									li.iSubItem = 2;
									SendMessageW(hLV, LVM_SETITEM, 0, (LPARAM)&li);
									////////////////最大循环次数
									wsprintfW(szValueBuf, L"%u", ItemHeader.uSingleLoopCount);
									li.pszText = szValueBuf;
									li.iSubItem = 3;
									SendMessageW(hLV, LVM_SETITEM, 0, (LPARAM)&li);

									delete[] li.pszText;
									iItemCount++;
								}
								else
									break;
							}
						}
					}
					CloseHandle(hFile);
				}
			} while (FindNextFileW(hFind, &wfd));
			FindClose(hFind);

		}

		if (g_StatSongs)
		{
			wsprintfW(szGroupTitleBuf, L"%04hd-%02hd", st.wYear, st.wMonth);
			lvigs.lvGroup.pszHeader = szGroupTitleBuf;
			lvigs.lvGroup.iGroupId = MAKELONG(st.wYear, st.wMonth);
			SendMessageW(hLV, LVM_INSERTGROUPSORTED, (WPARAM)&lvigs, 0);
			li.iGroupId = lvigs.lvGroup.iGroupId;
			struct TEMP_READLISTCONTEXT
			{
				HWND hLV;
				int iItemCount;
				LVITEMW* pli;
			} Context =
			{ hLV,iItemCount,&li };

			QKHTEnum(g_StatSongs,
				[](QKHASHTABLELINKNODE* p, LPARAM lParam)->BOOL
				{
					auto pContext = (TEMP_READLISTCONTEXT*)lParam;
					pContext->pli->mask |= LVIF_GROUPID;
					pContext->pli->pszText = (PWSTR)p->pKey;
					pContext->pli->iItem = pContext->iItemCount;
					pContext->pli->iSubItem = 0;
					int iIndex = SendMessageW(pContext->hLV, LVM_INSERTITEMW, 0, (LPARAM)pContext->pli);
					pContext->iItemCount++;

					WCHAR szValueBuf[36];
					pContext->pli->mask &= (~LVIF_GROUPID);

					auto pUnit = (SONGSSTATUNIT*)(p->pValue);
					QKARRAY hA;
					PCVOID pszArtist;
					for (int i = 0; i < pUnit->aArtists->iCount; ++i)
					{
						pszArtist = QKAGet(pUnit->aArtists, i);
						if (QKHTGet(m_SongsStatWithArtists, pszArtist, -1, &hA))
						{
							QKAAddValue(hA, iIndex);
						}
						else
						{
							hA = QKACreate(0);
							QKAAddValue(hA, iIndex);
							QKHTPut(m_SongsStatWithArtists, pszArtist, -1, hA, sizeof(QKARRAY), 0);
						}
					}
					////////////////播放次数
					wsprintfW(szValueBuf, L"%u", pUnit->uPlayingCount);
					pContext->pli->pszText = szValueBuf;
					pContext->pli->iSubItem = 1;
					SendMessageW(pContext->hLV, LVM_SETITEM, 0, (LPARAM)pContext->pli);
					////////////////播放时长
					wsprintfW(szValueBuf, L"%02u:%02u:%02u",
						pUnit->uSecond / 3600,
						(pUnit->uSecond % 3600) / 60,
						(pUnit->uSecond % 3600) % 60);
					pContext->pli->pszText = szValueBuf;
					pContext->pli->iSubItem = 2;
					SendMessageW(pContext->hLV, LVM_SETITEM, 0, (LPARAM)pContext->pli);
					////////////////最大循环次数
					wsprintfW(szValueBuf, L"%u", pUnit->uSingleLoopCount);
					pContext->pli->pszText = szValueBuf;
					pContext->pli->iSubItem = 3;
					SendMessageW(pContext->hLV, LVM_SETITEM, 0, (LPARAM)pContext->pli);
					return TRUE;
				},
				(LPARAM)&Context);
		}
		SendMessageW(hLV, WM_SETREDRAW, TRUE, 0);
	}


	delete[] pszFile;
}

INT_PTR CALLBACK DlgProc_PlayingStat(HWND hDlg, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
	switch (uMsg)
	{
	case WM_INITDIALOG:
	{
		HWND hLV;

		LVCOLUMNW lc;
		lc.mask = LVCF_TEXT | LVCF_WIDTH | LVCF_FMT;

		hLV = GetDlgItem(hDlg, IDC_LV_STATARTISTS);
		lc.pszText = (PWSTR)L"艺术家";
		lc.cx = DPI(150);
		lc.fmt = LVCFMT_LEFT;
		SendMessageW(hLV, LVM_INSERTCOLUMNW, 0, (LPARAM)&lc);
		lc.pszText = (PWSTR)L"播放次数";
		lc.cx = DPI(60);
		lc.fmt = LVCFMT_RIGHT;
		SendMessageW(hLV, LVM_INSERTCOLUMNW, 1, (LPARAM)&lc);
		lc.pszText = (PWSTR)L"播放总时间";
		lc.cx = DPI(80);
		lc.fmt = LVCFMT_RIGHT;
		SendMessageW(hLV, LVM_INSERTCOLUMNW, 2, (LPARAM)&lc);

		SendMessageW(hLV, LVM_ENABLEGROUPVIEW, TRUE, 0);


		hLV = GetDlgItem(hDlg, IDC_LV_STATSONGS);
		lc.pszText = (PWSTR)L"名称";
		lc.cx = DPI(150);
		lc.fmt = LVCFMT_LEFT;
		SendMessageW(hLV, LVM_INSERTCOLUMNW, 0, (LPARAM)&lc);
		lc.pszText = (PWSTR)L"播放次数";
		lc.cx = DPI(60);
		lc.fmt = LVCFMT_RIGHT;
		SendMessageW(hLV, LVM_INSERTCOLUMNW, 1, (LPARAM)&lc);
		lc.pszText = (PWSTR)L"播放总时间";
		lc.cx = DPI(80);
		lc.fmt = LVCFMT_RIGHT;
		SendMessageW(hLV, LVM_INSERTCOLUMNW, 2, (LPARAM)&lc);
		lc.pszText = (PWSTR)L"最多循环次数";
		lc.cx = DPI(80);
		lc.fmt = LVCFMT_RIGHT;
		SendMessageW(hLV, LVM_INSERTCOLUMNW, 3, (LPARAM)&lc);

		SendMessageW(hLV, LVM_ENABLEGROUPVIEW, TRUE, 0);

		UI_FillStatDlg(hDlg);
	}
	return FALSE;

	case WM_CLOSE:
		EndDialog(hDlg, 0);
		return TRUE;
	}
	return FALSE;
}