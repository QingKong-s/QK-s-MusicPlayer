/*
* Function.cpp
* ���������Ժ�����ʵ��
*/
#include <Windows.h>
#include <CommCtrl.h>
#include <shlobj_core.h>
#include <shlwapi.h>

#include <math.h>

#include "Function.h"
#include "GlobalVar.h"
#include "resource.h"
#include "WndMain.h"

HFONT QKCreateFont(LPCWSTR szFontName, int nPoint, int nWeight, bool IsItalic, bool IsUnderline, bool IsStrikeOut)
{
    //for the MM_TEXT mapping mode, you can use the following formula to specify a height for a font with a specified point size:
    //MM_TEXTӳ��ģʽ�¿������¹�ʽ������ָ����ֵ����ĸ߶ȣ�
    //Height = -MulDiv(PointSize, GetDeviceCaps(hDC, LOGPIXELSY), 72);     ��ժ��MSDN��
    HDC hDC = GetDC(NULL);
    int nSize;
    nSize = -MulDiv(nPoint, GetDeviceCaps(hDC, LOGPIXELSY), 72);
    //MulDiv(a, b, c) ���Ǽ��� a * b / c �������� a * b > 2^32 ʱ�Ա�֤�����ȷ
    ReleaseDC(NULL, hDC);
    return CreateFontW(nSize, 0, 0, 0, nWeight, IsItalic, IsUnderline, IsStrikeOut, 0, 0, 0, 0, 0, szFontName);
}
QKARRAY QKArray_Create(int iCount)
{
    if (iCount < 0)
        return NULL;

    QKARRAY pArray = (QKARRAY)HeapAlloc(
        GetProcessHeap(),
        HEAP_ZERO_MEMORY,
        sizeof(QKARRAYHEADER) + sizeof(LPVOID) * iCount
    );
    if (!pArray)
        return NULL;

    pArray->iCount = iCount;
    return pArray;
}
void QKArray_Delete(QKARRAY pArray, DWORD dwDeleteFlag)
{
    if (!pArray)
        return;
    switch (dwDeleteFlag)
    {
    case QKADF_NO:
        break;
    case QKADF_DELETE:
        for (int i = 0; i < pArray->iCount; ++i)
            delete QKArray_Get(pArray, i);
        break;
    case QKADF_DELETEARRAY:
        for (int i = 0; i < pArray->iCount; ++i)
            delete[] QKArray_Get(pArray, i);
        break;
    case QKADF_HEAPFREE:
        for (int i = 0; i < pArray->iCount; ++i)
            HeapFree(GetProcessHeap(), 0, QKArray_Get(pArray, i));
        break;
    }
    HeapFree(GetProcessHeap(), 0, pArray);
}
void QKArray_Clear(QKARRAY pArray, DWORD dwDeleteFlag)
{
    if (!pArray)
        return;
    switch (dwDeleteFlag)
    {
    case QKADF_NO:
        break;
    case QKADF_DELETE:
        for (int i = 0; i < pArray->iCount; ++i)
            delete QKArray_Get(pArray, i);
        break;
    case QKADF_DELETEARRAY:
        for (int i = 0; i < pArray->iCount; ++i)
            delete[] QKArray_Get(pArray, i);
        break;
    case QKADF_HEAPFREE:
        for (int i = 0; i < pArray->iCount; ++i)
            HeapFree(GetProcessHeap(), 0, QKArray_Get(pArray, i));
        break;
    }
    pArray->iCount = 0;
}
int QKArray_Add(QKARRAY* ppArray, LPVOID pMember)
{
    QKARRAY pArray = *ppArray;
    if (!pArray)
        return -1;

    SIZE_T dwCurrSize = HeapSize(GetProcessHeap(), 0, pArray);
    QKARRAY pArrayNew = (QKARRAY)HeapReAlloc(
        GetProcessHeap(),
        0,
        pArray,
        dwCurrSize + sizeof(LPVOID));
    if (!pArrayNew)
        return -1;

    memcpy((CHAR*)pArrayNew + dwCurrSize, &pMember, sizeof(LPVOID));
    *ppArray = pArrayNew;
    return pArrayNew->iCount++;//������ǰֵ����Ϊ������������һ
}
int QKArray_AddValue(QKARRAY* ppArray, LPVOID pMember)
{
    if (*ppArray == NULL)
        return -1;
    DWORD dwCurrSize = HeapSize(GetProcessHeap(), 0, *ppArray);
    LPVOID pArrayNew = HeapReAlloc(
        GetProcessHeap(),
        0,
        *ppArray,
        dwCurrSize + sizeof(LPVOID));
    if (pArrayNew == NULL)
        return -1;

    memcpy((CHAR*)pArrayNew + dwCurrSize, pMember, sizeof(int));
    *ppArray = (QKARRAY)pArrayNew;
    return ((QKARRAYHEADER*)pArrayNew)->iCount++;//������ǰֵ����Ϊ������������һ
}
int QKArray_Insert(QKARRAY* ppArray, LPVOID pMember, int iIndex)//��iIndex������
{
    if (*ppArray == NULL)
        return -1;
    if (iIndex < 0 || iIndex > ((QKARRAYHEADER*)*ppArray)->iCount)
        return -1;
    DWORD dwCurrSize = HeapSize(GetProcessHeap(), 0, *ppArray);
    QKARRAY pArrayNew = (QKARRAY)HeapReAlloc(
        GetProcessHeap(),
        0,
        *ppArray,
        dwCurrSize + sizeof(LPVOID));
	if (!pArrayNew)
		return -1;
	BYTE* pChanging = (BYTE*)pArrayNew + sizeof(QKARRAYHEADER) + sizeof(LPVOID) * iIndex;

    if (iIndex != pArrayNew->iCount)
        memmove(pChanging + sizeof(LPVOID), pChanging, (pArrayNew->iCount - iIndex) * sizeof(LPVOID));
	memcpy(pChanging, &pMember, sizeof(LPVOID));
	pArrayNew->iCount++;
    *ppArray = pArrayNew;
    return iIndex;
}
LPVOID QKArray_Get(QKARRAY pArray, DWORD dwIndex)//��0��ʼ
{
    if (pArray == NULL)
        return NULL;
    return (LPVOID)(*(LPVOID*)((CHAR*)pArray + sizeof(QKARRAYHEADER) + sizeof(LPVOID) * dwIndex));
}
LPVOID QKArray_GetValue(QKARRAY pArray, DWORD dwIndex)
{
    if (pArray == NULL)
        return NULL;
    return (LPVOID)((CHAR*)pArray + sizeof(QKARRAYHEADER) + sizeof(LPVOID) * dwIndex);
}
void QKArray_Set(QKARRAY pArray, DWORD dwIndex, LPVOID pMember, DWORD dwDeleteFlag)//��0��ʼ
{
    if (pArray == NULL)
        return;
    if (dwIndex > ((QKARRAYHEADER*)pArray)->iCount - 1 || dwIndex < 0)
    {
        return;
    }
    LPVOID pMember1 = QKArray_Get(pArray, dwIndex);
    if (IsBadReadPtr(pMember, 1))
    {
        switch (dwDeleteFlag)
        {
        case QKADF_NO:
            break;
        case QKADF_DELETE:
            delete pMember1;
            break;
        case QKADF_DELETEARRAY:
            delete[] pMember1;
            break;
        case QKADF_HEAPFREE:
            HeapFree(GetProcessHeap(), 0, pMember1);
            break;
        }
    }
    memcpy((CHAR*)pArray + sizeof(QKARRAYHEADER) + sizeof(LPVOID) * dwIndex, &pMember, sizeof(LPVOID));
}
int QKArray_GetCount(QKARRAY pArray)
{
	if (pArray == NULL)
		return -1;
	return pArray->iCount;
}
void QKArray_DeleteMember(QKARRAY* ppArray, DWORD dwIndex, DWORD dwDeleteFlag)
{
    if (!*ppArray)
        return;
    DWORD dwCount = ((QKARRAYHEADER*)*ppArray)->iCount;
    if (dwIndex > dwCount - 1 || dwIndex < 0)
        return;

    LPVOID pMember = QKArray_Get(*ppArray, dwIndex);
    if (IsBadReadPtr(pMember, 1))
    {
        switch (dwDeleteFlag)
        {
        case QKADF_NO:
            break;
        case QKADF_DELETE:
            delete pMember;
            break;
        case QKADF_DELETEARRAY:
            delete[] pMember;
            break;
        case QKADF_HEAPFREE:
            HeapFree(GetProcessHeap(), 0, pMember);
            break;
        }
    }
    if (dwIndex != dwCount - 1)//��ɾ����Ŀ����ĩβ
    {
        CHAR* pBase = (CHAR*)*ppArray + sizeof(QKARRAYHEADER) + sizeof(LPVOID) * dwIndex;
        memmove(
            pBase, 
            pBase + sizeof(LPVOID), 
            (dwCount - dwIndex - 1) * sizeof(LPVOID));
    }
    DWORD dwCurrSize = dwCount * sizeof(LPVOID) + sizeof(QKARRAYHEADER);
    LPVOID pArrayNew = HeapReAlloc(
        GetProcessHeap(),
        HEAP_GENERATE_EXCEPTIONS,
        *ppArray,
        dwCurrSize - sizeof(LPVOID));
    *ppArray = (QKARRAY)pArrayNew;
    if (pArrayNew == NULL)
        return;

    ((QKARRAYHEADER*)pArrayNew)->iCount--;
}
LPVOID QKArray_GetDataPtr(QKARRAY pArray)
{
    return (BYTE*)pArray + sizeof(QKARRAYHEADER);
}
INT_PTR CALLBACK DlgProc_InputBox(HWND hDlg, UINT message, WPARAM wParam, LPARAM lParam)
{
    QKINPUTBOXCOMTEXT* pContext;
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
    while (pszOrg[i]==' '|| pszOrg[i] == '��'|| pszOrg[i] == '\t')
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
UINT QKMessageBox(
    PCWSTR pszMainInstruction,
    PCWSTR pszContent,
    HICON hIcon,
    PCWSTR pszWndTitle,
    HWND hParent,
    PCWSTR pszChackBoxTitle,//��ΪNull����ʾ��ѡ��
    UINT iButtonCount,
    PCWSTR pszButton1Title,
    PCWSTR pszButton2Title,
    PCWSTR pszButton3Title,
    UINT iDefButton,
    BOOL IsCenterPos,
    _Out_ BOOL* IsCheck
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
UINT32 QKByteStreamToBEUINT32(BYTE* p)
{
	return (UINT32)(*(p) << 24 | *(p + 1) << 16 | *(p + 2) << 8 | *(p + 3));
}

/*
 * Ŀ�꣺��ID3v2������������ָ�����봦���ı�
 *
 * ������
 * pStream �ֽ���ָ�룻δָ��iTextEncodingʱָ�������ı�֡��ָ��iTextEncodingʱָ���ַ���
 * iLength ���ȣ�δָ��iTextEncodingʱ��ʾ�����ı�֡���ȣ�����1B�ı����ǣ�������βNULL����ָ��iTextEncodingʱ��ʾ�ַ������ȣ�������βNULL��
 * iTextEncoding �Զ����ı����룻-1��ȱʡ��ָʾ���������ı�֡
 *
 * ����ֵ������UTF-16LE�ı�ָ�룬ʹ����Ϻ����ʹ��delete[]ɾ��
 * ����������
 * ��ע��
 */
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
        iBufferSize = iLength / sizeof(WCHAR) + 1;
        pBuffer = new WCHAR[iBufferSize];
        lstrcpynW(pBuffer, (PWSTR)pStream, iBufferSize);//������βNULL
        break;
    case 2://UTF-16BE
        iBufferSize = iLength / sizeof(WCHAR);
        pBuffer = new WCHAR[iBufferSize + 1];
        LCMapStringEx(LOCALE_NAME_USER_DEFAULT, LCMAP_BYTEREV, (PCWSTR)pStream, iBufferSize, pBuffer, iBufferSize, NULL, NULL, 0);// ��ת�ֽ���
        ZeroMemory(pBuffer + iBufferSize, sizeof(WCHAR));// ���ӽ�βNULL
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
    if (memcmp(by, "ID3", 3) == 0)
    {
        ID3v2_Header Header = { 0 };
        SetFilePointer(hFile, 0, NULL, FILE_BEGIN);
        ReadFile(hFile, &Header, sizeof(Header), &dwLengthRead, NULL);//������ǩͷ
        if (dwLengthRead < sizeof(ID3v2_Header))
            return;
        if (Header.Ver == 3)// ID3v2.3��ǩ(https://id3.org)
        {
            DWORD dwTotalSize =
                ((Header.Size[0] & 0x7F) << 21) |
                ((Header.Size[1] & 0x7F) << 14) |
                ((Header.Size[2] & 0x7F) << 7) |
                (Header.Size[3] & 0x7F);// 28λ���ݣ�������ǩͷ
            HANDLE hMapping = CreateFileMappingW(hFile, NULL, PAGE_READONLY, 0, dwTotalSize, NULL);//ӳ��ID3v2���ڴ�
            BYTE* pFile = (BYTE*)MapViewOfFile(hMapping, FILE_MAP_READ, 0, 0, dwTotalSize);
            //�ο����ϣ�https://blog.csdn.net/u010650845/article/details/53520426
            if (pFile)
            {
                BYTE* pFrame;
                DWORD dwOffest = sizeof(ID3v2_Header);
                DWORD dwUnitSize;
                CHAR FrameID[4];
                while (true)
                {
                    pFrame = pFile + dwOffest;
                    dwUnitSize =
                        (((ID3v2_UnitHeader*)pFrame)->Size[0] << 24) |
                        (((ID3v2_UnitHeader*)pFrame)->Size[1] << 16) |
                        (((ID3v2_UnitHeader*)pFrame)->Size[2] << 8) |
                        ((ID3v2_UnitHeader*)pFrame)->Size[3];// 32λ���ݣ�������֡ͷ
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
                            if (GdipCreateBitmapFromStream(pPicStream, &pmi->pGdipImage) != Ok)// �������ֽ���
                                pmi->pGdipImage = NULL;
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
    }
    else if (memcmp(by, "fLaC", 4) == 0)
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

                for (int i = 0; i < uCount; ++i)
                {
                    ReadFile(hFile, &t, 4, &dwLengthRead, NULL);// ��ǩ��С
                    pBuffer = HeapAlloc(GetProcessHeap(), 0, t + 1);
                    ReadFile(hFile, pBuffer, t, &dwLengthRead, NULL);// ����ǩ
                    *(CHAR*)((BYTE*)pBuffer + t) = '\0';

                    t = MultiByteToWideChar(CP_UTF8, 0, (CHAR*)pBuffer, -1, NULL, 0);
                    PWSTR pBuf = (PWSTR)HeapAlloc(GetProcessHeap(), 0, t * sizeof(WCHAR));
                    MultiByteToWideChar(CP_UTF8, 0, (CHAR*)pBuffer, -1, pBuf, t);// ת������
                    HeapFree(GetProcessHeap(), 0, pBuffer);

                    UINT uPos = QKStrInStr(pBuf, L"=");

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
            case 6:// ͼƬ
            {
                SetFilePointer(hFile, 4, NULL, FILE_CURRENT);// ����ͼƬ����

                ReadFile(hFile, &t, 4, &dwLengthRead, NULL);
                t = QKByteStreamToBEUINT32((BYTE*)&t);
                SetFilePointer(hFile, t, NULL, FILE_CURRENT);// ����MIME�����ַ���

                ReadFile(hFile, &t, 4, &dwLengthRead, NULL);
                t = QKByteStreamToBEUINT32((BYTE*)&t);
                SetFilePointer(hFile, t, NULL, FILE_CURRENT);// ���������ַ���

                SetFilePointer(hFile, 16, NULL, FILE_CURRENT);// �������ȡ��߶ȡ�ɫ�����ͼ��ɫ��

                ReadFile(hFile, &t, 4, &dwLengthRead, NULL);
                t = QKByteStreamToBEUINT32((BYTE*)&t);// ͼƬ���ݳ���

                pBuffer = HeapAlloc(GetProcessHeap(), 0, t);
                ReadFile(hFile, pBuffer, t, &dwLengthRead, NULL);// ��ͼƬ
                IStream* pPicStream = SHCreateMemStream((BYTE*)pBuffer, t);// ����������
                if (pPicStream)
                {
                    if (GdipCreateBitmapFromStream(pPicStream, &pmi->pGdipImage) != Ok)// �������ֽ���
                        pmi->pGdipImage = NULL;
                    pPicStream->Release();
                }
                HeapFree(GetProcessHeap(), 0, pBuffer);
            }
            break;
            default:
                SetFilePointer(hFile, dwSize, NULL, FILE_CURRENT);// ������
            }

        } while (!(Header.by & 0x80));
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
    if (mi->pGdipImage)
        GdipDisposeImage(mi->pGdipImage);
    ZeroMemory(mi, sizeof(MUSICINFO));
}
/*
 * Ŀ�꣺�������ʱ���ǩ
 *
 * ������
 * TimeLabel ʱ���ǩ����
 * pszLrc ��ʱ���ǩ���������г�Ա����Ӧ�ĸ��
 *
 * ����ֵ��
 * �������������ı���ǩת���ɸ������������䰴����װ�ص����������
 * ��ע����ȡ������ݸ���������������ɺ�����ԭ����
 */
void GetLrcData_ProcLabel(QKARRAY* Result, QKARRAY TimeLabel, LPWSTR pszLrc)
{
    if (!TimeLabel)
        return;
    if (TimeLabel->iCount <= 0)
        return;

    for (int i = 0; i < TimeLabel->iCount; i++)
    {
        LPWSTR pszTimeLabel = (LPWSTR)QKArray_Get(TimeLabel, i);
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

        QKArray_Add(Result, p);
    }
}
/*
 * Ŀ�꣺����������
 *
 * ������
 * x �����������
 *
 * ����ֵ��
 * ����������
 * ��ע��
 */
void Lrc_ClearArray(QKARRAY x)
{
    for (int i = 0; i < x->iCount; ++i)
    {
        delete[]((LRCDATA*)QKArray_Get(x, i))->pszLrc;
    }
    QKArray_Delete(x, QKADF_DELETE);
}
int QKIsTextUTF8(char* str, ULONGLONG length)
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
/*
 * Ŀ�꣺��ȡ�������
 *
 * ������
 * pStream ���������ļ�����LRC�ļ������ֽ���������Ϊ�ֽ�������ʱ���������ƻ�������ʹ�ø���
 * iSize ������LRC�ֽ�������ò���ָʾ�ֽ�������
 * bFileName ָʾpStream�Ƿ�Ϊ�ļ���
 * Result ������飬���ú���ǰ����Ӧ��ʼ�����
 * iDefTextCode Ĭ���ı����룬0 �Զ���1 GB2312��2 UTF-8��3 UTF-16LE��4 UTF-16BE
 *
 * ����ֵ��
 * ����������
 * ��ע����д�����㷨�е����
 */
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
        if (memcmp(pBuffer, cHeader, 2) == 0)// UTF-16BE
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
                        goto GetLrc_UTF16LE;
                    else
                    {
                        i = IS_TEXT_UNICODE_UNICODE_MASK | IS_TEXT_UNICODE_NULL_BYTES;
                        if (IsTextUnicode(pBuffer, dwBytes, &i))
                            goto GetLrc_UTF16BE;
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
UTF16_SkipOthers:// UTF-16�����ֱ��봦����ʽ��ͬ�������������ֱ����������
    //                                                                          ������
    /////////////׼���ָ�
    QKARRAY LrcLines = QKArray_Create(0);// ����ÿ�и��
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
        QKArray_Add(&LrcLines, pszLine);
    }
    else
    {
        // ΪʲôҪ��ôд��û�����еĸ���ļ����Ǳ�̬�����ֻ��з�һ����.......
        // ��ʵ�ⲿ���߼���֮����ϵģ�֮ǰ�Ľ�����������û�������bushi��
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
                QKArray_Add(&LrcLines, pszLine);
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
        iLineLength = dwLength - iStrPos2 + 1;//����ĩβһ���ı�
        if (iLineLength > 0)
        {
            pszLine = new WCHAR[iLineLength + 1];
            lstrcpynW(pszLine, pBuffer + iStrPos2 - 1, iLineLength + 1);
            QKArray_Add(&LrcLines, pszLine);
        }
    }
    HeapFree(GetProcessHeap(), 0, (BYTE*)pBuffer - iBufPtrOffest);//�ͷ��ļ����ݻ�����
    //                                                                          �ı�������
    //�����ļ��ָ����
    ////////////////////////////����ÿ�б�ǩ
    int iStrPos3;
    QKARRAY SameUnitTimeLabel = NULL;
    PWSTR pszLrc;
    PWSTR pszTimeLabel;
    DWORD dwLrcLength;
    DWORD dwLength2;

    for (int i = 0; i < LrcLines->iCount; i++)// ��ÿһ��
    {
        pszLine = (LPWSTR)QKArray_Get(LrcLines, i);// ����һ��
        iStrPos1 = QKStrInStr(pszLine, L"[");// ������������
        iStrPos2 = 1;
        if (iStrPos1 <= 0)// �Ҳ�����������
            continue;// ��ѭ��β��������һ�У�   
        SameUnitTimeLabel = QKArray_Create(0);// ��ͬ��ʱ���ǩ
        //                                                                      �ı������飬ͬʱ������
        while (true)// ����ѭ��ȡ��ǩ��һ���п����ж����ǩ��
        {
            iStrPos2 = QKStrInStr(pszLine, L"]", iStrPos2);
            if (iStrPos2 <= 0 || iStrPos2 <= iStrPos1)
                iStrPos1 = iStrPos2 = 0;// �����Ŵ���ֱ�ӽ�����һ��ѭ����md����ļ����淶�������������ʿɲ�����д�ݴ������꣩
            dwLength = iStrPos2 - iStrPos1 - 1;
            iStrPos3 = QKStrInStr(pszLine, L"[", iStrPos1 + dwLength + 1);

            if (iStrPos3 - iStrPos2 - 1 == 0)// �����ţ����������֣�[xx:xx][yy:yy]zzzzzzzzzzzzz
            {
                pszTimeLabel = new WCHAR[dwLength + 1];
                lstrcpynW(pszTimeLabel, pszLine + iStrPos1, dwLength + 1);
                QKArray_Add(&SameUnitTimeLabel, pszTimeLabel);
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
                QKArray_Add(&SameUnitTimeLabel, pszTimeLabel);

                GetLrcData_ProcLabel(Result, SameUnitTimeLabel, pszLrc);
                QKArray_Delete(SameUnitTimeLabel, QKADF_DELETEARRAY);
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
                QKArray_Add(&SameUnitTimeLabel, pszTimeLabel);
                dwLength2 = iStrPos3 - iStrPos2 - 1;

                GetLrcData_ProcLabel(Result, SameUnitTimeLabel, pszLrc);
                QKArray_Delete(SameUnitTimeLabel, QKADF_DELETEARRAY);
                delete[] pszLrc;
                SameUnitTimeLabel = QKArray_Create(0);
                iStrPos1 = iStrPos3;
                iStrPos2 = iStrPos1 + 1;
            }
            iStrPos1 = iStrPos2;
        }
    }
    QKArray_Delete(LrcLines, QKADF_DELETEARRAY);
    //                                                                          ***����ʱ����
    ////////////////////////////�������飬���ںϲ����
    int iCount = (*Result)->iCount;
    LRCDATA* pi, * pj;
    for (int i = 0; i != iCount; ++i)
    {
        for (int j = 0; j != iCount; ++j)
        {
            pi = (LRCDATA*)QKArray_Get((*Result), i);
            pj = (LRCDATA*)QKArray_Get((*Result), j);
            if (pi->fTime < pj->fTime)
            {
                QKArray_Set((*Result), j, pi);
                QKArray_Set((*Result), i, pj);
            }
        }
    }
    ////////////////////////////�ϲ�ʱ����ͬ�ĸ��
    QKARRAY ArrLastTime = QKArray_Create(0);// ��Ա��float��
    QKARRAY ArrDelIndex = QKArray_Create(0);// ��Ա��int��
    //                                                                          ��һʱ�����飬��ɾ����������
    PWSTR p1, p2, pszNewLrc;
    LRCDATA* p, * pItem;
    for (int i = 0; i < iCount; i++)
    {
        p = (LRCDATA*)QKArray_Get(*Result, i);
        int iLastTimeCount = ArrLastTime->iCount;
        if (iLastTimeCount != 0 && i != 0)
        {
            if (*(float*)QKArray_GetValue(ArrLastTime, 0) == p->fTime)
            {
                pItem = (LRCDATA*)QKArray_Get(*Result, i - iLastTimeCount);
                p1 = pItem->pszLrc;
                p2 = p->pszLrc;
                QKStrTrim(p1);
                QKStrTrim(p2);
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
                    *pszNewLrc = '\0';
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
                QKArray_AddValue(&ArrDelIndex, &i);
            }
            else
            {
                QKArray_Delete(ArrLastTime);
                ArrLastTime = QKArray_Create(0);
            }
        }
        QKArray_AddValue(&ArrLastTime, &p->fTime);
    }
    QKArray_Delete(ArrLastTime);
    //                                                                          ��ɾ����������
    ////////////////////////////ɾ�������Ա
    for (int i = 0; i < ArrDelIndex->iCount; i++)
    {
        int iIndex = *((int*)QKArray_GetValue(ArrDelIndex, ArrDelIndex->iCount - i - 1));// ����ɾ
        LRCDATA* p = (LRCDATA*)QKArray_Get(*Result, iIndex);
        delete[] p->pszLrc;

        QKArray_DeleteMember(Result, iIndex, QKADF_DELETE);
    }
    QKArray_Delete(ArrDelIndex);
    for (int i = 0; i < (*Result)->iCount; i++)
    {
        p = (LRCDATA*)QKArray_Get(*Result, i);
        if (i != (*Result)->iCount - 1)
            p->fDelay = ((LRCDATA*)QKArray_Get(*Result, i + 1))->fTime - p->fTime;
        else
            p->fDelay = g_llLength / 1000 - p->fTime;
    }
    //                                                                          ***����ʱ����
    ////////////////////////////��������
}