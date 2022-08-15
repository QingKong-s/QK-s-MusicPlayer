#include <Windows.h>
#include <d2d1.h>
#include <dwrite.h>
#include <windowsx.h>


#include "MyProject.h"
#include "GlobalVar.h"
#include "resource.h"
#include "Function.h"
#include "WndLrc.h"
#include "QKCtrl.h"
#include "WndMain.h"
#include "WndList.h"

HDC             m_hCDC_Lrc      = NULL;
HBITMAP         m_hBitmap       = NULL;

BOOL            m_bShowBK       = FALSE;

int             m_iLrcBT        = 0;
int             m_iMouseHoverBT = 0;
RECT            m_rcLrcText;//�������ı��������λ�ü����

int             m_cxClient      = 0,
                m_cyClient      = 0,
                m_xLrcRgn       = 0,
                m_yLrcRgn       = 0,
                m_cxLrcRgn      = 0,
                m_cyLrcRgn      = 0;

LRCHSCROLLINFO          m_LrcScrollInfo         = { -1 };

ID2D1Factory*           m_pD2DFactory           = NULL;// D2D����
ID2D1DCRenderTarget*    m_pD2DRenderTarget      = NULL;// ��ȾĿ��
IDWriteFactory*         m_pDWFactory            = NULL;// DW����
IDWriteTextFormat*      m_pDWTextFormat         = NULL;// �ı���ʽ
ID2D1SolidColorBrush*   m_pD2DSolidBrush1       = NULL,// ��߻�ˢ
                       *m_pD2DSolidBrush2       = NULL;// ��Ӱ��ˢ

QKDWTextRenderer_DrawOutline::QKDWTextRenderer_DrawOutline(ID2D1RenderTarget* pD2DRenderTarget, ID2D1Brush* pD2DBrushBody, ID2D1SolidColorBrush* pD2DBrushOutline)
{
    m_pD2DRenderTarget_Outline = pD2DRenderTarget;
    m_pD2DBrushBody = pD2DBrushBody;
    m_pD2DBrushOutline = pD2DBrushOutline;
}
QKDWTextRenderer_DrawOutline::~QKDWTextRenderer_DrawOutline()
{

}
HRESULT QKDWTextRenderer_DrawOutline::DrawGlyphRun(void* clientDrawingContext, float baselineOriginX, float baselineOriginY, DWRITE_MEASURING_MODE measuringMode,
	DWRITE_GLYPH_RUN const* glyphRun, DWRITE_GLYPH_RUN_DESCRIPTION const* glyphRunDescription, IUnknown* clientDrawingEffect)
{
    ID2D1PathGeometry* pD2DPathGeometry;// ·��ͼ��
    ID2D1TransformedGeometry* pD2DTransformedGeometry;// �任ͼ��
    ID2D1GeometrySink* pD2DGeometrySink;// ���ν�����
    m_pD2DFactory->CreatePathGeometry(&pD2DPathGeometry);
    
    pD2DPathGeometry->Open(&pD2DGeometrySink);
    glyphRun->fontFace->GetGlyphRunOutline(glyphRun->fontEmSize,glyphRun->glyphIndices,glyphRun->glyphAdvances,glyphRun->glyphOffsets,
        glyphRun->glyphCount,glyphRun->isSideways,glyphRun->bidiLevel, pD2DGeometrySink);// ȡ��������
    pD2DGeometrySink->Close();

    float fShandowOffest = 3;

    m_pD2DFactory->CreateTransformedGeometry(pD2DPathGeometry, 
        D2D1::Matrix3x2F::Translation(baselineOriginX + fShandowOffest, baselineOriginY + fShandowOffest), &pD2DTransformedGeometry);// ƽ��
    m_pD2DRenderTarget_Outline->FillGeometry(pD2DTransformedGeometry, m_pD2DSolidBrush2);// ����Ӱ
    pD2DTransformedGeometry->Release();

    m_pD2DFactory->CreateTransformedGeometry(pD2DPathGeometry, 
        D2D1::Matrix3x2F::Translation(baselineOriginX, baselineOriginY), &pD2DTransformedGeometry);
    m_pD2DRenderTarget_Outline->DrawGeometry(pD2DTransformedGeometry, m_pD2DBrushOutline, DPI(1.5));// ���
    m_pD2DRenderTarget_Outline->FillGeometry(pD2DTransformedGeometry, m_pD2DBrushBody);// ���
    pD2DTransformedGeometry->Release();

    pD2DPathGeometry->Release();
    pD2DGeometrySink->Release();
    return S_OK;
}
void LrcWnd_CreateRes()
{
    D2D1CreateFactory(D2D1_FACTORY_TYPE_SINGLE_THREADED, &m_pD2DFactory);
    DWriteCreateFactory(DWRITE_FACTORY_TYPE_SHARED, __uuidof(IDWriteFactory), reinterpret_cast<IUnknown**>(&m_pDWFactory));
    m_pDWFactory->CreateTextFormat(
        L"΢���ź�",
        0,
        DWRITE_FONT_WEIGHT_REGULAR,
        DWRITE_FONT_STYLE_NORMAL,
        DWRITE_FONT_STRETCH_NORMAL,
        DPI(40),
        L"zh-cn",
        &m_pDWTextFormat
    );
    m_pDWTextFormat->SetParagraphAlignment(DWRITE_PARAGRAPH_ALIGNMENT_NEAR);// ������뵽����
}
void LrcWnd_DeleteRes()
{
    m_pD2DSolidBrush1->Release();
    m_pD2DSolidBrush1 = NULL;
    m_pD2DSolidBrush2->Release();
    m_pD2DSolidBrush2 = NULL;
    m_pDWTextFormat->Release();
    m_pDWTextFormat = NULL;
    m_pD2DRenderTarget->Release();
    m_pD2DRenderTarget = NULL;
    m_pDWFactory->Release();
    m_pDWFactory = NULL;
    m_pD2DFactory->Release();
    m_pD2DFactory = NULL;
    DeleteDC(m_hCDC_Lrc);
    m_hCDC_Lrc = NULL;
    DeleteObject(m_hBitmap);
    m_hBitmap = NULL;
}
void LrcWnd_Show()
{
    int cxDef = DPI(580), cyDef = DPI(155);
    int cxScr = GetSystemMetrics(SM_CXFULLSCREEN),
        cyScr = GetSystemMetrics(SM_CYFULLSCREEN);
    if (IsWindow(g_hLrcWnd))
    {
        DestroyWindow(g_hLrcWnd);
    }
	else
	{
		g_hLrcWnd = CreateWindowExW(
			WS_EX_NOACTIVATE | WS_EX_LAYERED | WS_EX_TOPMOST | WS_EX_TOOLWINDOW,
			LRCWNDCLASS, NULL, WS_POPUP | WS_VISIBLE,
			(cxScr - cxDef) / 2,
			cyScr - cyDef - 50,
			cxDef,
			cyDef,
			NULL, NULL, g_hInst, NULL);
        SetTimer(g_hLrcWnd, IDT_LRC, TIMERELAPSE_LRCWND, NULL);
        LrcWnd_DrawLrc();
    }
}
BOOL LrcWnd_Init()
{
    WNDCLASSEXW wcex = { 0 };
    wcex.cbSize = sizeof(WNDCLASSEX);
    wcex.style = CS_HREDRAW | CS_VREDRAW;
    wcex.lpfnWndProc = WndProc_Lrc;
    wcex.hInstance = g_hInst;
    wcex.lpszClassName = LRCWNDCLASS;
    wcex.hCursor = LoadCursorW(NULL, IDC_ARROW);
    if (!RegisterClassExW(&wcex))
        return FALSE;
    return TRUE;
}
LRESULT CALLBACK WndProc_Lrc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam)
{
    static BOOL bLBTDown = FALSE;
    static BOOL bShowBKOld = FALSE;
    switch (message)
    {
    case WM_CREATE:
    {
        LrcWnd_CreateRes();
    }
    return 0;
    case WM_MOUSEACTIVATE:
        return MA_NOACTIVATE;//������ڣ�Ҳ�����������Ϣ
    case WM_NCLBUTTONDOWN://�ǿͻ�����갴���¼�
    {
        bLBTDown = TRUE;
        LRESULT lResult = DefWindowProcW(hWnd, message, wParam, lParam);
        bLBTDown = FALSE;
        return lResult;
    }
    case WM_LBUTTONDOWN:
    {
        //SetCapture(hWnd);
        bLBTDown = TRUE;
        int iRet = LrcWnd_HitTest();
        if (iRet < 0)
        {
            m_iLrcBT = iRet;
            LrcWnd_DrawLrc();
        }
    }
    break;
    case WM_LBUTTONUP:
    {
        //ReleaseCapture();
        bLBTDown = FALSE;
        int iRet = LrcWnd_HitTest();
        switch (iRet)
        {
        case LRCHITTEST_LAST:
            SendMessageW(g_hBKBtm, BTMBKM_DOBTOPE, 0, 0);
            break;
        case LRCHITTEST_PLAY:
            SendMessageW(g_hBKBtm, BTMBKM_DOBTOPE, 1, 0);
            break;
        case LRCHITTEST_NEXT:
            SendMessageW(g_hBKBtm, BTMBKM_DOBTOPE, 2, 0);
            break;
        case LRCHITTEST_CLOSE:
            DestroyWindow(hWnd);
            return 0;
        }
        m_iLrcBT = 0;
        LrcWnd_DrawLrc();
    }
    break;
    case WM_NCHITTEST:
	{
        if (m_bShowBK)
        {
            int iRet = LrcWnd_HitTest();
            if (iRet != m_iMouseHoverBT)
            {
                m_iMouseHoverBT = iRet;
                LrcWnd_DrawLrc();
            }
            if (iRet > 0)
                return iRet;
        }
        else
        {
            m_bShowBK = TRUE;
            if (m_bShowBK != bShowBKOld)
            {
                bShowBKOld = m_bShowBK;
                LrcWnd_DrawLrc();
            }
        }
	}
	break;
	case WM_SIZE:
	{
		m_cxClient = LOWORD(lParam);
		m_cyClient = HIWORD(lParam);

		m_xLrcRgn = DPIS_DTLRCFRAME;
		m_yLrcRgn = DPIS_DTLRCFRAME + DPIS_DTLRCEDGE * 2 + GC.cyBT;
		m_cxLrcRgn = m_cxClient - m_xLrcRgn * 2;
		m_cyLrcRgn = m_cyClient - m_yLrcRgn - m_xLrcRgn;

		if (m_hCDC_Lrc)
			DeleteDC(m_hCDC_Lrc);
		if (m_hBitmap)
			DeleteObject(m_hBitmap);

		HDC hDC = GetDC(g_hLrcWnd);
		m_hCDC_Lrc = CreateCompatibleDC(hDC);
		m_hBitmap = CreateCompatibleBitmap(hDC, m_cxClient, m_cyClient);
		ReleaseDC(g_hLrcWnd, hDC);
		SelectObject(m_hCDC_Lrc, m_hBitmap);

		if (m_pD2DRenderTarget)
			m_pD2DRenderTarget->Release();
        if (m_pD2DSolidBrush1)
            m_pD2DSolidBrush1->Release();
        if (m_pD2DSolidBrush2)
            m_pD2DSolidBrush2->Release();

		D2D1_RENDER_TARGET_PROPERTIES DCRTProp =
		{
			D2D1_RENDER_TARGET_TYPE_DEFAULT,
			{DXGI_FORMAT_B8G8R8A8_UNORM,D2D1_ALPHA_MODE_PREMULTIPLIED},
			0,
			0,
			D2D1_RENDER_TARGET_USAGE_NONE,
			D2D1_FEATURE_LEVEL_DEFAULT
		};
		HRESULT hr = m_pD2DFactory->CreateDCRenderTarget(&DCRTProp, &m_pD2DRenderTarget);
		RECT rc = { 0,0,m_cxClient,m_cyClient };
		m_pD2DRenderTarget->BindDC(m_hCDC_Lrc, &rc);
        m_pD2DRenderTarget->CreateSolidColorBrush(D2D1::ColorF(D2D1::ColorF::Black), &m_pD2DSolidBrush1);// ������ɫ��ˢ
		m_pD2DRenderTarget->CreateSolidColorBrush(D2D1::ColorF(0, 0, 0, 0.4), &m_pD2DSolidBrush2);

		m_LrcScrollInfo.iIndex = -1;
		m_LrcScrollInfo.bWndSizeChangedFlag = TRUE;
        LrcWnd_DrawLrc();
	}
    return 0;
	case WM_TIMER:
	{
        switch (wParam)
        {
        case IDT_LRC:
        {
            
            POINT pt;
            GetPhysicalCursorPos(&pt);
			ScreenToClient(hWnd, &pt);
			RECT rc = { 0,0,m_cxClient,m_cyClient };
			if (!PtInRect(&rc, pt))
				m_bShowBK = FALSE;

			if (bLBTDown)
				m_bShowBK = TRUE;

			if (m_bShowBK != bShowBKOld)
			{
				bShowBKOld = m_bShowBK;
				LrcWnd_DrawLrc();
            }
        }
        return 0;
        case IDT_ANIMATION:
        {
            if (g_iCurrLrcIndex < 0)
                return 0;
            LRCDATA* p = (LRCDATA*)QKArray_Get(g_Lrc, g_iCurrLrcIndex);
            float fLastTime = g_fTime - p->fTime;// �Ѿ�������ʱ��
            static int iLastx1 = 0, iLastx2 = 0;// �ϴ���ߣ�������ϴ�һ���Ͳ�Ҫ�ٻ���
            int ii;
            BOOL bRedraw = FALSE;
            if (m_LrcScrollInfo.cx1 != -1)
            {
                if (fLastTime > m_LrcScrollInfo.fNoScrollingTime1)
                {
                    if (fLastTime < p->fDelay - m_LrcScrollInfo.fNoScrollingTime1)
                    {
                        ii = m_cxLrcRgn / 2 - fLastTime * m_LrcScrollInfo.cx1 / p->fDelay;
                        if (ii != iLastx1)
                        {
                            iLastx1 = m_LrcScrollInfo.x1 = ii;
                            bRedraw = TRUE;
                        }
                    }
                    else
                    {
                        ii = m_cxLrcRgn - m_LrcScrollInfo.cx1;
                        if (ii != iLastx1)
                        {
                            iLastx1 = m_LrcScrollInfo.x1 = ii;
                            bRedraw = TRUE;
                        }
                    }
                }
                else
                {
                    if (iLastx1 != 0)
                    {
                        iLastx1 = m_LrcScrollInfo.x1 = 0;
                        bRedraw = TRUE;
                    }
                }
            }

            if (m_LrcScrollInfo.cx2 != -1)
            {
                if (fLastTime > m_LrcScrollInfo.fNoScrollingTime2)
                {
                    if (fLastTime < p->fDelay - m_LrcScrollInfo.fNoScrollingTime2)
                    {
                        ii = m_cxLrcRgn / 2 - fLastTime * m_LrcScrollInfo.cx2 / p->fDelay;
                        if (ii != iLastx2)
                        {
                            iLastx2 = m_LrcScrollInfo.x2 = ii;
                            bRedraw = TRUE;
                        }
                    }
                    else
                    {
                        ii = m_cxLrcRgn - m_LrcScrollInfo.cx2;
                        if (ii != iLastx2)
                        {
                            iLastx2 = m_LrcScrollInfo.x2 = ii;
                            bRedraw = TRUE;
                        }
                    }
                }
                else
                {
                    if (iLastx2 != 0)
                    {
                        iLastx2 = m_LrcScrollInfo.x2 = 0;
                        bRedraw = TRUE;
                    }
                }
            }

            if (bRedraw)
                LrcWnd_DrawLrc();
        }
        return 0;
        }
    }
    return 0;
    case WM_GETMINMAXINFO://���ƴ��ڴ�С
    {
        LPMINMAXINFO pInfo = (LPMINMAXINFO)lParam;
        pInfo->ptMinTrackSize.x = DPI(580);
        pInfo->ptMinTrackSize.y = DPI(150);
    }
    return 0;
    case WM_DESTROY:
    {
        KillTimer(hWnd, IDT_LRC);
        g_hLrcWnd = NULL;
		LrcWnd_DeleteRes();
	}
	return 0;
	}
	return DefWindowProcW(hWnd, message, wParam, lParam);
}
void LrcWnd_DrawLrc()//  �ģɣңţãԡ��ء����ģ�����
{
	if (!g_hLrcWnd)
		return;
	RECT rc;
	GetClientRect(g_hLrcWnd, &rc);
    D2D_RECT_F rcF = {0};
    ID2D1SolidColorBrush* pD2DSolidBrush;
    m_pD2DRenderTarget->BeginDraw();
    m_pD2DRenderTarget->Clear(D2D1::ColorF(0, 0, 0, 0));
	if (m_bShowBK)
	{
        m_pD2DRenderTarget->CreateSolidColorBrush(D2D1::ColorF(0x3F3F3F,0.5), &pD2DSolidBrush);// ����������ˢ
        m_pD2DRenderTarget->FillRectangle(D2D1::RectF(0, 0, m_cxClient, m_cyClient), pD2DSolidBrush);
        pD2DSolidBrush->SetColor(D2D1::ColorF(0x97D2CB, 0.5));// ת����ɫ���߿�ˢ
        m_pD2DRenderTarget->DrawRectangle(D2D1::RectF(DPIS_DTLRCEDGE, DPIS_DTLRCEDGE, m_cxClient - DPIS_DTLRCEDGE, m_cyClient - DPIS_DTLRCEDGE),
            pD2DSolidBrush, DPIS_DTLRCEDGE);
        pD2DSolidBrush->Release();// ɾ����ˢ
		int iLeft = (m_cxClient - GC.cyBT * DTLRCBTNCOUNT) / 2;
		if (m_iMouseHoverBT < 0)// ����ť����
		{
			if (m_iLrcBT != 0)
                m_pD2DRenderTarget->CreateSolidColorBrush(D2D1::ColorF(0x00FFFF, 0.5), &pD2DSolidBrush);
			else
                m_pD2DRenderTarget->CreateSolidColorBrush(D2D1::ColorF(0xA3FFFF, 0.5), &pD2DSolidBrush);// ������ť������ˢ
            D2D1_RECT_F rcF;
            rcF.left = iLeft + (GC.cyBT + DPIS_DTLRCEDGE) * (abs(m_iMouseHoverBT) - 1);
            rcF.top = DPIS_DTLRCFRAME + DPIS_DTLRCEDGE;
            rcF.right = rcF.left + GC.cyBT;
            rcF.bottom = rcF.top + GC.cyBT;
            m_pD2DRenderTarget->FillRectangle(&rcF, pD2DSolidBrush);
            pD2DSolidBrush->Release();// ɾ����ˢ
		}
		int iIconOffest = (GC.cyBT - GC.iIconSize) / 2;
		int iIconTop = DPIS_DTLRCFRAME + DPIS_DTLRCEDGE + iIconOffest;
		int iIconStep = GC.cyBT + DPIS_DTLRCEDGE;
        rcF = { (float)(iLeft + iIconOffest) ,(float)iIconTop };
        rcF.left = iLeft + iIconOffest;
        rcF.top = iIconTop;
        rcF.right = rcF.left + GC.cyBT;
        rcF.bottom = rcF.top + GC.cyBT;
        m_pD2DRenderTarget->EndDraw();
		DrawIconEx(m_hCDC_Lrc, iLeft + iIconOffest, iIconTop, GR.hiLast2, 0, 0, 0, NULL, DI_NORMAL);
		iLeft += iIconStep;
		DrawIconEx(m_hCDC_Lrc, iLeft + iIconOffest, iIconTop, g_bPlayIcon ? GR.hiPlay2 : GR.hiPause2, 0, 0, 0, NULL, DI_NORMAL);
		iLeft += iIconStep;
		DrawIconEx(m_hCDC_Lrc, iLeft + iIconOffest, iIconTop, GR.hiNext2, 0, 0, 0, NULL, DI_NORMAL);
		iLeft += iIconStep;
		DrawIconEx(m_hCDC_Lrc, iLeft + iIconOffest, iIconTop, GR.hiCross2, 0, 0, 0, NULL, DI_NORMAL);
        m_pD2DRenderTarget->BeginDraw();
	}
	PWSTR pszLrc = NULL;
    UINT32 uStr1Length = 0, uStr2Length = 0;
    LRCDATA* p = NULL;
	switch (g_iLrcState)
	{
	case LRCSTATE_STOP:
		pszLrc = (PWSTR)L"��յ����ֲ����� - VC++/Win32";
        uStr1Length = lstrlenW(pszLrc);
		break;
	case LRCSTATE_NOLRC:
	NoLrc:
        if (g_hStream)
        {
            pszLrc = ((PLAYERLISTUNIT*)QKArray_Get(g_ItemData, g_iCurrFileIndex))->pszName;
            uStr1Length = lstrlenW(pszLrc);
        }
		break;
	case LRCSTATE_NORMAL:
        if (g_iCurrLrcIndex >= 0)
        {
            p = (LRCDATA*)QKArray_Get(g_Lrc, g_iCurrLrcIndex);
            pszLrc = p->pszLrc;
            if (p->iOrgLength == -1)
            {
                uStr1Length = lstrlenW(pszLrc);
            }
            else
            {
                uStr1Length = p->iOrgLength;
                uStr2Length = lstrlenW(p->pszLrc + p->iOrgLength + 1);
            }
        }
		else//�и�ʵ���ǰ����С��0��˵����û������һ��
		{
			g_iLrcState = LRCSTATE_NOLRC;
			goto NoLrc;
		}
		break;
	}

    if (!pszLrc)
    {
        m_pD2DRenderTarget->EndDraw();
        return;
    }
	
	IDWriteTextLayout* pDWTextLayout, * pDWTextLayout2;// �ı�����
	DWRITE_TEXT_METRICS DWTextMetrics, DWTextMetrics2;// �ı�����
	D2D1_LINEAR_GRADIENT_BRUSH_PROPERTIES GradientBrushProp;// ����ˢ����
	ID2D1GradientStopCollection* pD2DGradientStopCollection;// �ݶȵ㼯��
	D2D1_GRADIENT_STOP GradientStop[2];// �ݶȵ�
	ID2D1LinearGradientBrush* pD2DGradientBrush;// ����ˢ
    QKDWTextRenderer_DrawOutline* pMyRenderer;// �Զ����ı���Ⱦ��

    float x, iShandowOffest = 2;
    if (p)
    {
        m_pDWTextFormat->SetWordWrapping(DWRITE_WORD_WRAPPING_NO_WRAP);// ��ֹ�Զ�����
        if (p->iOrgLength == -1)// ����
        {
            m_pDWTextFormat->SetTextAlignment(DWRITE_TEXT_ALIGNMENT_LEADING);
            m_pDWFactory->CreateTextLayout(pszLrc, uStr1Length, m_pDWTextFormat, m_cxClient, m_cyClient, &pDWTextLayout);// �����ı�����
            pDWTextLayout->GetMetrics(&DWTextMetrics);
            if (g_iCurrLrcIndex != m_LrcScrollInfo.iIndex && g_iLrcState == LRCSTATE_NORMAL)
            {
                m_LrcScrollInfo.iIndex = g_iCurrLrcIndex;
                if (!m_LrcScrollInfo.bWndSizeChangedFlag)
                    m_LrcScrollInfo.x1 = m_LrcScrollInfo.x2 = 0;
                else
                    m_LrcScrollInfo.bWndSizeChangedFlag = FALSE;

                KillTimer(g_hMainWnd, IDT_ANIMATION);
                if (DWTextMetrics.width > m_cxLrcRgn)
                {
                    m_LrcScrollInfo.cx1 = DWTextMetrics.width;// �����ˣ���Ҫ��������
                    m_LrcScrollInfo.fNoScrollingTime1 = m_cxLrcRgn * p->fDelay / DWTextMetrics.width / 2;
                    SetTimer(g_hLrcWnd, IDT_ANIMATION, TIMERELAPSE_ANIMATION, NULL);
                }
                else
                {
                    m_LrcScrollInfo.cx1 = -1;
                    m_LrcScrollInfo.x1 = m_LrcScrollInfo.x2 = 0;
                }
            }

            if (m_LrcScrollInfo.cx1 == -1)// �������
            {
                pDWTextLayout->Release();
                m_pDWTextFormat->SetTextAlignment(DWRITE_TEXT_ALIGNMENT_CENTER);
                m_pDWFactory->CreateTextLayout(pszLrc, uStr1Length, m_pDWTextFormat, m_cxClient, m_cyClient, &pDWTextLayout);// ���´����ı�����
            NotShowingLrc:
                m_rcLrcText.left = (m_cxClient - DWTextMetrics.width) / 2;
                m_rcLrcText.top = m_yLrcRgn;
                m_rcLrcText.right = m_rcLrcText.left + DWTextMetrics.width;
                m_rcLrcText.bottom = m_rcLrcText.top + DWTextMetrics.height;
            }
            else
            {
                m_rcLrcText.left = m_xLrcRgn;
                m_rcLrcText.top = m_yLrcRgn;
                m_rcLrcText.right = m_xLrcRgn + m_cxLrcRgn;
                m_rcLrcText.bottom = m_yLrcRgn + DWTextMetrics.height;
            }
        
            GradientBrushProp = { D2D1::Point2F(0,m_yLrcRgn),D2D1::Point2F(0,m_yLrcRgn + DWTextMetrics.height) };

            GradientStop[0] = { 0.0f,D2D1::ColorF(0x00FF00,1) };
            GradientStop[1] = { 1.0f,D2D1::ColorF(0x0000FF,1) };

            m_pD2DRenderTarget->CreateGradientStopCollection(GradientStop, 2, &pD2DGradientStopCollection);// �����ݶȵ㼯��
            m_pD2DRenderTarget->CreateLinearGradientBrush(GradientBrushProp, pD2DGradientStopCollection, &pD2DGradientBrush);// �������Խ��仭ˢ
            pD2DGradientStopCollection->Release();// ɾ���ݶȵ㼯��

			pMyRenderer = new QKDWTextRenderer_DrawOutline(m_pD2DRenderTarget, pD2DGradientBrush, m_pD2DSolidBrush1);// �����Զ����ı���Ⱦ��
			m_pD2DRenderTarget->PushAxisAlignedClip(D2D1::RectF(m_xLrcRgn, m_yLrcRgn, m_xLrcRgn + m_cxLrcRgn, m_yLrcRgn + m_cyLrcRgn),
				D2D1_ANTIALIAS_MODE_PER_PRIMITIVE);// ѹ����������
			if (m_LrcScrollInfo.cx1 == -1)
				x = 0;
			else
				x = m_LrcScrollInfo.x1 + m_xLrcRgn;
			pDWTextLayout->Draw(NULL, pMyRenderer, x, m_yLrcRgn);
			m_pD2DRenderTarget->PopAxisAlignedClip();// ������������
			pMyRenderer->Release();// ɾ���Զ����ı���Ⱦ��
			pD2DGradientBrush->Release();// ɾ�����Խ��仭ˢ
			pDWTextLayout->Release();// ɾ���ı�����
        }
        else// ˫��
        {
            m_pDWTextFormat->SetTextAlignment(DWRITE_TEXT_ALIGNMENT_LEADING);
            m_pDWFactory->CreateTextLayout(pszLrc, uStr1Length, m_pDWTextFormat, m_cxClient, m_cyClient, &pDWTextLayout);// �����ı�����
            pDWTextLayout->GetMetrics(&DWTextMetrics);
            m_pDWFactory->CreateTextLayout(pszLrc + p->iOrgLength + 1, uStr2Length, m_pDWTextFormat, m_cxClient, m_cyClient, &pDWTextLayout2);// �����ı�����
            pDWTextLayout2->GetMetrics(&DWTextMetrics2);
            if (g_iCurrLrcIndex != m_LrcScrollInfo.iIndex && g_iLrcState == LRCSTATE_NORMAL)
            {
                m_LrcScrollInfo.iIndex = g_iCurrLrcIndex;
                if (!m_LrcScrollInfo.bWndSizeChangedFlag)
                    m_LrcScrollInfo.x1 = m_LrcScrollInfo.x2 = 0;
                else
                    m_LrcScrollInfo.bWndSizeChangedFlag = FALSE;

                KillTimer(g_hMainWnd, IDT_ANIMATION);
                if (DWTextMetrics.width > m_cxLrcRgn)
                {
                    m_LrcScrollInfo.cx1 = DWTextMetrics.width;// �����ˣ���Ҫ��������
                    m_LrcScrollInfo.fNoScrollingTime1 = m_cxLrcRgn * p->fDelay / DWTextMetrics.width / 2;
                    SetTimer(g_hLrcWnd, IDT_ANIMATION, TIMERELAPSE_ANIMATION, NULL);
                }
                else
                {
                    m_LrcScrollInfo.cx1 = -1;
                    m_LrcScrollInfo.x1 = m_LrcScrollInfo.x2 = 0;
                }

                if (DWTextMetrics2.width > m_cxLrcRgn)
                {
                    m_LrcScrollInfo.cx2 = DWTextMetrics2.width;// �����ˣ���Ҫ��������
                    m_LrcScrollInfo.fNoScrollingTime2 = m_cxLrcRgn * p->fDelay / DWTextMetrics2.width / 2;
                    SetTimer(g_hLrcWnd, IDT_ANIMATION, TIMERELAPSE_ANIMATION, NULL);
                }
                else
                {
                    m_LrcScrollInfo.cx2 = -1;
                    m_LrcScrollInfo.x1 = m_LrcScrollInfo.x2 = 0;
                }
            }

            if (m_LrcScrollInfo.cx1 == -1)
            {
                pDWTextLayout->Release();
                m_pDWTextFormat->SetTextAlignment(DWRITE_TEXT_ALIGNMENT_CENTER);
                m_pDWFactory->CreateTextLayout(pszLrc, uStr1Length, m_pDWTextFormat, m_cxClient, m_cyClient, &pDWTextLayout);// ���´����ı�����
            }

			if (m_LrcScrollInfo.cx2 == -1)
			{
				pDWTextLayout2->Release();
				m_pDWTextFormat->SetTextAlignment(DWRITE_TEXT_ALIGNMENT_CENTER);
				m_pDWFactory->CreateTextLayout(pszLrc + p->iOrgLength + 1, uStr2Length, m_pDWTextFormat, m_cxClient, m_cyClient, &pDWTextLayout2);// ���´����ı�����
			}

			if (m_LrcScrollInfo.cx1 != -1 || m_LrcScrollInfo.cx2 != -1)
			{
				m_rcLrcText.left = m_xLrcRgn;
				m_rcLrcText.top = m_yLrcRgn;
				m_rcLrcText.right = m_xLrcRgn + m_cxLrcRgn;
				m_rcLrcText.bottom = m_yLrcRgn + DWTextMetrics.height + DWTextMetrics2.height;
			}
			else
			{
				x = max(DWTextMetrics.width, DWTextMetrics2.width);
				m_rcLrcText.left = (m_cxClient - x) / 2;
				m_rcLrcText.top = m_yLrcRgn;
				m_rcLrcText.right = m_rcLrcText.left + x;
				m_rcLrcText.bottom = m_rcLrcText.top + DWTextMetrics.height + DWTextMetrics2.height;
			}

			GradientBrushProp = { D2D1::Point2F(0,m_yLrcRgn),D2D1::Point2F(0,DWTextMetrics.height + DWTextMetrics2.height) };
            GradientStop[0] = { 0.0f,D2D1::ColorF(0x00FF00,1) };
            GradientStop[1] = { 1.0f,D2D1::ColorF(0x0000FF,1) };

            m_pD2DRenderTarget->CreateGradientStopCollection(GradientStop, 2, &pD2DGradientStopCollection);// �����ݶȵ㼯��
			m_pD2DRenderTarget->CreateLinearGradientBrush(GradientBrushProp, pD2DGradientStopCollection, &pD2DGradientBrush);// �������Խ��仭ˢ
			pD2DGradientStopCollection->Release();// ɾ���ݶȵ㼯��

			pMyRenderer = new QKDWTextRenderer_DrawOutline(m_pD2DRenderTarget, pD2DGradientBrush, m_pD2DSolidBrush1);// �����Զ����ı���Ⱦ��
			m_pD2DRenderTarget->PushAxisAlignedClip(D2D1::RectF(m_xLrcRgn, m_yLrcRgn, m_xLrcRgn + m_cxLrcRgn, m_yLrcRgn + m_cyLrcRgn),
				D2D1_ANTIALIAS_MODE_PER_PRIMITIVE);// ѹ����������
            if (m_LrcScrollInfo.cx1 == -1)
                x = 0;
            else
                x = m_LrcScrollInfo.x1 + m_xLrcRgn;

			pDWTextLayout->Draw(NULL, pMyRenderer, x, m_yLrcRgn);

			if (m_LrcScrollInfo.cx2 == -1)
				x = 0;
			else
				x = m_LrcScrollInfo.x2 + m_xLrcRgn;

			pDWTextLayout2->Draw(NULL, pMyRenderer, x, m_yLrcRgn + DWTextMetrics.height);
			m_pD2DRenderTarget->PopAxisAlignedClip();// ������������
			pMyRenderer->Release();// ɾ���Զ����ı���Ⱦ��
			pD2DGradientBrush->Release();// ɾ�����Խ��仭ˢ
			pDWTextLayout->Release();// ɾ���ı�����
			pDWTextLayout2->Release();
        }
    }
    else
    {
        m_pDWTextFormat->SetTextAlignment(DWRITE_TEXT_ALIGNMENT_CENTER);
        m_pDWTextFormat->SetWordWrapping(DWRITE_WORD_WRAPPING_WRAP);// �Զ�����
        m_pDWFactory->CreateTextLayout(pszLrc, uStr1Length, m_pDWTextFormat, m_cxClient, m_cyClient, &pDWTextLayout);// ���´����ı�����
        pDWTextLayout->GetMetrics(&DWTextMetrics);
        m_LrcScrollInfo.cx1 = -1;
        goto NotShowingLrc;
    }

    m_pD2DRenderTarget->EndDraw();
    static POINT pt = { 0 };
    SIZEL size = { m_cxClient,m_cyClient };
    BLENDFUNCTION bf;
    bf.BlendOp = AC_SRC_OVER;
    bf.BlendFlags = 0;
    bf.SourceConstantAlpha = 255;
    bf.AlphaFormat = AC_SRC_ALPHA;
    UpdateLayeredWindow(g_hLrcWnd, NULL, NULL, &size, m_hCDC_Lrc, &pt, 0, &bf, ULW_ALPHA);// ��ʾ
}
int LrcWnd_HitTest()//��ʴ������в���
{
    POINT pt;
    GetCursorPos(&pt);
    ScreenToClient(g_hLrcWnd, &pt);
    RECT rc;
    ///////////////////////////���Ա߿�����
    //���
    rc.left = 0;
    rc.top = 0;
    rc.right = DPIS_DTLRCFRAME;
    rc.bottom = m_cyClient;
    if (PtInRect(&rc, pt))//���
    {
        rc.top = 0;
        rc.bottom = DPIS_DTLRCFRAME;
        if (PtInRect(&rc, pt))//����
            return HTTOPLEFT;

        rc.top = m_cyClient - DPIS_DTLRCFRAME;
        rc.bottom = m_cyClient;
        if (PtInRect(&rc, pt))//����
            return HTBOTTOMLEFT;
        return HTLEFT;
    }

    rc.left = m_cxClient - DPIS_DTLRCFRAME;
    rc.top = 0;
    rc.right = m_cxClient;
    rc.bottom = m_cyClient;
    if (PtInRect(&rc, pt))//�ұ�
    {
        rc.top = 0;
        rc.bottom = DPIS_DTLRCFRAME;
        if (PtInRect(&rc, pt))//����
            return HTTOPRIGHT;

        rc.top = m_cyClient - DPIS_DTLRCFRAME;
        rc.bottom = m_cyClient;
        if (PtInRect(&rc, pt))//����
            return HTBOTTOMRIGHT;
        return HTRIGHT;
    }

    rc.left = DPIS_DTLRCFRAME;
    rc.top = 0;
    rc.right = m_cxClient - DPIS_DTLRCFRAME;
    rc.bottom = DPIS_DTLRCFRAME;
    if (PtInRect(&rc, pt))//����
        return HTTOP;

    rc.left = DPIS_DTLRCFRAME;
    rc.top = m_cyClient - DPIS_DTLRCFRAME;
	rc.right = m_cxClient - DPIS_DTLRCFRAME;
	rc.bottom = m_cyClient;
	if (PtInRect(&rc, pt))//�ױ�
		return HTBOTTOM;
	///////////////////////////���԰�ť����
	int iLeft = (m_cxClient - GC.cyBT * DTLRCBTNCOUNT) / 2;
	rc.left = iLeft;
	rc.top = DPIS_DTLRCFRAME + DPIS_DTLRCEDGE;
	rc.right = rc.left + GC.cyBT;
	rc.bottom = rc.top + GC.cyBT;
	if (PtInRect(&rc, pt))
		return LRCHITTEST_LAST;

	rc.left += (GC.cyBT + DPIS_DTLRCEDGE);
	rc.right += (GC.cyBT + DPIS_DTLRCEDGE);
	if (PtInRect(&rc, pt))
		return LRCHITTEST_PLAY;

	rc.left += (GC.cyBT + DPIS_DTLRCEDGE);
	rc.right += (GC.cyBT + DPIS_DTLRCEDGE);
	if (PtInRect(&rc, pt))
		return LRCHITTEST_NEXT;

	rc.left += (GC.cyBT + DPIS_DTLRCEDGE);
	rc.right += (GC.cyBT + DPIS_DTLRCEDGE);
	if (PtInRect(&rc, pt))
		return LRCHITTEST_CLOSE;

	return HTCAPTION;
}