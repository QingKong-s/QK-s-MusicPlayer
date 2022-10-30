#pragma once
#include "Windows.h"
#include "d2d1.h"
#include "dwrite.h"

#define LRCHITTEST_LAST			-1
#define LRCHITTEST_PLAY			-2
#define LRCHITTEST_NEXT			-3
#define LRCHITTEST_CLOSE		-4

#define DTLRCBTNCOUNT			4

BOOL LrcWnd_Init();
void LrcWnd_Show();
void LrcWnd_DrawLrc();
int LrcWnd_HitTest();
void SettingsUpd_WndLrc();
LRESULT CALLBACK WndProc_Lrc(HWND, UINT, WPARAM, LPARAM);
class QKDWTextRenderer_DrawOutline : public IDWriteTextRenderer
{
private:
	LONG m_uRefCount;
	ID2D1Brush* m_pD2DBrushBody;
	ID2D1SolidColorBrush* m_pD2DBrushOutline;
	ID2D1RenderTarget* m_pD2DRenderTarget_Outline;

public:
	QKDWTextRenderer_DrawOutline(ID2D1RenderTarget* pD2DRenderTarget, ID2D1Brush* pD2DBrushBody, ID2D1SolidColorBrush* pD2DBrushOutline);

	~QKDWTextRenderer_DrawOutline();

	STDMETHOD(DrawGlyphRun)(
		void* clientDrawingContext,
		FLOAT                              baselineOriginX,
		FLOAT                              baselineOriginY,
		DWRITE_MEASURING_MODE              measuringMode,
		DWRITE_GLYPH_RUN const* glyphRun,
		DWRITE_GLYPH_RUN_DESCRIPTION const* glyphRunDescription,
		IUnknown* clientDrawingEffect
		);

	STDMETHOD(DrawUnderline)(
		void* clientDrawingContext,
		FLOAT                  baselineOriginX,
		FLOAT                  baselineOriginY,
		DWRITE_UNDERLINE const* underline,
		IUnknown* clientDrawingEffect
		)
	{
		return E_NOTIMPL;
	}

	STDMETHOD(DrawStrikethrough)(
		void* clientDrawingContext,
		FLOAT                      baselineOriginX,
		FLOAT                      baselineOriginY,
		DWRITE_STRIKETHROUGH const* strikethrough,
		IUnknown* clientDrawingEffect
		)
	{
		return E_NOTIMPL;
	}

	STDMETHOD(DrawInlineObject)(
		void* clientDrawingContext,
		FLOAT               originX,
		FLOAT               originY,
		IDWriteInlineObject* inlineObject,
		BOOL                isSideways,
		BOOL                isRightToLeft,
		IUnknown* clientDrawingEffect
		)
	{
		return E_NOTIMPL;
	}

	STDMETHOD(IsPixelSnappingDisabled)(
		void* clientDrawingContext,
		BOOL* isDisabled
		)
	{
		*isDisabled = FALSE;
		return S_OK;
	}

	STDMETHOD(GetCurrentTransform)(
		void* clientDrawingContext,
		DWRITE_MATRIX* transform
		)
	{
		m_pD2DRenderTarget_Outline->GetTransform(reinterpret_cast<D2D1_MATRIX_3X2_F*>(transform));
		return S_OK;
	}

	STDMETHOD(GetPixelsPerDip)(
		void* clientDrawingContext,
		FLOAT* pixelsPerDip
		)
	{
		float x, y;

		m_pD2DRenderTarget_Outline->GetDpi(&x, &y);
		*pixelsPerDip = x / 96;
		return S_OK;
	}

	HRESULT STDMETHODCALLTYPE QueryInterface(REFIID iid, void FAR* FAR* ppvObj)
	{
		if (iid == IID_IUnknown)
		{
			*ppvObj = this;
			AddRef();
			return S_OK;
		}
		return E_NOINTERFACE;
	}

	ULONG STDMETHODCALLTYPE AddRef()
	{
		++m_uRefCount;
		return m_uRefCount;
	}

	ULONG STDMETHODCALLTYPE Release()
	{
		--m_uRefCount;
		ULONG i = m_uRefCount;
		if (m_uRefCount == 0)
			delete this;
		return i;
	}
};