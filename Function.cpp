/*
* Function.cpp
* ���������Ժ�����ʵ��
*/
#include <Windows.h>
#include <CommCtrl.h>
#include <shlobj_core.h>
#include <shlwapi.h>

#include <math.h>
#include <assert.h>

#include "MyProject.h"
#include "Function.h"
#include "GlobalVar.h"
#include "resource.h"
#include "WndMain.h"

HFONT QKCreateFont(PCWSTR pszFontName, int nPoint, int nWeight, BOOL IsItalic, BOOL IsUnderline, BOOL IsStrikeOut)
{
	// for the MM_TEXT mapping mode, you can use the following formula to specify a height for a font with a specified point size:
	// MM_TEXTӳ��ģʽ�¿������¹�ʽ������ָ����ֵ����ĸ߶ȣ�
	// Height = -MulDiv(PointSize, GetDeviceCaps(hDC, LOGPIXELSY), 72);
	HDC hDC = GetDC(NULL);
	int iSize;
	iSize = -MulDiv(nPoint, GetDeviceCaps(hDC, LOGPIXELSY), 72);
	ReleaseDC(NULL, hDC);
	return CreateFontW(iSize, 0, 0, 0, nWeight, IsItalic, IsUnderline, IsStrikeOut, 0, 0, 0, 0, 0, pszFontName);
}
/////////////////////////////////////�����

QKARRAY QKACreate(int iCount, int iGrow, HANDLE hHeap, QKARYDELPROC pDelProc)
{
	assert(iCount >= 0);
	assert(iGrow >= 0);
	if (!hHeap)
		hHeap = GetProcessHeap();

	auto pArray = (QKARRAYHEADER*)HeapAlloc(hHeap, HEAP_ZERO_MEMORY, sizeof(QKARRAYHEADER));
	if (!pArray)
		return NULL;

	pArray->iCount = iCount;
	pArray->iGrow = iGrow;
	pArray->hHeap = hHeap;
	pArray->pProc = pDelProc;
	pArray->iCountAllocated = iCount + pArray->iGrow;
	if (pArray->iCountAllocated <= 0)
		pArray->iCountAllocated = 1;

	pArray->pData = (BYTE*)HeapAlloc(hHeap, HEAP_ZERO_MEMORY, PTRSIZE * pArray->iCountAllocated);
	if (!pArray->pData)
	{
		HeapFree(hHeap, 0, pArray);
		return NULL;
	}

	return pArray;
}
void QKADelete(QKARRAY pArray, UINT uDeleteFlag)
{
	assert(pArray);
	switch (uDeleteFlag)
	{
	case QKADF_NO:
		break;

	case QKADF_DELETE:
		for (int i = 0; i < pArray->iCount; ++i)
			delete QKAGet(pArray, i);
		break;

	case QKADF_DELETEARRAY:
		for (int i = 0; i < pArray->iCount; ++i)
			delete[] QKAGet(pArray, i);
		break;

	case QKADF_HEAPFREE:
		for (int i = 0; i < pArray->iCount; ++i)
			HeapFree(pArray->hHeap, 0, QKAGet(pArray, i));
		break;

	case QKADF_DELPROC:
		if (pArray->pProc)
		{
			for (int i = 0; i < pArray->iCount; ++i)
				pArray->pProc(QKAGet(pArray, i));
		}
		break;
	}
	HeapFree(pArray->hHeap, 0, pArray->pData);
	HeapFree(pArray->hHeap, 0, pArray);
}
void QKAClear(QKARRAY pArray, UINT uDeleteFlag)
{
	assert(pArray);
	switch (uDeleteFlag)
	{
	case QKADF_NO:
		break;

	case QKADF_DELETE:
		for (int i = 0; i < pArray->iCount; ++i)
			delete QKAGet(pArray, i);
		break;

	case QKADF_DELETEARRAY:
		for (int i = 0; i < pArray->iCount; ++i)
			delete[] QKAGet(pArray, i);
		break;

	case QKADF_HEAPFREE:
		for (int i = 0; i < pArray->iCount; ++i)
			HeapFree(pArray->hHeap, 0, QKAGet(pArray, i));
		break;

	case QKADF_DELPROC:
		if (pArray->pProc)
			for (int i = 0; i < pArray->iCount; ++i)
				pArray->pProc(QKAGet(pArray, i));
		break;
	}
	HeapFree(pArray->hHeap, 0, pArray->pData);
	pArray->pData = (BYTE*)HeapAlloc(pArray->hHeap, HEAP_ZERO_MEMORY, PTRSIZE * pArray->iGrow);
	pArray->iCount = 0;
	pArray->iCountAllocated = pArray->iGrow;
}
int QKAAdd(QKARRAY pArray, void* pData)
{
	assert(pArray);
	assert(pArray->iCountAllocated >= pArray->iCount);
	if (pArray->iCountAllocated == pArray->iCount)
	{
		pArray->iCountAllocated += pArray->iGrow;
		BYTE* pData = (BYTE*)HeapReAlloc(pArray->hHeap, HEAP_ZERO_MEMORY,
			pArray->pData, PTRSIZE * pArray->iCountAllocated);
		if (!pData)
			return -1;
		pArray->pData = pData;
	}
	*(void**)(pArray->pData + PTRSIZE * pArray->iCount) = pData;
	return pArray->iCount++;
}
int QKAAddValue(QKARRAY pArray, UINT uValue)
{
	assert(pArray);
	assert(pArray->iCountAllocated >= pArray->iCount);
	if (pArray->iCountAllocated == pArray->iCount)
	{
		pArray->iCountAllocated += pArray->iGrow;
		BYTE* pData = (BYTE*)HeapReAlloc(pArray->hHeap, HEAP_ZERO_MEMORY,
			pArray->pData, PTRSIZE * pArray->iCountAllocated);
		if (!pData)
			return -1;
		pArray->pData = pData;
	}
	*(UINT*)(pArray->pData + PTRSIZE * pArray->iCount) = uValue;
	return pArray->iCount++;
}
int QKAInsert(QKARRAY pArray, void* pData, int iIndex)
{
	assert(pArray);
	if (iIndex == pArray->iCount)
		return QKAAdd(pArray, pData);
	assert(pArray->iCountAllocated >= pArray->iCount);
	assert(iIndex >= 0 && iIndex < pArray->iCount);
	if (pArray->iCountAllocated == pArray->iCount)
	{
		pArray->iCountAllocated += pArray->iGrow;
		BYTE* pData = (BYTE*)HeapReAlloc(pArray->hHeap, HEAP_ZERO_MEMORY,
			pArray->pData, PTRSIZE * pArray->iCountAllocated);
		if (!pData)
			return -1;
		pArray->pData = pData;
	}

	BYTE* pChanged = pArray->pData + PTRSIZE * iIndex;
	memmove(pChanged + PTRSIZE, pChanged, PTRSIZE * (pArray->iCount - iIndex));
	*(void**)pChanged = pData;
	pArray->iCount++;
	return iIndex;
}
void QKASet(QKARRAY pArray, int iIndex, void* pData, UINT uDeleteFlag)
{
	assert(pArray);
	assert(iIndex >= 0 && iIndex < pArray->iCount);

	void* pOld = QKAGet(pArray, iIndex);
	switch (uDeleteFlag)
	{
	case QKADF_NO:
		break;

	case QKADF_DELETE:
		delete pOld;
		break;

	case QKADF_DELETEARRAY:
		delete[] pOld;
		break;

	case QKADF_HEAPFREE:
		HeapFree(pArray->hHeap, 0, pOld);
		break;

	case QKADF_DELPROC:
		if (pArray->pProc)
			pArray->pProc(pOld);
		break;
	}

	*(void**)(pArray->pData + PTRSIZE * iIndex) = pData;
}
BOOL QKADeleteMember(QKARRAY pArray, int iIndex, UINT uDeleteFlag, int iDelCount)
{
	assert(pArray);
	int iEnd = iIndex + iDelCount;
	switch (uDeleteFlag)
	{
	case QKADF_NO:
		break;

	case QKADF_DELETE:
		for (int i = iIndex; i < iEnd; ++i)
			delete QKAGet(pArray, i);
		break;

	case QKADF_DELETEARRAY:
		for (int i = iIndex; i < iEnd; ++i)
			delete[] QKAGet(pArray, i);
		break;

	case QKADF_HEAPFREE:
		for (int i = iIndex; i < iEnd; ++i)
			HeapFree(pArray->hHeap, 0, QKAGet(pArray, i));
		break;

	case QKADF_DELPROC:
		if (pArray->pProc)
			for (int i = iIndex; i < iEnd; ++i)
				pArray->pProc(QKAGet(pArray, i));
		break;
	}

	assert(iIndex + iDelCount <= pArray->iCount);
	BYTE* pSrc = pArray->pData + PTRSIZE * iEnd, 
		* pDst = pArray->pData + PTRSIZE * iIndex;
	if (iIndex + iDelCount != pArray->iCount)// �ж���û��ɾ��β��ûɾ��β���ƶ��ڴ�
		memmove(pDst, pSrc, PTRSIZE * (pArray->iCount - iEnd));

	// �жϿճ���λ����û�г���Ԥ�����С����ֻɾ��һ��
	if (pArray->iCountAllocated - pArray->iCount + iDelCount >= pArray->iGrow && iDelCount != 1)
	{
		pArray->iCountAllocated = pArray->iCount + pArray->iGrow;// ������Ԥ�����С����ô�ط�����ڴ���С
		BYTE* pData = (BYTE*)HeapReAlloc(pArray->hHeap, 0, pArray->pData, PTRSIZE * pArray->iCountAllocated);
		if (!pData)
			return FALSE;
		pArray->pData = pData;
	}
	else
		pArray->iCountAllocated += iDelCount;// û��������ֻɾ��һ�����ط���
	pArray->iCount -= iDelCount;
	return TRUE;
}
BOOL QKATrimSize(QKARRAY pArray, BOOL bReserveGrowSpace)
{
	assert(pArray);
	assert(pArray->iCountAllocated >= pArray->iCount);
	BYTE* pData;
	if (bReserveGrowSpace)
	{
		if (pArray->iCountAllocated - pArray->iCount > pArray->iGrow)
		{
			pArray->iCountAllocated = pArray->iCount + pArray->iGrow;
			goto ReAllocArray;
		}
	}
	else
	{
		if (pArray->iCountAllocated > pArray->iCount)
		{
			pArray->iCountAllocated = pArray->iCount;
			goto ReAllocArray;
		}
	}
	return FALSE;
ReAllocArray:
	pData = (BYTE*)HeapReAlloc(pArray->hHeap, 0, pArray->pData, PTRSIZE * pArray->iCountAllocated);
	if (!pData)
		return FALSE;
	pArray->pData = pData;
	return TRUE;
}
/////////////////////////////////////�����

INT_PTR CALLBACK DlgProc_InputBox(HWND hDlg, UINT message, WPARAM wParam, LPARAM lParam)
{
	QKINPUTBOXCOMTEXT* pContext = NULL;
	pContext = (QKINPUTBOXCOMTEXT*)GetPropW(hDlg, PROP_INPUTBOXCONTEXT);
	switch (message)
	{
	case WM_INITDIALOG:
	{
		pContext = (QKINPUTBOXCOMTEXT*)lParam;
		SetPropW(hDlg, PROP_INPUTBOXCONTEXT, pContext);
		SetWindowTextW(hDlg, pContext->pszTitle);
		SetDlgItemTextW(hDlg, IDC_ST_INPUT, pContext->pszTip);
		SetDlgItemTextW(hDlg, IDC_ED_INPUT, *(pContext->ppszBuffer));
		SetFocus(GetDlgItem(hDlg, IDC_ED_INPUT));
	}
	return FALSE;
	case WM_COMMAND:
	{
		if (LOWORD(wParam) == IDOK)
		{
			pContext->iButton = IDOK;
			int iBufferSize = GetWindowTextLengthW(GetDlgItem(hDlg, IDC_ED_INPUT)) + 1;
			*(pContext->ppszBuffer) = new WCHAR[iBufferSize];
			GetWindowTextW(GetDlgItem(hDlg, IDC_ED_INPUT), *(pContext->ppszBuffer), iBufferSize);
			EndDialog(hDlg, 0);
		}
		else if (LOWORD(wParam) == IDCANCEL)
		{
			pContext->iButton = IDCANCEL;
			EndDialog(hDlg, 0);
		}
	}
	return TRUE;
	case WM_CLOSE:
	{
		pContext->iButton = IDCANCEL;
		EndDialog(hDlg, 0);
	}
	return TRUE;
	}
	return FALSE;
}
BOOL QKInputBox(PCWSTR pszTitle, PCWSTR pszTip, PWSTR* ppszBuffer, HWND hParent)
{
	QKINPUTBOXCOMTEXT Context;
	Context.pszTitle = pszTitle;
	Context.pszTip = pszTip;
	Context.ppszBuffer = ppszBuffer;
	DialogBoxParamW(g_hInst, MAKEINTRESOURCEW(IDD_INPUT), hParent, DlgProc_InputBox, (LPARAM)&Context);
	return (Context.iButton == IDOK);
}
int QKStrInStr(PCWSTR pszOrg, PCWSTR pszSubStr, int iStartPos)
{
	int iSubStrCount = lstrlenW(pszSubStr);
	int iCount = lstrlenW(pszOrg) - iSubStrCount + 1 - (iStartPos - 1);
	if (iCount <= 0 || iStartPos <= 0)
		return 0;
	for (int i = 0; i < iCount; i++)
	{
		if (CompareStringW(
			LOCALE_USER_DEFAULT,
			LINGUISTIC_IGNORECASE,
			pszOrg + iStartPos - 1 + i,
			iSubStrCount,
			pszSubStr,
			iSubStrCount
		) == CSTR_EQUAL)
			return iStartPos + i;
	}
	return 0;
}
int QKStrInStrCS(PCWSTR pszOrg, PCWSTR pszSubStr, int iStartPos)
{
	int iSubStrCount = lstrlenW(pszSubStr);
	int iCount = lstrlenW(pszOrg) - iSubStrCount + 1 - (iStartPos - 1);
	if (iCount <= 0 || iStartPos <= 0)
		return 0;
	for (int i = 0; i < iCount; i++)
	{
		if (CompareStringW(
			LOCALE_USER_DEFAULT,
			0,
			pszOrg + iStartPos - 1 + i,
			iSubStrCount,
			pszSubStr,
			iSubStrCount
		) == CSTR_EQUAL)
			return iStartPos + i;
	}
	return 0;
}
void QKStrTrim(PWSTR pszOrg)
{

	int i = 0, j = 0;
	while (pszOrg[i] == ' ' || pszOrg[i] == '��' || pszOrg[i] == '\t')
		++i;
	j = lstrlenW(pszOrg) - 1;
	while (pszOrg[j] == ' ' || pszOrg[j] == '��' || pszOrg[j] == '\t')
	{
		if (j == 0)
			break;
		--j;
	}
	++j;
	pszOrg[j] = '\0';
	PWSTR p = pszOrg + i;
	if (i)
		while (*pszOrg++ = *p++);
}
UINT QKMessageBox(PCWSTR pszMainInstruction, PCWSTR pszContent, HICON hIcon, PCWSTR pszWndTitle, HWND hParent, PCWSTR pszChackBoxTitle, UINT iButtonCount,
	PCWSTR pszButton1Title, PCWSTR pszButton2Title, PCWSTR pszButton3Title, UINT iDefButton, BOOL IsCenterPos, BOOL* IsCheck
)
{
	if (iButtonCount < 1 && iButtonCount >3)
	{
		return 0;
	}
	TASKDIALOGCONFIG tdc = { 0 };
	tdc.cbSize = sizeof(TASKDIALOGCONFIG);
	TASKDIALOG_BUTTON tdb[3];
	tdc.cButtons = iButtonCount;
	tdc.pButtons = tdb;
	tdc.dwFlags = TDF_ALLOW_DIALOG_CANCELLATION | (IsCenterPos ? TDF_POSITION_RELATIVE_TO_WINDOW : 0);
	tdc.pszMainInstruction = pszMainInstruction;
	tdc.pszContent = pszContent;
	tdc.pszWindowTitle = pszWndTitle;
	tdc.pszVerificationText = pszChackBoxTitle;
	tdc.hMainIcon = hIcon;
	tdc.hwndParent = hParent;
	tdc.nDefaultButton = iDefButton;
	switch (iButtonCount)
	{
	case 1:
		tdb[0].nButtonID = QKMSGBOX_BTID_1;
		tdb[0].pszButtonText = !pszButton1Title ? L"ȷ��(&O)" : pszButton1Title;
	case 2:
		tdb[0].nButtonID = QKMSGBOX_BTID_1;
		tdb[0].pszButtonText = !pszButton1Title ? L"��(&Y)" : pszButton1Title;
		tdb[1].nButtonID = QKMSGBOX_BTID_2;
		tdb[1].pszButtonText = !pszButton2Title ? L"��(&N)" : pszButton2Title;
	case 3:
		tdb[0].nButtonID = QKMSGBOX_BTID_1;
		tdb[0].pszButtonText = !pszButton1Title ? L"��(&Y)" : pszButton1Title;
		tdb[1].nButtonID = QKMSGBOX_BTID_2;
		tdb[1].pszButtonText = !pszButton2Title ? L"��(&N)" : pszButton2Title;
		tdb[2].nButtonID = QKMSGBOX_BTID_3;
		tdb[2].pszButtonText = !pszButton3Title ? L"ȡ��(&C)" : pszButton3Title;
	}
	int iButton, iRadio;
	BOOL bCheckBox;
	TaskDialogIndirect(&tdc, &iButton, &iRadio, &bCheckBox);
	if (IsCheck)
		*IsCheck = bCheckBox;
	return iButton;
}
BOOL QKGradientFill(HDC hDC, RECT* Rect, COLORREF Color1, COLORREF Color2, ULONG Mode)
{
	TRIVERTEX tv[2] = { 0 };
	tv[0].x = Rect->left;
	tv[0].y = Rect->top;
	tv[0].Red = GetRValue(Color1) << 8;
	tv[0].Green = GetGValue(Color1) << 8;
	tv[0].Blue = GetBValue(Color1) << 8;

	tv[1].x = Rect->right;
	tv[1].y = Rect->bottom;
	tv[1].Red = GetRValue(Color2) << 8;
	tv[1].Green = GetGValue(Color2) << 8;
	tv[1].Blue = GetBValue(Color2) << 8;

	GRADIENT_RECT gr[1] = { 0 };
	gr[0].UpperLeft = 0;//���Ͻ�����Ϊ��һ����Ա
	gr[0].LowerRight = 1;//���½�����Ϊ�ڶ�����Ա

	return GradientFill(hDC, tv, 2, gr, 1, Mode);
}
void QKOutputLastErrorCode()
{
	WCHAR buf[32];
	wsprintfW(buf, L"%d\n", GetLastError());
	OutputDebugStringW(buf);
}
void QKOutputDebugInt(int i)
{
	WCHAR buf[32];
	wsprintfW(buf, L"%d\n", i);
	OutputDebugStringW(buf);
}
UINT QKGDIClrToCommonClr(COLORREF cr)
{
	BYTE by[4];
	UINT u;
	memcpy(by, &cr, 4);
	by[3] = by[0];
	by[0] = by[2];
	by[2] = by[3];
	by[3] = 0;
	memcpy(&u, by, 4);
	return u;
}
COLORREF QKCommonClrToGDIClr(UINT u)
{
	BYTE by[4];
	COLORREF cr;
	memcpy(by, &u, 4);
	by[3] = by[0];
	by[0] = by[2];
	by[2] = by[3];
	by[3] = 0;
	memcpy(&cr, by, 4);
	return cr;
}
UINT QKGetDPIForWindow(HWND hWnd)
{
	if (pGetDpiForWindow)
		return pGetDpiForWindow(hWnd);
	else
	{
		HDC hDC = GetDC(hWnd);
		int i = GetDeviceCaps(hDC, LOGPIXELSX);
		ReleaseDC(hWnd, hDC);
		return i;
	}
}
BOOL QKStrToBool(PCWSTR psz)
{
	if (lstrcmpiW(L"true", psz) == 0)
		return TRUE;
	else
		return FALSE;
}
UINT32 QKBEUINT32ToUINT32(BYTE* p)
{
	return (UINT32)((UINT32)p[0] << 24 | (UINT32)p[1] << 16 | (UINT32)p[2] << 8 | (UINT32)p[3]);
}
UINT32 QKSynchsafeUINT32ToUINT32(BYTE* p)
{
	return (UINT32)(((UINT32)p[0] & 0x7F) << 21) | (((UINT32)p[1] & 0x7F) << 14) | (((UINT32)p[2] & 0x7F) << 7) | ((UINT32)p[3] & 0x7F);
}

/// <summary>
/// [����ID3v2��������]��ָ�����봦���ı�
/// </summary>
/// <param name="pStream">�ֽ���ָ�룻δָ��iTextEncodingʱָ�������ı�֡��ָ��iTextEncodingʱָ���ַ���</param>
/// <param name="iLength">���ȣ�δָ��iTextEncodingʱ��ʾ�����ı�֡���ȣ�����1B�ı����ǣ�������βNULL����ָ��iTextEncodingʱ��ʾ�ַ������ȣ�������βNULL��</param>
/// <param name="iTextEncoding">�Զ����ı����룻-1��ȱʡ��ָʾ��������ı�֡</param>
/// <returns>����UTF-16LE�ı�ָ�룬ʹ����Ϻ����ʹ��delete[]ɾ��</returns>
PWSTR GetMP3ID3v2_ProcString(BYTE* pStream, int iLength, int iTextEncoding = -1)
{
	int iType = 0, iBufferSize;
	PWSTR pBuffer = NULL;
	if (iTextEncoding == -1)
	{
		memcpy(&iType, pStream, 1);
		++pStream;// �����ı������־
		--iLength;
	}
	else
		iType = iTextEncoding;

	switch (iType)
	{
	case 0://ISO-8859-1����Latin-1��������-1��
		iBufferSize = MultiByteToWideChar(CP_ACP, 0, (PCCH)pStream, iLength, NULL, 0);
		if (iBufferSize == 0)
			return NULL;
		pBuffer = new WCHAR[iBufferSize + 1];//������βNULL
		ZeroMemory(pBuffer, (iBufferSize + 1) * sizeof(WCHAR));
		MultiByteToWideChar(CP_ACP, 0, (PCCH)pStream, iLength, pBuffer, iBufferSize);//iLength��������βNULL�����ת������ַ���Ҳ�������
		break;
	case 1://UTF-16LE
		if (*(PWSTR)pStream == L'\xFEFF')// ��BOM��Ҫ�����������ϣֵ��һ���ҿ��ܻ��淢�ֲ������BOM������.....��
		{
			pStream += sizeof(WCHAR);
			iLength -= sizeof(WCHAR);
		}
		iBufferSize = iLength / sizeof(WCHAR) + 1;
		pBuffer = new WCHAR[iBufferSize];
		lstrcpynW(pBuffer, (PWSTR)pStream, iBufferSize);//������βNULL
		break;
	case 2://UTF-16BE
		if (*(PWSTR)pStream == L'\xFFFE')// ��BOM
		{
			pStream += sizeof(WCHAR);
			iLength -= sizeof(WCHAR);
		}
		iBufferSize = iLength / sizeof(WCHAR);
		pBuffer = new WCHAR[iBufferSize + 1];
		LCMapStringEx(LOCALE_NAME_USER_DEFAULT, LCMAP_BYTEREV, (PCWSTR)pStream, iBufferSize, pBuffer, iBufferSize, NULL, NULL, 0);// ��ת�ֽ���
		ZeroMemory(pBuffer + iBufferSize, sizeof(WCHAR));// ��ӽ�βNULL
		break;
	case 3://UTF-8
		iBufferSize = MultiByteToWideChar(CP_UTF8, 0, (PCCH)pStream, iLength, NULL, 0);
		if (iBufferSize == 0)
			return NULL;
		pBuffer = new WCHAR[iBufferSize + 1];
		ZeroMemory(pBuffer, (iBufferSize + 1) * sizeof(WCHAR));
		MultiByteToWideChar(CP_UTF8, 0, (PCCH)pStream, iLength, pBuffer, iBufferSize);//iLength��������βNULL�����ת������ַ���Ҳ�������
		break;
	}

	return pBuffer;
}
void MusicInfo_Get(PCWSTR pszFile, MUSICINFO* pmi)
{
	ZeroMemory(pmi, sizeof(MUSICINFO));
	HANDLE hFile = CreateFileW(
		pszFile,
		GENERIC_READ,
		FILE_SHARE_READ,
		NULL,
		OPEN_EXISTING,
		FILE_ATTRIBUTE_NORMAL,
		NULL
	);
	if (hFile == INVALID_HANDLE_VALUE)
		return;
	DWORD dwLengthRead;
	BYTE by[4];
	ReadFile(hFile, by, 4, &dwLengthRead, NULL);// ���ļ�ͷ
	if (memcmp(by, "ID3", 3) == 0)// ID3v2
	{
		ID3v2_Header Header = { 0 };
		ID3v2_ExtHeader ExtHeader = { 0 };
		SetFilePointer(hFile, 0, NULL, FILE_BEGIN);
		ReadFile(hFile, &Header, sizeof(Header), &dwLengthRead, NULL);// ������ǩͷ
		if (dwLengthRead < sizeof(ID3v2_Header))
			return;

		DWORD dwTotalSize = QKSynchsafeUINT32ToUINT32(Header.Size);// 28λ���ݣ�������ǩͷ����չͷ
		HANDLE hMapping = CreateFileMappingW(hFile, NULL, PAGE_READONLY, 0, dwTotalSize, NULL);// ӳ��ID3v2���ڴ�
		BYTE* pFile = (BYTE*)MapViewOfFile(hMapping, FILE_MAP_READ, 0, 0, dwTotalSize);
		if (pFile)
		{
			BYTE* pFrame;
			DWORD dwOffest;

			if (Header.Ver == 3)// 2.3
			{
				if (Header.Flags & 0x20)// ����չͷ
					dwOffest = sizeof(ID3v2_Header) + 4 + QKBEUINT32ToUINT32(ExtHeader.ExtHeaderSize);
				else
					dwOffest = sizeof(ID3v2_Header);
			}
			else if (Header.Ver = 4)// 2.4
			{
				if (Header.Flags & 0x20)// ����չͷ
					dwOffest = sizeof(ID3v2_Header) + QKSynchsafeUINT32ToUINT32(ExtHeader.ExtHeaderSize);// 2.4������ͬ����ȫ��������������ߴ�����˼�¼�ߴ���ĸ��ֽ�
				else
					dwOffest = sizeof(ID3v2_Header);
			}

			DWORD dwUnitSize;
			CHAR FrameID[4];
			while (true)
			{
				pFrame = pFile + dwOffest;

				if (Header.Ver == 3)
					dwUnitSize = QKBEUINT32ToUINT32(pFrame + 4);// 2.3��32λ���ݣ�������֡ͷ��ƫ4B��
				else if (Header.Ver = 4)
					dwUnitSize = QKSynchsafeUINT32ToUINT32(pFrame + 4);//2.4��28λ���ݣ�ͬ����ȫ������

				memcpy(FrameID, ((ID3v2_UnitHeader*)pFrame)->ID, 4);// ��֡��ʶ
				pFrame += sizeof(ID3v2_UnitHeader);// ����֡ͷ
				if (memcmp(FrameID, "TIT2", 4) == 0)// ����
					pmi->pszTitle = GetMP3ID3v2_ProcString(pFrame, dwUnitSize);
				else if (memcmp(FrameID, "TPE1", 4) == 0)// ����
					pmi->pszArtist = GetMP3ID3v2_ProcString(pFrame, dwUnitSize);
				else if (memcmp(FrameID, "TALB", 4) == 0)// ר��
					pmi->pszAlbum = GetMP3ID3v2_ProcString(pFrame, dwUnitSize);
				else if (memcmp(FrameID, "USLT", 4) == 0)// ��ͬ�����
				{
					/*
					<֡ͷ>��֡��ʶΪUSLT��
					�ı�����						$xx
					��Ȼ���Դ���					$xx xx xx
					��������						<�ַ���> $00 (00)
					���							<�ַ���>
					*/
					DWORD cb = dwUnitSize;
					BYTE byEncodeingType = *pFrame;// ���ı�����
					++pFrame;// �����ı�����
					CHAR byLangCode[3];
					memcpy(byLangCode, pFrame, 3);// ����Ȼ���Դ���
					pFrame += 3;// ������Ȼ���Դ���
					int t;
					if (byEncodeingType == 0 || byEncodeingType == 3)// ISO-8859-1��UTF-8
						t = lstrlenA((PCSTR)pFrame) + 1;
					else// UTF-16LE��UTF-16BE
						t = (lstrlenW((PCWSTR)pFrame) + 1) * sizeof(WCHAR);
					pFrame += t;// ������������
					cb -= (t + 4);
					// ��ʱpFrameָ�����ı�
					pmi->pszLrc = GetMP3ID3v2_ProcString(pFrame, cb, byEncodeingType);
				}
				else if (memcmp(FrameID, "COMM", 4) == 0)// ��ע
				{
					/*
					<֡ͷ>��֡��ʶΪCOMM��
					�ı�����						$xx
					��Ȼ���Դ���					$xx xx xx
					��עժҪ						<�ַ���> $00 (00)
					��ע							<�ַ���>
					*/
					DWORD cbComment = dwUnitSize;
					BYTE byEncodeingType = *pFrame;// ���ı�����
					++pFrame;// �����ı�����
					CHAR byLangCode[3];
					memcpy(byLangCode, pFrame, 3);// ����Ȼ���Դ���
					pFrame += 3;// ������Ȼ���Դ���
					int t;
					if (byEncodeingType == 0 || byEncodeingType == 3)// ISO-8859-1��UTF-8
						t = lstrlenA((PCSTR)pFrame) + 1;
					else// UTF-16LE��UTF-16BE
						t = (lstrlenW((PCWSTR)pFrame) + 1) * sizeof(WCHAR);
					pFrame += t;// ������עժҪ
					cbComment -= (t + 4);
					// ��ʱpFrameָ��ע�ַ���
					pmi->pszComment = GetMP3ID3v2_ProcString(pFrame, cbComment, byEncodeingType);
				}
				else if (memcmp(FrameID, "APIC", 4) == 0)// ͼƬ
				{
					/*
					<֡ͷ>��֡��ʶΪAPIC��
					�ı�����                        $xx
					MIME ����                       <ASCII�ַ���>$00����'image/bmp'��
					ͼƬ����                        $xx
					����                            <�ַ���>$00(00)
					<ͼƬ����>
					*/
					BYTE* pImageData = pFrame;
					BYTE byEncodeingType = *pImageData;// ���ı�����
					int t, cbImageSize = dwUnitSize;
					++pImageData;// �����ı�����
					t = lstrlenA((PCSTR)pImageData);
					pImageData += t;// ����MIME�����ַ���
					pImageData += 2;// ����MIME��βNULL��ͼƬ����
					cbImageSize -= (t + 3);
					if (byEncodeingType == 0 || byEncodeingType == 3)// ISO-8859-1��UTF-8
						t = lstrlenA((PCSTR)pImageData) + 1;
					else// UTF-16LE��UTF-16BE
						t = lstrlenW((PCWSTR)pImageData) * sizeof(WCHAR) + 2;
					pImageData += t; cbImageSize -= t;// ���������ַ����ͽ�βNULL

					IStream* pPicStream = SHCreateMemStream(pImageData, cbImageSize);// ����������
					if (pPicStream)
					{
						WICCreateBitmap(pPicStream, &pmi->pWICBitmap);
						pPicStream->Release();
					}
				}
				dwOffest += (dwUnitSize + sizeof(ID3v2_UnitHeader));
				if (dwOffest >= dwTotalSize)// �Ƿ񳬽�
					break;
			}
			UnmapViewOfFile(pFile);
		}
		CloseHandle(hMapping);

	}
	else if (memcmp(by, "fLaC", 4) == 0)// Flac
	{
		FLAC_Header Header;
		DWORD dwSize;
		UINT t;
		void* pBuffer;
		do
		{
			ReadFile(hFile, &Header, sizeof(FLAC_Header), &dwLengthRead, NULL);
			dwSize = Header.bySize[2] | Header.bySize[1] << 8 | Header.bySize[0] << 16;
			switch (Header.by & 0x7F)
			{
			case 4:// ��ǩ��Ϣ��ע�⣺��һ������С����
			{
				ReadFile(hFile, &t, 4, &dwLengthRead, NULL);// ��������Ϣ��С
				SetFilePointer(hFile, t, NULL, FILE_CURRENT);// ������������Ϣ
				UINT uCount;
				ReadFile(hFile, &uCount, 4, &dwLengthRead, NULL);// ��ǩ��

				for (UINT i = 0; i < uCount; ++i)
				{
					ReadFile(hFile, &t, 4, &dwLengthRead, NULL);// ��ǩ��С
					pBuffer = HeapAlloc(GetProcessHeap(), 0, t + 1);
					ReadFile(hFile, pBuffer, t, &dwLengthRead, NULL);// ����ǩ
					*(CHAR*)((BYTE*)pBuffer + t) = '\0';

					t = MultiByteToWideChar(CP_UTF8, 0, (CHAR*)pBuffer, -1, NULL, 0);
					PWSTR pBuf = (PWSTR)HeapAlloc(GetProcessHeap(), 0, t * sizeof(WCHAR));
					MultiByteToWideChar(CP_UTF8, 0, (CHAR*)pBuffer, -1, pBuf, t);// ת�����룬UTF-8��UTF-16LE
					HeapFree(GetProcessHeap(), 0, pBuffer);

					UINT uPos = QKStrInStr(pBuf, L"=");// �ҵȺ�

					if (QKStrInStr(pBuf, L"TITLE"))
					{
						pmi->pszTitle = new WCHAR[t - uPos];
						lstrcpyW(pmi->pszTitle, pBuf + uPos);
					}
					else if (QKStrInStr(pBuf, L"ALBUM"))
					{
						pmi->pszAlbum = new WCHAR[t - uPos];
						lstrcpyW(pmi->pszAlbum, pBuf + uPos);
					}
					else if (QKStrInStr(pBuf, L"ARTIST"))
					{
						pmi->pszArtist = new WCHAR[t - uPos];
						lstrcpyW(pmi->pszArtist, pBuf + uPos);
					}
					else if (QKStrInStr(pBuf, L"DESCRIPTION"))
					{
						pmi->pszComment = new WCHAR[t - uPos];
						lstrcpyW(pmi->pszComment, pBuf + uPos);
					}
					else if (QKStrInStr(pBuf, L"LYRICS"))
					{
						pmi->pszLrc = new WCHAR[t - uPos];
						lstrcpyW(pmi->pszLrc, pBuf + uPos);
					}

					HeapFree(GetProcessHeap(), 0, pBuf);
				}
			}
			break;
			case 6:// ͼƬ�������
			{
				SetFilePointer(hFile, 4, NULL, FILE_CURRENT);// ����ͼƬ����

				ReadFile(hFile, &t, 4, &dwLengthRead, NULL);
				t = QKBEUINT32ToUINT32((BYTE*)&t);// ������ֽڵ���������ͬ
				SetFilePointer(hFile, t, NULL, FILE_CURRENT);// ����MIME�����ַ���

				ReadFile(hFile, &t, 4, &dwLengthRead, NULL);
				t = QKBEUINT32ToUINT32((BYTE*)&t);
				SetFilePointer(hFile, t, NULL, FILE_CURRENT);// ���������ַ���

				SetFilePointer(hFile, 16, NULL, FILE_CURRENT);// ������ȡ��߶ȡ�ɫ�����ͼ��ɫ��

				ReadFile(hFile, &t, 4, &dwLengthRead, NULL);
				t = QKBEUINT32ToUINT32((BYTE*)&t);// ͼƬ���ݳ���

				pBuffer = HeapAlloc(GetProcessHeap(), 0, t);
				ReadFile(hFile, pBuffer, t, &dwLengthRead, NULL);// ��ͼƬ
				IStream* pPicStream = SHCreateMemStream((BYTE*)pBuffer, t);// ����������
				if (pPicStream)
				{
					WICCreateBitmap(pPicStream, &pmi->pWICBitmap);
					pPicStream->Release();
				}
				HeapFree(GetProcessHeap(), 0, pBuffer);
			}
			break;
			default:
				SetFilePointer(hFile, dwSize, NULL, FILE_CURRENT);// ������
			}

		} while (!(Header.by & 0x80));// ������λ���ж��ǲ������һ����
	}
	CloseHandle(hFile);
}
void MusicInfo_Release(MUSICINFO* mi)
{
	delete[] mi->pszTitle;
	delete[] mi->pszArtist;
	delete[] mi->pszAlbum;
	delete[] mi->pszComment;
	delete[] mi->pszLrc;
	if (mi->pWICBitmap)
		mi->pWICBitmap->Release();
	ZeroMemory(mi, sizeof(MUSICINFO));
}

/// <summary>
/// [��ȡ������ݸ�������]������ʱ���ǩ�����ı���ǩת���ɸ������������䰴����װ�ص���������У�������ɺ�����ԭ����
/// </summary>
/// <param name="Result">�������</param>
/// <param name="TimeLabel">ʱ���ǩ����</param>
/// <param name="pszLrc">��ʱ���ǩ���������г�Ա����Ӧ�ĸ��</param>
void GetLrcData_ProcLabel(QKARRAY* Result, QKARRAY TimeLabel, PWSTR pszLrc)
{
	if (!TimeLabel)
		return;
	if (TimeLabel->iCount <= 0)
		return;

	for (int i = 0; i < TimeLabel->iCount; i++)
	{
		LPWSTR pszTimeLabel = (LPWSTR)QKAGet(TimeLabel, i);
		int iStrPos1, iStrPos2;
		iStrPos1 = QKStrInStr(pszTimeLabel, L":");
		if (iStrPos1 <= 0)
			continue;// ûð�ţ���ѭ��β
		iStrPos2 = QKStrInStr(pszTimeLabel, L":", iStrPos1 + 1);
		PWSTR pszTempTime;
		DWORD dwLength;
		int M, S, MS;
		BOOL IsMS = TRUE;
		float fTime;
		if (iStrPos2 <= 0)// �Ƿ�[��:��:����]
		{
			iStrPos2 = QKStrInStr(pszTimeLabel, L".", iStrPos1 + 1);
			if (iStrPos2 <= 0)// �Ƿ�[��:��.����]
			{
				IsMS = FALSE;// [��:��]
				iStrPos2 = lstrlenW(pszTimeLabel) + 1;
			}
		}
		///////////////////ȡ����
		dwLength = iStrPos1 - 1;
		pszTempTime = new WCHAR[dwLength + 1];
		lstrcpynW(pszTempTime, pszTimeLabel, dwLength + 1);
		if (!StrToIntExW(pszTempTime, STIF_DEFAULT, &M))
			continue;// ת��ʧ�ܣ���ѭ��β
		fTime = (float)M * 60.0f;
		delete[] pszTempTime;
		///////////////////ȡ��
		dwLength = iStrPos2 - iStrPos1 - 1;
		pszTempTime = new WCHAR[dwLength + 1];
		lstrcpynW(pszTempTime, pszTimeLabel + iStrPos1, dwLength + 1);
		if (!StrToIntExW(pszTempTime, STIF_DEFAULT, &S))
			continue;
		fTime += S;
		delete[] pszTempTime;
		///////////////////ȡ����
		if (IsMS)
		{
			dwLength = lstrlenW(pszTimeLabel) - iStrPos2;

			if (dwLength == 2)// ֻ����λxxʱ��ʾxx0�����Ƚ����ˣ���λ��ʮ���룩
			{
				++dwLength;
				pszTempTime = new WCHAR[dwLength + 1];
				lstrcpynW(pszTempTime, pszTimeLabel + iStrPos2, dwLength + 1);
				lstrcatW(pszTempTime, L"0");// ��0
			}
			else
			{
				pszTempTime = new WCHAR[dwLength + 1];
				lstrcpynW(pszTempTime, pszTimeLabel + iStrPos2, dwLength + 1);
			}

			if (!StrToIntExW(pszTempTime, STIF_DEFAULT, &MS))
				continue;
			fTime += ((float)MS / 1000.0f);
			delete[] pszTempTime;
		}
		///////////////////
		if (fTime < 0)
			continue;
		LRCDATA* p = new LRCDATA;
		p->fTime = fTime;
		p->cy = -1;// ������Ч
		p->iOrgLength = -1;// ȱʡ-1��˵��û�еڶ���
		p->pszLrc = new WCHAR[lstrlenW(pszLrc) + 1];// �ÿ���һ�ݣ���ͬ��Ա��Ȼ�ǻ��������
		lstrcpyW(p->pszLrc, pszLrc);

		QKAAdd(*Result, p);
	}
}
void Lrc_ClearArray(QKARRAY x)
{
	for (int i = 0; i < x->iCount; ++i)
	{
		delete[]((LRCDATA*)QKAGet(x, i))->pszLrc;
	}
	QKADelete(x, QKADF_DELETE);
}
BOOL QKIsTextUTF8(char* str, ULONGLONG length)
{
	int i;
	DWORD nBytes = 0;//UFT8����1-6���ֽڱ���,ASCII��һ���ֽ�
	UCHAR chr;
	BOOL bAllAscii = TRUE; //���ȫ������ASCII, ˵������UTF-8
	for (i = 0; i < length; i++)
	{
		chr = *(str + i);
		if ((chr & 0x80) != 0) // �ж��Ƿ�ASCII����,�������,˵���п�����UTF-8,ASCII��7λ����,����һ���ֽڴ�,���λ���Ϊ0,o0xxxxxxx
			bAllAscii = FALSE;
		if (nBytes == 0) //�������ASCII��,Ӧ���Ƕ��ֽڷ�,�����ֽ���
		{
			if (chr >= 0x80)
			{
				if (chr >= 0xFC && chr <= 0xFD)
					nBytes = 6;
				else if (chr >= 0xF8)
					nBytes = 5;
				else if (chr >= 0xF0)
					nBytes = 4;
				else if (chr >= 0xE0)
					nBytes = 3;
				else if (chr >= 0xC0)
					nBytes = 2;
				else
				{
					return FALSE;
				}
				nBytes--;
			}
		}
		else //���ֽڷ��ķ����ֽ�,ӦΪ 10xxxxxx
		{
			if ((chr & 0xC0) != 0x80)
			{
				return FALSE;
			}
			nBytes--;
		}
	}
	if (nBytes > 0) //Υ������
	{
		return FALSE;
	}
	if (bAllAscii) //���ȫ������ASCII, ˵������UTF-8
	{
		return FALSE;
	}
	return TRUE;
}
void Lrc_ParseLrcData(void* pStream, int iSize, BOOL bFileName, QKARRAY* IDResult, QKARRAY* Result, int iDefTextCode)
{
	//                                                                          ��������Ҫ�ͷŵĶ����
	////////////////////////////���벢�����з��ָ��ļ�
	PWSTR pBuffer = NULL;
	int iBufPtrOffest = 0;
	DWORD dwBytes;
	if (bFileName)
	{
		/////////////׼���ļ���
		WCHAR szLrcFile[MAX_PATH];
		lstrcpyW(szLrcFile, (PWSTR)pStream);
		PathRemoveExtensionW(szLrcFile);
		lstrcatW(szLrcFile, L".lrc");// ת��lrc��չ��
		/////////////��
		HANDLE hFile = CreateFileW(
			szLrcFile,
			GENERIC_READ,
			FILE_SHARE_READ | FILE_SHARE_WRITE | FILE_SHARE_DELETE,
			NULL,
			OPEN_EXISTING,
			FILE_ATTRIBUTE_NORMAL,
			NULL
		);// ���ļ�                                                                �ļ����
		if (hFile == INVALID_HANDLE_VALUE)// ��ʧ��
			return;
		dwBytes = GetFileSize(hFile, NULL);// ȡ�ֽ���
		if (dwBytes < 5)// [a:b]  �����������������ҪΪ�˶Ա�BOM��ʱ��Խ��
		{
			CloseHandle(hFile);
			return;// ��ʧ��
			//                                                                      ***�������˳������ͷ�
		}

		DWORD dwBytesRead;
		pBuffer = (PWSTR)HeapAlloc(GetProcessHeap(), 0, dwBytes + 2);// ��һ���ڴ��������ļ���Ϣ����ʵ�ļ�ӳ��Ҳ���Ե�����
		*(WCHAR*)((BYTE*)pBuffer + dwBytes) = L'\0';// �ѽ�βNULL����
		//                                                                          �ļ������������
		ReadFile(hFile, pBuffer, dwBytes, &dwBytesRead, NULL);// ����
		CloseHandle(hFile);
	}
	else
	{
		dwBytes = iSize;

		pBuffer = (PWSTR)HeapAlloc(GetProcessHeap(), 0, iSize + 2);
		memcpy(pBuffer, pStream, iSize);
		*(WCHAR*)((BYTE*)pBuffer + iSize) = L'\0';// �ѽ�βNULL����
	}
	/////////////�жϡ�ת������
	DWORD dwLength;
	UINT uCode;

	CHAR cHeader[3] = { (CHAR)0xFF,(CHAR)0xFE,0 };// UTF-16LE BOM:FF FE
	if (memcmp(pBuffer, cHeader, 2) == 0)// ���BOM
	{
		iBufPtrOffest = 2;// ����BOM

		pBuffer = (PWSTR)((BYTE*)pBuffer + iBufPtrOffest);
		dwBytes -= iBufPtrOffest;
		goto GetLrc_UTF16LE;
	}
	else
	{
		cHeader[0] = (CHAR)0xFE;
		cHeader[1] = (CHAR)0xFF;
		if (memcmp(pBuffer, cHeader, 2) == 0)// UTF-16BE BOM:FE FF
		{
			iBufPtrOffest = 2;// ����BOM

			pBuffer = (PWSTR)((BYTE*)pBuffer + iBufPtrOffest);
			dwBytes -= iBufPtrOffest;
			goto GetLrc_UTF16BE;
		}
		else
		{
			// UTF-8 BOM:EF BB BF 
			cHeader[0] = (CHAR)0xEF;
			cHeader[1] = (CHAR)0xBB;
			cHeader[2] = (CHAR)0xBF;
			if (memcmp(pBuffer, cHeader, 3) == 0)// ���BOM
			{
				iBufPtrOffest = 3;// ����BOM

				pBuffer = (PWSTR)((BYTE*)pBuffer + iBufPtrOffest);
				dwBytes -= iBufPtrOffest;

				*((CHAR*)pBuffer + dwBytes) = '\0';
				dwLength = MultiByteToWideChar(CP_UTF8, 0, (CHAR*)pBuffer, -1, NULL, 0);// ����-1������ֵ��ʾ���ֽ���������βNULL

				PWSTR pBuf = (PWSTR)HeapAlloc(GetProcessHeap(), 0, dwLength * sizeof(WCHAR));
				MultiByteToWideChar(CP_UTF8, 0, (CHAR*)pBuffer, -1, pBuf, dwLength);// ת������
				HeapFree(GetProcessHeap(), 0, (BYTE*)pBuffer - iBufPtrOffest);

				iBufPtrOffest = 0;
				pBuffer = pBuf;
				dwLength--;// ������βNULL
			}
			else// ��BOM
			{
				switch (iDefTextCode)
				{
				case 0:// �Զ�
				{
					int i = IS_TEXT_UNICODE_REVERSE_MASK | IS_TEXT_UNICODE_NULL_BYTES;
					if (IsTextUnicode(pBuffer, dwBytes, &i))//  �Ȳ�UTF-16BE����Ȼ�������
						goto GetLrc_UTF16BE;
					else
					{
						i = IS_TEXT_UNICODE_UNICODE_MASK | IS_TEXT_UNICODE_NULL_BYTES;
						if (IsTextUnicode(pBuffer, dwBytes, &i))
							goto GetLrc_UTF16LE;
						else if (QKIsTextUTF8((char*)pBuffer, dwBytes))
							goto GetLrc_UTF8;
						else
							goto GetLrc_GB2312;
					}
				}
				return;
				case 1:// GB2312
				{
				GetLrc_GB2312:
					uCode = 936;
					*((CHAR*)pBuffer + dwBytes) = '\0';// �ض�
				}
				break;// ����switchת����
				case 2:// UTF-8
				{
				GetLrc_UTF8:
					uCode = CP_UTF8;
					*((CHAR*)pBuffer + dwBytes) = '\0';// �ض�
				}
				break;// ����switchת����
				case 3:// UTF-16LE
				{
				GetLrc_UTF16LE:// ��BOM���Զ��жϵ�������
					dwLength = dwBytes / sizeof(WCHAR);
					goto UTF16_SkipOthers;// ����ȥ
				}
				break;
				case 4:// UTF-16BE
				{
				GetLrc_UTF16BE:// ��BOM���Զ��жϵ�������
					dwLength = dwBytes / sizeof(WCHAR);
					PWSTR pBuf = (PWSTR)HeapAlloc(GetProcessHeap(), 0, dwBytes + sizeof(WCHAR));
					LCMapStringEx(LOCALE_NAME_USER_DEFAULT, LCMAP_BYTEREV, (PCWSTR)pBuffer, dwLength, pBuf, dwLength, NULL, NULL, 0);// ��ת�ֽ���
					*((WCHAR*)pBuf + dwLength) = L'\0';
					HeapFree(GetProcessHeap(), 0, (BYTE*)pBuffer - iBufPtrOffest);
					pBuffer = pBuf;
					iBufPtrOffest = 0;
					goto UTF16_SkipOthers;// ����ȥ
				}
				break;
				}

				dwLength = MultiByteToWideChar(uCode, 0, (CHAR*)pBuffer, -1, NULL, 0);
				PWSTR pBuf = (PWSTR)HeapAlloc(GetProcessHeap(), 0, dwLength * sizeof(WCHAR));
				MultiByteToWideChar(uCode, 0, (CHAR*)pBuffer, -1, pBuf, dwLength);// ת������
				HeapFree(GetProcessHeap(), 0, (BYTE*)pBuffer - iBufPtrOffest);
				pBuffer = pBuf;
				dwLength--;// ������βNULL
				iBufPtrOffest = 0;
			}
		}
	}
UTF16_SkipOthers:// UTF-16�����ֱ��봦��ʽ��ͬ�������������ֱ����������
	//                                                                          ������
	/////////////׼���ָ�
	QKARRAY LrcLines = QKACreate(0);// ����ÿ�и��
	//                                                                          ���������ı�������
	/////////////���ֻ��з�
	WCHAR szDiv1[3] = L"\r\n";// CRLF
	WCHAR szDiv2[2] = L"\n";// LF
	WCHAR szDiv3[2] = L"\r";// CR

	BOOL b1 = FALSE, b2 = FALSE, b3 = FALSE;

	int i1 = QKStrInStr(pBuffer, szDiv1),
		i2 = QKStrInStr(pBuffer, szDiv2),
		i3 = QKStrInStr(pBuffer, szDiv3);

	if (i1)
		b1 = TRUE;
	if (i2)
		b2 = TRUE;
	if (i3)
		b3 = TRUE;

	PWSTR pszLine;// ÿ������
	int iLineLength;// ÿ�����ݳ���

	int iStrPos1;
	int iStrPos2 = 1;
	int iDivLength;

	if (!b1 && !b2 && !b3)// �޻��з�
	{
		pszLine = new WCHAR[dwLength + 1];
		lstrcpynW(pszLine, pBuffer, dwLength + 1);//ֱ�Ӷ�����
		QKAAdd(LrcLines, pszLine);
	}
	else
	{
		// ΪʲôҪ��ôд��û���еĸ���ļ����Ǳ�̬�����ֻ��з�һ����.......
		// ��ʵ�ⲿ���߼���֮����ϵģ�֮ǰ�Ľ�����������û�������bushi
		if (b1)// CRLF
		{
			// ˼·��iStrPos1 = min(i1, i2, i3)
			iStrPos1 = i1;
			iDivLength = 2;
			if (b2 && i1 >= i2)// LF
			{
				iStrPos1 = i2;
				iDivLength = 1;
			}
			if (b3 && i3 < iStrPos1)// CR
			{
				iStrPos1 = i3;
				iDivLength = 1;
			}
		}
		else
		{
			// ˼·��iStrPos1 = min(i2, i3)
			iDivLength = 1;
			if (b2 && b3)// û��CRLF����CR��LFͬʱ����
			{
				if (i2 < i3)
					iStrPos1 = i2;
				else
					iStrPos1 = i3;
			}
			else if (b2)// LF
				iStrPos1 = i2;
			else// CR
				iStrPos1 = i3;
		}

		while (iStrPos1)
		{
			iLineLength = iStrPos1 - iStrPos2;
			if (iLineLength > 0)
			{
				pszLine = new WCHAR[iLineLength + 1];
				lstrcpynW(pszLine, pBuffer + iStrPos2 - 1, iLineLength + 1);// ����һ��
				QKAAdd(LrcLines, pszLine);
			}
			iStrPos2 = iStrPos1 + iDivLength;// �������з�
			/////////////ȡ��һ���з�λ��
			if (b1)
				i1 = QKStrInStr(pBuffer, szDiv1, iStrPos2);
			if (b2)
				i2 = QKStrInStr(pBuffer, szDiv2, iStrPos2);
			if (b3)
				i3 = QKStrInStr(pBuffer, szDiv3, iStrPos2);

			iStrPos1 = 0;
			if (i1)// CRLF
			{
				iStrPos1 = i1;
				iDivLength = 2;
				if (i2 && i1 >= i2)// LF
				{
					iStrPos1 = i2;
					iDivLength = 1;
				}
				if (i3 && i3 < iStrPos1)// CR
				{
					iStrPos1 = i3;
					iDivLength = 1;
				}
			}
			else
			{
				if (i2 && i3)// CR  LF
				{
					iDivLength = 1;
					if (i2 < i3)
						iStrPos1 = i2;
					else
						iStrPos1 = i3;
				}
				else if (i2)// LF
				{
					iDivLength = 1;
					iStrPos1 = i2;
				}
				else if (i3)// CR������������ǲ�һ���ģ�������else if
				{
					iDivLength = 1;
					iStrPos1 = i3;
				}
			}
		}
		iLineLength = dwLength - iStrPos2 + 1;// ����ĩβһ���ı�
		if (iLineLength > 0)
		{
			pszLine = new WCHAR[iLineLength + 1];
			lstrcpynW(pszLine, pBuffer + iStrPos2 - 1, iLineLength + 1);
			QKAAdd(LrcLines, pszLine);
		}
	}
	HeapFree(GetProcessHeap(), 0, (BYTE*)pBuffer - iBufPtrOffest);// �ͷ��ļ����ݻ�����
	//                                                                          �ı�������
	// �����ļ��ָ����
	////////////////////////////����ÿ�б�ǩ
	int iStrPos3;
	QKARRAY SameUnitTimeLabel = NULL;
	PWSTR pszLrc;
	PWSTR pszTimeLabel;
	DWORD dwLrcLength;
	DWORD dwLength2;

	for (int i = 0; i < LrcLines->iCount; i++)// ��ÿһ��
	{
		pszLine = (LPWSTR)QKAGet(LrcLines, i);// ����һ��
		iStrPos1 = QKStrInStr(pszLine, L"[");// ������������
		iStrPos2 = 1;
		if (iStrPos1 <= 0)// �Ҳ�����������
			continue;// ��ѭ��β��������һ�У�   
		SameUnitTimeLabel = QKACreate(0);// ��ͬ��ʱ���ǩ
		//                                                                      �ı������飬ͬʱ������
		while (true)// ����ѭ��ȡ��ǩ��һ���п����ж����ǩ��
		{
			iStrPos2 = QKStrInStr(pszLine, L"]", iStrPos2);
			if (iStrPos2 <= 0 || iStrPos2 <= iStrPos1)
				iStrPos1 = iStrPos2 = 0;// �����Ŵ���ֱ�ӽ�����һ��ѭ����md����ļ����淶�������������꣩
			dwLength = iStrPos2 - iStrPos1 - 1;
			iStrPos3 = QKStrInStr(pszLine, L"[", iStrPos1 + dwLength + 1);

			if (iStrPos3 - iStrPos2 - 1 == 0)// �����ţ����������֣�[xx:xx][yy:yy]zzzzzzzzzzzzz
			{
				pszTimeLabel = new WCHAR[dwLength + 1];
				lstrcpynW(pszTimeLabel, pszLine + iStrPos1, dwLength + 1);
				QKAAdd(SameUnitTimeLabel, pszTimeLabel);
				iStrPos1 = iStrPos3;
				iStrPos2 = iStrPos1 + 1;
			}
			else if (iStrPos3 == 0 || iStrPos2 == 0)// û����һ����ǩ�ˣ���һ�е�ͷ��
			{
				dwLrcLength = lstrlenW(pszLine) - iStrPos2;
				pszLrc = new WCHAR[dwLrcLength + 1];
				lstrcpynW(pszLrc, pszLine + iStrPos2, dwLrcLength + 1);// ȡ���

				pszTimeLabel = new WCHAR[dwLength + 1];
				lstrcpynW(pszTimeLabel, pszLine + iStrPos1, dwLength + 1);// ȡ��ǩ
				QKAAdd(SameUnitTimeLabel, pszTimeLabel);

				GetLrcData_ProcLabel(Result, SameUnitTimeLabel, pszLrc);
				QKADelete(SameUnitTimeLabel, QKADF_DELETEARRAY);
				delete[] pszLrc;
				break;
			}
			else// ������һ���е�һ�䣬������[xx:xx]aaaaaaaaa[yy:yy]bbbbbbbbbbbb�����ڴ�����a��b��
			{
				dwLrcLength = iStrPos3 - iStrPos2 - 1;
				pszLrc = new WCHAR[dwLrcLength + 1];
				lstrcpynW(pszLrc, pszLine + iStrPos2, dwLrcLength + 1);

				pszTimeLabel = new WCHAR[dwLength + 1];
				lstrcpynW(pszTimeLabel, pszLine + iStrPos1, dwLength + 1);
				QKAAdd(SameUnitTimeLabel, pszTimeLabel);
				dwLength2 = iStrPos3 - iStrPos2 - 1;

				GetLrcData_ProcLabel(Result, SameUnitTimeLabel, pszLrc);
				QKADelete(SameUnitTimeLabel, QKADF_DELETEARRAY);
				delete[] pszLrc;
				SameUnitTimeLabel = QKACreate(0);
				iStrPos1 = iStrPos3;
				iStrPos2 = iStrPos1 + 1;
			}
			iStrPos1 = iStrPos2;
		}
	}
	QKADelete(LrcLines, QKADF_DELETEARRAY);
	//                                                                          ***����ʱ����
	////////////////////////////�������飬���ںϲ����
	QKARRAY pArray = *Result;
	int iCount = pArray->iCount;
	LRCDATA* pi, * pj;

	for (int i = 0; i <= iCount - 1; ++i)
	{
		for (int j = 0; j < iCount - 1 - i; ++j)
		{
			pi = (LRCDATA*)QKAGet(pArray, j);
			pj = (LRCDATA*)QKAGet(pArray, j + 1);
			if (pi->fTime > pj->fTime)
			{
				QKASet(pArray, j + 1, pi);
				QKASet(pArray, j, pj);
			}
		}
	}
	////////////////////////////�ϲ�ʱ����ͬ�ĸ��
	QKARRAY ArrLastTime = QKACreate(0);// ��Ա��float��
	QKARRAY ArrDelIndex = QKACreate(0);// ��Ա��int��
	//                                                                          ��һʱ�����飬��ɾ����������
	PWSTR p1, p2, pszNewLrc;
	LRCDATA* p, * pItem;
	for (int i = 0; i < iCount; i++)
	{
		p = (LRCDATA*)QKAGet(pArray, i);
		int iLastTimeCount = ArrLastTime->iCount;
		if (iLastTimeCount != 0 && i != 0)
		{
			if (QKReInterpretNumber(QKAGetValue(ArrLastTime, 0), float) == p->fTime)
			{
				pItem = (LRCDATA*)QKAGet(pArray, i - iLastTimeCount);
				p1 = pItem->pszLrc;
				p2 = p->pszLrc;
				QKStrTrim(p1);
				QKStrTrim(p2);// ɾ��β��
				int iLen1 = lstrlenW(p1), iLen2 = lstrlenW(p2);

				if (iLen1 && !iLen2)// ֻ�е�һ��
				{
				}// ʲô������
				else if (!iLen1 && iLen2)// ֻ�еڶ���
				{
					pszNewLrc = new WCHAR[iLen2 + 1];
					lstrcpyW(pszNewLrc, p2);
					delete[] pItem->pszLrc;
					pItem->pszLrc = pszNewLrc;
					pItem->iOrgLength = -1;
				}
				else if (!iLen1 && !iLen2)// ������û��
				{
					pszNewLrc = new WCHAR;
					*pszNewLrc = L'\0';
					delete[] pItem->pszLrc;
					pItem->pszLrc = pszNewLrc;
					pItem->iOrgLength = -1;
				}
				else// ��������
				{
					pItem->iOrgLength = iLen1;
					pszNewLrc = new WCHAR[iLen1 + iLen2 + 2];
					lstrcpyW(pszNewLrc, p1);
					lstrcatW(pszNewLrc, L"\n");// ���з�����
					lstrcatW(pszNewLrc, p2);

					delete[] pItem->pszLrc;
					pItem->pszLrc = pszNewLrc;
				}
				QKAAddValue(ArrDelIndex, i);
			}
			else
			{
				QKADelete(ArrLastTime);
				ArrLastTime = QKACreate(0);
			}
		}
		QKAAddValue(ArrLastTime, QKReInterpretNumber(p->fTime, UINT));
	}
	QKADelete(ArrLastTime);
	//                                                                          ��ɾ����������
	////////////////////////////ɾ�������Ա
	for (int i = 0; i < ArrDelIndex->iCount; i++)
	{
		int iIndex = QKAGetValue(ArrDelIndex, ArrDelIndex->iCount - i - 1);// ����ɾ
		LRCDATA* p = (LRCDATA*)QKAGet(pArray, iIndex);
		delete[] p->pszLrc;

		QKADeleteMember(pArray, iIndex, QKADF_DELETE);
	}
	QKADelete(ArrDelIndex);
	for (int i = 0; i < pArray->iCount; i++)
	{
		p = (LRCDATA*)QKAGet(pArray, i);
		if (i != pArray->iCount - 1)
			p->fDelay = ((LRCDATA*)QKAGet(pArray, i + 1))->fTime - p->fTime;
		else
			p->fDelay = g_llLength / 1000 - p->fTime;
	}
	QKATrimSize(pArray, FALSE);
	//                                                                          ***����ʱ����
	////////////////////////////��������
}
HRESULT WICCreateBitmap(PWSTR pszFile, IWICBitmap** ppWICBitmap)
{
	*ppWICBitmap = NULL;
	IWICFormatConverter* pWICConverter;
	g_pWICFactory->CreateFormatConverter(&pWICConverter);

	IWICBitmapDecoder* pWICDecoder;
	IWICBitmapFrameDecode* pWICFrameDecoder;
	g_pWICFactory->CreateDecoderFromFilename(pszFile, NULL, GENERIC_READ, WICDecodeMetadataCacheOnDemand, &pWICDecoder);

	pWICDecoder->GetFrame(0, &pWICFrameDecoder);

	pWICConverter->Initialize(pWICFrameDecoder, GUID_WICPixelFormat32bppPBGRA,
		WICBitmapDitherTypeNone, NULL, 0.0f, WICBitmapPaletteTypeMedianCut);

	g_pWICFactory->CreateBitmapFromSource(pWICConverter, WICBitmapNoCache, ppWICBitmap);

	pWICFrameDecoder->Release();
	pWICDecoder->Release();
	pWICConverter->Release();

	return S_OK;
}
HRESULT WICCreateBitmap(IStream* pStream, IWICBitmap** ppWICBitmap)
{
	*ppWICBitmap = NULL;
	IWICFormatConverter* pWICConverter;
	g_pWICFactory->CreateFormatConverter(&pWICConverter);

	IWICBitmapDecoder* pWICDecoder;
	IWICBitmapFrameDecode* pWICFrameDecoder;
	g_pWICFactory->CreateDecoderFromStream(pStream, NULL, WICDecodeMetadataCacheOnDemand, &pWICDecoder);

	pWICDecoder->GetFrame(0, &pWICFrameDecoder);

	pWICConverter->Initialize(pWICFrameDecoder, GUID_WICPixelFormat32bppPBGRA,
		WICBitmapDitherTypeNone, NULL, 0.0f, WICBitmapPaletteTypeMedianCut);

	g_pWICFactory->CreateBitmapFromSource(pWICConverter, WICBitmapNoCache, ppWICBitmap);

	pWICFrameDecoder->Release();
	pWICDecoder->Release();
	pWICConverter->Release();

	return S_OK;
}
void QKRcClientToScreen(HWND hWnd, RECT* prc)
{
	ClientToScreen(hWnd, (POINT*)prc);
	ClientToScreen(hWnd, (POINT*)(((POINT*)prc) + 1));
}
void QKRcScreenToClient(HWND hWnd, RECT* prc)
{
	ScreenToClient(hWnd, (POINT*)prc);
	ScreenToClient(hWnd, (POINT*)(((POINT*)prc) + 1));
}
void QKGDIColorToD2DColor(COLORREF cr, D2D1_COLOR_F* D2DCr, int iAlpha)
{
	D2DCr->a = (float)iAlpha / 255.f;
	D2DCr->r = (float)GetRValue(cr) / 255.f;
	D2DCr->g = (float)GetGValue(cr) / 255.f;
	D2DCr->b = (float)GetBValue(cr) / 255.f;
}
/////////////////////////////////////INI������

HQKINI QKINIParse(PCWSTR pszFile)
{
	HANDLE hFile = CreateFileW(pszFile, GENERIC_READ | GENERIC_WRITE, FILE_SHARE_READ, NULL, OPEN_ALWAYS, FILE_ATTRIBUTE_NORMAL, NULL);
	if (hFile == INVALID_HANDLE_VALUE)
		return NULL;

	QKINIDESC* p = new QKINIDESC;
	ZeroMemory(p, sizeof(QKINIDESC));

	p->hFile = hFile;

	DWORD dwFileSize = GetFileSize(hFile, NULL);
	if (!dwFileSize)
	{
		p->iType = QKINI_NEWFILE;
		return p;
	}
	p->iType = QKINI_NORMAL;
	PWSTR pszBuf = (PWSTR)HeapAlloc(GetProcessHeap(), HEAP_ZERO_MEMORY, dwFileSize + sizeof(WCHAR));
	p->pszContent = pszBuf;
	DWORD dw;
	ReadFile(hFile, pszBuf, dwFileSize, &dw, NULL);
	p->dwSize = lstrlenW(pszBuf);
	return p;
}
void QKINIClose(HQKINI hINI)
{
	CloseHandle(hINI->hFile);
	HeapFree(GetProcessHeap(), 0, hINI->pszContent);
	delete hINI;
}
int QKINIReadString(HQKINI hINI, PCWSTR pszSectionName, PCWSTR pszKeyName, PCWSTR pszDefStr, PWSTR pszRetStr, int iMaxBufSize)
{
	if (!hINI)
		goto CopyDefString;
	if (hINI->iType != QKINI_NORMAL)
		return 0;

	int iPos;
	PWSTR psz;
	int iTempLen;
	int iLen;
	//////////////////�ҽ���
	psz = new WCHAR[lstrlenW(pszSectionName) + 3];
	lstrcpyW(psz, L"[");
	lstrcatW(psz, pszSectionName);
	lstrcatW(psz, L"]");
	iPos = QKStrInStr(hINI->pszContent, psz);
	iTempLen = lstrlenW(psz);
	delete[] psz;
	if (iPos)
	{
		//////////////////�Ҽ���
		psz = new WCHAR[lstrlenW(pszKeyName) + 4];
		lstrcpyW(psz, L"\r\n");
		lstrcatW(psz, pszKeyName);
		lstrcatW(psz, L"=");
		iPos = QKStrInStr(hINI->pszContent, psz, iPos + iTempLen);
		iTempLen = lstrlenW(psz);
		delete[] psz;
		if (iPos)
		{
			//////////////////�һ���
			int iStart = iPos + iTempLen - 1;
			iPos = QKStrInStr(hINI->pszContent, L"\r\n", iStart);
			int iEnd;
			if (iPos)
				iEnd = iPos;
			else
				iEnd = hINI->dwSize;
			iLen = iEnd - iStart;
			if (iMaxBufSize == 0)
				return iLen;
			if (iLen > 1)
			{
				if (iLen > iMaxBufSize)
				{
					lstrcpynW(pszRetStr, hINI->pszContent + iStart, iMaxBufSize);
					return -iLen;
				}
				else
				{
					lstrcpynW(pszRetStr, hINI->pszContent + iStart, iLen);
					return iLen;
				}
			}
		}
	}
CopyDefString:
	if (pszDefStr)
	{
		iLen = lstrlenW(pszDefStr) + 1;
		if (iLen > iMaxBufSize)
		{
			lstrcpynW(pszRetStr, pszDefStr, iMaxBufSize);
			return -iLen;
		}
		else
		{
			lstrcpynW(pszRetStr, pszDefStr, iLen);
			return iLen;
		}
	}
	else
		return 0;
}
void QKINIWriteString(HQKINI hINI, PCWSTR pszSectionName, PCWSTR pszKeyName, PCWSTR pszString)
{
	if (hINI->iType == QKINI_NEWFILE)
	{
		hINI->pszContent = (PWSTR)HeapAlloc(GetProcessHeap(), HEAP_ZERO_MEMORY, sizeof(WCHAR));
		hINI->dwSize = lstrlenW(hINI->pszContent);
	}

	int iPos, iSectionPos, iKeyPos;
	PWSTR pszSection, pszKey;
	int iSectionLen, iKeyLen;
	int iLen;
	//////////////////�ҽ���
	pszSection = new WCHAR[lstrlenW(pszSectionName) + 3];
	lstrcpyW(pszSection, L"[");
	lstrcatW(pszSection, pszSectionName);
	lstrcatW(pszSection, L"]");
	iSectionPos = QKStrInStr(hINI->pszContent, pszSection);
	iSectionLen = lstrlenW(pszSection);

	if (iSectionPos)
	{
		//////////////////�Ҽ���
		pszKey = new WCHAR[lstrlenW(pszKeyName) + 4];
		lstrcpyW(pszKey, L"\r\n");
		lstrcatW(pszKey, pszKeyName);
		lstrcatW(pszKey, L"=");
		iKeyPos = QKStrInStr(hINI->pszContent, pszKey, iSectionPos + iSectionLen);
		iKeyLen = lstrlenW(pszKey);

		if (iKeyPos)
		{
			//////////////////�һ���
			int iStart = iKeyPos + iKeyLen - 1;
			iPos = QKStrInStr(hINI->pszContent, L"\r\n", iStart);
			int iEnd;
			if (iPos)
				iEnd = iPos;
			else
				iEnd = hINI->dwSize;
			iLen = iEnd - iStart - 1;
			int iValueLen = lstrlenW(pszString);
			PWSTR pStart;
			if (iLen > 0)// ��ֵ
			{
				if (iLen > iValueLen)
				{
					pStart = hINI->pszContent + iStart;
					memmove(pStart + iValueLen, pStart + iLen, (lstrlenW(pStart + iLen) + 1) * sizeof(WCHAR));// ��ǰ�ƶ�

					memcpy(pStart, pszString, lstrlenW(pszString) * sizeof(WCHAR));// дֵ
				}
				else if (iLen == iValueLen)
				{
					pStart = hINI->pszContent + iStart;
					memcpy(pStart, pszString, lstrlenW(pszString) * sizeof(WCHAR));// дֵ
				}
				else
				{
					hINI->pszContent = (PWSTR)HeapReAlloc(GetProcessHeap(), HEAP_ZERO_MEMORY, hINI->pszContent,
						(hINI->dwSize + (iValueLen - iLen) + 1 /*��βNULL*/) * sizeof(WCHAR));
					pStart = hINI->pszContent + iStart;
					memmove(pStart + iValueLen, pStart + iLen, lstrlenW(pStart + iLen) * sizeof(WCHAR));// ����ƶ�

					memcpy(pStart, pszString, lstrlenW(pszString) * sizeof(WCHAR));// дֵ
				}
			}
			else// û��ֵ
			{
				hINI->pszContent = (PWSTR)HeapReAlloc(GetProcessHeap(), HEAP_ZERO_MEMORY, hINI->pszContent,
					(hINI->dwSize + iValueLen + 1 /*��βNULL*/) * sizeof(WCHAR));
				pStart = hINI->pszContent + iStart;
				memmove(pStart + iValueLen, pStart, lstrlenW(pStart) * sizeof(WCHAR));// ����ƶ�

				memcpy(pStart, pszString, lstrlenW(pszString) * sizeof(WCHAR));// дֵ
			}
		}
		else// û�м�����ӵ���β��
		{
			iPos = QKStrInStr(hINI->pszContent, L"\r\n[", iSectionPos + iSectionLen);// ����һ��
			int iAddtionLen =
				2 /*���з�*/ +
				lstrlenW(pszKeyName) + 1 /*�Ⱥ�*/ + lstrlenW(pszString);
			hINI->pszContent = (PWSTR)HeapReAlloc(GetProcessHeap(), HEAP_ZERO_MEMORY, hINI->pszContent,
				(hINI->dwSize + iAddtionLen + 1 /*��βNULL*/) * sizeof(WCHAR));
			PWSTR pStart = hINI->pszContent + iPos - 1;
			memmove(pStart + iAddtionLen, pStart, lstrlenW(pStart) * sizeof(WCHAR));// ����ƶ�
			*(pStart) = L'\0';// �ضϣ�������������ַ���

			lstrcatW(pStart, L"\r\n");
			lstrcatW(pStart, pszKeyName);// д����
			lstrcatW(pStart, L"=");
			memcpy(pStart + lstrlenW(pStart), pszString, lstrlenW(pszString) * sizeof(WCHAR));// дֵ
			//lstrcatW(pStart, pszString);// дֵ
		}
	}
	else// û�нڣ���ӵ��ļ�β��
	{
		hINI->pszContent = (PWSTR)HeapReAlloc(GetProcessHeap(), HEAP_ZERO_MEMORY, hINI->pszContent, (
			hINI->dwSize +
			2 /*���з�*/ +
			iSectionLen +
			2 /*���з�*/ +
			lstrlenW(pszKeyName) + 1 /*�Ⱥ�*/ + lstrlenW(pszString) +
			1 /*��βNULL*/
			) * sizeof(WCHAR));

		lstrcatW(hINI->pszContent, L"\r\n");
		lstrcatW(hINI->pszContent, pszSection);// д����
		lstrcatW(hINI->pszContent, L"\r\n");
		lstrcatW(hINI->pszContent, pszKeyName);// д����
		lstrcatW(hINI->pszContent, L"=");
		lstrcatW(hINI->pszContent, pszString);// дֵ
	}

	hINI->dwSize = lstrlenW(hINI->pszContent);
}
BOOL QKINISave(HQKINI hINI)
{
	DWORD dw;
	SetFilePointer(hINI->hFile, 0, NULL, FILE_BEGIN);
	BOOL b = WriteFile(hINI->hFile, hINI->pszContent, hINI->dwSize * sizeof(WCHAR), &dw, NULL);
	if (b)
		SetEndOfFile(hINI->hFile);
	return b;
}
int QKINIReadInt(HQKINI hINI, PCWSTR pszSectionName, PCWSTR pszKeyName, int iDefValue)
{
	WCHAR pszDef[20];
	wsprintfW(pszDef, L"%d", iDefValue);
	PWSTR pszBuf = QKINIReadString2(hINI, pszSectionName, pszKeyName, pszDef);

	int i;
	i = StrToIntW(pszBuf);
	delete[] pszBuf;
	return i;
}
void QKINIWriteInt(HQKINI hINI, PCWSTR pszSectionName, PCWSTR pszKeyName, int iValue)
{
	WCHAR pszBuf[20];
	wsprintfW(pszBuf, L"%d", iValue);
	QKINIWriteString(hINI, pszSectionName, pszKeyName, pszBuf);
}
PWSTR QKINIReadString2(HQKINI hINI, PCWSTR pszSectionName, PCWSTR pszKeyName, PCWSTR pszDefStr)
{
	if (!hINI)
		goto CopyDefString;
	if (hINI->iType != QKINI_NORMAL)
		return 0;

	int iPos;
	PWSTR psz, pszRet;
	int iTempLen;
	int iLen;
	//////////////////�ҽ���
	psz = new WCHAR[lstrlenW(pszSectionName) + 3];
	lstrcpyW(psz, L"[");
	lstrcatW(psz, pszSectionName);
	lstrcatW(psz, L"]");
	iPos = QKStrInStr(hINI->pszContent, psz);
	iTempLen = lstrlenW(psz);
	delete[] psz;
	if (iPos)
	{
		//////////////////�Ҽ���
		psz = new WCHAR[lstrlenW(pszKeyName) + 4];
		lstrcpyW(psz, L"\r\n");
		lstrcatW(psz, pszKeyName);
		lstrcatW(psz, L"=");
		iPos = QKStrInStr(hINI->pszContent, psz, iPos + iTempLen);
		iTempLen = lstrlenW(psz);
		delete[] psz;
		if (iPos)
		{
			//////////////////�һ���
			int iStart = iPos + iTempLen - 1;
			iPos = QKStrInStr(hINI->pszContent, L"\r\n", iStart);
			int iEnd;
			if (iPos)
				iEnd = iPos;
			else
				iEnd = hINI->dwSize;
			iLen = iEnd - iStart;

			if (iLen > 1)
			{
				pszRet = new WCHAR[iLen];
				lstrcpynW(pszRet, hINI->pszContent + iStart, iLen);
				return pszRet;
			}
		}
	}
CopyDefString:
	if (pszDefStr)
	{
		iLen = lstrlenW(pszDefStr) + 1;
		pszRet = new WCHAR[iLen];
		lstrcpyW(pszRet, pszDefStr);
		return pszRet;
	}
	else
		return NULL;
}
/////////////////////////////////////INI������

BOOL QKIsPrime(int iNum) {
	if (iNum == 2)
		return TRUE;
	if (iNum % 2 == 0 || iNum < 2)
		return FALSE;
	else
	{
		for (int i = 3; i * i <= iNum; i += 2)
		{
			if (iNum % i == 0)
				return FALSE;
		}
		return TRUE;
	}
}
/////////////////////////////////////��ϣ���

QKHASHTABLE QKHTCreate(int iCount, BOOL bKeyInt, BOOL bValueInt, QKHTDELPROC pKeyProc, QKHTDELPROC pValueProc, HANDLE hHeap)
{
	if (iCount <= 0)
		return NULL;

	if (!hHeap)
		hHeap = GetProcessHeap();

	auto p = (QKHASHTABLEHEADER*)HeapAlloc(hHeap, 0, sizeof(QKHASHTABLEHEADER));
	if (!p)
		return NULL;

	p->iCount = iCount;
	p->ary = QKACreate(iCount);

	p->iP = iCount;
	for (int i = iCount; i > 0; --i)
	{
		if (QKIsPrime(i))
		{
			p->iP = i;
			break;
		}
	}

	p->hHeap = hHeap;
	p->bKeyInt = bKeyInt;
	p->bValueInt = bValueInt;

	return p;
}
UINT QKHTGetHashCode(PCVOID pData, int iLen)
{
	if (iLen <= 0)
		iLen = (lstrlenW((PCWSTR)pData) + 1) * sizeof(WCHAR);
	assert(iLen > 0);

	CHAR* p = (CHAR*)pData;
	UINT uHash = 0;
	for (int i = 0; i < iLen; ++i, ++p)
	{
		uHash = 31 * uHash + *p;
	}

	return uHash;
}
BOOL QKHTPut(QKHASHTABLE hHT, PCVOID pKey, int iKeyLen, PCVOID pValue, int iValueLen, UINT uValue, 
	BOOL bCover, UINT* puHashCode, QKHTPUTKEYEXISTPROC pKeyExistProc)
{
	assert(hHT);
	assert(pKey);

	if (iKeyLen <= 0)
		iKeyLen = (lstrlenW((PCWSTR)pKey) + 1) * sizeof(WCHAR);
	assert(iKeyLen > 0);

	UINT uHashCode = QKHTGetHashCode(pKey, iKeyLen);
	if (puHashCode)
		*puHashCode = uHashCode;

	int iIndex = (uHashCode & 0x7FFFFFFF) % hHT->iP;

	if (!hHT->bValueInt && pValue)
	{
		if (iValueLen <= 0)
			iValueLen = (lstrlenW((PCWSTR)pValue) + 1) * sizeof(WCHAR);
		assert(iValueLen > 0);
	}

	auto p = (QKHASHTABLELINKNODE*)QKAGet(hHT->ary, iIndex);
	if (p)
	{
		while (TRUE)
		{
			if (p->iKeyLen == iKeyLen)
				if (memcmp(p->pKey, pKey, iKeyLen) == 0)
				{
					if (bCover)
					{
						if (pKeyExistProc)
						{
							if (pKeyExistProc(p))
								return FALSE;
						}

						if (hHT->bValueInt)
							p->uValue = uValue;
						else
						{
							if (hHT->pValueProc)
								hHT->pValueProc(p->pValue);
							p->pValue = (void*)pValue;
							p->iValueLen = iValueLen;
						}
						return TRUE;
					}
					else
						return FALSE;
				}

			if (p->pNext)
				p = p->pNext;
			else
				break;
		}

		p->pNext = (QKHASHTABLELINKNODE*)HeapAlloc(hHT->hHeap, HEAP_ZERO_MEMORY, sizeof(QKHASHTABLELINKNODE));
		p = p->pNext;
	}
	else
	{
		p = (QKHASHTABLELINKNODE*)HeapAlloc(hHT->hHeap, HEAP_ZERO_MEMORY, sizeof(QKHASHTABLELINKNODE));
		QKASet(hHT->ary, iIndex, p);
	}

	if (!p)
		return FALSE;

	p->uHashCode = uHashCode;
	p->pKey = (void*)pKey;
	p->iKeyLen = iKeyLen;

	if (hHT->bValueInt)
		p->uValue = uValue;
	else
	{
		p->pValue = (void*)pValue;
		p->iValueLen = iValueLen;
	}
	return TRUE;
}
BOOL QKHTGet(QKHASHTABLE hHT, PCVOID pKey, int iKeyLen, void* ppValue, BOOL bHasHashCode, UINT uHashCode)
{
	assert(hHT);
	if (iKeyLen <= 0)
		iKeyLen = (lstrlenW((PCWSTR)pKey) + 1) * sizeof(WCHAR);
	assert(iKeyLen > 0);
	if (!bHasHashCode)
		uHashCode = QKHTGetHashCode(pKey, iKeyLen);
	int iIndex = (uHashCode & 0x7FFFFFFF) % hHT->iP;

	auto p = (QKHASHTABLELINKNODE*)QKAGet(hHT->ary, iIndex);
	if (p)
	{
		do
		{
			if (p->iKeyLen == iKeyLen)
				if (memcmp(p->pKey, pKey, iKeyLen) == 0)
				{
					if (hHT->bValueInt)
						*((UINT*)ppValue) = p->uValue;
					else
						*((void**)ppValue) = p->pValue;
					return TRUE;
				}

			p = p->pNext;
		} while (p);
	}
	*((void**)ppValue) = NULL;
	return FALSE;
}
BOOL QKHTDelKey(QKHASHTABLE hHT, PCVOID pKey, int iKeyLen, BOOL bHasHashCode, UINT uHashCode)
{
	assert(hHT);
	if (iKeyLen <= 0)
		iKeyLen = (lstrlenW((PCWSTR)pKey) + 1) * sizeof(WCHAR);
	assert(iKeyLen > 0);
	if (!bHasHashCode)
		uHashCode = QKHTGetHashCode(pKey, iKeyLen);
	int iIndex = (uHashCode & 0x7FFFFFFF) % hHT->iP;

	auto p = (QKHASHTABLELINKNODE*)QKAGet(hHT->ary, iIndex);
	QKHASHTABLELINKNODE* pPrev = NULL;
	if (p)
	{
		do
		{
			if (p->iKeyLen == iKeyLen)
				if (memcmp(p->pKey, pKey, iKeyLen) == 0)
				{
					if (pPrev)
						pPrev = p->pNext;
					else
						QKASet(hHT->ary, iIndex, p->pNext);

					if (hHT->pKeyProc)
						hHT->pKeyProc(p->pKey);
					if (hHT->pValueProc)
						hHT->pValueProc(p->pKey);
					HeapFree(hHT->hHeap, 0, p);
					return TRUE;
				}

			pPrev = p;
			p = p->pNext;
		} while (p);
	}
	return FALSE;
}
void QKHTDelete(QKHASHTABLE hHT)
{
	if (!hHT)
		return;
	QKHASHTABLELINKNODE* p, * pPrev;
	for (int i = 0; i < hHT->iCount; ++i)
	{
		p = (QKHASHTABLELINKNODE*)QKAGet(hHT->ary, i);
		while (p)
		{
			if (!hHT->bKeyInt && hHT->pKeyProc)
				hHT->pKeyProc(p->pKey);

			if (!hHT->bValueInt && hHT->pValueProc && p->pValue)
				hHT->pValueProc(p->pValue);

			pPrev = p;
			p = p->pNext;
			HeapFree(hHT->hHeap, 0, pPrev);
		}
	}
	QKADelete(hHT->ary);
	HeapFree(hHT->hHeap, 0, hHT);
}
void QKHTEnum(QKHASHTABLE hHT, QKHTENUMPROC pProc, LPARAM lParam)
{
	assert(hHT);
	QKHASHTABLELINKNODE* p;
	for (int i = 0; i < hHT->iCount; ++i)
	{
		p = (QKHASHTABLELINKNODE*)QKAGet(hHT->ary, i);
		while (p)
		{
			if (!pProc(p, lParam))
				return;
			p = p->pNext;
		}
	}
}
/////////////////////////////////////��ϣ���