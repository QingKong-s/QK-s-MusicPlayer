#pragma once
#include "Windows.h"
#include "Function.h"

typedef HRESULT(CALLBACK* QKDRAGGIVEFEEDBACK)(DWORD);
typedef HRESULT(CALLBACK* QKONDRAGENTER)(IDataObject*, DWORD, POINTL, DWORD*);
typedef HRESULT(CALLBACK* QKONDRAGOVER)(DWORD, POINTL, DWORD*);
typedef HRESULT(CALLBACK* QKONDRAGLEAVE)(void);
typedef HRESULT(CALLBACK* QKONDRAGDROP)(IDataObject*, DWORD, POINTL, DWORD*);

class CDropTarget : public IDropTarget
{
public:
    CDropTarget(QKONDRAGENTER pOnEnter, QKONDRAGOVER pOnOver, QKONDRAGLEAVE pOnLeave, QKONDRAGDROP pOnDrop);
    // IUnknown
    IFACEMETHODIMP QueryInterface(REFIID riid, void** ppv);
    IFACEMETHODIMP_(ULONG) AddRef(void);
    IFACEMETHODIMP_(ULONG) Release(void);
    // IDropTarget
    IFACEMETHODIMP DragEnter(IDataObject* pDataObj, DWORD grfKeyState, POINTL pt, DWORD* pdwEffect);
    IFACEMETHODIMP DragOver(DWORD grfKeyState, POINTL pt, DWORD* pdwEffect);
    IFACEMETHODIMP DragLeave(void);
    IFACEMETHODIMP Drop(IDataObject* pDataObj, DWORD grfKeyState, POINTL pt, DWORD* pdwEffect);
private:
    BOOL                m_isAllowDrag = FALSE;
    BOOL                m_isDataAvailable = FALSE;
    HWND                m_hTargetWnd = NULL;
    volatile LONG       m_lRefCount = 1;
    //IDropTargetHelper* m_pDropTargetHelper = NULL;

    QKONDRAGENTER       m_pOnEnter;
    QKONDRAGOVER        m_pOnOver;
    QKONDRAGLEAVE       m_pOnLeave;
    QKONDRAGDROP        m_pOnDrop;
};

class CDropSource : public IDropSource
{
public:
    CDropSource(QKDRAGGIVEFEEDBACK pProcGiveFeedBack);
    // IUnknown
    HRESULT __stdcall QueryInterface(REFIID iid, void** ppvObject);
    ULONG   __stdcall AddRef(void);
    ULONG   __stdcall Release(void);
    // IDropSource
    HRESULT __stdcall QueryContinueDrag(BOOL fEscapePressed, DWORD grfKeyState);
    HRESULT __stdcall GiveFeedback(DWORD dwEffect);
private:
    LONG                m_lRefCount;
    QKDRAGGIVEFEEDBACK  m_pGiveFeedBack;
};

class CDataObject :public IDataObject
{
public:
    CDataObject(FORMATETC* fmtetc, STGMEDIUM* stgmed, BOOL fRelease, int iCount);
    ~CDataObject();
    // IUnknown
    HRESULT __stdcall QueryInterface(REFIID iid, void** ppvObject);
    ULONG   __stdcall AddRef(void);
    ULONG   __stdcall Release(void);

    // IDataObject
    HRESULT __stdcall GetData(FORMATETC* pFormatEtc, STGMEDIUM* pmedium);
    HRESULT __stdcall GetDataHere(FORMATETC* pFormatEtc, STGMEDIUM* pmedium);
    HRESULT __stdcall QueryGetData(FORMATETC* pFormatEtc);
    HRESULT __stdcall GetCanonicalFormatEtc(FORMATETC* pFormatEct, FORMATETC* pFormatEtcOut);
    HRESULT __stdcall SetData(FORMATETC* pFormatEtc, STGMEDIUM* pMedium, BOOL fRelease);
    HRESULT __stdcall EnumFormatEtc(DWORD dwDirection, IEnumFORMATETC** ppEnumFormatEtc);
    HRESULT __stdcall DAdvise(FORMATETC* pFormatEtc, DWORD advf, IAdviseSink*, DWORD*);
    HRESULT __stdcall DUnadvise(DWORD      dwConnection);
    HRESULT __stdcall EnumDAdvise(IEnumSTATDATA** ppEnumAdvise);
private:
    LONG m_lRefCount;
    LONG m_nNumFormats;
    FORMATETC* m_pFormatEtc;
    STGMEDIUM* m_pStgMedium;
    BOOL* m_pbRelease;
    int LookupFormatEtc(FORMATETC* pFormatEtc);
};

void QKMakeDropSource(QKARRAY Files, QKDRAGGIVEFEEDBACK pGiveFeedBack, CDataObject** ppDataObject, CDropSource** ppSource, BOOL fRelease);