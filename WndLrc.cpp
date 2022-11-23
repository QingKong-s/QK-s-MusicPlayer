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

HDC             m_hCDCDTLrc      = NULL;
HBITMAP         m_hBitmap       = NULL;

BOOL            m_bShowBK       = FALSE;

int             m_iLrcBT        = 0;
int             m_iMouseHoverBT = 0;

int             m_cxClient      = 0,
                m_cyClient      = 0,
                m_xLrcRgn       = 0,
                m_yLrcRgn       = 0,
                m_cxLrcRgn      = 0,
                m_cyLrcRgn      = 0;

LRCHSCROLLINFO          m_LrcScrollInfo         = { -1 };

ID2D1DCRenderTarget*    m_pD2DRenderTarget      = NULL;// 渲染目标
IDWriteTextFormat*      m_pDWTextFormat         = NULL;// 文本格式
ID2D1SolidColorBrush*   m_pD2DSolidBrush1       = NULL,// 描边画刷
                       *m_pD2DSolidBrush2       = NULL;// 阴影画刷

D2D1_COLOR_F            m_D2DCrDTLrc1 = { 0 };// 渐变上
D2D1_COLOR_F            m_D2DCrDTLrc2 = { 0 };// 渐变下

D2D1_COLOR_F            m_D2DCrDTLrcBorder = { 0 };// 描边

QKDWTextRenderer_DrawOutline::QKDWTextRenderer_DrawOutline(ID2D1RenderTarget* pD2DRenderTarget, ID2D1Brush* pD2DBrushBody, ID2D1SolidColorBrush* pD2DBrushOutline)
{
    m_uRefCount = 1;
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
    ID2D1PathGeometry* pD2DPathGeometry;// 路径图形
    ID2D1TransformedGeometry* pD2DTransformedGeometry;// 变换图形
    ID2D1GeometrySink* pD2DGeometrySink;// 几何接收器
    g_pD2DFactory->CreatePathGeometry(&pD2DPathGeometry);
    
    pD2DPathGeometry->Open(&pD2DGeometrySink);
    glyphRun->fontFace->GetGlyphRunOutline(glyphRun->fontEmSize,glyphRun->glyphIndices,glyphRun->glyphAdvances,glyphRun->glyphOffsets,
        glyphRun->glyphCount,glyphRun->isSideways,glyphRun->bidiLevel, pD2DGeometrySink);// 取字形轮廓
    pD2DGeometrySink->Close();

    if (GS.bDTLrcShandow)
    {
        float fShandowOffest = 3;// 阴影偏移

        g_pD2DFactory->CreateTransformedGeometry(pD2DPathGeometry,
            D2D1::Matrix3x2F::Translation(baselineOriginX + fShandowOffest, baselineOriginY + fShandowOffest), &pD2DTransformedGeometry);// 平移
        m_pD2DRenderTarget_Outline->FillGeometry(pD2DTransformedGeometry, m_pD2DSolidBrush2);// 画阴影
        pD2DTransformedGeometry->Release();
	}

	g_pD2DFactory->CreateTransformedGeometry(pD2DPathGeometry,
		D2D1::Matrix3x2F::Translation(baselineOriginX, baselineOriginY), &pD2DTransformedGeometry);
	if (GS.bEnableDTLrcBorder)
		m_pD2DRenderTarget_Outline->DrawGeometry(pD2DTransformedGeometry, m_pD2DBrushOutline, DPIF(1.5f));// 描边
	m_pD2DRenderTarget_Outline->FillGeometry(pD2DTransformedGeometry, m_pD2DBrushBody);// 填充
	pD2DTransformedGeometry->Release();

	pD2DPathGeometry->Release();
    pD2DGeometrySink->Release();
    return S_OK;
}
void LrcWnd_CreateRes()
{
    g_pDWFactory->CreateTextFormat(
        GS.pszDTLrcFontName,
        0,
        DWRITE_FONT_WEIGHT_REGULAR,
        DWRITE_FONT_STYLE_NORMAL,
        DWRITE_FONT_STRETCH_NORMAL,
        DPIF((float)GS.iDTLrcFontSize),
        L"zh-cn",
        &m_pDWTextFormat
    );
    m_pDWTextFormat->SetParagraphAlignment(DWRITE_PARAGRAPH_ALIGNMENT_NEAR);// 段落对齐到顶边
}
void LrcWnd_DeleteRes()
{
    SAFE_RELEASE(m_pD2DSolidBrush1);
    SAFE_RELEASE(m_pD2DSolidBrush2);
    SAFE_RELEASE(m_pDWTextFormat);
    SAFE_RELEASE(m_pD2DRenderTarget);

    DeleteDC(m_hCDCDTLrc);
    m_hCDCDTLrc = NULL;
    DeleteObject(m_hBitmap);
    m_hBitmap = NULL;
}
void LrcWnd_Show()
{
    int cxDef = DPI(580), cyDef = DPI(160);
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
        SettingsUpd_WndLrc();
    }
    return 0;
    case WM_MOUSEACTIVATE:
        return MA_NOACTIVATE;//不激活窗口，也不丢弃鼠标消息
    case WM_NCLBUTTONDOWN://非客户区鼠标按键事件
    {
        bLBTDown = TRUE;
        LRESULT lResult = DefWindowProcW(hWnd, message, wParam, lParam);
        bLBTDown = FALSE;
        return lResult;
    }
    case WM_LBUTTONDOWN:
    {
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
        bLBTDown = FALSE;
        int iRet = LrcWnd_HitTest();
        switch (iRet)
        {
        case LRCHITTEST_LAST:
            SendMessageW(g_hBKLeft, LEFTBKM_DOBTOPE, 0, 0);
            break;
        case LRCHITTEST_PLAY:
            SendMessageW(g_hBKLeft, LEFTBKM_DOBTOPE, 1, 0);
            break;
        case LRCHITTEST_NEXT:
            SendMessageW(g_hBKLeft, LEFTBKM_DOBTOPE, 2, 0);
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

		if (m_hCDCDTLrc)
			DeleteDC(m_hCDCDTLrc);
		if (m_hBitmap)
			DeleteObject(m_hBitmap);

		HDC hDC = GetDC(g_hLrcWnd);
		m_hCDCDTLrc = CreateCompatibleDC(hDC);
		m_hBitmap = CreateCompatibleBitmap(hDC, m_cxClient, m_cyClient);
		ReleaseDC(g_hLrcWnd, hDC);
		SelectObject(m_hCDCDTLrc, m_hBitmap);

		SAFE_RELEASE(m_pD2DRenderTarget);
		SAFE_RELEASE(m_pD2DSolidBrush1);
		SAFE_RELEASE(m_pD2DSolidBrush2);

		D2D1_RENDER_TARGET_PROPERTIES DCRTProp =
		{
			D2D1_RENDER_TARGET_TYPE_DEFAULT,
			{DXGI_FORMAT_B8G8R8A8_UNORM,D2D1_ALPHA_MODE_PREMULTIPLIED},
			0,
			0,
			D2D1_RENDER_TARGET_USAGE_NONE,
			D2D1_FEATURE_LEVEL_DEFAULT
		};
		g_pD2DFactory->CreateDCRenderTarget(&DCRTProp, &m_pD2DRenderTarget);
		RECT rc = { 0,0,m_cxClient,m_cyClient };
		m_pD2DRenderTarget->BindDC(m_hCDCDTLrc, &rc);
        m_pD2DRenderTarget->CreateSolidColorBrush(m_D2DCrDTLrcBorder, &m_pD2DSolidBrush1);
		m_pD2DRenderTarget->CreateSolidColorBrush(D2D1::ColorF(0, 0, 0, 0.4f), &m_pD2DSolidBrush2);

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
            float fLastTime = g_fTime - p->fTime;// 已经持续的时间
            static int iLastx1 = 0, iLastx2 = 0;// 上次左边，如果跟上次一样就不要再画了
            int ii;
            BOOL bRedraw = FALSE;
            if (m_LrcScrollInfo.cx1 != -1)
            {
                if (fLastTime > m_LrcScrollInfo.fNoScrollingTime1)
                {
                    if (fLastTime < p->fDelay - m_LrcScrollInfo.fNoScrollingTime1)
                    {
                        ii = m_cxLrcRgn / 2 - (int)(fLastTime * m_LrcScrollInfo.cx1 / p->fDelay);
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
                        ii = m_cxLrcRgn / 2 - (int)(fLastTime * m_LrcScrollInfo.cx2 / p->fDelay);
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
    case WM_GETMINMAXINFO://限制窗口大小
    {
        LPMINMAXINFO pInfo = (LPMINMAXINFO)lParam;
        pInfo->ptMinTrackSize.x = DPI(580);
        pInfo->ptMinTrackSize.y = DPI(160);
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
void LrcWnd_DrawLrc()
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
        m_pD2DRenderTarget->CreateSolidColorBrush(D2D1::ColorF(0x3F3F3F,0.5), &pD2DSolidBrush);// 创建背景画刷
        m_pD2DRenderTarget->FillRectangle(D2D1::RectF(0, 0, (FLOAT)m_cxClient, (FLOAT)m_cyClient), pD2DSolidBrush);
        pD2DSolidBrush->SetColor(D2D1::ColorF(0x97D2CB, 0.5));// 转换颜色，边框画刷
        m_pD2DRenderTarget->DrawRectangle(
            D2D1::RectF(
                (FLOAT)DPIS_DTLRCEDGE, 
                (FLOAT)DPIS_DTLRCEDGE, 
                (FLOAT)(m_cxClient - DPIS_DTLRCEDGE), 
                (FLOAT)(m_cyClient - DPIS_DTLRCEDGE)),
            pD2DSolidBrush, (FLOAT)DPIS_DTLRCEDGE);
        pD2DSolidBrush->Release();// 删除画刷
		int iLeft = (m_cxClient - GC.cyBT * DTLRCBTNCOUNT) / 2;
		if (m_iMouseHoverBT < 0)// 画按钮背景
		{
			if (m_iLrcBT != 0)
                m_pD2DRenderTarget->CreateSolidColorBrush(D2D1::ColorF(0x00FFFF, 0.5f), &pD2DSolidBrush);
			else
                m_pD2DRenderTarget->CreateSolidColorBrush(D2D1::ColorF(0xA3FFFF, 0.5f), &pD2DSolidBrush);// 创建按钮背景画刷
            D2D1_RECT_F rcF;
            rcF.left = (FLOAT)(iLeft + (GC.cyBT + DPIS_DTLRCEDGE) * (abs(m_iMouseHoverBT) - 1));
            rcF.top = (FLOAT)(DPIS_DTLRCFRAME + DPIS_DTLRCEDGE);
            rcF.right = (FLOAT)(rcF.left + GC.cyBT);
            rcF.bottom = (FLOAT)(rcF.top + GC.cyBT);
            m_pD2DRenderTarget->FillRectangle(&rcF, pD2DSolidBrush);
            pD2DSolidBrush->Release();// 删除画刷
		}
		int iIconOffest = (GC.cyBT - GC.iIconSize) / 2;
		int iIconTop = DPIS_DTLRCFRAME + DPIS_DTLRCEDGE + iIconOffest;
		int iIconStep = GC.cyBT + DPIS_DTLRCEDGE;
        rcF = { (FLOAT)(iLeft + iIconOffest) ,(FLOAT)iIconTop };
        rcF.left = (FLOAT)(iLeft + iIconOffest);
        rcF.top = (FLOAT)iIconTop;
        rcF.right = rcF.left + GC.cyBT;
        rcF.bottom = rcF.top + GC.cyBT;
        m_pD2DRenderTarget->EndDraw();
		DrawIconEx(m_hCDCDTLrc, iLeft + iIconOffest, iIconTop, GR.hiLast2, 0, 0, 0, NULL, DI_NORMAL);
		iLeft += iIconStep;
		DrawIconEx(m_hCDCDTLrc, iLeft + iIconOffest, iIconTop, g_bPlayIcon ? GR.hiPlay2 : GR.hiPause2, 0, 0, 0, NULL, DI_NORMAL);
		iLeft += iIconStep;
		DrawIconEx(m_hCDCDTLrc, iLeft + iIconOffest, iIconTop, GR.hiNext2, 0, 0, 0, NULL, DI_NORMAL);
		iLeft += iIconStep;
		DrawIconEx(m_hCDCDTLrc, iLeft + iIconOffest, iIconTop, GR.hiCross2, 0, 0, 0, NULL, DI_NORMAL);
        m_pD2DRenderTarget->BeginDraw();
	}
	PWSTR pszLrc = NULL;
    UINT32 uStr1Length = 0, uStr2Length = 0;
    LRCDATA* p = NULL;
	switch (g_iLrcState)
	{
	case LRCSTATE_STOP:
		pszLrc = (PWSTR)L"晴空的音乐播放器 - VC++/Win32";
        uStr1Length = lstrlenW(pszLrc);
		break;
	case LRCSTATE_NOLRC:
	NoLrc:// 没播到第一句时跳过来
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
		else//有歌词但当前索引小于0，说明还没播到第一句
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
	
	IDWriteTextLayout* pDWTextLayout, * pDWTextLayout2;// 文本布局
	DWRITE_TEXT_METRICS DWTextMetrics, DWTextMetrics2;// 文本度量
	D2D1_LINEAR_GRADIENT_BRUSH_PROPERTIES GradientBrushProp;// 线性刷属性
	ID2D1GradientStopCollection* pD2DGradientStopCollection;// 梯度点集合
	D2D1_GRADIENT_STOP GradientStop[2];// 梯度点
	ID2D1LinearGradientBrush* pD2DGradientBrush;// 线性刷
    QKDWTextRenderer_DrawOutline* pMyRenderer;// 自定义文本渲染器

    float x, iShandowOffest = 2;
    if (p)
    {
        m_pDWTextFormat->SetWordWrapping(DWRITE_WORD_WRAPPING_NO_WRAP);// 禁止自动换行
        if (p->iOrgLength == -1)// 单行
        {
            m_pDWTextFormat->SetTextAlignment(DWRITE_TEXT_ALIGNMENT_LEADING);
            g_pDWFactory->CreateTextLayout(pszLrc, uStr1Length, m_pDWTextFormat, (FLOAT)m_cxClient, (FLOAT)m_cyClient, &pDWTextLayout);// 创建文本布局
            pDWTextLayout->GetMetrics(&DWTextMetrics);
            if (g_iCurrLrcIndex != m_LrcScrollInfo.iIndex && g_iLrcState == LRCSTATE_NORMAL)
            {
                m_LrcScrollInfo.iIndex = g_iCurrLrcIndex;
                if (!m_LrcScrollInfo.bWndSizeChangedFlag)
                    m_LrcScrollInfo.x1 = m_LrcScrollInfo.x2 = 0;
                else
                    m_LrcScrollInfo.bWndSizeChangedFlag = FALSE;

                KillTimer(g_hLrcWnd, IDT_ANIMATION);
                if (DWTextMetrics.width > m_cxLrcRgn)
                {
                    m_LrcScrollInfo.cx1 = (int)DWTextMetrics.width;// 超长了，需要后续滚动
                    m_LrcScrollInfo.fNoScrollingTime1 = m_cxLrcRgn * p->fDelay / DWTextMetrics.width / 2;
                    SetTimer(g_hLrcWnd, IDT_ANIMATION, TIMERELAPSE_ANIMATION, NULL);
                }
                else
                {
                    m_LrcScrollInfo.cx1 = -1;
                    m_LrcScrollInfo.x1 = m_LrcScrollInfo.x2 = 0;
                }
            }

            if (m_LrcScrollInfo.cx1 == -1)// 无需滚动
            {
                pDWTextLayout->Release();
                m_pDWTextFormat->SetTextAlignment(DWRITE_TEXT_ALIGNMENT_CENTER);
                g_pDWFactory->CreateTextLayout(pszLrc, uStr1Length, m_pDWTextFormat, (FLOAT)m_cxClient, (FLOAT)m_cyClient, &pDWTextLayout);// 重新创建文本布局
            }

        NotShowingLrc:
            GradientBrushProp = { D2D1::Point2F(0, (FLOAT)m_yLrcRgn),D2D1::Point2F(0, m_yLrcRgn + DWTextMetrics.height) };

            GradientStop[0] = { 0.0f,m_D2DCrDTLrc1 };
            GradientStop[1] = { 1.0f,m_D2DCrDTLrc2 };

            m_pD2DRenderTarget->CreateGradientStopCollection(GradientStop, 2, &pD2DGradientStopCollection);// 创建梯度点集合
            m_pD2DRenderTarget->CreateLinearGradientBrush(GradientBrushProp, pD2DGradientStopCollection, &pD2DGradientBrush);// 创建线性渐变画刷
            pD2DGradientStopCollection->Release();// 删除梯度点集合

			pMyRenderer = new QKDWTextRenderer_DrawOutline(m_pD2DRenderTarget, pD2DGradientBrush, m_pD2DSolidBrush1);// 创建自定义文本渲染器
			m_pD2DRenderTarget->PushAxisAlignedClip(D2D1::RectF(
                (FLOAT)m_xLrcRgn, 
                (FLOAT)m_yLrcRgn, 
                (FLOAT)(m_xLrcRgn + m_cxLrcRgn), 
                (FLOAT)(m_yLrcRgn + m_cyLrcRgn)),
				D2D1_ANTIALIAS_MODE_PER_PRIMITIVE);// 压入轴对齐剪辑
			if (m_LrcScrollInfo.cx1 == -1)
				x = 0;
			else
                x = (float)(m_LrcScrollInfo.x1 + m_xLrcRgn);
			pDWTextLayout->Draw(NULL, pMyRenderer, x, (FLOAT)m_yLrcRgn);
			m_pD2DRenderTarget->PopAxisAlignedClip();// 弹出轴对齐剪辑
			pMyRenderer->Release();// 删除自定义文本渲染器
			pD2DGradientBrush->Release();// 删除线性渐变画刷
			pDWTextLayout->Release();// 删除文本布局
        }
        else// 双行
        {
            m_pDWTextFormat->SetTextAlignment(DWRITE_TEXT_ALIGNMENT_LEADING);
            g_pDWFactory->CreateTextLayout(pszLrc, uStr1Length, m_pDWTextFormat, (FLOAT)m_cxClient, (FLOAT)m_cyClient, &pDWTextLayout);// 创建文本布局
            pDWTextLayout->GetMetrics(&DWTextMetrics);
            g_pDWFactory->CreateTextLayout(pszLrc + p->iOrgLength + 1, uStr2Length, m_pDWTextFormat, (FLOAT)m_cxClient, (FLOAT)m_cyClient, &pDWTextLayout2);// 创建文本布局
            pDWTextLayout2->GetMetrics(&DWTextMetrics2);
            if (g_iCurrLrcIndex != m_LrcScrollInfo.iIndex && g_iLrcState == LRCSTATE_NORMAL)
            {
                m_LrcScrollInfo.iIndex = g_iCurrLrcIndex;
                if (!m_LrcScrollInfo.bWndSizeChangedFlag)
                    m_LrcScrollInfo.x1 = m_LrcScrollInfo.x2 = 0;
                else
                    m_LrcScrollInfo.bWndSizeChangedFlag = FALSE;

                KillTimer(g_hLrcWnd, IDT_ANIMATION);
                if (DWTextMetrics.width > m_cxLrcRgn)
                {
                    m_LrcScrollInfo.cx1 = (int)DWTextMetrics.width;// 超长了，需要后续滚动
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
                    m_LrcScrollInfo.cx2 = (int)DWTextMetrics2.width;// 超长了，需要后续滚动
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
                g_pDWFactory->CreateTextLayout(pszLrc, uStr1Length, m_pDWTextFormat, (FLOAT)m_cxClient, (FLOAT)m_cyClient, &pDWTextLayout);// 重新创建文本布局
            }

			if (m_LrcScrollInfo.cx2 == -1)
			{
				pDWTextLayout2->Release();
				m_pDWTextFormat->SetTextAlignment(DWRITE_TEXT_ALIGNMENT_CENTER);
				g_pDWFactory->CreateTextLayout(pszLrc + p->iOrgLength + 1, uStr2Length, m_pDWTextFormat, (FLOAT)m_cxClient, (FLOAT)m_cyClient, &pDWTextLayout2);// 重新创建文本布局
			}

			GradientBrushProp = { D2D1::Point2F(0, (FLOAT)m_yLrcRgn),D2D1::Point2F(0, DWTextMetrics.height + DWTextMetrics2.height) };
            GradientStop[0] = { 0.0f,m_D2DCrDTLrc1 };
            GradientStop[1] = { 1.0f,m_D2DCrDTLrc2 };

            m_pD2DRenderTarget->CreateGradientStopCollection(GradientStop, 2, &pD2DGradientStopCollection);// 创建梯度点集合
			m_pD2DRenderTarget->CreateLinearGradientBrush(GradientBrushProp, pD2DGradientStopCollection, &pD2DGradientBrush);// 创建线性渐变画刷
			pD2DGradientStopCollection->Release();// 删除梯度点集合

			pMyRenderer = new QKDWTextRenderer_DrawOutline(m_pD2DRenderTarget, pD2DGradientBrush, m_pD2DSolidBrush1);// 创建自定义文本渲染器
            m_pD2DRenderTarget->PushAxisAlignedClip(D2D1::RectF(
                (FLOAT)m_xLrcRgn, 
                (FLOAT)m_yLrcRgn, 
                (FLOAT)(m_xLrcRgn + m_cxLrcRgn), 
                (FLOAT)(m_yLrcRgn + m_cyLrcRgn)),
				D2D1_ANTIALIAS_MODE_PER_PRIMITIVE);// 压入轴对齐剪辑
            if (m_LrcScrollInfo.cx1 == -1)
                x = 0;
            else
                x = (float)(m_LrcScrollInfo.x1 + m_xLrcRgn);

			pDWTextLayout->Draw(NULL, pMyRenderer, x, (FLOAT)m_yLrcRgn);

			if (m_LrcScrollInfo.cx2 == -1)
				x = 0;
			else
                x = (float)(m_LrcScrollInfo.x2 + m_xLrcRgn);

			pDWTextLayout2->Draw(NULL, pMyRenderer, x, m_yLrcRgn + DWTextMetrics.height);
			m_pD2DRenderTarget->PopAxisAlignedClip();// 弹出轴对齐剪辑
			pMyRenderer->Release();// 删除自定义文本渲染器
			pD2DGradientBrush->Release();// 删除线性渐变画刷
			pDWTextLayout->Release();// 删除文本布局
			pDWTextLayout2->Release();
        }
    }
    else
    {
        m_pDWTextFormat->SetTextAlignment(DWRITE_TEXT_ALIGNMENT_CENTER);
        m_pDWTextFormat->SetWordWrapping(DWRITE_WORD_WRAPPING_WRAP);// 自动换行
        g_pDWFactory->CreateTextLayout(pszLrc, uStr1Length, m_pDWTextFormat, (FLOAT)m_cxClient, (FLOAT)m_cyClient, &pDWTextLayout);// 重新创建文本布局
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
    bf.SourceConstantAlpha = 0xFF;
    bf.AlphaFormat = AC_SRC_ALPHA;
    UpdateLayeredWindow(g_hLrcWnd, NULL, NULL, &size, m_hCDCDTLrc, &pt, 0, &bf, ULW_ALPHA);// 显示
}
int LrcWnd_HitTest()// 歌词窗口命中测试
{
    POINT pt;
    GetCursorPos(&pt);
    ScreenToClient(g_hLrcWnd, &pt);
    RECT rc;
    ///////////////////////////测试边框区域
    // 左边
    rc.left = 0;
    rc.top = 0;
    rc.right = DPIS_DTLRCFRAME;
    rc.bottom = m_cyClient;
    if (PtInRect(&rc, pt))// 左边
    {
        rc.top = 0;
        rc.bottom = DPIS_DTLRCFRAME;
        if (PtInRect(&rc, pt))// 左上
            return HTTOPLEFT;

        rc.top = m_cyClient - DPIS_DTLRCFRAME;
        rc.bottom = m_cyClient;
        if (PtInRect(&rc, pt))// 左下
            return HTBOTTOMLEFT;
        return HTLEFT;
    }

    rc.left = m_cxClient - DPIS_DTLRCFRAME;
    rc.top = 0;
    rc.right = m_cxClient;
    rc.bottom = m_cyClient;
    if (PtInRect(&rc, pt))// 右边
    {
        rc.top = 0;
        rc.bottom = DPIS_DTLRCFRAME;
        if (PtInRect(&rc, pt))// 右上
            return HTTOPRIGHT;

        rc.top = m_cyClient - DPIS_DTLRCFRAME;
        rc.bottom = m_cyClient;
        if (PtInRect(&rc, pt))// 右下
            return HTBOTTOMRIGHT;
        return HTRIGHT;
    }

    rc.left = DPIS_DTLRCFRAME;
    rc.top = 0;
    rc.right = m_cxClient - DPIS_DTLRCFRAME;
    rc.bottom = DPIS_DTLRCFRAME;
    if (PtInRect(&rc, pt))// 顶边
        return HTTOP;

    rc.left = DPIS_DTLRCFRAME;
    rc.top = m_cyClient - DPIS_DTLRCFRAME;
	rc.right = m_cxClient - DPIS_DTLRCFRAME;
	rc.bottom = m_cyClient;
	if (PtInRect(&rc, pt))// 底边
		return HTBOTTOM;
	///////////////////////////测试按钮区域
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
void SettingsUpd_WndLrc()
{
	SAFE_RELEASE(m_pDWTextFormat);
    SAFE_RELEASE(m_pD2DSolidBrush1);
	LrcWnd_CreateRes();
	QKGDIColorToD2DColor(GS.crDTLrc1, &m_D2DCrDTLrc1, GS.iDTLrcTransparent);
	QKGDIColorToD2DColor(GS.crDTLrc2, &m_D2DCrDTLrc2, GS.iDTLrcTransparent);

	QKGDIColorToD2DColor(GS.crDTLrcBorder, &m_D2DCrDTLrcBorder, GS.iDTLrcTransparent);
	if (m_pD2DRenderTarget)
		m_pD2DRenderTarget->CreateSolidColorBrush(m_D2DCrDTLrcBorder, &m_pD2DSolidBrush1);

	LrcWnd_DrawLrc();
}