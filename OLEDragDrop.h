#pragma once
#include <Windows.h>

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
    HRESULT STDMETHODCALLTYPE QueryInterface(REFIID riid, void** ppv);
    ULONG STDMETHODCALLTYPE AddRef(void);
    ULONG STDMETHODCALLTYPE Release(void);
    // IDropTarget
    HRESULT STDMETHODCALLTYPE DragEnter(IDataObject* pDataObj, DWORD grfKeyState, POINTL pt, DWORD* pdwEffect);
    HRESULT STDMETHODCALLTYPE DragOver(DWORD grfKeyState, POINTL pt, DWORD* pdwEffect);
    HRESULT STDMETHODCALLTYPE DragLeave(void);
    HRESULT STDMETHODCALLTYPE Drop(IDataObject* pDataObj, DWORD grfKeyState, POINTL pt, DWORD* pdwEffect);
private:
    BOOL                m_isAllowDrag = FALSE;
    BOOL                m_isDataAvailable = FALSE;
    HWND                m_hTargetWnd = NULL;
    volatile LONG       m_lRefCount = 1;

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
    HRESULT STDMETHODCALLTYPE QueryInterface(REFIID iid, void** ppvObject);
    ULONG STDMETHODCALLTYPE AddRef(void);
    ULONG STDMETHODCALLTYPE Release(void);
    // IDropSource
    HRESULT STDMETHODCALLTYPE QueryContinueDrag(BOOL fEscapePressed, DWORD grfKeyState);
    HRESULT STDMETHODCALLTYPE GiveFeedback(DWORD dwEffect);
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
    HRESULT STDMETHODCALLTYPE QueryInterface(REFIID iid, void** ppvObject);
    ULONG STDMETHODCALLTYPE AddRef(void);
    ULONG STDMETHODCALLTYPE Release(void);
    // IDataObject
    HRESULT STDMETHODCALLTYPE GetData(FORMATETC* pFormatEtc, STGMEDIUM* pmedium);
    HRESULT STDMETHODCALLTYPE GetDataHere(FORMATETC* pFormatEtc, STGMEDIUM* pmedium);
    HRESULT STDMETHODCALLTYPE QueryGetData(FORMATETC* pFormatEtc);
    HRESULT STDMETHODCALLTYPE GetCanonicalFormatEtc(FORMATETC* pFormatEct, FORMATETC* pFormatEtcOut);
    HRESULT STDMETHODCALLTYPE SetData(FORMATETC* pFormatEtc, STGMEDIUM* pMedium, BOOL fRelease);
    HRESULT STDMETHODCALLTYPE EnumFormatEtc(DWORD dwDirection, IEnumFORMATETC** ppEnumFormatEtc);
    HRESULT STDMETHODCALLTYPE DAdvise(FORMATETC* pFormatEtc, DWORD advf, IAdviseSink*, DWORD*);
    HRESULT STDMETHODCALLTYPE DUnadvise(DWORD      dwConnection);
    HRESULT STDMETHODCALLTYPE EnumDAdvise(IEnumSTATDATA** ppEnumAdvise);
private:
    LONG                m_lRefCount;
    LONG                m_nNumFormats;
    FORMATETC*          m_pFormatEtc;
    STGMEDIUM*          m_pStgMedium;
    BOOL*               m_pbRelease;
    int LookupFormatEtc(FORMATETC* pFormatEtc);
};
/*
* 目标：制拖放源
*
* 参数：
* Files 要拖放的文件数组
* pGiveFeedBack GiveFeedBack回调
* ppDataObject 返回数据对象
* ppSource 返回拖放源对象
* fRelease 是否由数据对象释放，为TRUE则数据对象析构时释放持有数据
*
* 返回值：
* 备注：
*/
void QKMakeDropSource(QKARRAY Files, QKDRAGGIVEFEEDBACK pGiveFeedBack, CDataObject** ppDataObject, CDropSource** ppSource, BOOL fRelease);