#include "windows.h"
#include "CommCtrl.h"
#include "shlobj_core.h"
#include "shlwapi.h"
#include "Function.h"
#include "math.h"
#include "GlobalVar.h"
#include "resource.h"

#include "MainWnd.h"

HFONT QKCreateFont(LPCWSTR szFontName, int nPoint, int nWeight, bool IsItalic, bool IsUnderline, bool IsStrikeOut)
{
    //for the MM_TEXT mapping mode, you can use the following formula to specify a height for a font with a specified point size:
    //MM_TEXT映射模式下可用以下公式来计算指定磅值字体的高度：
    //Height = -MulDiv(PointSize, GetDeviceCaps(hDC, LOGPIXELSY), 72);     （摘自MSDN）
    HDC hDC = GetDC(NULL);
    int nSize;
    nSize = -MulDiv(nPoint, GetDeviceCaps(hDC, LOGPIXELSY), 72);
    //MulDiv(a, b, c) 就是计算 a * b / c ，不过在 a * b > 2^32 时仍保证结果正确
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
    return pArrayNew->iCount++;//返回先前值，因为索引与数量差一
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
    return ((QKARRAYHEADER*)pArrayNew)->iCount++;//返回先前值，因为索引与数量差一
}
int QKArray_Insert(QKARRAY* ppArray, LPVOID pMember, int iIndex)//在iIndex处插入
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
LPVOID QKArray_Get(QKARRAY pArray, DWORD dwIndex)//从0开始
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
void QKArray_Set(QKARRAY pArray, DWORD dwIndex, LPVOID pMember, DWORD dwDeleteFlag)//从0开始
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
    if (dwIndex != dwCount - 1)//欲删除项目不在末尾
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
    while (pszOrg[i]==' '|| pszOrg[i] == '　'|| pszOrg[i] == '\t')
        ++i;
    j = lstrlenW(pszOrg) - 1;
    while (pszOrg[j] == ' ' || pszOrg[j] == '　' || pszOrg[j] == '\t')
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
    PCWSTR pszChackBoxTitle,//设为Null则不显示复选框
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
        tdb[0].pszButtonText = !pszButton1Title ? L"确定(&O)" : pszButton1Title;
    case 2:
        tdb[0].nButtonID = QKMSGBOX_BTID_1;
        tdb[0].pszButtonText = !pszButton1Title ? L"是(&Y)" : pszButton1Title;
        tdb[1].nButtonID = QKMSGBOX_BTID_2;
        tdb[1].pszButtonText = !pszButton2Title ? L"否(&N)" : pszButton2Title;
    case 3:
        tdb[0].nButtonID = QKMSGBOX_BTID_1;
        tdb[0].pszButtonText = !pszButton1Title ? L"是(&Y)" : pszButton1Title;
        tdb[1].nButtonID = QKMSGBOX_BTID_2;
        tdb[1].pszButtonText = !pszButton2Title ? L"否(&N)" : pszButton2Title;
        tdb[2].nButtonID = QKMSGBOX_BTID_3;
        tdb[2].pszButtonText = !pszButton3Title ? L"取消(&C)" : pszButton3Title;
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
    gr[0].UpperLeft = 0;//左上角坐标为第一个成员
    gr[0].LowerRight = 1;//右下角坐标为第二个成员

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
 * 目标：读ID3v2辅助函数：按指定编码处理文本
 *
 * 参数：
 * pStream 字节流指针；未指定iTextEncoding时指向整个文本帧，指定iTextEncoding时指向字符串
 * iLength 长度；未指定iTextEncoding时表示整个文本帧长度（包括1B的编码标记，不含结尾NULL），指定iTextEncoding时表示字符串长度（不含结尾NULL）
 * iTextEncoding 自定义文本编码；-1（缺省）指示处理的是文本帧
 *
 * 返回值：返回UTF-16LE文本指针，使用完毕后必须使用delete[]删除
 * 操作简述：
 * 备注：
 */
PWSTR GetMP3ID3v2_ProcString(BYTE* pStream, int iLength, int iTextEncoding = -1)
{
    int iType = 0, iBufferSize;
    PWSTR pBuffer = NULL;
    if (iTextEncoding == -1)
    {
        memcpy(&iType, pStream, 1);
        ++pStream;// 跳过文本编码标志
        --iLength;
    }
    else
        iType = iTextEncoding;

    switch (iType)
    {
    case 0://ISO-8859-1，即Latin-1（拉丁语-1）
        iBufferSize = MultiByteToWideChar(CP_ACP, 0, (PCCH)pStream, iLength, NULL, 0);
        if (iBufferSize == 0)
            return NULL;
        pBuffer = new WCHAR[iBufferSize + 1];//包含结尾NULL
        ZeroMemory(pBuffer, (iBufferSize + 1) * sizeof(WCHAR));
        MultiByteToWideChar(CP_ACP, 0, (PCCH)pStream, iLength, pBuffer, iBufferSize);//iLength不包含结尾NULL，因此转换后的字符串也不会包含
        break;
    case 1://UTF-16LE
        iBufferSize = iLength / sizeof(WCHAR) + 1;
        pBuffer = new WCHAR[iBufferSize];
        lstrcpynW(pBuffer, (PWSTR)pStream, iBufferSize);//包括结尾NULL
        break;
    case 2://UTF-16BE
        iBufferSize = iLength / sizeof(WCHAR);
        pBuffer = new WCHAR[iBufferSize + 1];
        LCMapStringEx(LOCALE_NAME_USER_DEFAULT, LCMAP_BYTEREV, (PCWSTR)pStream, iBufferSize, pBuffer, iBufferSize, NULL, NULL, 0);// 反转字节序
        ZeroMemory(pBuffer + iBufferSize, sizeof(WCHAR));// 添加结尾NULL
        break;
    case 3://UTF-8
        iBufferSize = MultiByteToWideChar(CP_UTF8, 0, (PCCH)pStream, iLength, NULL, 0);
        if (iBufferSize == 0)
            return NULL;
        pBuffer = new WCHAR[iBufferSize + 1];
        ZeroMemory(pBuffer, (iBufferSize + 1) * sizeof(WCHAR));
        MultiByteToWideChar(CP_UTF8, 0, (PCCH)pStream, iLength, pBuffer, iBufferSize);//iLength不包含结尾NULL，因此转换后的字符串也不会包含
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
    ReadFile(hFile, by, 4, &dwLengthRead, NULL);// 读文件头
    if (memcmp(by, "ID3", 3) == 0)
    {
        ID3v2_Header Header = { 0 };
        SetFilePointer(hFile, 0, NULL, FILE_BEGIN);
        ReadFile(hFile, &Header, sizeof(Header), &dwLengthRead, NULL);//读出标签头
        if (dwLengthRead < sizeof(ID3v2_Header))
            return;
        if (Header.Ver == 3)// ID3v2.3标签(https://id3.org)
        {
            DWORD dwTotalSize =
                ((Header.Size[0] & 0x7F) << 21) |
                ((Header.Size[1] & 0x7F) << 14) |
                ((Header.Size[2] & 0x7F) << 7) |
                (Header.Size[3] & 0x7F);// 28位数据，包括标签头
            HANDLE hMapping = CreateFileMappingW(hFile, NULL, PAGE_READONLY, 0, dwTotalSize, NULL);//映射ID3v2到内存
            BYTE* pFile = (BYTE*)MapViewOfFile(hMapping, FILE_MAP_READ, 0, 0, dwTotalSize);
            //参考资料：https://blog.csdn.net/u010650845/article/details/53520426
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
                        ((ID3v2_UnitHeader*)pFrame)->Size[3];// 32位数据，不包括帧头
                    memcpy(FrameID, ((ID3v2_UnitHeader*)pFrame)->ID, 4);// 读帧标识
                    pFrame += sizeof(ID3v2_UnitHeader);// 跳过帧头
                    if (memcmp(FrameID, "TIT2", 4) == 0)// 标题
                        pmi->pszTitle = GetMP3ID3v2_ProcString(pFrame, dwUnitSize);
                    else if (memcmp(FrameID, "TPE1", 4) == 0)// 作者
                        pmi->pszArtist = GetMP3ID3v2_ProcString(pFrame, dwUnitSize);
                    else if (memcmp(FrameID, "TALB", 4) == 0)// 专辑
                        pmi->pszAlbum = GetMP3ID3v2_ProcString(pFrame, dwUnitSize);
                    else if (memcmp(FrameID, "USLT", 4) == 0)// 不同步歌词
                    {
                        /*
                        <帧头>（帧标识为USLT）
                        文本编码						$xx
                        自然语言代码					$xx xx xx
                        内容描述						<字符串> $00 (00)
                        歌词							<字符串>
                        */
                        DWORD cb = dwUnitSize;
                        BYTE byEncodeingType = *pFrame;// 读文本编码
                        ++pFrame;// 跳过文本编码
                        CHAR byLangCode[3];
                        memcpy(byLangCode, pFrame, 3);// 读自然语言代码
                        pFrame += 3;// 跳过自然语言代码
                        int t;
                        if (byEncodeingType == 0 || byEncodeingType == 3)// ISO-8859-1或UTF-8
                            t = lstrlenA((PCSTR)pFrame) + 1;
                        else// UTF-16LE或UTF-16BE
                            t = (lstrlenW((PCWSTR)pFrame) + 1) * sizeof(WCHAR);
                        pFrame += t;// 跳过内容描述
                        cb -= (t + 4);
                        // 此时pFrame指向歌词文本
                        pmi->pszLrc = GetMP3ID3v2_ProcString(pFrame, cb, byEncodeingType);
                    }
                    else if (memcmp(FrameID, "COMM", 4) == 0)// 备注
                    {
                        /*
                        <帧头>（帧标识为COMM）
                        文本编码						$xx
                        自然语言代码					$xx xx xx
                        备注摘要						<字符串> $00 (00)
                        备注							<字符串>
                        */
                        DWORD cbComment = dwUnitSize;
                        BYTE byEncodeingType = *pFrame;// 读文本编码
                        ++pFrame;// 跳过文本编码
                        CHAR byLangCode[3];
                        memcpy(byLangCode, pFrame, 3);// 读自然语言代码
                        pFrame += 3;// 跳过自然语言代码
                        int t;
                        if (byEncodeingType == 0 || byEncodeingType == 3)// ISO-8859-1或UTF-8
                            t = lstrlenA((PCSTR)pFrame) + 1;
                        else// UTF-16LE或UTF-16BE
                            t = (lstrlenW((PCWSTR)pFrame) + 1) * sizeof(WCHAR);
                        pFrame += t;// 跳过备注摘要
                        cbComment -= (t + 4);
                        // 此时pFrame指向备注字符串
                        pmi->pszComment = GetMP3ID3v2_ProcString(pFrame, cbComment, byEncodeingType);
                    }
                    else if (memcmp(FrameID, "APIC", 4) == 0)// 图片
                    {
                        /*
                        <帧头>（帧标识为APIC）
                        文本编码                        $xx
                        MIME 类型                       <ASCII字符串>$00（如'image/bmp'）
                        图片类型                        $xx
                        描述                            <字符串>$00(00)
                        <图片数据>
                        */
                        BYTE* pImageData = pFrame;
                        BYTE byEncodeingType = *pImageData;// 读文本编码
                        int t, cbImageSize = dwUnitSize;
                        ++pImageData;// 跳过文本编码
                        t = lstrlenA((PCSTR)pImageData);
                        pImageData += t;// 跳过MIME类型字符串
                        pImageData += 2;// 跳过MIME结尾NULL和图片类型
                        cbImageSize -= (t + 3);
                        if (byEncodeingType == 0 || byEncodeingType == 3)// ISO-8859-1或UTF-8
                            t = lstrlenA((PCSTR)pImageData) + 1;
                        else// UTF-16LE或UTF-16BE
                            t = lstrlenW((PCWSTR)pImageData) * sizeof(WCHAR) + 2;
                        pImageData += t; cbImageSize -= t;// 跳过描述字符串和结尾NULL

                        IStream* pPicStream = SHCreateMemStream(pImageData, cbImageSize);// 创建流对象
                        if (pPicStream)
                        {
                            if (GdipCreateBitmapFromStream(pPicStream, &pmi->pGdipImage) != Ok)// 创建自字节流
                                pmi->pGdipImage = NULL;
                            pPicStream->Release();
                        }
                    }
                    dwOffest += (dwUnitSize + sizeof(ID3v2_UnitHeader));
                    if (dwOffest >= dwTotalSize)// 是否超界
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
            case 4:// 标签信息，注意：这一部分是小端序
            {
                ReadFile(hFile, &t, 4, &dwLengthRead, NULL);// 编码器信息大小
                SetFilePointer(hFile, t, NULL, FILE_CURRENT);// 跳过编码器信息
                UINT uCount;
                ReadFile(hFile, &uCount, 4, &dwLengthRead, NULL);// 标签数

                for (int i = 0; i < uCount; ++i)
                {
                    ReadFile(hFile, &t, 4, &dwLengthRead, NULL);// 标签大小
                    pBuffer = HeapAlloc(GetProcessHeap(), 0, t + 1);
                    ReadFile(hFile, pBuffer, t, &dwLengthRead, NULL);// 读标签
                    *(CHAR*)((BYTE*)pBuffer + t) = '\0';

                    t = MultiByteToWideChar(CP_UTF8, 0, (CHAR*)pBuffer, -1, NULL, 0);
                    PWSTR pBuf = (PWSTR)HeapAlloc(GetProcessHeap(), 0, t * sizeof(WCHAR));
                    MultiByteToWideChar(CP_UTF8, 0, (CHAR*)pBuffer, -1, pBuf, t);// 转换编码
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
            case 6:// 图片
            {
                SetFilePointer(hFile, 4, NULL, FILE_CURRENT);// 跳过图片类型

                ReadFile(hFile, &t, 4, &dwLengthRead, NULL);
                t = QKByteStreamToBEUINT32((BYTE*)&t);
                SetFilePointer(hFile, t, NULL, FILE_CURRENT);// 跳过MIME类型字符串

                ReadFile(hFile, &t, 4, &dwLengthRead, NULL);
                t = QKByteStreamToBEUINT32((BYTE*)&t);
                SetFilePointer(hFile, t, NULL, FILE_CURRENT);// 跳过描述字符串

                SetFilePointer(hFile, 16, NULL, FILE_CURRENT);// 跳过宽度、高度、色深、索引图颜色数

                ReadFile(hFile, &t, 4, &dwLengthRead, NULL);
                t = QKByteStreamToBEUINT32((BYTE*)&t);// 图片数据长度

                pBuffer = HeapAlloc(GetProcessHeap(), 0, t);
                ReadFile(hFile, pBuffer, t, &dwLengthRead, NULL);// 读图片
                IStream* pPicStream = SHCreateMemStream((BYTE*)pBuffer, t);// 创建流对象
                if (pPicStream)
                {
                    if (GdipCreateBitmapFromStream(pPicStream, &pmi->pGdipImage) != Ok)// 创建自字节流
                        pmi->pGdipImage = NULL;
                    pPicStream->Release();
                }
                HeapFree(GetProcessHeap(), 0, pBuffer);
            }
            break;
            default:
                SetFilePointer(hFile, dwSize, NULL, FILE_CURRENT);// 跳过块
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
 * 目标：处理歌词时间标签
 *
 * 参数：
 * TimeLabel 时间标签数组
 * pszLrc 与时间标签数组里所有成员都对应的歌词
 *
 * 返回值：
 * 操作简述：将文本标签转换成浮点数，并将其按次序装载到歌词数组中
 * 备注：读取歌词数据辅助函数，处理完成后不销毁原数组
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
            continue;// 没冒号，到循环尾
        iStrPos2 = QKStrInStr(pszTimeLabel, L":", iStrPos1 + 1);
        PWSTR pszTempTime;
        DWORD dwLength;
        int M, S, MS;
        BOOL IsMS = TRUE;
        float fTime;
        if (iStrPos2 <= 0)// 是否[分:秒:毫秒]
        {
            iStrPos2 = QKStrInStr(pszTimeLabel, L".", iStrPos1 + 1);
            if (iStrPos2 <= 0)// 是否[分:秒.毫秒]
            {
                IsMS = FALSE;// [分:秒]
                iStrPos2 = lstrlenW(pszTimeLabel) + 1;
            }
        }
        ///////////////////取分钟
        dwLength = iStrPos1 - 1;
        pszTempTime = new WCHAR[dwLength + 1];
        lstrcpynW(pszTempTime, pszTimeLabel, dwLength + 1);
        if (!StrToIntExW(pszTempTime, STIF_DEFAULT, &M))
            continue;// 转换失败，到循环尾
        fTime = (float)M * 60.0f;
        delete[] pszTempTime;
        ///////////////////取秒
        dwLength = iStrPos2 - iStrPos1 - 1;
        pszTempTime = new WCHAR[dwLength + 1];
        lstrcpynW(pszTempTime, pszTimeLabel + iStrPos1, dwLength + 1);
        if (!StrToIntExW(pszTempTime, STIF_DEFAULT, &S))
            continue;
        fTime += S;
        delete[] pszTempTime;
        ///////////////////取毫秒
        if (IsMS)
        {
            dwLength = lstrlenW(pszTimeLabel) - iStrPos2;

            if (dwLength == 2)// 只有两位xx时表示xx0（精度降低了，单位是十毫秒）
            {
                ++dwLength;
                pszTempTime = new WCHAR[dwLength + 1];
                lstrcpynW(pszTempTime, pszTimeLabel + iStrPos2, dwLength + 1);
                lstrcatW(pszTempTime, L"0");// 补0
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
        p->cy = -1;// 负数无效
        p->iOrgLength = -1;// 缺省-1，说明没有第二行
        p->pszLrc = new WCHAR[lstrlenW(pszLrc) + 1];// 得拷贝一份，不同成员当然是互相独立的
        lstrcpyW(p->pszLrc, pszLrc);

        QKArray_Add(Result, p);
    }
}
/*
 * 目标：清除歌词数据
 *
 * 参数：
 * x 歌词数据数组
 *
 * 返回值：
 * 操作简述：
 * 备注：
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
    DWORD nBytes = 0;//UFT8可用1-6个字节编码,ASCII用一个字节
    UCHAR chr;
    BOOL bAllAscii = TRUE; //如果全部都是ASCII, 说明不是UTF-8
    for (i = 0; i < length; i++)
    {
        chr = *(str + i);
        if ((chr & 0x80) != 0) // 判断是否ASCII编码,如果不是,说明有可能是UTF-8,ASCII用7位编码,但用一个字节存,最高位标记为0,o0xxxxxxx
            bAllAscii = FALSE;
        if (nBytes == 0) //如果不是ASCII码,应该是多字节符,计算字节数
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
        else //多字节符的非首字节,应为 10xxxxxx
        {
            if ((chr & 0xC0) != 0x80)
            {
                return FALSE;
            }
            nBytes--;
        }
    }
    if (nBytes > 0) //违返规则
    {
        return FALSE;
    }
    if (bAllAscii) //如果全部都是ASCII, 说明不是UTF-8
    {
        return FALSE;
    }
    return TRUE;
}
/*
 * 目标：读取歌词数据
 *
 * 参数：
 * pStream 输入流，文件名或LRC文件数据字节流。当作为字节流输入时，函数复制缓冲区并使用副本
 * iSize 若输入LRC字节流，则该参数指示字节流长度
 * bFileName 指示pStream是否为文件名
 * Result 结果数组，调用函数前数组应初始化完毕
 * iDefTextCode 默认文本编码，0 自动；1 GB2312；2 UTF-8；3 UTF-16LE；4 UTF-16BE
 *
 * 返回值：
 * 操作简述：
 * 备注：我写的这算法有点哈人
 */
void Lrc_ParseLrcData(void* pStream, int iSize, BOOL bFileName, QKARRAY* IDResult, QKARRAY* Result, int iDefTextCode)
{
    //                                                                          ↓这里是要释放的对象↓
    ////////////////////////////读入并按换行符分割文件
    PWSTR pBuffer = NULL;
    int iBufPtrOffest = 0;
    DWORD dwBytes;
    if (bFileName)
    {
        /////////////准备文件名
        WCHAR szLrcFile[MAX_PATH];
        lstrcpyW(szLrcFile, (PWSTR)pStream);
        PathRemoveExtensionW(szLrcFile);
        lstrcatW(szLrcFile, L".lrc");// 转换lrc扩展名
        /////////////打开
        HANDLE hFile = CreateFileW(
            szLrcFile,
            GENERIC_READ,
            FILE_SHARE_READ | FILE_SHARE_WRITE | FILE_SHARE_DELETE,
            NULL,
            OPEN_EXISTING,
            FILE_ATTRIBUTE_NORMAL,
            NULL
        );// 打开文件                                                                文件句柄
        if (hFile == INVALID_HANDLE_VALUE)// 打开失败
            return;
        dwBytes = GetFileSize(hFile, NULL);// 取字节数
        if (dwBytes < 5)// [a:b]  最短情况，处理这个主要为了对比BOM的时候不越界
        {
            CloseHandle(hFile);
            return;// 打开失败
            //                                                                      ***若出错退出，则释放
        }

        DWORD dwBytesRead;
        pBuffer = (PWSTR)HeapAlloc(GetProcessHeap(), 0, dwBytes + 2);// 分一块内存用来放文件信息（其实文件映射也可以的啦）
        *(WCHAR*)((BYTE*)pBuffer + dwBytes) = L'\0';// 把结尾NULL安上
        //                                                                          文件句柄，缓冲区
        ReadFile(hFile, pBuffer, dwBytes, &dwBytesRead, NULL);// 读入
        CloseHandle(hFile);
    }
    else
    {
        dwBytes = iSize;

        pBuffer = (PWSTR)HeapAlloc(GetProcessHeap(), 0, iSize + 2);
        memcpy(pBuffer, pStream, iSize);
        *(WCHAR*)((BYTE*)pBuffer + iSize) = L'\0';// 把结尾NULL安上
    }
    /////////////判断、转换编码
    DWORD dwLength;
    UINT uCode;

    CHAR cHeader[3] = { (CHAR)0xFF,(CHAR)0xFE,0 };// UTF-16LE BOM:FF FE
    if (memcmp(pBuffer, cHeader, 2) == 0)// 检查BOM
    {
        iBufPtrOffest = 2;// 跳过BOM

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
            iBufPtrOffest = 2;// 跳过BOM

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
            if (memcmp(pBuffer, cHeader, 3) == 0)// 检查BOM
            {
                iBufPtrOffest = 3;// 跳过BOM

                pBuffer = (PWSTR)((BYTE*)pBuffer + iBufPtrOffest);
                dwBytes -= iBufPtrOffest;

                *((CHAR*)pBuffer + dwBytes) = '\0';
                dwLength = MultiByteToWideChar(CP_UTF8, 0, (CHAR*)pBuffer, -1, NULL, 0);// 用了-1，返回值表示的字节数包含结尾NULL

                PWSTR pBuf = (PWSTR)HeapAlloc(GetProcessHeap(), 0, dwLength * sizeof(WCHAR));
                MultiByteToWideChar(CP_UTF8, 0, (CHAR*)pBuffer, -1, pBuf, dwLength);// 转换编码
                HeapFree(GetProcessHeap(), 0, (BYTE*)pBuffer - iBufPtrOffest);

                iBufPtrOffest = 0;
                pBuffer = pBuf;
                dwLength--;// 减掉结尾NULL
            }
            else// 无BOM
            {
                switch (iDefTextCode)
                {
                case 0:// 自动
                {
                    int i = IS_TEXT_UNICODE_REVERSE_MASK | IS_TEXT_UNICODE_NULL_BYTES;
                    if (IsTextUnicode(pBuffer, dwBytes, &i))//  先测UTF-16BE，不然会出问题
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
                    *((CHAR*)pBuffer + dwBytes) = '\0';// 截断
                }
                break;// 跳出switch转编码
                case 2:// UTF-8
                {
                GetLrc_UTF8:
                    uCode = CP_UTF8;
                    *((CHAR*)pBuffer + dwBytes) = '\0';// 截断
                }
                break;// 跳出switch转编码
                case 3:// UTF-16LE
                {
                GetLrc_UTF16LE:// 有BOM和自动判断的跳过来
                    dwLength = dwBytes / sizeof(WCHAR);
                    goto UTF16_SkipOthers;// 跳出去
                }
                break;
                case 4:// UTF-16BE
                {
                GetLrc_UTF16BE:// 有BOM和自动判断的跳过来
                    dwLength = dwBytes / sizeof(WCHAR);
                    PWSTR pBuf = (PWSTR)HeapAlloc(GetProcessHeap(), 0, dwBytes + sizeof(WCHAR));
                    LCMapStringEx(LOCALE_NAME_USER_DEFAULT, LCMAP_BYTEREV, (PCWSTR)pBuffer, dwLength, pBuf, dwLength, NULL, NULL, 0);// 反转字节序
                    *((WCHAR*)pBuf + dwLength) = L'\0';
                    HeapFree(GetProcessHeap(), 0, (BYTE*)pBuffer - iBufPtrOffest);
                    pBuffer = pBuf;
                    iBufPtrOffest = 0;
                    goto UTF16_SkipOthers;// 跳出去
                }
                break;
                }

                dwLength = MultiByteToWideChar(uCode, 0, (CHAR*)pBuffer, -1, NULL, 0);
                PWSTR pBuf = (PWSTR)HeapAlloc(GetProcessHeap(), 0, dwLength * sizeof(WCHAR));
                MultiByteToWideChar(uCode, 0, (CHAR*)pBuffer, -1, pBuf, dwLength);// 转换编码
                HeapFree(GetProcessHeap(), 0, (BYTE*)pBuffer - iBufPtrOffest);
                pBuffer = pBuf;
                dwLength--;// 减掉结尾NULL
                iBufPtrOffest = 0;
            }
        }
    }
UTF16_SkipOthers:// UTF-16的两种编码处理方式不同，它俩处理完后直接跳到这里
    //                                                                          缓冲区
    /////////////准备分割
    QKARRAY LrcLines = QKArray_Create(0);// 储存每行歌词
    //                                                                          缓冲区，文本行数组
    /////////////三种换行符
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

    PWSTR pszLine;// 每行内容
    int iLineLength;// 每行内容长度

    int iStrPos1;
    int iStrPos2 = 1;
    int iDivLength;

    if (!b1 && !b2 && !b3)// 无换行符
    {
        pszLine = new WCHAR[dwLength + 1];
        lstrcpynW(pszLine, pBuffer, dwLength + 1);//直接读到底
        QKArray_Add(&LrcLines, pszLine);
    }
    else
    {
        // 为什么要这么写？没错，有的歌词文件就是变态到几种换行符一起用.......
        // 其实这部分逻辑是之后加上的，之前的解决方案是让用户快爬（bushi）
        if (b1)// CRLF
        {
            // 思路：iStrPos1 = min(i1, i2, i3)
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
            // 思路：iStrPos1 = min(i2, i3)
            iDivLength = 1;
            if (b2 && b3)// 没有CRLF，但CR和LF同时存在
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
                lstrcpynW(pszLine, pBuffer + iStrPos2 - 1, iLineLength + 1);// 拷贝一行
                QKArray_Add(&LrcLines, pszLine);
            }
            iStrPos2 = iStrPos1 + iDivLength;// 跳过换行符
            /////////////取下一换行符位置
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
                else if (i3)// CR，这里跟上面是不一样的，必须用else if
                {
                    iDivLength = 1;
                    iStrPos1 = i3;
                }
            }
        }
        iLineLength = dwLength - iStrPos2 + 1;//处理末尾一行文本
        if (iLineLength > 0)
        {
            pszLine = new WCHAR[iLineLength + 1];
            lstrcpynW(pszLine, pBuffer + iStrPos2 - 1, iLineLength + 1);
            QKArray_Add(&LrcLines, pszLine);
        }
    }
    HeapFree(GetProcessHeap(), 0, (BYTE*)pBuffer - iBufPtrOffest);//释放文件内容缓冲区
    //                                                                          文本行数组
    //至此文件分割完毕
    ////////////////////////////处理每行标签
    int iStrPos3;
    QKARRAY SameUnitTimeLabel = NULL;
    PWSTR pszLrc;
    PWSTR pszTimeLabel;
    DWORD dwLrcLength;
    DWORD dwLength2;

    for (int i = 0; i < LrcLines->iCount; i++)// 读每一行
    {
        pszLine = (LPWSTR)QKArray_Get(LrcLines, i);// 读入一行
        iStrPos1 = QKStrInStr(pszLine, L"[");// 先找左中括号
        iStrPos2 = 1;
        if (iStrPos1 <= 0)// 找不到左中括号
            continue;// 到循环尾（处理下一行）   
        SameUnitTimeLabel = QKArray_Create(0);// 相同的时间标签
        //                                                                      文本行数组，同时间数组
        while (true)// 行中循环取标签（一行中可能有多个标签）
        {
            iStrPos2 = QKStrInStr(pszLine, L"]", iStrPos2);
            if (iStrPos2 <= 0 || iStrPos2 <= iStrPos1)
                iStrPos1 = iStrPos2 = 0;// 中括号错误，直接进行下一次循环，md歌词文件不规范的爬爬爬，劳资可不给你写容错（暴躁）
            dwLength = iStrPos2 - iStrPos1 - 1;
            iStrPos3 = QKStrInStr(pszLine, L"[", iStrPos1 + dwLength + 1);

            if (iStrPos3 - iStrPos2 - 1 == 0)// 紧贴着，类似于这种：[xx:xx][yy:yy]zzzzzzzzzzzzz
            {
                pszTimeLabel = new WCHAR[dwLength + 1];
                lstrcpynW(pszTimeLabel, pszLine + iStrPos1, dwLength + 1);
                QKArray_Add(&SameUnitTimeLabel, pszTimeLabel);
                iStrPos1 = iStrPos3;
                iStrPos2 = iStrPos1 + 1;
            }
            else if (iStrPos3 == 0 || iStrPos2 == 0)// 没有下一个标签了，这一行到头了
            {
                dwLrcLength = lstrlenW(pszLine) - iStrPos2;
                pszLrc = new WCHAR[dwLrcLength + 1];
                lstrcpynW(pszLrc, pszLine + iStrPos2, dwLrcLength + 1);// 取歌词

                pszTimeLabel = new WCHAR[dwLength + 1];
                lstrcpynW(pszTimeLabel, pszLine + iStrPos1, dwLength + 1);// 取标签
                QKArray_Add(&SameUnitTimeLabel, pszTimeLabel);

                GetLrcData_ProcLabel(Result, SameUnitTimeLabel, pszLrc);
                QKArray_Delete(SameUnitTimeLabel, QKADF_DELETEARRAY);
                delete[] pszLrc;
                break;
            }
            else// 处理完一行中的一句，类似于[xx:xx]aaaaaaaaa[yy:yy]bbbbbbbbbbbb，现在处理完a或b了
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
    //                                                                          ***无临时对象
    ////////////////////////////排序数组，便于合并歌词
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
    ////////////////////////////合并时间相同的歌词
    QKARRAY ArrLastTime = QKArray_Create(0);// 成员是float型
    QKARRAY ArrDelIndex = QKArray_Create(0);// 成员是int型
    //                                                                          上一时间数组，需删除索引数组
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

                if (iLen1 && !iLen2)// 只有第一个
                {
                }// 什么都不做
                else if (!iLen1 && iLen2)// 只有第二个
                {
                    pszNewLrc = new WCHAR[iLen2 + 1];
                    lstrcpyW(pszNewLrc, p2);
                    delete[] pItem->pszLrc;
                    pItem->pszLrc = pszNewLrc;
                    pItem->iOrgLength = -1;
                }
                else if (!iLen1 && !iLen2)// 两个都没有
                {
                    pszNewLrc = new WCHAR;
                    *pszNewLrc = '\0';
                    delete[] pItem->pszLrc;
                    pItem->pszLrc = pszNewLrc;
                    pItem->iOrgLength = -1;
                }
                else// 两个都有
                {
                    pItem->iOrgLength = iLen1;
                    pszNewLrc = new WCHAR[iLen1 + iLen2 + 2];
                    lstrcpyW(pszNewLrc, p1);
                    lstrcatW(pszNewLrc, L"\n");// 换行符连接
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
    //                                                                          需删除索引数组
    ////////////////////////////删除多余成员
    for (int i = 0; i < ArrDelIndex->iCount; i++)
    {
        int iIndex = *((int*)QKArray_GetValue(ArrDelIndex, ArrDelIndex->iCount - i - 1));// 倒着删
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
    //                                                                          ***无临时对象
    ////////////////////////////结束！！
}