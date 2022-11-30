#include "OLEDragDrop.h"

#include <Windows.h>
#include <ole2.h>
#include <ShObjIdl.h>
#include <shlobj_core.h>
#include <oleidl.h>
#include <shlobj.h>
#include <shlwapi.h>

#include "Function.h"

void QKMakeDropSource(QKARRAY Files, QKDRAGGIVEFEEDBACK pGiveFeedBack, CDataObject** ppDataObject, CDropSource** ppDropSource, BOOL fRelease)
{
    // 制格式描述
    FORMATETC fe =
    {
        CF_HDROP,
        NULL,
        DVASPECT_CONTENT,
        -1,
        TYMED_HGLOBAL
    };
    // 制存储介质
    STGMEDIUM sm;
    sm.tymed = TYMED_HGLOBAL;
    sm.pUnkForRelease = NULL;
    // 制拖放文件结构
    DROPFILES df =
    {
        sizeof(DROPFILES),
        { 0 },
        TRUE,
        TRUE
    };
    // 制HDROP
	SIZE_T uSize = sizeof(DROPFILES) + sizeof(WCHAR);// 列表以双空结尾，每个文件以单空分隔，先加一个宽字符
    int* iLength = new int[Files->iCount];
    for (int i = 0; i < Files->iCount; ++i)
    {
        iLength[i] = lstrlenW((PWSTR)QKArray_Get(Files, i));
        uSize += (iLength[i] + 1) * sizeof(WCHAR);
    }
	HGLOBAL hGlobal = GlobalAlloc(GMEM_MOVEABLE | GMEM_DDESHARE | GMEM_ZEROINIT, uSize);
    PWSTR p = (PWSTR)GlobalLock(hGlobal);
    memcpy(p, &df, sizeof(DROPFILES));
	p = (PWSTR)((BYTE*)p + sizeof(DROPFILES));
    for (int i = 0; i < Files->iCount; ++i)
    {
        lstrcpyW(p, (PWSTR)QKArray_Get(Files, i));
        PathFileExistsW((PWSTR)QKArray_Get(Files, i));
        p += (iLength[i] + 1);
    }
    GlobalUnlock(hGlobal);
    sm.hGlobal = hGlobal;
    // 制数据对象和拖放源
	*ppDataObject = new CDataObject(&fe, &sm, fRelease, 1);
    *ppDropSource = new CDropSource(pGiveFeedBack);
}

CDataObject::CDataObject(FORMATETC* fmtetc, STGMEDIUM* stgmed, BOOL fRelease, int iCount)
{
    m_uRefCount = 1;
    m_nNumFormats = iCount;

    m_pFormatEtc = new FORMATETC[iCount];
    m_pStgMedium = new STGMEDIUM[iCount];
    m_pbRelease = new BOOL[iCount];

    for (int i = 0; i < iCount; i++)
    {
        m_pFormatEtc[i] = fmtetc[i];
        m_pStgMedium[i] = stgmed[i];
        m_pbRelease[i] = fRelease;
    }
}
CDataObject::~CDataObject()
{
    delete[] m_pFormatEtc;
    delete[] m_pStgMedium;
    delete[] m_pbRelease;
}
int CDataObject::LookupFormatEtc(FORMATETC* pFormatEtc)
{
    for (int i = 0; i < m_nNumFormats; i++)
    {
        if ((m_pFormatEtc[i].tymed & pFormatEtc->tymed) &&
            m_pFormatEtc[i].cfFormat == pFormatEtc->cfFormat &&
            m_pFormatEtc[i].dwAspect == pFormatEtc->dwAspect)
            return i;
    }

    return -1;
}
HRESULT STDMETHODCALLTYPE CDataObject::QueryInterface(REFIID iid, void** ppvObject)
{
    static const QITAB qit[] =
    {
        QITABENT(CDataObject, IDataObject),
        { 0 }
    };

    return QISearch(this, qit, iid, ppvObject);
}
ULONG STDMETHODCALLTYPE CDataObject::AddRef(void)
{
    ++m_uRefCount;
    return m_uRefCount;
}
ULONG STDMETHODCALLTYPE CDataObject::Release(void)
{
    --m_uRefCount;
    ULONG i = m_uRefCount;
    if (m_uRefCount == 0)
	{
		for (int i = 0; i < m_nNumFormats; i++)
		{
			if (m_pbRelease[i])
				ReleaseStgMedium(m_pStgMedium + i);
		}
		delete this;
    }
    return i;
}
HRESULT STDMETHODCALLTYPE CDataObject::QueryGetData(FORMATETC* pFormatEtc)
{
    return (LookupFormatEtc(pFormatEtc) == -1) ? DV_E_FORMATETC : S_OK;
}
HRESULT STDMETHODCALLTYPE CDataObject::GetData(FORMATETC* pFormatEtc, STGMEDIUM* pStgMedium)
{
    int iIndex = LookupFormatEtc(pFormatEtc);

    if (iIndex == -1)
        return DV_E_FORMATETC;

    pStgMedium->tymed = m_pFormatEtc[iIndex].tymed;
    pStgMedium->pUnkForRelease = 0;

	switch (m_pFormatEtc[iIndex].tymed)
	{
	case TYMED_HGLOBAL:
	{
        HGLOBAL hG = m_pStgMedium[iIndex].hGlobal;
        int iSize = GlobalSize(hG);
		pStgMedium->hGlobal = GlobalAlloc(GMEM_ZEROINIT | GMEM_MOVEABLE | GMEM_DDESHARE, iSize);
        memcpy(GlobalLock(pStgMedium->hGlobal), GlobalLock(hG), iSize);
        GlobalUnlock(hG);
		GlobalUnlock(pStgMedium->hGlobal);
	}
	break;

	default:
		return DV_E_FORMATETC;
	}
	return S_OK;
}
HRESULT STDMETHODCALLTYPE CDataObject::EnumFormatEtc(DWORD dwDirection, IEnumFORMATETC** ppEnumFormatEtc)
{
    IEnumFORMATETC* p;
    SHCreateStdEnumFmtEtc(m_nNumFormats, m_pFormatEtc, &p);
    *ppEnumFormatEtc = p;
    return S_OK;
}
HRESULT STDMETHODCALLTYPE CDataObject::DAdvise(FORMATETC* pFormatEtc, DWORD advf, IAdviseSink* pAdvSink, DWORD* pdwConnection)
{
    return OLE_E_ADVISENOTSUPPORTED;
}
HRESULT STDMETHODCALLTYPE CDataObject::DUnadvise(DWORD dwConnection)
{
    return OLE_E_ADVISENOTSUPPORTED;
}
HRESULT STDMETHODCALLTYPE CDataObject::EnumDAdvise(IEnumSTATDATA** ppEnumAdvise)
{
    return OLE_E_ADVISENOTSUPPORTED;
}
HRESULT STDMETHODCALLTYPE CDataObject::GetDataHere(FORMATETC* pFormatEtc, STGMEDIUM* pMedium)
{
    return DATA_E_FORMATETC;
}
HRESULT STDMETHODCALLTYPE CDataObject::GetCanonicalFormatEtc(FORMATETC* pFormatEct, FORMATETC* pFormatEtcOut)
{
    pFormatEtcOut->ptd = NULL;
    return E_NOTIMPL;
}
HRESULT STDMETHODCALLTYPE CDataObject::SetData(FORMATETC* pFormatEtc, STGMEDIUM* pMedium, BOOL fRelease)
{
    int iIndex = LookupFormatEtc(pFormatEtc);
    if (iIndex == -1)
    {
        ++m_nNumFormats;
        FORMATETC* pf = new FORMATETC[m_nNumFormats];
        STGMEDIUM* ps = new STGMEDIUM[m_nNumFormats];
        BOOL* pb = new BOOL[m_nNumFormats];
        for (int i = 0; i < m_nNumFormats - 1; ++i)
        {
            pf[i] = m_pFormatEtc[i];
            ps[i] = m_pStgMedium[i];
            pb[i] = m_pbRelease[i];
        }
        pf[m_nNumFormats - 1] = pFormatEtc[0];
        ps[m_nNumFormats - 1] = pMedium[0];
        pb[m_nNumFormats - 1] = fRelease;
        delete[] m_pFormatEtc;
        delete[] m_pStgMedium;
        delete[] m_pbRelease;
        m_pFormatEtc = pf;
        m_pStgMedium = ps;
        m_pbRelease = pb;
    }
    else
    {
        m_pFormatEtc[iIndex] = pFormatEtc[0];
        m_pStgMedium[iIndex] = pMedium[0];
        m_pbRelease[iIndex] = fRelease;
    }

    return S_OK;
}

CDropSource::CDropSource(QKDRAGGIVEFEEDBACK pGiveFeedBack)
{
    m_uRefCount = 1;
    m_pGiveFeedBack = pGiveFeedBack;
}
HRESULT STDMETHODCALLTYPE CDropSource::QueryContinueDrag(BOOL fEscapePressed, DWORD grfKeyState)
{
    if (fEscapePressed)
        return DRAGDROP_S_CANCEL;

    if ((grfKeyState & MK_LBUTTON) == 0)
        return DRAGDROP_S_DROP;

    return S_OK;
}
HRESULT STDMETHODCALLTYPE CDropSource::GiveFeedback(DWORD dwEffect)
{
    return m_pGiveFeedBack(dwEffect);
}
HRESULT STDMETHODCALLTYPE CDropSource::QueryInterface(REFIID iid, void** ppvObject)
{
    static const QITAB qit[] =
    {
        QITABENT(CDropSource, IDropSource),
        { 0 }
    };

    return QISearch(this, qit, iid, ppvObject);
}
ULONG STDMETHODCALLTYPE CDropSource::AddRef(void)
{
    m_uRefCount++;
    return m_uRefCount;
}
ULONG STDMETHODCALLTYPE CDropSource::Release(void)
{
    m_uRefCount--;
    ULONG i = m_uRefCount;
    if (m_uRefCount == 0)
        delete this;
    return i;
}


CDropTarget::CDropTarget(QKONDRAGENTER pOnEnter, QKONDRAGOVER pOnOver, QKONDRAGLEAVE pOnLeave, QKONDRAGDROP pOnDrop)
{
    m_pOnEnter = pOnEnter;
    m_pOnOver = pOnOver;
    m_pOnLeave = pOnLeave;
    m_pOnDrop = pOnDrop;
    //CoCreateInstance(CLSID_DragDropHelper, NULL, CLSCTX_INPROC_SERVER, IID_PPV_ARGS(&m_pDropTargetHelper));
}
HRESULT STDMETHODCALLTYPE CDropTarget::QueryInterface(REFIID iid, void** ppvObject)
{
    static const QITAB qit[] =
    {
        QITABENT(CDropTarget, IDropTarget),
        { 0 }
    };

    return QISearch(this, qit, iid, ppvObject);
}
ULONG STDMETHODCALLTYPE CDropTarget::AddRef(void)
{
    m_uRefCount++;
    return m_uRefCount;
}
ULONG STDMETHODCALLTYPE CDropTarget::Release(void)
{
    m_uRefCount--;
    ULONG i = m_uRefCount;
    if (m_uRefCount == 0)
        delete this;
    return i;
}
HRESULT STDMETHODCALLTYPE CDropTarget::DragEnter(IDataObject* pDataObj, DWORD grfKeyState, POINTL pt, DWORD* pdwEffect)
{
    if (!pdwEffect)
        return E_INVALIDARG;

    return m_pOnEnter(pDataObj, grfKeyState, pt, pdwEffect);
}
HRESULT STDMETHODCALLTYPE CDropTarget::DragOver(DWORD grfKeyState, POINTL pt, DWORD* pdwEffect)
{
    if (!pdwEffect)
        return E_INVALIDARG;

    return m_pOnOver(grfKeyState, pt, pdwEffect);
}
HRESULT STDMETHODCALLTYPE CDropTarget::DragLeave(void)
{
    return m_pOnLeave();
}
HRESULT STDMETHODCALLTYPE CDropTarget::Drop(IDataObject* pDataObj, DWORD grfKeyState, POINTL pt, DWORD* pdwEffect)
{
    if (!pdwEffect)
        return E_INVALIDARG;

    return m_pOnDrop(pDataObj, grfKeyState, pt, pdwEffect);
}