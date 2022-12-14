#define _CRT_SECURE_NO_WARNINGS 1

#include "WndMain.h"

#include <Windows.h>
#include <commdlg.h>
#include <shlwapi.h>
#include <CommCtrl.h>
#include <vsstyle.h>
#include <Shobjidl.h>
#include <shlobj_core.h>
#include <windowsx.h>
#include <UxTheme.h>
#include <dwmapi.h>
#include <d2d1.h>
#include <d2d1_1.h>
#include <d2d1effects.h>
#include <process.h>

#include <math.h>
#include <stdio.h>

#include "bass.h"
#include "bass_fx.h"
#include "bassmidi.h"

#include "GlobalVar.h"
#include "Function.h"
#include "Resource.h"
#include "MyProject.h"
#include "QKCtrl.h"
#include "WndLrc.h"
#include "OLEDragDrop.h"
#include "WndEffect.h"
#include "WndList.h"
#include "WndOptions.h"
#include "PlayingStatistics.h"

CURRMUSICINFO                   m_CurrSongInfo              = { 0 };        //当前信息（在顶部显示）

ID2D1DeviceContext*             m_pD2DDCLeftBK              = NULL;
ID2D1GdiInteropRenderTarget*    m_pD2DGdiInteropRTLeftBK    = NULL;
ID2D1Bitmap1*                   m_pD2DBmpLeftBK             = NULL;
ID2D1Bitmap1*                   m_pD2DBmpLeftBK2            = NULL;

IDXGISurface1*                  m_pDXGISfceLeftBK           = NULL;
IDXGISwapChain1*                m_pDXGIScLeftBK             = NULL;

ID2D1SolidColorBrush*           m_pD2DBrMyBlue              = NULL;
ID2D1SolidColorBrush*           m_pD2DBrMyBlue2             = NULL;
//********************歌词绘制相关
IDWriteTextFormat*              m_pDWTFLrc                  = NULL;// 字体
ID2D1SolidColorBrush*           m_pD2DBrLrc1                = NULL;// 常规
ID2D1SolidColorBrush*           m_pD2DBrLrc2                = NULL;// 高亮

DWORD           m_uThreadFlagWaves          = THREADFLAG_STOP;              // 线程工作状态标志
HANDLE          m_htdWaves                  = NULL;         // 线程句柄
DWORD*          m_pdwWavesData              = NULL;         // 指向波形信息数组，左声道：低WORD，右声道：高WORD
int             m_iWavesDataCount           = 0;            // 波形计数

BOOL            m_IsDraw[3]                 = { 0 };        // 绘制标志

BOOL            m_bLrcShow                  = TRUE;

RECT            m_rcLrcShow                 = { 0 };// 滚动歌词区
RECT            m_rcBtmBK                   = { 0 };// 底部按钮区
RECT            m_rcWaves                   = { 0 };// 波形区
RECT            m_rcSpe                     = { 0 };// 频谱区
RECT            m_rcAlbum                   = { 0 };// 旋转封面区

D2D_RECT_F      m_D2DRcLrcShow              = { 0 };// 滚动歌词区
D2D_RECT_F      m_D2DRcBtmBK                = { 0 };// 底部按钮区
D2D_RECT_F      m_D2DRcWaves                = { 0 };// 波形区
D2D_RECT_F      m_D2DRcSpe                  = { 0 };// 频谱区
D2D_RECT_F      m_D2DRcAlbum                = { 0 };// 旋转封面区

int             m_cxLeftBK                  = 0,
                m_cyLeftBK                  = 0,
                m_cxLrcShow                 = 0,
                m_cyLrcShow                 = 0,
                m_cyAlbum                   = 0;

int             m_iLrcSBPos                 = -1;
BOOL            m_bLockLrcScroll            = FALSE;

int             m_iLrcCenter                = -1;
int             m_iLrcMouseHover            = -1;
int             m_iLrcFixedIndex            = -1;

LRCHSCROLLINFO  m_LrcHScrollInfo            = { -1 };// 歌词水平滚动信息；仅适用于当前播放行
LRCVSCROLLINFO  m_LrcVScrollInfo            = { 0 };

int             m_iDrawingID                = 0;

int             m_iLastLrcIndex[2]          = { -1,-1 };// 0：上次中心；1：上次高亮

HDC             m_hdcLeftBK                 = NULL;

BOOL            m_bWndMinimized             = FALSE;

float           m_fRotationAngle            = 0.f;

float           m_fPlayedTime               = 0.f;    // 已播放的时间（秒），用于统计信息
MMRESULT        m_uTimerIDPlayedTime        = 0;    // 定时器ID

void UI_PreProcessAlbumImage(IWICBitmap** ppWICBitmap)
{
    IWICBitmap* pWICBitmap = *ppWICBitmap;
    if (pWICBitmap)
    {
        UINT cx0, cy0, cx, cy;
        pWICBitmap->GetSize(&cx0, &cy0);

        if (max(cx0, cy0) > (UINT)DPIS_LARGEIMAGE)// 限制图片大小
        {
            if (cx0 >= cy0)// 宽度较大
            {
                cy = (UINT)((float)cy0 / (float)cx0 * DPIS_LARGEIMAGE);
                cx = DPIS_LARGEIMAGE;
            }
            else// 高度较大
            {
                cx = (UINT)((float)cx0 / (float)cy0 * DPIS_LARGEIMAGE);
                cy = DPIS_LARGEIMAGE;
            }

            IWICBitmapScaler* pWICBitmapScaler;
            g_pWICFactory->CreateBitmapScaler(&pWICBitmapScaler);
            
            pWICBitmapScaler->Initialize(pWICBitmap, cx, cy, WICBitmapInterpolationModeCubic);
            pWICBitmap->Release();

            g_pWICFactory->CreateBitmapFromSource(pWICBitmapScaler, WICBitmapNoCache, ppWICBitmap);
            pWICBitmapScaler->Release();
        }
    }
    else
		WICCreateBitmap(g_pszDefPic, ppWICBitmap);
}

void MainWnd_ReleaseCurrInfo()
{
	delete[] m_CurrSongInfo.pszName;
    SAFE_RELEASE(m_CurrSongInfo.pD2DBmpOrgAlbum);
    SAFE_RELEASE(m_CurrSongInfo.pD2DBrushOrgAlbum);
	MusicInfo_Release(&m_CurrSongInfo.mi);
    ZeroMemory(&m_CurrSongInfo, sizeof(CURRMUSICINFO));
}

void CALLBACK SyncProc_End(HSYNC handle, DWORD channel, DWORD data, void* user)
{
    PostMessageW(g_hMainWnd, MAINWNDM_AUTONEXT, 0, 0);
}

void CALLBACK TimerProc_PlayedTime(UINT uTimerID, UINT uMsg, DWORD_PTR dwUser, DWORD_PTR dw1, DWORD_PTR dw2)
{
    m_fPlayedTime += ((float)TIMERELAPSE_PLAYEDTIME / 1000.f);
}

int UI_DrawLrcItem(int iIndex, int y, BOOL bTop, BOOL bClearBK, BOOL bImmdShow, BOOL bCenterLine, int* yOut)
{
    if (m_cxLrcShow <= 0 || m_cyLrcShow <= 0)
        return -1;
    BOOL bCurr = (iIndex == g_iCurrLrcIndex);

    LRCDATA* p = (LRCDATA*)QKAGet(g_Lrc, iIndex);

    IDWriteTextLayout* pDWTextLayout1;
    IDWriteTextLayout* pDWTextLayout2;
    ID2D1SolidColorBrush* pD2DBrush;

    if (bCurr)
        pD2DBrush = m_pD2DBrLrc2;
    else
        pD2DBrush = m_pD2DBrLrc1;

    D2D_RECT_F D2DRcF1, D2DRcF2;

    if (y == -1 && !bCenterLine)
    {
        if (p->iDrawID != m_iDrawingID)
            return -1;
        y = p->iLastTop;
        bTop = TRUE;
    }

    int cy1, cy2;
    int cx1, cx2;
    UINT32 uStrLen1, uStrLen2;
    DWRITE_TEXT_METRICS Metrics1, Metrics2;

    if (GS.bForceTwoLines)// 是否禁止换行
    {
        m_pDWTFLrc->SetWordWrapping(DWRITE_WORD_WRAPPING_NO_WRAP);// 禁止自动换行
        if (p->iOrgLength == -1)// 只有一行
        {
            m_pDWTFLrc->SetTextAlignment(DWRITE_TEXT_ALIGNMENT_LEADING);
            uStrLen1 = lstrlenW(p->pszLrc);

            if (bCurr)
            {
                if (iIndex != m_LrcHScrollInfo.iIndex)
                {
                    g_pDWFactory->CreateTextLayout(p->pszLrc, uStrLen1, m_pDWTFLrc, (FLOAT)m_cxLrcShow, (FLOAT)m_cyLrcShow, &pDWTextLayout1);
					pDWTextLayout1->GetMetrics(&m_LrcHScrollInfo.Metrics1);
                    pDWTextLayout1->Release();

                    cx1 = (int)m_LrcHScrollInfo.Metrics1.width;
                    cy1 = (int)m_LrcHScrollInfo.Metrics1.height;

                    m_LrcHScrollInfo.iIndex = iIndex;
                    if (!m_LrcHScrollInfo.bWndSizeChangedFlag)
                        m_LrcHScrollInfo.x1 = m_LrcHScrollInfo.x2 = 0;
                    else
                        m_LrcHScrollInfo.bWndSizeChangedFlag = FALSE;

                    KillTimer(g_hMainWnd, IDT_ANIMATION);
                    if (cx1 > m_cxLrcShow)
                    {
						m_LrcHScrollInfo.cx1 = cx1;// 超长了，需要后续滚动
						m_LrcHScrollInfo.fNoScrollingTime1 = m_cxLrcShow * p->fDelay / m_LrcHScrollInfo.cx1 / 2;
						SetTimer(g_hMainWnd, IDT_ANIMATION, TIMERELAPSE_ANIMATION, TimerProc);
					}
					else
					{
						m_LrcHScrollInfo.cx1 = -1;
						m_LrcHScrollInfo.x1 = m_LrcHScrollInfo.x2 = 0;
					}
				}
                else// 横向滚动就別测高了
                {
                    cx1 = (int)m_LrcHScrollInfo.Metrics1.width;
                    cy1 = (int)m_LrcHScrollInfo.Metrics1.height;
                }
			}
            else
            {
                g_pDWFactory->CreateTextLayout(p->pszLrc, uStrLen1, m_pDWTFLrc, (FLOAT)m_cxLrcShow, (FLOAT)m_cyLrcShow, &pDWTextLayout1);
                pDWTextLayout1->GetMetrics(&Metrics1);
                pDWTextLayout1->Release();
                cx1 = (int)Metrics1.width;
                cy1 = (int)Metrics1.height;
            }

            if (cx1 > m_cxLrcShow)
                m_pDWTFLrc->SetTextAlignment(DWRITE_TEXT_ALIGNMENT_LEADING);
            else
                m_pDWTFLrc->SetTextAlignment(DWRITE_TEXT_ALIGNMENT_CENTER);

            p->cy = cy1;

            if (bCenterLine)
            {
                y -= p->cy / 2;
                bTop = TRUE;
                *yOut = y;
            }

            if (bTop)
                D2DRcF2 = { (float)m_rcLrcShow.left,(float)y,(float)m_rcLrcShow.right,(float)(y + cy1) };
            else
                D2DRcF2 = { (float)m_rcLrcShow.left,(float)(y - p->cy),(float)m_rcLrcShow.right,(float)y };
            D2DRcF1 = D2DRcF2;

            
            if (bImmdShow)
            {
                m_pD2DDCLeftBK->BeginDraw();
                if (D2DRcF2.top < m_rcLrcShow.top)
                    D2DRcF2.top = (FLOAT)m_rcLrcShow.top;
                if (D2DRcF2.bottom > m_rcLrcShow.bottom)
                    D2DRcF2.bottom = (FLOAT)m_rcLrcShow.bottom;
                m_pD2DDCLeftBK->PushAxisAlignedClip(&D2DRcF2, D2D1_ANTIALIAS_MODE_PER_PRIMITIVE);
                m_pD2DDCLeftBK->DrawBitmap(m_pD2DBmpLeftBK2, &D2DRcF1, 1.0f, D2D1_BITMAP_INTERPOLATION_MODE_LINEAR, &D2DRcF1);// 画背景
            }

            if (bCurr)
                D2DRcF1.left += m_LrcHScrollInfo.x1;

            p->rcItem = { (LONG)D2DRcF1.left,(LONG)D2DRcF1.top,(LONG)D2DRcF1.right,(LONG)D2DRcF1.bottom };
            p->iLastTop = (int)D2DRcF1.top;
            m_pD2DDCLeftBK->DrawTextW(p->pszLrc, uStrLen1, m_pDWTFLrc, &D2DRcF1, pD2DBrush);
            
        }
        else// 有两行
        {
            m_pDWTFLrc->SetTextAlignment(DWRITE_TEXT_ALIGNMENT_LEADING);
            uStrLen1 = p->iOrgLength;
            uStrLen2 = lstrlenW(p->pszLrc + p->iOrgLength + 1);

            if (bCurr)
            {
                if (iIndex != m_LrcHScrollInfo.iIndex)
                {
                    g_pDWFactory->CreateTextLayout(p->pszLrc, uStrLen1, m_pDWTFLrc, (FLOAT)m_cxLrcShow, (FLOAT)m_cyLrcShow, &pDWTextLayout1);// 创建文本布局
                    g_pDWFactory->CreateTextLayout(p->pszLrc + p->iOrgLength + 1, uStrLen2, m_pDWTFLrc, (FLOAT)m_cxLrcShow, (FLOAT)m_cyLrcShow, &pDWTextLayout2);// 创建文本布局
                    pDWTextLayout1->GetMetrics(&m_LrcHScrollInfo.Metrics1);
                    pDWTextLayout2->GetMetrics(&m_LrcHScrollInfo.Metrics2);
                    pDWTextLayout1->Release();
                    pDWTextLayout2->Release();

                    cx1 = (int)m_LrcHScrollInfo.Metrics1.width;
                    cx2 = (int)m_LrcHScrollInfo.Metrics2.width;
                    cy1 = (int)m_LrcHScrollInfo.Metrics1.height;
                    cy2 = (int)m_LrcHScrollInfo.Metrics2.height;

                    m_LrcHScrollInfo.iIndex = iIndex;
                    if (!m_LrcHScrollInfo.bWndSizeChangedFlag)
                        m_LrcHScrollInfo.x1 = m_LrcHScrollInfo.x2 = 0;
                    else
                        m_LrcHScrollInfo.bWndSizeChangedFlag = FALSE;

                    KillTimer(g_hMainWnd, IDT_ANIMATION);
                    if (cx1 > m_cxLrcShow)
                    {
                        m_LrcHScrollInfo.cx1 = cx1;// 超长了，需要后续滚动
                        m_LrcHScrollInfo.fNoScrollingTime1 = m_cxLrcShow * p->fDelay / m_LrcHScrollInfo.cx1 / 2;
                        SetTimer(g_hMainWnd, IDT_ANIMATION, TIMERELAPSE_ANIMATION, TimerProc);
                    }
                    else
                    {
                        m_LrcHScrollInfo.cx1 = -1;
                        m_LrcHScrollInfo.x1 = 0;
                    }

                    if (cx2 > m_cxLrcShow)
                    {
                        m_LrcHScrollInfo.cx2 = cx2;// 超长了，需要后续滚动
                        m_LrcHScrollInfo.fNoScrollingTime2 = m_cxLrcShow * p->fDelay / m_LrcHScrollInfo.cx2 / 2;
                        SetTimer(g_hMainWnd, IDT_ANIMATION, TIMERELAPSE_ANIMATION, TimerProc);
                    }
                    else
                    {
                        m_LrcHScrollInfo.cx2 = -1;
                        m_LrcHScrollInfo.x2 = 0;
                    }
                }
                else// 横向滚动就別测高了
                {
                    cx1 = (int)m_LrcHScrollInfo.Metrics1.width;
                    cx2 = (int)m_LrcHScrollInfo.Metrics2.width;
                    cy1 = (int)m_LrcHScrollInfo.Metrics1.height;
                    cy2 = (int)m_LrcHScrollInfo.Metrics2.height;
                }
            }
            else
            {
                g_pDWFactory->CreateTextLayout(p->pszLrc, uStrLen1, m_pDWTFLrc, (FLOAT)m_cxLrcShow, (FLOAT)m_cyLrcShow, &pDWTextLayout1);// 创建文本布局
                g_pDWFactory->CreateTextLayout(p->pszLrc + p->iOrgLength + 1, uStrLen2, m_pDWTFLrc, (FLOAT)m_cxLrcShow, (FLOAT)m_cyLrcShow, &pDWTextLayout2);// 创建文本布局
                pDWTextLayout1->GetMetrics(&Metrics1);
                pDWTextLayout2->GetMetrics(&Metrics2);
                pDWTextLayout1->Release();
                pDWTextLayout2->Release();
                cx1 = (int)Metrics1.width;
                cx2 = (int)Metrics2.width;
                cy1 = (int)Metrics1.height;
                cy2 = (int)Metrics2.height;
            }

            p->cy = cy1 + cy2;

            if (bCenterLine)
            {
                y -= p->cy / 2;
                bTop = TRUE;
                *yOut = y;
            }

            if (bTop)
            {
                D2DRcF2 = { (float)m_rcLrcShow.left,(float)y,(float)m_rcLrcShow.right,(float)(y + p->cy) };

                if (bImmdShow)
                {
                    m_pD2DDCLeftBK->BeginDraw();
                    D2DRcF1 = D2DRcF2;
                    if (D2DRcF2.top < m_rcLrcShow.top)
                        D2DRcF2.top = (FLOAT)m_rcLrcShow.top;
                    if (D2DRcF2.bottom > m_rcLrcShow.bottom)
                        D2DRcF2.bottom = (FLOAT)m_rcLrcShow.bottom;
                    m_pD2DDCLeftBK->PushAxisAlignedClip(&D2DRcF2, D2D1_ANTIALIAS_MODE_PER_PRIMITIVE);
                    m_pD2DDCLeftBK->DrawBitmap(m_pD2DBmpLeftBK2, &D2DRcF1, 1.0f, D2D1_BITMAP_INTERPOLATION_MODE_LINEAR, &D2DRcF1);
                }

                D2DRcF1.top = (FLOAT)y;
                p->iLastTop = (int)D2DRcF1.top;
                D2DRcF1.bottom = D2DRcF1.top + cy1;
                if (bCurr)
                    D2DRcF1.left = (FLOAT)(m_rcLrcShow.left + m_LrcHScrollInfo.x1);
                else
                    D2DRcF1.left = (FLOAT)m_rcLrcShow.left;
                D2DRcF1.right = (FLOAT)m_rcLrcShow.right;
                if (cx1 > m_cxLrcShow)
                    m_pDWTFLrc->SetTextAlignment(DWRITE_TEXT_ALIGNMENT_LEADING);
                else
                    m_pDWTFLrc->SetTextAlignment(DWRITE_TEXT_ALIGNMENT_CENTER);

                m_pD2DDCLeftBK->DrawTextW(p->pszLrc, uStrLen1, m_pDWTFLrc, &D2DRcF1, pD2DBrush);

                D2DRcF1.top = D2DRcF1.bottom;
                D2DRcF1.bottom += cy2;
                if (bCurr)
                    D2DRcF1.left = (FLOAT)(m_rcLrcShow.left + m_LrcHScrollInfo.x2);

                D2DRcF1.right = (FLOAT)m_rcLrcShow.right;
                if (cx2 > m_cxLrcShow)
                    m_pDWTFLrc->SetTextAlignment(DWRITE_TEXT_ALIGNMENT_LEADING);
                else
                    m_pDWTFLrc->SetTextAlignment(DWRITE_TEXT_ALIGNMENT_CENTER);

                m_pD2DDCLeftBK->DrawTextW(p->pszLrc + p->iOrgLength + 1, uStrLen2, m_pDWTFLrc, &D2DRcF1, pD2DBrush);
                D2DRcF1.top = (FLOAT)y;
                D2DRcF1.bottom = D2DRcF1.top + p->cy;
            }
            else
            {
                D2DRcF2 = { (float)m_rcLrcShow.left,(float)(y - p->cy),(float)m_rcLrcShow.right,(float)y };

                if (bImmdShow)
                {
                    m_pD2DDCLeftBK->BeginDraw();
                    D2DRcF1 = D2DRcF2;
                    if (D2DRcF2.top < m_rcLrcShow.top)
                        D2DRcF2.top = (FLOAT)m_rcLrcShow.top;
                    if (D2DRcF2.bottom > m_rcLrcShow.bottom)
                        D2DRcF2.bottom = (FLOAT)m_rcLrcShow.bottom;
                    m_pD2DDCLeftBK->PushAxisAlignedClip(&D2DRcF2, D2D1_ANTIALIAS_MODE_PER_PRIMITIVE);
                    m_pD2DDCLeftBK->DrawBitmap(m_pD2DBmpLeftBK2, &D2DRcF1, 1.0f, D2D1_BITMAP_INTERPOLATION_MODE_LINEAR, &D2DRcF1);
                }

                D2DRcF1.bottom = (FLOAT)y;
                D2DRcF1.top = D2DRcF1.bottom - cy2;
                if (bCurr)
                    D2DRcF1.left = (FLOAT)(m_rcLrcShow.left + m_LrcHScrollInfo.x2);
                else
                    D2DRcF1.left = (FLOAT)m_rcLrcShow.left;
                D2DRcF1.right = (FLOAT)m_rcLrcShow.right;
                if (cx2 > m_cxLrcShow)
                    m_pDWTFLrc->SetTextAlignment(DWRITE_TEXT_ALIGNMENT_LEADING);
                else
                    m_pDWTFLrc->SetTextAlignment(DWRITE_TEXT_ALIGNMENT_CENTER);

                m_pD2DDCLeftBK->DrawTextW(p->pszLrc + p->iOrgLength + 1, uStrLen2, m_pDWTFLrc, &D2DRcF1, pD2DBrush);

                D2DRcF1.bottom = D2DRcF1.top;
                D2DRcF1.top -= cy1;
                p->iLastTop = (int)D2DRcF1.top;
                if (bCurr)
                    D2DRcF1.left = (FLOAT)(m_rcLrcShow.left + m_LrcHScrollInfo.x1);
                D2DRcF1.right = (FLOAT)m_rcLrcShow.right;
                if (cx1 > m_cxLrcShow)
                    m_pDWTFLrc->SetTextAlignment(DWRITE_TEXT_ALIGNMENT_LEADING);
                else
                    m_pDWTFLrc->SetTextAlignment(DWRITE_TEXT_ALIGNMENT_CENTER);

                m_pD2DDCLeftBK->DrawTextW(p->pszLrc, uStrLen1, m_pDWTFLrc, &D2DRcF1, pD2DBrush);
                D2DRcF1.bottom = (FLOAT)y;
                D2DRcF1.top = D2DRcF1.bottom - p->cy;
            }
            p->rcItem = { (LONG)D2DRcF1.left,(LONG)D2DRcF1.top,(LONG)D2DRcF1.right,(LONG)D2DRcF1.bottom };
        }
    }
    else
    {
        m_pDWTFLrc->SetTextAlignment(DWRITE_TEXT_ALIGNMENT_CENTER);
        m_pDWTFLrc->SetWordWrapping(DWRITE_WORD_WRAPPING_WRAP);// 自动换行
        uStrLen1 = lstrlenW(p->pszLrc);
        g_pDWFactory->CreateTextLayout(p->pszLrc, uStrLen1, m_pDWTFLrc, (FLOAT)m_cxLrcShow, (FLOAT)m_cyLrcShow, &pDWTextLayout1);
        pDWTextLayout1->GetMetrics(&Metrics1);
        pDWTextLayout1->Release();
        cy1 = (int)Metrics1.height;
        p->cy = cy1;

        if (bCenterLine)
        {
            y -= p->cy / 2;
            bTop = TRUE;
            *yOut = y;
        }

        if (bTop)
            D2DRcF2 = { (float)m_rcLrcShow.left,(float)y,(float)m_rcLrcShow.right,(float)(y + p->cy) };
        else
            D2DRcF2 = { (float)m_rcLrcShow.left,(float)(y - p->cy),(float)m_rcLrcShow.right,(float)y };

        p->iLastTop = (int)D2DRcF2.top;

        D2DRcF1 = D2DRcF2;

        if (bImmdShow)
        {
            m_pD2DDCLeftBK->BeginDraw();
            if (D2DRcF2.top < m_rcLrcShow.top)
                D2DRcF2.top = (FLOAT)m_rcLrcShow.top;
            if (D2DRcF2.bottom > m_rcLrcShow.bottom)
                D2DRcF2.bottom = (FLOAT)m_rcLrcShow.bottom;
            m_pD2DDCLeftBK->PushAxisAlignedClip(&D2DRcF2, D2D1_ANTIALIAS_MODE_PER_PRIMITIVE);
            m_pD2DDCLeftBK->DrawBitmap(m_pD2DBmpLeftBK2, &D2DRcF1, 1.0f, D2D1_BITMAP_INTERPOLATION_MODE_LINEAR, &D2DRcF1);
        }
        m_pD2DDCLeftBK->DrawTextW(p->pszLrc, uStrLen1, m_pDWTFLrc, &D2DRcF1, pD2DBrush);
        p->rcItem = { (LONG)D2DRcF1.left,(LONG)D2DRcF1.top,(LONG)D2DRcF1.right,(LONG)D2DRcF1.bottom };
    }
    if (m_iLrcMouseHover == iIndex)
    {
        m_pD2DDCLeftBK->CreateSolidColorBrush(D2D1::ColorF(QKGDIClrToCommonClr(QKCOLOR_CYANDEEPER), 0.1f), &pD2DBrush);
        m_pD2DDCLeftBK->FillRectangle(D2DRcF1, pD2DBrush);
        pD2DBrush->Release();
    }

	if (bImmdShow)
	{
		m_pD2DDCLeftBK->PopAxisAlignedClip();
        m_pD2DDCLeftBK->EndDraw();
        m_pDXGIScLeftBK->Present(0, 0);
	}

	p->iDrawID = m_iDrawingID;// 已经绘制标记
	return p->cy;
}

void Playing_ApplyPrevEffect()
{
    BASS_ChannelGetAttribute(g_hStream, BASS_ATTRIB_FREQ, &g_fDefSpeed);// 保存默认速度    
    if (g_fSpeedChanged != SBV_INVALIDVALUE)
        BASS_ChannelSetAttribute(g_hStream, BASS_ATTRIB_FREQ, g_fSpeedChanged * g_fDefSpeed);
    if (g_fBlanceChanged != SBV_INVALIDVALUE)
        BASS_ChannelSetAttribute(g_hStream, BASS_ATTRIB_PAN, g_fBlanceChanged);

    if (g_bSlient)
        BASS_ChannelSetAttribute(g_hStream, BASS_ATTRIB_VOL, 0);
    else if (g_fVolChanged != SBV_INVALIDVALUE)
        BASS_ChannelSetAttribute(g_hStream, BASS_ATTRIB_VOL, g_fVolChanged);

    if (g_GlobalEffect.fTempo != SBV_INVALIDVALUE)
        BASS_ChannelSetAttribute(g_hStream, BASS_ATTRIB_TEMPO, g_GlobalEffect.fTempo);

    if (g_GlobalEffect.hFXChorus)
    {
        g_GlobalEffect.hFXChorus = BASS_ChannelSetFX(g_hStream, BASS_FX_DX8_CHORUS, 1);
        BASS_FXSetParameters(g_GlobalEffect.hFXChorus, &g_GlobalEffect.Chorus);
    }
    if (g_GlobalEffect.hFXCompressor)
    {
        g_GlobalEffect.hFXCompressor = BASS_ChannelSetFX(g_hStream, BASS_FX_DX8_COMPRESSOR, 1);
        BASS_FXSetParameters(g_GlobalEffect.hFXCompressor, &g_GlobalEffect.Compressor);
    }
    if (g_GlobalEffect.hFXDistortion)
    {
        g_GlobalEffect.hFXDistortion = BASS_ChannelSetFX(g_hStream, BASS_FX_DX8_DISTORTION, 1);
        BASS_FXSetParameters(g_GlobalEffect.hFXDistortion, &g_GlobalEffect.Distortion);
    }
    if (g_GlobalEffect.hFXEcho)
    {
        g_GlobalEffect.hFXEcho = BASS_ChannelSetFX(g_hStream, BASS_FX_DX8_ECHO, 1);
        BASS_FXSetParameters(g_GlobalEffect.hFXEcho, &g_GlobalEffect.Echo);
    }
    if (g_GlobalEffect.hFXFlanger)
    {
        g_GlobalEffect.hFXFlanger = BASS_ChannelSetFX(g_hStream, BASS_FX_DX8_FLANGER, 1);
        BASS_FXSetParameters(g_GlobalEffect.hFXFlanger, &g_GlobalEffect.Flanger);
    }
    if (g_GlobalEffect.hFXGargle)
    {
        g_GlobalEffect.hFXGargle = BASS_ChannelSetFX(g_hStream, BASS_FX_DX8_GARGLE, 1);
        BASS_FXSetParameters(g_GlobalEffect.hFXGargle, &g_GlobalEffect.Gargle);
    }
    if (g_GlobalEffect.hFXI3DL2Reverb)
    {
        g_GlobalEffect.hFXI3DL2Reverb = BASS_ChannelSetFX(g_hStream, BASS_FX_DX8_I3DL2REVERB, 1);
        BASS_FXSetParameters(g_GlobalEffect.hFXI3DL2Reverb, &g_GlobalEffect.I3DL2Reverb);
    }
    if (g_GlobalEffect.hFXReverb)
    {
        g_GlobalEffect.hFXReverb = BASS_ChannelSetFX(g_hStream, BASS_FX_DX8_REVERB, 1);
        BASS_FXSetParameters(g_GlobalEffect.hFXReverb, &g_GlobalEffect.Reverb);
    }
    if (g_GlobalEffect.hFXEQ[0])
    {
        for (int i = 0; i < 10; ++i)
        {
            g_GlobalEffect.hFXEQ[i] = BASS_ChannelSetFX(g_hStream, BASS_FX_DX8_PARAMEQ, 1);
            BASS_FXSetParameters(g_GlobalEffect.hFXEQ[i], &g_GlobalEffect.EQ[i]);
        }
    }
    if (g_GlobalEffect.hFXRotate)
    {
        g_GlobalEffect.hFXRotate = BASS_ChannelSetFX(g_hStream, BASS_FX_BFX_ROTATE, 1);
        BASS_FXSetParameters(g_GlobalEffect.hFXRotate, &g_GlobalEffect.Rotate);
    }
}

void Playing_PlayFile(int iIndex)
{
    //////////////清理遗留
    Playing_Stop(TRUE);
    Lrc_ClearArray(g_Lrc);
    g_Lrc = QKACreate(0);
    delete[] m_pdwWavesData;
    m_pdwWavesData = NULL;
    //////////////取现行信息
    PLAYERLISTUNIT* p = List_GetArrayItem(iIndex);
    //////取消上一个播放标记
    int iLastPlayingIndex = g_iCurrFileIndex;
	g_iCurrFileIndex = -1;
	if (iLastPlayingIndex != -1)
		SendMessageW(g_hLV, LVM_REDRAWITEMS, iLastPlayingIndex, iLastPlayingIndex);
	//////开始播放
	g_hStream = BASS_OpenMusic(p->pszFile,
		BASS_SAMPLE_FX | BASS_STREAM_DECODE,
		BASS_SAMPLE_FX | BASS_MUSIC_PRESCAN | BASS_STREAM_DECODE,
		BASS_SAMPLE_FX | BASS_STREAM_DECODE);// 解码

	if (g_iMusicType == MUSICTYPE_MIDI && g_hSoundFont)// 必须放到创建重采样流前面，否则报BASS_ERROR_HANDLE
		BASS_UpdateSoundFont(FALSE);

	g_hStream = BASS_FX_TempoCreate(g_hStream, BASS_SAMPLE_FX | BASS_FX_FREESOURCE);// 创建重采样流
	BASS_ChannelSetSync(g_hStream, BASS_SYNC_END | BASS_SYNC_ONETIME, 0, SyncProc_End, NULL);// 设置同步过程用来跟踪歌曲播放完毕的事件

	if (!g_hStream)
	{
        g_iCurrFileIndex = -1;
		Global_ShowError(L"文件播放失败", NULL, ECODESRC_BASS, g_hMainWnd);
        return;
    }

    BASS_ChannelPlay(g_hStream, TRUE);
    g_bPlaying = TRUE;
    g_llLength = (ULONGLONG)(BASS_ChannelBytes2Seconds(g_hStream, BASS_ChannelGetLength(g_hStream, BASS_POS_BYTE)) * 1000);
    m_uThreadFlagWaves = THREADFLAG_WORKING;
    m_htdWaves = CRTCreateThreadSmp(Thread_GetWavesData, p->pszFile);// 启动线程获取波形数据
    if (g_pITaskbarList)
        g_pITaskbarList->SetProgressState(g_hTBGhost, TBPF_NORMAL);
    //////置播放标记，判断是否要清除稍后播放标记
	g_iCurrFileIndex = iIndex;
	g_iLrcState = LRCSTATE_NOLRC;
	if (iIndex == g_iLaterPlay)
		g_iLaterPlay = -1;
	SendMessageW(g_hLV, LVM_REDRAWITEMS, iIndex, iIndex);
	//////////////取音频信息
    MainWnd_ReleaseCurrInfo();
    //////取名称
    m_CurrSongInfo.pszName = new WCHAR[lstrlenW(p->pszName) + 1];
    lstrcpyW(m_CurrSongInfo.pszName, p->pszName);
    SetWindowTextW(g_hMainWnd, m_CurrSongInfo.pszName);
    //////取标签信息
	MusicInfo_Get(p->pszFile, &m_CurrSongInfo.mi);
    UI_PreProcessAlbumImage(&m_CurrSongInfo.mi.pWICBitmap);
    if (m_CurrSongInfo.mi.pWICBitmap)
    {
        m_pD2DDCLeftBK->CreateBitmapFromWicBitmap(m_CurrSongInfo.mi.pWICBitmap, &m_CurrSongInfo.pD2DBmpOrgAlbum);
        m_pD2DDCLeftBK->CreateBitmapBrush(m_CurrSongInfo.pD2DBmpOrgAlbum, &m_CurrSongInfo.pD2DBrushOrgAlbum);
    }

    DwmInvalidateIconicBitmaps(g_hTBGhost);
    UI_RefreshBmpBrush();
    m_fRotationAngle = 0.f;
    //////解析Lrc歌词
    Lrc_ParseLrcData(p->pszFile, 0, TRUE, NULL, &g_Lrc, GS.iDefTextCode);
    if (!g_Lrc->iCount && m_CurrSongInfo.mi.pszLrc)
    {
        Lrc_ParseLrcData(
            m_CurrSongInfo.mi.pszLrc,
            (lstrlenW(m_CurrSongInfo.mi.pszLrc) + 1) * sizeof(WCHAR),
            FALSE, NULL, &g_Lrc, GS.iDefTextCode);
    }
    //////////////添加统计信息
	if (!p->Artists)
    {
        PS_SplitArtist(m_CurrSongInfo.mi.pszArtist, L"、", &p->Artists);
        if (!p->Artists)
        {
            int iDivPos = QKStrInStr(p->pszName, L" - ");
            if (iDivPos)
            {
                PS_SplitArtist(p->pszName + iDivPos - 1 + 3, L"、", &p->Artists);
                if (p->Artists)
                    p->bArtistsAvailble = TRUE;
                else
                    p->bArtistsAvailble = FALSE;
            }
            else
                p->bArtistsAvailble = FALSE;
        }
        else
            p->bArtistsAvailble = TRUE;
    }

	if (p->Artists)
		for (int i = 0; i < p->Artists->iCount; ++i)
			PS_AddArtistPlayingCount((PCWSTR)QKAGet(p->Artists, i));
	PS_AddSongPlayingCount(p->pszName, p->Artists);
	//////////////应用音效设置
	Playing_ApplyPrevEffect();
	//////////////更新UI
    m_IsDraw[0] = m_IsDraw[1] = m_IsDraw[2] = TRUE;
    Playing_PlayOrPause(POPF_PAUSEICON);
    SendMessageW(g_hBKLeft, LEFTBKM_LRCSHOW_SETMAX, g_Lrc->iCount - 1, 0);
    LrcWnd_DrawLrc();
    UI_UpdateLeftBK();
    g_llLength = (ULONGLONG)(BASS_ChannelBytes2Seconds(
        g_hStream,
        BASS_ChannelGetLength(g_hStream, BASS_POS_BYTE)
    ) * 1000);
    WndProc_LeftBK(g_hBKLeft, LEFTBKM_PROGBAR_SETMAX, (LPARAM)(g_llLength / 10), TRUE);

    SetTimer(g_hBKLeft, IDT_PGS, TIMERELAPSE_PGS, NULL);
}

void Playing_Stop(BOOL bNoGap)
{
    //////////////更新统计信息
    timeKillEvent(m_uTimerIDPlayedTime);
    m_uTimerIDPlayedTime = 0;
    if (g_iCurrFileIndex >= 0)
    {
        PLAYERLISTUNIT* p = List_GetArrayItem(g_iCurrFileIndex);
        QKARRAY pArtistsAry = p->Artists;
        if(pArtistsAry)
            for (int i = 0; i < pArtistsAry->iCount; ++i)
                PS_AddArtistPlayingTime((PCWSTR)QKAGet(pArtistsAry, i), m_fPlayedTime, FALSE, 0, NULL);
        PS_AddSongPlayingTime(p->pszName, m_fPlayedTime);
    }
    m_fPlayedTime = 0;
    //////////////
    KillTimer(g_hBKLeft, IDT_PGS);
    KillTimer(g_hMainWnd, IDT_ANIMATION);
    KillTimer(g_hMainWnd, IDT_ANIMATION2);

    StopThread_Waves();

    BASS_ChannelStop(g_hStream);
    BASS_FreeMusic(g_hStream);
    g_hStream = NULL;
    
    g_iCurrLrcIndex = -2;
    g_iLrcState = LRCSTATE_STOP;
    m_LrcHScrollInfo = { -1 };
    m_LrcVScrollInfo = { 0 };
    g_bPlaying = FALSE;
    m_iLrcSBPos = -1;
    m_iLrcFixedIndex = -1;
    m_iLrcMouseHover = -1;
    m_iLrcCenter = -1;
	m_iLastLrcIndex[0] = m_iLastLrcIndex[1] = -1;

    if (g_pITaskbarList)
        g_pITaskbarList->SetProgressState(g_hTBGhost, TBPF_NOPROGRESS);

    if (!bNoGap)
    {
        g_iCurrFileIndex = -1;
        m_IsDraw[0] = m_IsDraw[1] = m_IsDraw[2] = TRUE;
        
        LrcWnd_DrawLrc();
        SetWindowTextW(g_hMainWnd, L"未播放 - 晴空的音乐播放器");
        Playing_PlayOrPause(POPF_PLAYICON);
        m_IsDraw[0] = m_IsDraw[1] = m_IsDraw[2] = FALSE;
    }
    UI_UpdateLeftBK();
}

void Playing_PlayNext(BOOL bReverse)
{
    int iCount;
    if (g_iSearchResult != -1)
        iCount = g_iSearchResult;
    else
        iCount = g_ItemData->iCount;

    if (!iCount)
        return;

    int iIndex = g_iCurrFileIndex;
    if (g_iLaterPlay != -1)
    {
        iIndex = g_iLaterPlay;
        g_iLaterPlay = -1;
        SendMessageW(g_hLV, LVM_REDRAWITEMS, iIndex, iIndex);
        goto PlayFile;
    }

    if (bReverse)//倒序
    {
        for (int i = 0; i < g_ItemData->iCount; ++i)
        {
            --iIndex;
            if (iIndex < 0)
                iIndex = iCount - 1;

            if (!(List_GetArrayItem(iIndex)->dwFlags & QKLIF_IGNORED))
                goto PlayFile;
        }
    }
    else
    {
        for (int i = 0; i < g_ItemData->iCount; ++i)
        {
            ++iIndex;
            if (iIndex > iCount - 1)
                iIndex = 0;

            if (!(List_GetArrayItem(iIndex)->dwFlags & QKLIF_IGNORED))
                goto PlayFile;
        }
    }
    QKMessageBox(L"没有有效的项目", L"啥鼻，都忽略了你还听腻马啊", (HICON)TD_ERROR_ICON, L"错误");
    return;
PlayFile:
    Playing_PlayFile(iIndex);
}

void Playing_AutoNext()
{
    switch (SendMessageW(g_hBKLeft, LEFTBKM_TOOLBAR_GETREPEATMODE, 0, 0))
    {
    case REPEATMODE_TOTALLOOP://整体循环
        Playing_PlayNext();
        break;

	case REPEATMODE_SINGLELOOP://单曲循环
	{
		PLAYERLISTUNIT* p = List_GetArrayItem(g_iCurrFileIndex);
		if (!p)
		{
			Playing_Stop();
			return;
		}
		BASS_ChannelPlay(g_hStream, TRUE);
		BASS_ChannelSetSync(g_hStream, BASS_SYNC_END | BASS_SYNC_ONETIME, 0, SyncProc_End, NULL);
		SYSTEMTIME st;
		GetLocalTime(&st);
		PS_AddSongSingleLoop(p->pszName, &st);
	}
	break;

    case REPEATMODE_RADOM://随机播放

        break;

    case REPEATMODE_SINGLE://单曲播放
        Playing_Stop();
        break;

    case REPEATMODE_TOTAL:

        break;
    }
}

void Playing_PlayOrPause(UINT uFlag)
{
    if (uFlag == POPF_AUTOSWITCH)
    {
        DWORD dwState = BASS_ChannelIsActive(g_hStream);
        if (dwState == BASS_ACTIVE_PLAYING)
            g_bPlayIcon = TRUE;
        else if (dwState == BASS_ACTIVE_PAUSED)
            g_bPlayIcon = FALSE;
    }
    else
        g_bPlayIcon = (uFlag == POPF_PLAYICON);
    
    if (g_bPlayIcon)
    {
        THUMBBUTTON tb;
        tb.dwMask = THB_ICON | THB_TOOLTIP;
        tb.hIcon = GR.hiPlay2;
        tb.iId = IDTBB_PLAY;
        lstrcpyW(tb.szTip, L"播放");
        if (g_pITaskbarList)
            g_pITaskbarList->ThumbBarUpdateButtons(g_hTBGhost, 1, &tb);
        BASS_ChannelPause(g_hStream);
        KillTimer(g_hBKLeft, IDT_PGS);
        if (m_uTimerIDPlayedTime && g_iCurrFileIndex >= 0)
        {
            timeKillEvent(m_uTimerIDPlayedTime);
            m_uTimerIDPlayedTime = 0;
        }
    }
    else
    {
        THUMBBUTTON tb;
        tb.dwMask = THB_ICON | THB_TOOLTIP;
        tb.hIcon = GR.hiPause2;
        tb.iId = IDTBB_PLAY;
        lstrcpyW(tb.szTip, L"暂停");
        if (g_pITaskbarList)
            g_pITaskbarList->ThumbBarUpdateButtons(g_hTBGhost, 1, &tb);
        BASS_ChannelPlay(g_hStream, FALSE);
        SetTimer(g_hBKLeft, IDT_PGS, TIMERELAPSE_PGS, NULL);
        if (!m_uTimerIDPlayedTime && g_iCurrFileIndex >= 0)
        {
            m_uTimerIDPlayedTime = timeSetEvent(TIMERELAPSE_PLAYEDTIME, GS.uPlayedTimeTimerResolution,
                TimerProc_PlayedTime, 0, TIME_PERIODIC | TIME_KILL_SYNCHRONOUS);
        }
    }

    SendMessageW(g_hBKLeft,LEFTBKM_TOOLBAR_REDRAW, TRUE, TRUE);
    LrcWnd_DrawLrc();
}

void StopThread_Waves()
{
    DWORD dwExitCode;
    BOOL bResult = GetExitCodeThread(m_htdWaves, &dwExitCode);
    if (bResult && dwExitCode == STILL_ACTIVE)
    {
        m_uThreadFlagWaves = THREADFLAG_STOP;
        WaitForSingleObject(m_htdWaves, INFINITE);//等待线程退出
        m_uThreadFlagWaves = THREADFLAG_STOPED;
    }
    CloseHandle(m_htdWaves);
    m_htdWaves = NULL;//清空句柄
}

UINT __stdcall Thread_GetWavesData(void* p)//调用前必须释放先前的内存
{
    HSTREAM hStream = BASS_OpenMusic((PWSTR)p, BASS_STREAM_DECODE, BASS_MUSIC_DECODE | BASS_MUSIC_PRESCAN);
    int iCount = (int)(g_llLength / 20);
    if (iCount <= 0)
    {
        QKOutputDebugInt(BASS_ErrorGetCode());
        m_uThreadFlagWaves = THREADFLAG_ERROR;
        BASS_FreeMusic(hStream);
        return 0;
    }
    m_pdwWavesData = new DWORD[iCount];
    for (int i = 0; i < iCount; i++)
    {
        m_pdwWavesData[i] = BASS_ChannelGetLevel(hStream);
        if (m_uThreadFlagWaves == THREADFLAG_STOP)
            break;
    }
    BASS_FreeMusic(hStream);
    m_iWavesDataCount = iCount;//计数
    m_uThreadFlagWaves = THREADFLAG_STOPED;//已停止
    m_IsDraw[0] = TRUE;//立即重画
    return 0;
}

void UI_UpdateLeftBK()
{
	if (m_cxLeftBK <= 0 || m_cyLeftBK <= 0)
		return;
	IWICBitmap* pWICBitmapOrg = m_CurrSongInfo.mi.pWICBitmap;// 原始WIC位图

	////////////////////绘制内存位图
	m_pD2DDCLeftBK->BeginDraw();
	m_pD2DDCLeftBK->SetTarget(m_pD2DBmpLeftBK2);
	m_pD2DDCLeftBK->Clear();

	UINT cx0, cy0;
	float cxRgn/*截取区域宽*/, cyRgn/*截取区域高*/, cx/*缩放后图片宽*/, cy/*缩放后图片高*/;
	D2D_RECT_F D2DRectF;

	ID2D1SolidColorBrush* pD2DBrush;
	if (pWICBitmapOrg)
	{
		ID2D1Bitmap1* pD2DBitmapOrg;// 原始D2D位图
		m_pD2DDCLeftBK->CreateBitmapFromWicBitmap(pWICBitmapOrg, &pD2DBitmapOrg);
		/*
		情况一，客户区宽为最大边
		cxClient   cyClient
		-------- = --------
		 cxPic      cyRgn
		情况二，客户区高为最大边
		cyClient   cxClient
		-------- = --------
		 cyPic      cxRgn
		*/
		////////////////////处理缩放与截取（无论怎么改变窗口大小，用来模糊的封面图都要居中充满整个窗口）
		D2D_POINT_2F D2DPtF;
		pWICBitmapOrg->GetSize(&cx0, &cy0);
		cyRgn = (float)m_cyLeftBK / (float)m_cxLeftBK * (float)cx0;
		if (cyRgn <= cy0)// 情况一
		{
			cx = (float)m_cxLeftBK;
			cy = cx * cy0 / cx0;
			D2DPtF = { 0,(float)(m_cyLeftBK - cy) / 2 };
		}
		else// 情况二
		{
			cy = (float)m_cyLeftBK;
			cx = cx0 * cy / cy0;
			D2DPtF = { (float)(m_cxLeftBK - cx) / 2,0 };
		}
		////////////缩放
		IWICBitmapScaler* pWICBitmapScaler;// WIC位图缩放器
		g_pWICFactory->CreateBitmapScaler(&pWICBitmapScaler);
		pWICBitmapScaler->Initialize(pWICBitmapOrg, (UINT)cx, (UINT)cy, WICBitmapInterpolationModeCubic);// 缩放
		IWICBitmap* pWICBmpScaled;// 缩放后的WIC位图
		g_pWICFactory->CreateBitmapFromSource(pWICBitmapScaler, WICBitmapNoCache, &pWICBmpScaled);
		pWICBitmapScaler->Release();// 释放WIC位图缩放器
		ID2D1Bitmap1* pD2DBmpScaled;// 缩放后的D2D位图
		m_pD2DDCLeftBK->CreateBitmapFromWicBitmap(pWICBmpScaled, &pD2DBmpScaled);// 转换为D2D位图
		pWICBmpScaled->Release();// 释放缩放后的WIC位图
		////////////模糊 
		ID2D1Image* pD2DBmpBlurred;
		ID2D1Effect* pD2DEffect;
		m_pD2DDCLeftBK->CreateEffect(CLSID_D2D1GaussianBlur, &pD2DEffect);
		pD2DEffect->SetInput(0, pD2DBmpScaled);
		pD2DEffect->SetValue(D2D1_GAUSSIANBLUR_PROP_STANDARD_DEVIATION, 50.0f);// 标准偏差
		pD2DEffect->SetValue(D2D1_GAUSSIANBLUR_PROP_BORDER_MODE, D2D1_BORDER_MODE_HARD);// 硬边缘
		pD2DEffect->GetOutput(&pD2DBmpBlurred);
		pD2DEffect->Release();
		pD2DBmpScaled->Release();
		////////////画模糊背景
		m_pD2DDCLeftBK->DrawImage(pD2DBmpBlurred, &D2DPtF);
		pD2DBmpBlurred->Release();
		////////////画半透明遮罩
		m_pD2DDCLeftBK->CreateSolidColorBrush(D2D1::ColorF(D2D1::ColorF::White, 0.5), &pD2DBrush);
		D2DRectF = { 0,0,(float)m_cxLeftBK,(float)m_cyLeftBK };
		m_pD2DDCLeftBK->FillRectangle(&D2DRectF, pD2DBrush);
		pD2DBrush->Release();
		////////////////////处理封面图位置
		////////////制封面框矩形
		if (m_bLrcShow)// 是否显示滚动歌词
		{
			cxRgn = (float)m_cyAlbum + DPIS_EDGE * 2;
			cyRgn = cxRgn;
		}
		else
		{
			cxRgn = (float)m_cxLeftBK;
			cyRgn = (float)(m_cyLeftBK - DPIS_CYPROGBAR - DPIS_BT - DPIS_CYSPE - DPIS_CYTOPBK);
		}

		if (cxRgn >= cyRgn)// 高度较小
		{
			cy = cyRgn - DPIS_EDGE * 2;
			cx = cy;

            D2DRectF.left = (FLOAT)((cxRgn - cx) / 2);
            D2DRectF.top = (FLOAT)(DPIS_EDGE + DPIS_CYTOPBK);
			D2DRectF.right = D2DRectF.left + cx;
			D2DRectF.bottom = D2DRectF.top + cy;
		}
		else// 宽度较小
		{
			cx = cxRgn - DPIS_EDGE * 2;
			cy = cx;

			D2DRectF.left = (FLOAT)DPIS_EDGE;
            D2DRectF.top = (FLOAT)((cyRgn - cy) / 2 + DPIS_CYTOPBK);
			D2DRectF.right = D2DRectF.left + cx;
			D2DRectF.bottom = D2DRectF.top + cy;
		}
		////////////////////绘制封面图
        if (!g_bShowAlbum)
        {
            ////////////比例缩放
            float f;
            if (cx0 >= cy0)// 高度较小
            {
                f = cx / cx0 * cy0;// 缩放到封面框中，要保证 水平 方向显示完全 高度 要多大？
                D2DRectF.top += ((cy - f) / 2);
                D2DRectF.bottom = D2DRectF.top + f;
            }
            else// 宽度较小
            {
                f = cy / cy0 * cx0;// 缩放到封面框中，要保证 垂直 方向显示完全 宽度 要多大？
                D2DRectF.left += ((cx - f) / 2);
                D2DRectF.right = D2DRectF.left + f;
            }
            ////////////画封面图
            m_pD2DDCLeftBK->DrawBitmap(pD2DBitmapOrg, &D2DRectF);
        }

		pD2DBitmapOrg->Release();// 销毁原始D2D位图
	}
	else// 没有图就全刷白吧
	{
		m_pD2DDCLeftBK->CreateSolidColorBrush(D2D1::ColorF(D2D1::ColorF::White), &pD2DBrush);
		D2DRectF = { 0,0,(float)m_cxLeftBK,(float)m_cyLeftBK };
		m_pD2DDCLeftBK->FillRectangle(&D2DRectF, pD2DBrush);
		pD2DBrush->Release();
	}
	////////////////////画顶部提示信息
	///////////制文本矩形
	D2DRectF = { (float)DPIS_EDGE,(float)DPIS_GAPTOPTIP,(float)(m_cxLeftBK - DPIS_EDGE),(float)DPIS_CYTOPTITLE };
	PCWSTR psz = m_CurrSongInfo.pszName ? m_CurrSongInfo.pszName : L"未播放";
	///////////制溢出裁剪对象
	DWRITE_TRIMMING DWTrimming =
	{
		DWRITE_TRIMMING_GRANULARITY_CHARACTER,// 按字符裁剪
		0,
		0
	};
	IDWriteInlineObject* pDWInlineObj;// 省略号裁剪内联对象
	g_pDWFactory->CreateEllipsisTrimmingSign(g_pDWTFBig, &pDWInlineObj);// 创建省略号裁剪
	///////////置文本格式
	g_pDWTFBig->SetTextAlignment(DWRITE_TEXT_ALIGNMENT_LEADING);
	g_pDWTFBig->SetTrimming(&DWTrimming, pDWInlineObj);// 置溢出裁剪
	g_pDWTFBig->SetWordWrapping(DWRITE_WORD_WRAPPING_NO_WRAP);// 不换行
	g_pDWTFNormal->SetTextAlignment(DWRITE_TEXT_ALIGNMENT_LEADING);
	g_pDWTFNormal->SetTrimming(&DWTrimming, pDWInlineObj);// 置溢出裁剪
	g_pDWTFNormal->SetWordWrapping(DWRITE_WORD_WRAPPING_NO_WRAP);// 不换行
	///////////画大标题
	m_pD2DDCLeftBK->DrawTextW(psz, lstrlenW(psz), g_pDWTFBig, &D2DRectF, m_pD2DBrMyBlue);
	///////////画其他信息
	m_pD2DDCLeftBK->CreateSolidColorBrush(D2D1::ColorF(D2D1::ColorF::Black), &pD2DBrush);// 创建一个黑色画刷
	const static PCWSTR pszTip[4] =
	{
		L"标题：",
		L"艺术家：",
		L"专辑：",
		L"备注："
	};

	PCWSTR pszTip2[4] =
	{
		m_CurrSongInfo.mi.pszTitle,
		m_CurrSongInfo.mi.pszArtist,
		m_CurrSongInfo.mi.pszAlbum,
		m_CurrSongInfo.mi.pszComment
	};

	D2DRectF.left = (FLOAT)DPIS_EDGE;
	D2DRectF.right = D2DRectF.left + DPIS_CXTOPTIP;
    D2DRectF.top = (FLOAT)(DPIS_CYTOPTITLE + DPIS_GAPTOPTIP);
	D2DRectF.bottom = D2DRectF.top + DPIS_CYTOPTIP;

	for (int i = 0; i < sizeof(pszTip) / sizeof(PCWSTR); ++i)
	{
		m_pD2DDCLeftBK->DrawTextW(pszTip[i], lstrlenW(pszTip[i]), g_pDWTFNormal, &D2DRectF, pD2DBrush);
		if (pszTip2[i])
		{
			D2DRectF.left += (FLOAT)DPIS_CXTOPTIP;
            D2DRectF.right = (FLOAT)(m_cxLeftBK - DPIS_EDGE);
			m_pD2DDCLeftBK->DrawTextW(pszTip2[i], lstrlenW(pszTip2[i]), g_pDWTFNormal, &D2DRectF, pD2DBrush);
			D2DRectF.left = (FLOAT)DPIS_EDGE;
			D2DRectF.right = D2DRectF.left + DPIS_CXTOPTIP;
		}
		D2DRectF.top += DPIS_CYTOPTIP;
		D2DRectF.bottom = D2DRectF.top + DPIS_CYTOPTIP;
	}
	pD2DBrush->Release();// 删除画刷
    DWTrimming.granularity = DWRITE_TRIMMING_GRANULARITY_NONE;
    g_pDWTFBig->SetTrimming(&DWTrimming, NULL);
	g_pDWTFNormal->SetTrimming(&DWTrimming, NULL);// 置回原裁剪
	pDWInlineObj->Release();// 释放裁剪内联对象

	m_pD2DDCLeftBK->EndDraw();
    m_pD2DDCLeftBK->SetTarget(m_pD2DBmpLeftBK);
	////////////////////写到交换链缓冲区
	m_pD2DDCLeftBK->BeginDraw();
	m_pD2DDCLeftBK->DrawBitmap(m_pD2DBmpLeftBK2);
	m_pD2DDCLeftBK->EndDraw();
    m_IsDraw[0] = m_IsDraw[1] = m_IsDraw[2] = TRUE;
    if (g_bShowAlbum)
    {
        UI_VEDrawAlbum();
    }
    else
    {
        UI_VEDrawWaves(FALSE);
        UI_VEDrawSpe(FALSE);
    }
    UI_VEProcLrcShowing(FALSE);
	WndProc_LeftBK(g_hBKLeft, LEFTBKM_TOOLBAR_REDRAW, FALSE, TRUE);
    WndProc_LeftBK(g_hBKLeft, LEFTBKM_PROGBAR_REDRAW, FALSE, TRUE);
	////////////////////显示
	m_pDXGIScLeftBK->Present(0, 0);// 上屏
}

void UI_SeparateListWnd(BOOL b)
{
	const ULONG_PTR uCommStyle = WS_VISIBLE | WS_CLIPCHILDREN;
	RECT rc;
	if (b)
	{
		if (g_bListSeped)
			return;
		SetWindowLongPtrW(g_hBKList, GWL_STYLE, WS_OVERLAPPEDWINDOW | WS_POPUP | uCommStyle);
		SetParent(g_hBKList, NULL);
		ShowWindow(g_hSEB, SW_HIDE);
		g_bListSeped = TRUE;

		GetClientRect(g_hBKList, &rc);
		SendMessageW(g_hBKList, WM_SIZE, 0, MAKELONG(rc.right, rc.bottom));
		if (g_bListHidden)
			ShowWindow(g_hBKList, SW_HIDE);
	}
	else
	{
		if (!g_bListSeped)
			return;
		SetParent(g_hBKList, g_hMainWnd);
        ShowWindow(g_hSEB, SW_SHOW);
		SetWindowLongPtrW(g_hBKList, GWL_STYLE, WS_CHILD | uCommStyle);
		g_bListSeped = FALSE;
		if (g_bListHidden)
		{
			ShowWindow(g_hBKList, SW_HIDE);
            ShowWindow(g_hSEB, SW_HIDE);
		}
	}
	GetClientRect(g_hMainWnd, &rc);
	SendMessageW(g_hMainWnd, WM_SIZE, 0, MAKELONG(rc.right, rc.bottom));
    GetClientRect(g_hBKList, &rc);
    SendMessageW(g_hBKList, WM_SIZE, 0, MAKELONG(rc.right, rc.bottom));
}

void UI_ShowList(BOOL b)
{
	RECT rc;
	if (b)
	{
		if (!g_bListHidden)
			return;
		ShowWindow(g_hBKList, SW_SHOW);
		g_bListHidden = FALSE;
		if (!g_bListSeped)
			ShowWindow(g_hSEB, SW_SHOW);
	}
	else
	{
		if (g_bListHidden)
			return;
		ShowWindow(g_hBKList, SW_HIDE);
		ShowWindow(g_hSEB, SW_HIDE);
		g_bListHidden = TRUE;
	}
	GetClientRect(g_hMainWnd, &rc);
	SendMessageW(g_hMainWnd, WM_SIZE, 0, MAKELONG(rc.right, rc.bottom));
}

void UI_VEDrawAlbum(BOOL bImmdShow, BOOL bIndependlyDrawing)
{
    if (bIndependlyDrawing)
        m_pD2DDCLeftBK->BeginDraw();

    m_pD2DDCLeftBK->PushAxisAlignedClip(&m_D2DRcAlbum, D2D1_ANTIALIAS_MODE_ALIASED);// 设个剪辑区，防止边缘残留
    m_pD2DDCLeftBK->DrawBitmap(m_pD2DBmpLeftBK2, &m_D2DRcAlbum, 1.0f, D2D1_BITMAP_INTERPOLATION_MODE_LINEAR, &m_D2DRcAlbum);// 刷背景

    if (m_CurrSongInfo.pD2DBrushOrgAlbum)
    {
        static DWORD dwLevel = -1;
        D2D1_POINT_2F D2DPt1, D2DPt2;
        ID2D1SolidColorBrush* pD2DBrush;
        float cx = m_D2DRcAlbum.right - m_D2DRcAlbum.left, cy = m_D2DRcAlbum.bottom - m_D2DRcAlbum.top;
        m_pD2DDCLeftBK->CreateSolidColorBrush(D2D1::ColorF(D2D1::ColorF::White, 0.6f), &pD2DBrush);
        if (g_hStream)
        {
            /////////////////////////////准备数据
            const int iSampleCount = 100;
            int iBufSize = 2 * iSampleCount * sizeof(float);

            static float fData[200];

            if (!g_bPlayIcon)
            {
                BASS_ChannelGetData(g_hStream, fData, iBufSize);// 取频谱数据
                dwLevel = BASS_ChannelGetLevel(g_hStream);// 取电平

                m_fRotationAngle += 0.5f;
                if (m_fRotationAngle >= 360.f)
                    m_fRotationAngle = 0;
            }
            /////////////////////////////画频谱
            float fStep = cx / iSampleCount;
            
            int k, l;
            for (int j = 0; j < 2; ++j)
            {
                for (int i = 0; i < iSampleCount - 1; ++i)
                {
                    k = (int)((1 - fData[i * 2 + j]) * cy / 2);// 直接从Bass示例里狠狠地抄
                    if (k < 0)
                        k = 0;
                    if (k > cy)
                        k = (int)cy;
                    D2DPt1 = { m_D2DRcAlbum.left + i * fStep,(float)(m_D2DRcAlbum.top + k) };

                    l = (int)((1 - fData[(i + 1) * 2 + j]) * cy / 2);
                    if (l < 0)
                        l = 0;
                    if (l > cy)
                        l = (int)cy;
                    D2DPt2 = { m_D2DRcAlbum.left + (i + 1) * fStep,(float)(m_D2DRcAlbum.top + l) };
                    m_pD2DDCLeftBK->DrawLine(D2DPt1, D2DPt2, m_pD2DBrMyBlue, DPIF(1.F));
                }
            }
            /////////////////////////////画封面边缘
            D2D1_ELLIPSE D2DEllipse;
            D2DEllipse.point = { m_D2DRcAlbum.left + cx / 2.f,m_D2DRcAlbum.top + cy / 2.f };
            float fRadius;
            fRadius = min(cx / 2.f, cy / 2.f);
            D2DEllipse.radiusX = D2DEllipse.radiusY = fRadius;

            m_pD2DDCLeftBK->FillEllipse(&D2DEllipse, pD2DBrush);// 画外圈
            float fOffset = 0.f;
            fRadius -= GC.DS_ALBUMLEVEL;

            if (dwLevel != -1)
            {
                fOffset = ((float)(LOWORD(dwLevel) + HIWORD(dwLevel)) / 2.f) / 32768.f * GC.DS_ALBUMLEVEL;
                fRadius += fOffset;
                D2DEllipse.radiusX = D2DEllipse.radiusY = fRadius;
                pD2DBrush->SetColor(D2D1::ColorF(D2D1::ColorF::White, 0.8f));
                m_pD2DDCLeftBK->FillEllipse(&D2DEllipse, pD2DBrush);// 画电平指示
            }
            
            /////////////////////////////画封面
            D2D1_MATRIX_3X2_F Matrix = D2D1::Matrix3x2F::Rotation(m_fRotationAngle, D2DEllipse.point);// 制旋转矩阵
            m_pD2DDCLeftBK->SetTransform(Matrix);// 置旋转变换

            fRadius = fRadius - fOffset;
            D2DEllipse.radiusX = D2DEllipse.radiusY = fRadius;
            m_pD2DDCLeftBK->FillEllipse(&D2DEllipse, m_CurrSongInfo.pD2DBrushOrgAlbum);

            Matrix = D2D1::Matrix3x2F::Identity();
            m_pD2DDCLeftBK->SetTransform(Matrix);// 还原空变换
        }
        else
        {
            D2D1_ELLIPSE D2DEllipse;
            D2DEllipse.point = { m_D2DRcAlbum.left + cx / 2.f,m_D2DRcAlbum.top + cy / 2.f };
            float fRadius;
            fRadius = min(cx / 2.f, cy / 2.f);
            D2DEllipse.radiusX = D2DEllipse.radiusY = fRadius;

            m_pD2DDCLeftBK->FillEllipse(&D2DEllipse, pD2DBrush);// 画外圈

            fRadius -= GC.DS_ALBUMLEVEL;
            D2DEllipse.radiusX = D2DEllipse.radiusY = fRadius;
            m_pD2DDCLeftBK->FillEllipse(&D2DEllipse, m_CurrSongInfo.pD2DBrushOrgAlbum);
        }
        pD2DBrush->Release();
    }

    m_pD2DDCLeftBK->PopAxisAlignedClip();

    if (bIndependlyDrawing)
        m_pD2DDCLeftBK->EndDraw();
    if (bImmdShow)
        m_pDXGIScLeftBK->Present(0, 0);
}

BOOL UI_VEDrawWaves(BOOL bImmdShow, BOOL bIndependlyDrawing)
{
	if (!m_IsDraw[0])
		return FALSE;

	if (bIndependlyDrawing)
		m_pD2DDCLeftBK->BeginDraw();

	m_pD2DDCLeftBK->DrawBitmap(m_pD2DBmpLeftBK2, &m_D2DRcWaves, 1.0f, D2D1_BITMAP_INTERPOLATION_MODE_LINEAR, &m_D2DRcWaves);// 刷背景

	PCWSTR pszText = NULL;
	if (m_uThreadFlagWaves == THREADFLAG_WORKING)// 正在加载
		pszText = L"正在加载...";
	else if (!g_hStream)// 已停止
		pszText = L"未播放";
	else if (m_uThreadFlagWaves == THREADFLAG_ERROR)// 出错
		pszText = L"错误！";

	if (pszText)// 应当显示提示
	{
		m_IsDraw[0] = FALSE;// 下次不要再更新
		g_pDWTFBig->SetTextAlignment(DWRITE_TEXT_ALIGNMENT_CENTER);
		g_pDWTFBig->SetParagraphAlignment(DWRITE_PARAGRAPH_ALIGNMENT_CENTER);// 水平垂直都居中
		m_pD2DDCLeftBK->DrawTextW(pszText, lstrlenW(pszText), g_pDWTFBig, &m_D2DRcWaves, m_pD2DBrMyBlue);
		if (bIndependlyDrawing)
			m_pD2DDCLeftBK->EndDraw();
		return TRUE;
	}

	if (m_uThreadFlagWaves == THREADFLAG_STOPED)
	{
		int iCurrIndex = (int)(g_fTime * 1000.0 / 20.0);// 算数组索引    20ms一单位
		if (iCurrIndex < 0 || iCurrIndex > (int)m_iWavesDataCount - 1)
		{
			m_pD2DDCLeftBK->EndDraw();
			return TRUE;
		}

		int i = iCurrIndex;
		int x = m_rcWaves.left + m_cyAlbum / 2,
			y = m_rcWaves.top + DPIS_CYSPEHALF;
		D2D1_POINT_2F D2DPtF1, D2DPtF2;

		m_pD2DDCLeftBK->SetAntialiasMode(D2D1_ANTIALIAS_MODE_ALIASED);
        m_pD2DDCLeftBK->PushAxisAlignedClip(&m_D2DRcWaves, D2D1_ANTIALIAS_MODE_PER_PRIMITIVE);
		// 上面是右声道，下面是左声道
		while (true)// 向右画
		{
			D2DPtF1 = { (float)x, (float)(y - HIWORD(m_pdwWavesData[i]) * DPIS_CYSPEHALF / 32768) };
			D2DPtF2 = { (float)x, (float)(y + LOWORD(m_pdwWavesData[i]) * DPIS_CYSPEHALF / 32768) };
			m_pD2DDCLeftBK->DrawLine(D2DPtF1, D2DPtF2, m_pD2DBrMyBlue2, (FLOAT)DPIS_CXWAVESLINE);
			x += DPIS_CXWAVESLINE;
			i++;
			if (i > m_iWavesDataCount - 1 || x >= m_rcWaves.left + m_cyAlbum)
				break;
		}
		i = iCurrIndex;
		x = m_rcWaves.left + m_cyAlbum / 2;
		while (true)// 向左画
		{
			D2DPtF1 = { (FLOAT)x, (FLOAT)(y - HIWORD(m_pdwWavesData[i]) * DPIS_CYSPEHALF / 32768) };
			D2DPtF2 = { (FLOAT)x, (FLOAT)(y + LOWORD(m_pdwWavesData[i]) * DPIS_CYSPEHALF / 32768) };
			m_pD2DDCLeftBK->DrawLine(D2DPtF1, D2DPtF2, m_pD2DBrMyBlue2, (FLOAT)DPIS_CXWAVESLINE);
			x -= DPIS_CXWAVESLINE;
			i--;
			if (i < 0 || x < m_rcWaves.left)
				break;
		}
		x = m_rcWaves.left + m_cyAlbum / 2;

		D2DPtF1 = { (FLOAT)x, (FLOAT)m_rcWaves.top };
		D2DPtF2 = { (FLOAT)x,  (FLOAT)(m_rcWaves.top + DPIS_CYSPE) };
		ID2D1SolidColorBrush* pD2DBrush;
		m_pD2DDCLeftBK->CreateSolidColorBrush(D2D1::ColorF(D2D1::ColorF::Red), &pD2DBrush);
		m_pD2DDCLeftBK->DrawLine(D2DPtF1, D2DPtF2, pD2DBrush, (FLOAT)DPIS_CXWAVESLINE);
		pD2DBrush->Release();
        m_pD2DDCLeftBK->PopAxisAlignedClip();
		m_pD2DDCLeftBK->SetAntialiasMode(D2D1_ANTIALIAS_MODE_PER_PRIMITIVE);
		//if (bImmdShow)
		//{
		//	m_pD2DGdiInteropRTLeftBK->GetDC(D2D1_DC_INITIALIZE_MODE_COPY, &hDC);// 取DC
		//	HDC hDCWnd = GetDC(g_hBKLeft);
		//	BitBlt(hDCWnd, m_rcWaves.left, m_rcWaves.top, DPIS_CXSPE, DPIS_CYSPE, hDC, m_rcWaves.left, m_rcWaves.top, SRCCOPY);// 区域显示
		//	ReleaseDC(g_hBKLeft, hDCWnd);
		//	m_pD2DGdiInteropRTLeftBK->ReleaseDC(&m_rcWaves);
		//}
        if (bIndependlyDrawing)
            m_pD2DDCLeftBK->EndDraw();
        if (bImmdShow)
            m_pDXGIScLeftBK->Present(0, 0);
	}
    else
    {
        if (bIndependlyDrawing)
            m_pD2DDCLeftBK->EndDraw();
    }
    return TRUE;
}

void UI_VEDrawSpe(BOOL bImmdShow, BOOL bIndependlyDrawing)
{
	int iCount = SPECOUNT;
	int m = DPIS_CXSPE - (DPIS_CXSPEBAR + DPIS_CXSPEBARDIV) * SPECOUNT;

	static int cySpeOld[SPECOUNT] = { 0 }, cySpe[SPECOUNT] = { 0 }, yMaxMark[SPECOUNT] = { 0 };
	static int iTime[SPECOUNT] = { 0 };
	static float fData[128] = { 0 };

	if (BASS_ChannelGetData(g_hStream, fData, BASS_DATA_FFT256) == -1)
	{
		ZeroMemory(cySpeOld, sizeof(cySpeOld));
		ZeroMemory(cySpe, sizeof(cySpe));
	}
	if (bIndependlyDrawing)
		m_pD2DDCLeftBK->BeginDraw();

	m_pD2DDCLeftBK->DrawBitmap(m_pD2DBmpLeftBK2, &m_D2DRcSpe, 1.0f, D2D1_BITMAP_INTERPOLATION_MODE_LINEAR, &m_D2DRcSpe);// 刷背景
	for (int i = 0; i < iCount; i++)
	{
		++iTime[i];
		cySpe[i] = abs((int)(fData[i] * 300.0f));
		//////////////////频谱条
		if (cySpe[i] > DPIS_CYSPE)// 超高
			cySpe[i] = DPIS_CYSPE;// 回来

		if (cySpe[i] > cySpeOld[i])// 当前的大于先前的
			cySpeOld[i] = cySpe[i];// 顶上去
		else
			cySpeOld[i] -= SPESTEP_BAR;// 如果不大于就继续落

		if (cySpeOld[i] < 3)// 太低了
			cySpeOld[i] = 3;// 回来
		//////////////////峰值
		if (iTime[i] > 10)// 时间已到
			yMaxMark[i] -= SPESTEP_MAX;// 下落

		if (cySpeOld[i] > yMaxMark[i])// 频谱条大于峰值
		{
			yMaxMark[i] = cySpeOld[i];// 峰值顶上去，重置时间
			iTime[i] = 0;
		}

		if (yMaxMark[i] < 3)// 太低了
			yMaxMark[i] = 3;// 回来
		//////////////////绘制
		//////////制频谱条矩形
        float fBarWidth = (float)(m_cyAlbum / SPECOUNT);
        D2D_RECT_F D2DRectF;
        D2DRectF.left = (FLOAT)(m_rcSpe.left + (fBarWidth + 1) * i);
        D2DRectF.top = (FLOAT)(m_rcSpe.top + DPIS_CYSPE - cySpeOld[i]);
		D2DRectF.right = D2DRectF.left + fBarWidth;
        D2DRectF.bottom = (FLOAT)(m_rcSpe.top + DPIS_CYSPE);
        if (D2DRectF.right > m_D2DRcSpe.right)
            break;
		m_pD2DDCLeftBK->FillRectangle(&D2DRectF, m_pD2DBrMyBlue2);
		//////////制峰值指示矩形
        D2DRectF.top = (FLOAT)(m_rcSpe.top + DPIS_CYSPE - yMaxMark[i]);
		D2DRectF.bottom = D2DRectF.top + 3;
		m_pD2DDCLeftBK->FillRectangle(&D2DRectF, m_pD2DBrMyBlue2);
	}
	//if (bImmdShow)
	//{
	//	HDC hDC;
	//	HRESULT hr = m_pD2DGdiInteropRTLeftBK->GetDC(D2D1_DC_INITIALIZE_MODE_COPY, &hDC);
	//	HDC hDCWnd = GetDC(g_hBKLeft);
	//	BitBlt(hDCWnd, m_rcSpe.left, m_rcSpe.top, DPIS_CXSPE, DPIS_CYSPE, hDC, m_rcSpe.left, m_rcSpe.top, SRCCOPY);// 区域显示
	//	ReleaseDC(g_hBKLeft, hDCWnd);
	//	m_pD2DGdiInteropRTLeftBK->ReleaseDC(NULL);
	//}
	if (bIndependlyDrawing)
		m_pD2DDCLeftBK->EndDraw();
    if (bImmdShow)
        m_pDXGIScLeftBK->Present(0, 0);
}

void UI_VEDrawLrc(int yCenter, BOOL bImmdShow, BOOL bIndependlyDrawing)
{
    if (bIndependlyDrawing)
        m_pD2DDCLeftBK->BeginDraw();
	D2D_RECT_F D2DRectF = { (float)m_rcLrcShow.left,(float)m_rcLrcShow.top,(float)m_rcLrcShow.right,(float)m_rcLrcShow.bottom };
	m_pD2DDCLeftBK->DrawBitmap(m_pD2DBmpLeftBK2, &D2DRectF, 1.0f, D2D1_BITMAP_INTERPOLATION_MODE_LINEAR, &D2DRectF);// 刷背景
	m_pD2DDCLeftBK->PushAxisAlignedClip(&D2DRectF, D2D1_ANTIALIAS_MODE_ALIASED);

	int iTop;
	int iHeight = UI_DrawLrcItem(m_iLrcCenter, yCenter, TRUE, FALSE, FALSE, TRUE, &iTop);
	int iBottom = iTop - GS.iSCLrcLineGap;
	iTop += (GS.iSCLrcLineGap + iHeight);
	int i = m_iLrcCenter;
	////////////////////////////////////////////////向上画
	while (iBottom > m_rcLrcShow.top)
	{
		if (i - 1 < 0)
			break;
		--i;
		iHeight = UI_DrawLrcItem(i, iBottom, FALSE, FALSE, FALSE);
		iBottom -= (GS.iSCLrcLineGap + iHeight);
	}
	i = m_iLrcCenter;
	////////////////////////////////////////////////向下画
	while (iTop < m_rcLrcShow.bottom)
	{
		if (i + 1 >= g_Lrc->iCount)
			break;
		++i;
		iHeight = UI_DrawLrcItem(i, iTop, TRUE, FALSE, FALSE);
		iTop += (GS.iSCLrcLineGap + iHeight);
	}

	m_pD2DDCLeftBK->PopAxisAlignedClip();
	//if (bImmdShow)
	//{
	//	HDC hDC;
	//	HRESULT hr = m_pD2DGdiInteropRTLeftBK->GetDC(D2D1_DC_INITIALIZE_MODE_COPY, &hDC);
	//	HDC hDCWnd = GetDC(g_hBKLeft);
	//	BitBlt(hDCWnd, m_rcLrcShow.left, m_rcLrcShow.top, m_rcLrcShow.right - m_rcLrcShow.left, m_rcLrcShow.bottom - m_rcLrcShow.top,
	//		hDC, m_rcLrcShow.left, m_rcLrcShow.top, SRCCOPY);// 区域显示
	//	ReleaseDC(g_hBKLeft, hDCWnd);
	//	m_pD2DGdiInteropRTLeftBK->ReleaseDC(NULL);
	//}
	if (bIndependlyDrawing)
		m_pD2DDCLeftBK->EndDraw();
    if (bImmdShow)
        m_pDXGIScLeftBK->Present(0, 0);
}

BOOL UI_VEProcLrcShowing(BOOL bImmdShow, BOOL bIndependlyDrawing, BOOL bOnlyDrawDTLrc)
{
    BOOL bSwitchLrc;
    if (m_IsDraw[2])
    {
        PCWSTR pszText = NULL;
        if (!g_hStream)
        {
            pszText = L"晴空的音乐播放器 - VC++/Win32";
            g_iLrcState = LRCSTATE_STOP;
            g_iCurrLrcIndex = -2;
        }
        else if (!g_Lrc->iCount)
        {
            pszText = L"<无歌词>";
            g_iLrcState = LRCSTATE_NOLRC;
            g_iCurrLrcIndex = -2;
        }

        if (pszText)
        {
            if (bIndependlyDrawing)
                m_pD2DDCLeftBK->BeginDraw();

            D2D_RECT_F D2DRectF = { (float)m_rcLrcShow.left,(float)m_rcLrcShow.top,(float)m_rcLrcShow.right,(float)m_rcLrcShow.bottom };
            m_pD2DDCLeftBK->DrawBitmap(m_pD2DBmpLeftBK2, &D2DRectF, 1.0f, D2D1_BITMAP_INTERPOLATION_MODE_LINEAR, &D2DRectF);

            m_pDWTFLrc->SetTextAlignment(DWRITE_TEXT_ALIGNMENT_CENTER);
            m_pDWTFLrc->SetParagraphAlignment(DWRITE_PARAGRAPH_ALIGNMENT_NEAR);

            m_pD2DDCLeftBK->DrawTextW(pszText, lstrlenW(pszText), m_pDWTFLrc, &D2DRectF, m_pD2DBrLrc1, D2D1_DRAW_TEXT_OPTIONS_CLIP);
            //if (bImmdShow)
            //{
            //    HDC hDC;
            //    HRESULT hr = m_pD2DGdiInteropRTLeftBK->GetDC(D2D1_DC_INITIALIZE_MODE_COPY, &hDC);
            //    HDC hDCWnd = GetDC(g_hBKLeft);
            //    BitBlt(hDCWnd, m_rcLrcShow.left, m_rcLrcShow.top, m_rcLrcShow.right - m_rcLrcShow.left, m_rcLrcShow.bottom - m_rcLrcShow.top,
            //        hDC, m_rcLrcShow.left, m_rcLrcShow.top, SRCCOPY);// 区域显示
            //    ReleaseDC(g_hBKLeft, hDCWnd);
            //    m_pD2DGdiInteropRTLeftBK->ReleaseDC(NULL);
            //}
            if (bIndependlyDrawing)
                m_pD2DDCLeftBK->EndDraw();
            if (bImmdShow)
                m_pDXGIScLeftBK->Present(0, 0);
            return TRUE;
        }
    }

    m_iLrcCenter = -1;
	g_iCurrLrcIndex = -2;
	if (g_Lrc->iCount)
	{
		int iArrayCount = g_Lrc->iCount;
		float fTempTime;

		for (int i = 0; i < iArrayCount; i++)// 遍历歌词数组
		{
			fTempTime = ((LRCDATA*)QKAGet(g_Lrc, i))->fTime;
			if (g_fTime < ((LRCDATA*)QKAGet(g_Lrc, 0))->fTime)// 还没播到第一句
			{
				m_iLrcCenter = 0;
				g_iCurrLrcIndex = -1;
				break;
			}
			else if (i == iArrayCount - 1)// 最后一句
			{
				if (g_fTime >= fTempTime)
				{
					m_iLrcCenter = g_iCurrLrcIndex = i;
					break;
				}
			}
			else if (g_fTime >= fTempTime && g_fTime < ((LRCDATA*)QKAGet(g_Lrc, i + 1))->fTime)// 左闭右开，判断歌词区间
			{
				m_iLrcCenter = g_iCurrLrcIndex = i;
				break;
			}
		}
        
		if (m_iLrcSBPos != -1)
			m_iLrcCenter = m_iLrcSBPos;
        if (m_iLrcFixedIndex != -1)
            m_iLrcCenter = m_iLrcFixedIndex;
        // 索引查找完毕
        if (m_iLrcCenter != m_iLastLrcIndex[0] || g_iCurrLrcIndex != m_iLastLrcIndex[1])
        {
            m_IsDraw[2] = TRUE;
            bSwitchLrc = TRUE;
        }
        else
            bSwitchLrc = FALSE;
    }

    if (m_IsDraw[2])
    {
        ++m_iDrawingID;
        if (!m_iDrawingID)
            ++m_iDrawingID;
        g_iLrcState = LRCSTATE_NORMAL;
        if (!bOnlyDrawDTLrc)
        {
            if (bSwitchLrc && GS.bLrcAnimation && m_iLrcCenter != m_iLastLrcIndex[0] && m_iLrcSBPos == -1 && m_iLastLrcIndex[0] != -1)
            {
                if (m_iLastLrcIndex[0] > g_Lrc->iCount - 1)
                    m_iLastLrcIndex[0] = g_Lrc->iCount - 1;;
                LRCDATA* p1 = (LRCDATA*)QKAGet(g_Lrc, m_iLrcCenter), * p2 = (LRCDATA*)QKAGet(g_Lrc, m_iLastLrcIndex[0]);
                IDWriteTextLayout* pDWTextLayout;
                if (GS.bForceTwoLines)
                {
                    m_pDWTFLrc->SetWordWrapping(DWRITE_WORD_WRAPPING_NO_WRAP);
                }
                else
                {
                    m_pDWTFLrc->SetWordWrapping(DWRITE_WORD_WRAPPING_WRAP);
                }
                DWRITE_TEXT_METRICS Metrics;
                g_pDWFactory->CreateTextLayout(p1->pszLrc, lstrlenW(p1->pszLrc), m_pDWTFLrc, (FLOAT)m_cxLeftBK, (FLOAT)m_cyLeftBK, &pDWTextLayout);
                pDWTextLayout->GetMetrics(&Metrics);
                int iHeight = (int)Metrics.height;
                pDWTextLayout->Release();

                g_pDWFactory->CreateTextLayout(p2->pszLrc, lstrlenW(p2->pszLrc), m_pDWTFLrc, (FLOAT)m_cxLeftBK, (FLOAT)m_cyLeftBK, &pDWTextLayout);
                pDWTextLayout->GetMetrics(&Metrics);
                int iHeight2 = (int)Metrics.height;
                pDWTextLayout->Release();

                int iTop = m_rcLrcShow.top + (m_cyLrcShow - iHeight2) / 2;// 上一句顶边
                m_LrcVScrollInfo.iDestTop = m_rcLrcShow.top + m_cyLrcShow / 2;
                m_LrcVScrollInfo.fDelay = 0.1f;
                m_LrcVScrollInfo.fTime = g_fTime;
                float ff;
                if (m_iLrcCenter > m_iLastLrcIndex[0])// 下一句在上一句的下方
                {
                    m_LrcVScrollInfo.bDirection = TRUE;
                    m_LrcVScrollInfo.iSrcTop = m_LrcVScrollInfo.iDestTop + iHeight / 2 + iHeight2 / 2 + GS.iSCLrcLineGap;
                    m_LrcVScrollInfo.iDistance = m_LrcVScrollInfo.iSrcTop - m_LrcVScrollInfo.iDestTop;
                    ff = p1->fTime - p2->fTime;
                    if (m_LrcVScrollInfo.fDelay > ff)
                        m_LrcVScrollInfo.fDelay = ff / 2;
                }
                else// 下一句在上一句的上方
                {
                    m_LrcVScrollInfo.bDirection = FALSE;
                    m_LrcVScrollInfo.iSrcTop = m_LrcVScrollInfo.iDestTop - (iHeight / 2 + iHeight2 / 2 + GS.iSCLrcLineGap);
                    m_LrcVScrollInfo.iDistance = m_LrcVScrollInfo.iDestTop - m_LrcVScrollInfo.iSrcTop;
                    ff = p2->fTime - p1->fTime;
                    if (m_LrcVScrollInfo.fDelay > ff)
                        m_LrcVScrollInfo.fDelay = ff / 2;
                }
                KillTimer(g_hMainWnd, IDT_ANIMATION2);
                SetTimer(g_hMainWnd, IDT_ANIMATION2, TIMERELAPSE_ANIMATION2, TimerProc);
                LrcWnd_DrawLrc();
                m_iLastLrcIndex[0] = m_iLrcCenter;
                m_iLastLrcIndex[1] = g_iCurrLrcIndex;
                m_IsDraw[2] = FALSE;
                return TRUE;
            }

            UI_VEDrawLrc(m_rcLrcShow.top + m_cyLrcShow / 2, bImmdShow, bIndependlyDrawing);
        }

        m_iLastLrcIndex[0] = m_iLrcCenter;
        m_iLastLrcIndex[1] = g_iCurrLrcIndex;
        m_IsDraw[2] = FALSE;

        LrcWnd_DrawLrc();
        return TRUE;
    }
    else
        return FALSE;
}

void UI_RefreshBmpBrush()
{
	if (g_bShowAlbum && m_CurrSongInfo.pD2DBmpOrgAlbum)
	{
		if (m_CurrSongInfo.pD2DBrushOrgAlbum)
			m_CurrSongInfo.pD2DBrushOrgAlbum->Release();

		float fRadius, cx, cy;
		cx = m_D2DRcAlbum.right - m_D2DRcAlbum.left - GC.DS_ALBUMLEVEL * 2;
		cy = m_D2DRcAlbum.bottom - m_D2DRcAlbum.top - GC.DS_ALBUMLEVEL * 2;
		fRadius = min(cx / 2.f, cy / 2.f);
		float xStart = m_D2DRcAlbum.left + GC.DS_ALBUMLEVEL + cx / 2.f - fRadius, yStart = m_D2DRcAlbum.top + GC.DS_ALBUMLEVEL + cy / 2.f - fRadius;
        float fSize = fRadius * 2;
		float fScaleFactor;
		D2D1_SIZE_F D2DSize = m_CurrSongInfo.pD2DBmpOrgAlbum->GetSize();
		if (D2DSize.width > D2DSize.height)// 宽度较大
		{
			fScaleFactor = fSize / D2DSize.height;
			xStart -= ((D2DSize.width - D2DSize.height) / 2.f * fSize / D2DSize.height);
		}
		else// 高度较大
		{
			fScaleFactor = fSize / D2DSize.width;
			yStart -= ((D2DSize.height - D2DSize.width) / 2.f * fSize / D2DSize.width);
		}

		D2D1_MATRIX_3X2_F Matrix, Matrix2;

		Matrix = D2D1::Matrix3x2F::Translation(xStart, yStart);// 制平移矩阵
		D2D1::Matrix3x2F* MatrixObj1 = D2D1::Matrix3x2F::ReinterpretBaseType(&Matrix);// 转类

		Matrix2 = D2D1::Matrix3x2F::Scale(fScaleFactor, fScaleFactor, D2D1::Point2F(xStart, yStart));// 制缩放矩阵
		D2D1::Matrix3x2F* MatrixObj2 = D2D1::Matrix3x2F::ReinterpretBaseType(&Matrix2);// 转类

		Matrix = ((*MatrixObj1) * (*MatrixObj2));// 矩阵相乘

		D2D1_BRUSH_PROPERTIES D2DBrushProp = { 1.f,Matrix };
        m_pD2DDCLeftBK->CreateBitmapBrush(m_CurrSongInfo.pD2DBmpOrgAlbum, NULL, &D2DBrushProp, &m_CurrSongInfo.pD2DBrushOrgAlbum);
    }
}

LRESULT CALLBACK WndProc_TBGhost(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
    switch (uMsg)
    {
    case WM_CREATE:
    {
        ChangeWindowMessageFilter(WM_DWMSENDICONICTHUMBNAIL, MSGFLT_ADD);
        ChangeWindowMessageFilter(WM_DWMSENDICONICLIVEPREVIEWBITMAP, MSGFLT_ADD);
        BOOL b = TRUE;
        DwmSetWindowAttribute(hWnd, DWMWA_HAS_ICONIC_BITMAP, &b, sizeof(b));
        DwmSetWindowAttribute(hWnd, DWMWA_FORCE_ICONIC_REPRESENTATION, &b, sizeof(b));
    }
    return 0;

    case WM_DWMSENDICONICTHUMBNAIL:
    {
        if (!m_CurrSongInfo.mi.pWICBitmap)
            return 0;

        IWICBitmap* pWICBitmapOrg = m_CurrSongInfo.mi.pWICBitmap;

        UINT cx0, cy0, cx, cy, cxMax = HIWORD(lParam), cyMax = LOWORD(lParam);

        IWICBitmapLock* pWICBmpLock;
        WICRect WICRc = { 0 };
        pWICBitmapOrg->GetSize((UINT*)&WICRc.Width, (UINT*)&WICRc.Height);
        pWICBitmapOrg->Lock(&WICRc, WICBitmapLockRead, &pWICBmpLock);// 取位图锁

        UINT uBufSize;
        BYTE* pPicData;
        pWICBmpLock->GetDataPointer(&uBufSize, &pPicData);// 取数据指针
        UINT uStride;
        pWICBmpLock->GetStride(&uStride);// 取跨步
        pWICBmpLock->GetSize(&cx0, &cy0);// 取尺寸

        GpBitmap* pGdipBitmap;
        GdipCreateBitmapFromScan0(cx0, cy0, uStride, PixelFormat32bppArgb, pPicData, &pGdipBitmap);// WIC位图转GP位图

        HBITMAP hBitmap;
        if ((float)cxMax / (float)cyMax > (float)cx0 / (float)cy0)// y对齐
        {
            cy = cyMax;
            cx = cx0 * cy / cy0;
        }
        else// x对齐
        {
            cx = cxMax;
            cy = cx * cy0 / cx0;
        }
        GpBitmap* pGdipBitmapBK;
        GpGraphics* pGdipGraphics;
        GdipCreateBitmapFromScan0(cxMax, cyMax, 0, PixelFormat32bppArgb, NULL, &pGdipBitmapBK);// 建背景位图
        GdipGetImageGraphicsContext(pGdipBitmapBK, &pGdipGraphics);// 取背景图GP图形
        GdipGraphicsClear(pGdipGraphics, 0x00000000);
        GdipDrawImageRectRect(pGdipGraphics, pGdipBitmap,
            (REAL)(cxMax - cx) / 2, (REAL)(cyMax - cy) / 2, (REAL)cx, (REAL)cy,
            0, 0, (REAL)cx0, (REAL)cy0,
            UnitPixel, NULL, NULL, NULL);// 原始图缩放到背景图上
        GdipDeleteGraphics(pGdipGraphics);// 释放GP图形
        GdipCreateHBITMAPFromBitmap(pGdipBitmapBK, &hBitmap, 0x00000000);// GP位图转GDI位图
        DwmSetIconicThumbnail(hWnd, hBitmap, 0);// 置缩略图

        GdipDisposeImage(pGdipBitmap);
        GdipDisposeImage(pGdipBitmapBK);// 删除GP位图
        DeleteObject(hBitmap);// 删除GDI位图
        pWICBmpLock->Release();// 释放WIC位图锁
    }
    return 0;

    case WM_DWMSENDICONICLIVEPREVIEWBITMAP:
    {
        HBITMAP hBitmap;
        GpBitmap* pGdipImage;
        GdipCreateBitmapFromScan0(1, 1, 0, PixelFormat32bppArgb, NULL, &pGdipImage);// 1x1
        POINT pt = { 0 };
        GdipCreateHBITMAPFromBitmap(pGdipImage, &hBitmap, 0x00000000);
        DwmSetIconicLivePreviewBitmap(g_hTBGhost, hBitmap, &pt, 0);
        GdipDisposeImage(pGdipImage);
        DeleteObject(hBitmap);
    }
	return 0;

	case WM_ACTIVATE:// 窗口激活，转发（点击缩略图激活）
	{
        if (g_pITaskbarList)
            g_pITaskbarList->SetTabActive(hWnd, g_hMainWnd, 0);
        if (IsIconic(g_hMainWnd))
            ShowWindow(g_hMainWnd, SW_RESTORE);
		SetForegroundWindow(g_hMainWnd);
	}
	return 0;

    case WM_SYSCOMMAND:// 系统菜单，转发（最大化、最小化、关闭）
        return SendMessageW(g_hMainWnd, WM_SYSCOMMAND, wParam, lParam);

    case WM_COMMAND:// 任务栏工具栏按钮点击，转发
        return SendMessageW(g_hMainWnd, WM_COMMAND, wParam, lParam);

    }
    return DefWindowProcW(hWnd, uMsg, wParam, lParam);
}

LRESULT CALLBACK WndProc(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
    static int cxClient, cyClient;
    if (uMsg == WM_TASKBARBUTTONCREATED)
    {
        g_pITaskbarList->UnregisterTab(g_hTBGhost);
        g_pITaskbarList->RegisterTab(g_hTBGhost, hWnd);
        g_pITaskbarList->SetTabOrder(g_hTBGhost, hWnd);
        THUMBBUTTON tb[3];
        THUMBBUTTONMASK dwMask = THB_ICON | THB_TOOLTIP;
        tb[0].dwMask = dwMask;
        tb[0].hIcon = GR.hiLast2;
        tb[0].iId = IDTBB_LAST;
        lstrcpyW(tb[0].szTip, L"上一曲");

        tb[1].dwMask = dwMask;
        tb[1].hIcon = GR.hiPlay2;
        tb[1].iId = IDTBB_PLAY;
        lstrcpyW(tb[1].szTip, L"播放");

        tb[2].dwMask = dwMask;
        tb[2].hIcon = GR.hiNext2;
        tb[2].iId = IDTBB_NEXT;
        lstrcpyW(tb[2].szTip, L"下一曲");
        g_pITaskbarList->ThumbBarAddButtons(g_hTBGhost, 3, tb);

        return 0;
    }

	switch (uMsg)
	{
    case WM_COMMAND:
    {
		if (HIWORD(wParam) == THBN_CLICKED)
        {
            switch (LOWORD(wParam))
            {
            case IDTBB_LAST:
                SendMessageW(g_hBKLeft, LEFTBKM_TOOLBAR_DOBTOPE, 0, 0);
                return 0;
            case IDTBB_PLAY:
                SendMessageW(g_hBKLeft, LEFTBKM_TOOLBAR_DOBTOPE, 1, 0);
                return 0;
            case IDTBB_NEXT:
                SendMessageW(g_hBKLeft, LEFTBKM_TOOLBAR_DOBTOPE, 2, 0);
                return 0;
            }
        }
    }
    return 0;

    case WM_CREATE:
    {
        g_Lrc = QKACreate(0);
        g_ItemData = QKACreate(0);
        PS_LoadStatFile(NULL);
        ///////////////////////////初始化.....
        g_hMainWnd = hWnd;
        g_iDPI = QKGetDPIForWindow(hWnd);
        UI_UpdateDPISize();
        g_cxBKList = DPIS_DEFCXLV;
        
        HWND hCtrl, hCtrl2;
        ///////////////////////////左侧背景（剪辑子窗口）
        hCtrl2 = CreateWindowExW(0, BKWNDCLASS, NULL, WS_CHILD | WS_VISIBLE | WS_CLIPCHILDREN,
            0, 0, 0, 0,
            hWnd, (HMENU)IDC_BK_LEFT, g_hInst, NULL);
        g_hBKLeft = hCtrl2;
        RECT rc;
        GetClientRect(g_hBKLeft, &rc);

        DXGI_SWAP_CHAIN_DESC1 DXGIScDesc =
        {
            8,
            8,
            DXGI_FORMAT_B8G8R8A8_UNORM,
            FALSE,
            {1, 0},
            DXGI_USAGE_RENDER_TARGET_OUTPUT,
            2,
            DXGI_SCALING_STRETCH,
            DXGI_SWAP_EFFECT_DISCARD,
            DXGI_ALPHA_MODE_UNSPECIFIED,
            DXGI_SWAP_CHAIN_FLAG_GDI_COMPATIBLE// GDI兼容
        };
        HRESULT hr = g_pDXGIFactory->CreateSwapChainForHwnd(g_pDXGIDevice, g_hBKLeft, &DXGIScDesc, NULL, NULL, &m_pDXGIScLeftBK);// 创建交换链

        hr = g_pD2DDevice->CreateDeviceContext(D2D1_DEVICE_CONTEXT_OPTIONS_NONE, &m_pD2DDCLeftBK);// 创建设备上下文
        m_pD2DDCLeftBK->QueryInterface(IID_PPV_ARGS(&m_pD2DGdiInteropRTLeftBK));// 取GDI兼容渲染目标（鸣谢：福仔）
        m_pD2DDCLeftBK->CreateSolidColorBrush(c_D2DClrCyanDeeper, &m_pD2DBrMyBlue);
        m_pD2DDCLeftBK->CreateSolidColorBrush(D2D1::ColorF(0x469EDA), &m_pD2DBrMyBlue2);

        hr = m_pDXGIScLeftBK->GetBuffer(0, IID_PPV_ARGS(&m_pDXGISfceLeftBK));// 取缓冲区
        D2D1_BITMAP_PROPERTIES1 D2DBmpProp =
        {
            {DXGI_FORMAT_B8G8R8A8_UNORM,D2D1_ALPHA_MODE_PREMULTIPLIED},
            96,
            96,
            D2D1_BITMAP_OPTIONS_TARGET | D2D1_BITMAP_OPTIONS_CANNOT_DRAW | D2D1_BITMAP_OPTIONS_GDI_COMPATIBLE,
            NULL
        };

        hr = m_pD2DDCLeftBK->CreateBitmapFromDxgiSurface(m_pDXGISfceLeftBK, &D2DBmpProp, &m_pD2DBmpLeftBK);// 创建位图自DXGI表面

        D2D_SIZE_U D2DSizeU = { 8,8 };
        D2DBmpProp.bitmapOptions = D2D1_BITMAP_OPTIONS_TARGET | D2D1_BITMAP_OPTIONS_GDI_COMPATIBLE;
        m_pD2DDCLeftBK->CreateBitmap(D2DSizeU, NULL, 0, D2DBmpProp, &m_pD2DBmpLeftBK2);// 创建一幅内存位图

        SetWindowLongPtrW(hCtrl2, GWLP_WNDPROC, (LONG_PTR)WndProc_LeftBK);
        SendMessageW(hCtrl2, LEFTBKM_TOOLBAR_INIT, 0, 0);

        ///////////////////////////分隔条
        hCtrl = CreateWindowExW(0, QKCCN_SEPARATEBAR, NULL, WS_CHILD | WS_VISIBLE,
            0, 0, 0, 0, hWnd, (HMENU)IDC_SEB, g_hInst, NULL);
        g_hSEB = hCtrl;
        SendMessageW(hCtrl, QKCSEBM_SETCOLOR, QKCOLOR_WHITE, 0);
        SendMessageW(hCtrl, QKCSEBM_SETCOLOR2, 0x9097D2CB, 0);
        SendMessageW(hCtrl, QKCSEBM_SETPROC, (WPARAM)QKCProc_SEB, 0);
        ///////////////////////////列表容器
		hCtrl = CreateWindowExW(0, WNDCLASS_LIST, NULL, WS_CHILD | WS_VISIBLE | WS_CLIPCHILDREN,
            0, 0, 0, 0,
            hWnd, (HMENU)IDC_BK_LIST, g_hInst, NULL);
        g_hBKList = hCtrl;

        UI_PreProcessAlbumImage(&m_CurrSongInfo.mi.pWICBitmap);
        if (m_CurrSongInfo.mi.pWICBitmap)
        {
            m_pD2DDCLeftBK->CreateBitmapFromWicBitmap(m_CurrSongInfo.mi.pWICBitmap, &m_CurrSongInfo.pD2DBmpOrgAlbum);
            m_pD2DDCLeftBK->CreateBitmapBrush(m_CurrSongInfo.pD2DBmpOrgAlbum, &m_CurrSongInfo.pD2DBrushOrgAlbum);
        }
        UI_RefreshBmpBrush();
        SettingsUpd_WndMain();
	}
	return 0;

	case WM_DESTROY:
	{
        Playing_Stop();
        DestroyWindow(g_hTBGhost);
        if (IsWindow(g_hLrcWnd))
            DestroyWindow(g_hLrcWnd);
        if (g_pITaskbarList)
            g_pITaskbarList->Release();

        m_pD2DBrMyBlue->Release();
        m_pD2DBrMyBlue2->Release();
        m_pD2DBmpLeftBK->Release();
        m_pD2DBmpLeftBK2->Release();
        m_pD2DGdiInteropRTLeftBK->Release();
        m_pD2DDCLeftBK->Release();
        m_pDXGISfceLeftBK->Release();
        m_pDXGIScLeftBK->Release();
        m_pDWTFLrc->Release();
        m_pD2DBrLrc1->Release();
        m_pD2DBrLrc2->Release();

        PS_SaveStatFile();
        MainWnd_ReleaseCurrInfo();
        Lrc_ClearArray(g_Lrc);
        delete[] m_pdwWavesData;

		PostQuitMessage(0);
	}
	return 0;

    case WM_SIZE:
    {
        if (wParam == SIZE_MINIMIZED)
        {
            m_bWndMinimized = TRUE;
            return 0;
        }
        else
            m_bWndMinimized = FALSE;
        cxClient = LOWORD(lParam);
        cyClient = HIWORD(lParam);

		int cxLeft;

        if (g_bListSeped || g_bListHidden)
            cxLeft = cxClient;
        else
        {
            cxLeft = cxClient - g_cxBKList - DPIS_GAP;
            SetWindowPos(g_hSEB, NULL,
                cxLeft,
                0,
                DPIS_GAP,
                cyClient,
                SWP_NOZORDER);

            SetWindowPos(g_hBKList, NULL,
                cxLeft + DPIS_GAP,
                0,
                g_cxBKList,
                cyClient,
                SWP_NOZORDER);

            SendMessageW(g_hSEB, QKCSEBM_SETRANGE, DPI(80), cxClient - DPI(80));
        }

        SetWindowPos(g_hBKLeft, NULL, 0, 0,
            cxLeft,
            cyClient,
            SWP_NOZORDER | SWP_NOMOVE);
    }
    return 0;

    case WM_GETMINMAXINFO:// 限制窗口大小
	{
		LPMINMAXINFO pInfo = (LPMINMAXINFO)lParam;
		pInfo->ptMinTrackSize.x = DPI(450);
		pInfo->ptMinTrackSize.y = DPI(640);
	}
	return 0;

	case WM_DPICHANGED:
	{
		g_iDPI = HIWORD(wParam);
		UI_UpdateDPISize();
        ImageList_Destroy((HIMAGELIST)SendMessageW(g_hLV, LVM_GETIMAGELIST, LVSIL_SMALL, 0));
        SendMessageW(g_hLV, LVM_SETIMAGELIST, LVSIL_SMALL, (LPARAM)ImageList_Create(1, DPIS_CYLVITEM, 0, 1, 0));// ILC_COLOR缺省
		RECT* p = (RECT*)lParam;
		SetWindowPos(hWnd, NULL, p->left, p->top, p->right - p->left, p->bottom - p->top, SWP_NOZORDER);

        ///////////////////更新主窗口DPI尺寸相关
        m_cyAlbum = DPI(GS.iAlbumPicSize2);
        SAFE_RELEASE(m_pDWTFLrc);
        g_pDWFactory->CreateTextFormat(GS.pszSCLrcFontName, NULL, DWRITE_FONT_WEIGHT_BOLD, DWRITE_FONT_STYLE_NORMAL, DWRITE_FONT_STRETCH_NORMAL,
            DPIF(GS.iSCLrcFontSize), L"zh-cn", &m_pDWTFLrc);
	}
	return 0;

    case WM_SETTEXT:// 设置标题，转发，否则预览时不会显示标题（鸣谢：nlmhc）
        SetWindowTextW(g_hTBGhost, (PCWSTR)lParam);
        break;

    case MAINWNDM_AUTONEXT:
    {
        WndProc_LeftBK(g_hBKLeft, LEFTBKM_PROGBAR_SETPOS, WndProc_LeftBK(g_hBKLeft, LEFTBKM_PROGBAR_GETMAX, 0, 0), TRUE);
        Playing_AutoNext();
    }
    return 0;

	}
    return DefWindowProcW(hWnd, uMsg, wParam, lParam);
}

LRESULT CALLBACK WndProc_LeftBK(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
    //*********************滚动歌词*********************//
    static int      iDelayTime      = 0;        // 滚动条隐藏计时
	static int      iLastIndex      = -1;
    static RECT     rcLrcSB         = { 0 };    // 滚动条矩形
    static RECT     rcLrcSBThumb    = { 0 };    // 滑块矩形
    static int      iThumbSize      = 80,       // 滑块大小
                    iSBMax          = 0;        // 滚动条最大位置
    static BOOL     bSBLBtnDown     = FALSE;    // 滚动条左键是否按下
	static int      iCursorOffest   = 0;        // 左键按下时，光标离滑块顶边的距离
    //*********************底部工具栏*********************//
    static WCHAR    szTime[48]      = L"00:00/00:00";       // 时间文本
    static int      iRepeatMode     = REPEATMODE_TOTALLOOP; // 循环模式
    static RECT     rcTimeText      = { m_rcBtmBK.left,m_rcBtmBK.top,m_rcBtmBK.left + DPIS_CXTIME,m_rcBtmBK.bottom };// 时间文本矩形
    static int      iHot            = -1,                   // 热点项
                    iPushed         = -1,                   // 按下项
                    iLastHot        = -1,                   // 上一个热点项
                    iLastOver       = -1;                   // 上一个悬停项
    static BOOL     bBTLrcPushed    = FALSE;                // 歌词按钮是否被检查
    static HWND     hToolTip;                               // 工具提示
    static BOOL     bLBTDown        = FALSE;                // 工具栏左键是否按下
	static TTTOOLINFOW ti           = { sizeof(TTTOOLINFOW),TTF_TRACK | TTF_IDISHWND | TTF_ABSOLUTE,hWnd,(UINT_PTR)hWnd,{0},g_hInst,NULL,0 };// 提示工具信息
    static HWND     hDlgEffect      = NULL,                 // 音效对话框
                    hDlgOptinos     = NULL;                 // 设置对话框
    static int      i;
    static D2D_RECT_F D2DRcTimeLabel = { (float)m_rcBtmBK.left,(float)m_rcBtmBK.top,(float)(m_rcBtmBK.left + DPIS_CXTIME),(float)m_rcBtmBK.bottom };// 时间文本矩形
    //*********************进度条*********************//
    static RECT     rcTrackBar      = { 0 };    // 进度条矩形
    static D2D_RECT_F D2DRcTrackBar = { 0 };    // 进度条矩形
    static BOOL     bTBLBtnDown     = FALSE;    // 进度条左键是否按下
    static UINT     uTBMax          = 0;        // 进度条最大值
    static UINT     uTBPos          = 0;        // 进度条位置
	switch (uMsg)
	{
	case WM_PAINT:
	{
		ValidateRect(hWnd, NULL);
		if (m_pDXGIScLeftBK)
			m_pDXGIScLeftBK->Present(0, 0);
	}
	return 0;

    case WM_SIZE:
    {
        m_cxLeftBK = LOWORD(lParam);
        m_cyLeftBK = HIWORD(lParam);
        /////////////////////////调整窗口，更新绘制矩形
        int xSpe, ySpe, xWaves, yWaves;
        if (m_bLrcShow)
        {
            if (g_bShowAlbum)
            {
                ////////////////制滚动歌词区矩形
                m_rcLrcShow =
                {
                    DPIS_EDGE,
					DPIS_CYTOPBK + DPIS_EDGE * 3 + m_cyAlbum,
                    m_cxLeftBK - DPIS_EDGE,
                    m_cyLeftBK - GC.cyBT - DPIS_CYPROGBAR
                };
                m_cxLrcShow = m_rcLrcShow.right - m_rcLrcShow.left;
                m_cyLrcShow = m_rcLrcShow.bottom - m_rcLrcShow.top;
                m_D2DRcLrcShow = { (float)m_rcLrcShow.left,(float)m_rcLrcShow.top,(float)m_rcLrcShow.right,(float)m_rcLrcShow.bottom };
                
                xSpe = ySpe = 0;
                m_rcSpe = { 0 };
                m_D2DRcSpe = { 0 };
                
                xWaves = yWaves = 0;
                m_rcWaves = { 0 };
                m_D2DRcWaves = { 0 };

                m_rcAlbum =
                {
                    DPIS_EDGE,
                    DPIS_CYTOPBK + DPIS_EDGE * 2,
                    m_cxLeftBK - DPIS_EDGE,
                    DPIS_CYTOPBK + DPIS_EDGE * 2 + m_cyAlbum
                };
                m_D2DRcAlbum = { (float)m_rcAlbum.left,(float)m_rcAlbum.top,(float)m_rcAlbum.right,(float)m_rcAlbum.bottom };
            }
            else
            {
                ////////////////制滚动歌词区矩形
                m_rcLrcShow =
                {
                    m_cyAlbum + DPIS_EDGE * 2,
                    DPIS_CYTOPBK + DPIS_EDGE,
                    m_cxLeftBK - DPIS_EDGE,
                    m_cyLeftBK - GC.cyBT - DPIS_CYPROGBAR
                };
                m_cxLrcShow = m_rcLrcShow.right - m_rcLrcShow.left;
                m_cyLrcShow = m_rcLrcShow.bottom - m_rcLrcShow.top;
                m_D2DRcLrcShow = { (float)m_rcLrcShow.left,(float)m_rcLrcShow.top,(float)m_rcLrcShow.right,(float)m_rcLrcShow.bottom };
                ////////////////制频谱区矩形
                xSpe = DPIS_EDGE;
                ySpe = DPIS_CYTOPBK + DPIS_EDGE + m_cyAlbum + DPIS_EDGE;
                m_rcSpe = { xSpe,ySpe,xSpe + m_cyAlbum,ySpe + DPIS_CYSPE };
                m_D2DRcSpe = { (float)m_rcSpe.left,(float)m_rcSpe.top,(float)m_rcSpe.right,(float)m_rcSpe.bottom };
                ////////////////制波形区矩形
                xWaves = DPIS_EDGE;
                yWaves = m_rcSpe.top + DPIS_EDGE + DPIS_CYSPE;
                m_rcWaves = { xWaves,yWaves,xWaves + m_cyAlbum,yWaves + DPIS_CYSPE };
                m_D2DRcWaves = { (float)m_rcWaves.left,(float)m_rcWaves.top,(float)m_rcWaves.right,(float)m_rcWaves.bottom };
            }

            m_LrcHScrollInfo.iIndex = -1;
            m_LrcHScrollInfo.bWndSizeChangedFlag = TRUE;
        }
        else
        {
            if (g_bShowAlbum)
            {
                m_rcAlbum =
                {
                    DPIS_EDGE,
                    DPIS_CYTOPBK + DPIS_EDGE * 2,
                    m_cxLeftBK - DPIS_EDGE,
                    m_cyLeftBK - GC.cyBT - DPIS_CYPROGBAR
                };
                m_D2DRcAlbum = { (float)m_rcAlbum.left,(float)m_rcAlbum.top,(float)m_rcAlbum.right,(float)m_rcAlbum.bottom };
            }
            else
            {
                int cxSpe/* = (m_cxLeftBK - DPIS_EDGE * 2) / (DPIS_CXSPEBAR + DPIS_CXSPEBARDIV) * (DPIS_CXSPEBAR + DPIS_CXSPEBARDIV);
                if (cxSpe > m_cyAlbum)
                    cxSpe */=m_cyAlbum;
                ////////////////清零滚动歌词区矩形
                m_rcLrcShow = { 0 };
                m_cxLrcShow = 0;
                m_cyLrcShow = 0;
                m_D2DRcLrcShow = { 0 };
                ////////////////制频谱区矩形
                xSpe = (m_cxLeftBK - (cxSpe * 2 + DPIS_EDGE)) / 2;
                ySpe = m_cyLeftBK - GC.cyBT - DPIS_CYPROGBAR - DPIS_CYSPE;
                m_rcSpe = { xSpe,ySpe,xSpe + m_cyAlbum,ySpe + DPIS_CYSPE };
                m_D2DRcSpe = { (float)m_rcSpe.left,(float)m_rcSpe.top,(float)m_rcSpe.right,(float)m_rcSpe.bottom };
                ////////////////制波形区矩形
                xWaves = m_rcSpe.left + DPIS_EDGE + m_cyAlbum;
                yWaves = m_rcSpe.top;
                m_rcWaves = { xWaves,yWaves,xWaves + m_cyAlbum,yWaves + DPIS_CYSPE };
                m_D2DRcWaves = { (float)m_rcWaves.left,(float)m_rcWaves.top,(float)m_rcWaves.right,(float)m_rcWaves.bottom };
            }
        }

        UI_RefreshBmpBrush();

        rcTrackBar = { DPI(6),m_cyLeftBK - GC.cyBT - DPIS_CYPROGBAR,m_cxLeftBK - DPI(6),0 };
        rcTrackBar.bottom = rcTrackBar.top + DPIS_CYPROGBAR;
        D2DRcTrackBar = { (float)rcTrackBar.left,(float)rcTrackBar.top,(float)rcTrackBar.right,(float)rcTrackBar.bottom };

        int xBtmBK = (m_cxLeftBK - GC.cxBKBtm) / 2;
        int yBtmBK = m_cyLeftBK - GC.cyBT;

        m_rcBtmBK = { xBtmBK,yBtmBK,xBtmBK + GC.cxBKBtm,yBtmBK + GC.cyBT };

        rcLrcSB =
        {
            m_cxLeftBK - DPIS_CXLRCTB * 3 / 4,
            m_rcLrcShow.top,
            m_cxLeftBK - DPIS_CXLRCTB / 4,
            m_rcLrcShow.bottom
        };// 制滚动条矩形

        if (iSBMax)
        {
            rcLrcSBThumb.left = rcLrcSB.left;
            rcLrcSBThumb.top = m_rcLrcShow.top + m_iLrcSBPos * (rcLrcSB.bottom - rcLrcSB.top - iThumbSize) / iSBMax;
            rcLrcSBThumb.right = rcLrcSB.right;
            rcLrcSBThumb.bottom = rcLrcSBThumb.top + iThumbSize;
        }

        if (!m_pDXGIScLeftBK || !m_pD2DDCLeftBK)
            return 0;

        m_pD2DDCLeftBK->SetTarget(NULL);// 移除引用
        m_pD2DBmpLeftBK->Release();
        m_pDXGISfceLeftBK->Release();
        HRESULT hr = m_pDXGIScLeftBK->ResizeBuffers(0, 0, 0, DXGI_FORMAT_UNKNOWN, DXGI_SWAP_CHAIN_FLAG_GDI_COMPATIBLE);

        hr = m_pDXGIScLeftBK->GetBuffer(0, IID_PPV_ARGS(&m_pDXGISfceLeftBK));
		D2D1_BITMAP_PROPERTIES1 D2DBmpProp =
		{
			{DXGI_FORMAT_B8G8R8A8_UNORM,D2D1_ALPHA_MODE_PREMULTIPLIED},
			96,
			96,
		    D2D1_BITMAP_OPTIONS_TARGET | D2D1_BITMAP_OPTIONS_CANNOT_DRAW | D2D1_BITMAP_OPTIONS_GDI_COMPATIBLE,
			NULL
		};
        hr = m_pD2DDCLeftBK->CreateBitmapFromDxgiSurface(m_pDXGISfceLeftBK, &D2DBmpProp, &m_pD2DBmpLeftBK);

        m_pD2DBmpLeftBK2->Release();
        D2D_SIZE_U D2DSizeU = { (UINT32)m_cxLeftBK, (UINT32)m_cyLeftBK };
        D2DBmpProp.bitmapOptions = D2D1_BITMAP_OPTIONS_TARGET | D2D1_BITMAP_OPTIONS_GDI_COMPATIBLE;
        m_pD2DDCLeftBK->CreateBitmap(D2DSizeU, NULL, 0, D2DBmpProp, &m_pD2DBmpLeftBK2);

        UI_UpdateLeftBK();
    }
    return 0;

    case WM_LBUTTONDOWN:
    {
        iDelayTime = 5;// 移动鼠标重置滚动歌词隐藏延时
        POINT pt = { GET_X_LPARAM(lParam), GET_Y_LPARAM(lParam) };
        //***************命中滚动歌词
        if (PtInRect(&m_rcLrcShow, pt))
            SetFocus(hWnd);
        //******命中滚动条
        else if (PtInRect(&rcLrcSB, pt))
        {
            if (PtInRect(&rcLrcSBThumb, pt))// 命中滑块
            {
                bSBLBtnDown = TRUE;
                iCursorOffest = pt.y - m_rcLrcShow.top - m_iLrcSBPos * (rcLrcSB.bottom - rcLrcSB.top - iThumbSize) / iSBMax;// 计算光标偏移
                SetCapture(hWnd);
            }
        }
        //***************命中工具栏
        else if (PtInRect(&m_rcBtmBK, pt))
        {
            iPushed = HitTest_BtmBK(pt);// 判断命中了哪个按钮
			if (iPushed >= 0)
            {
                SetCapture(hWnd);
				WndProc_LeftBK(hWnd, LEFTBKM_TOOLBAR_REDRAW, TRUE, TRUE);
            }
        }
        //***************命中进度条
        else if (PtInRect(&rcTrackBar, pt))
        {
            bTBLBtnDown = TRUE;
            SetCapture(hWnd);
        }
    }
    return 0;

    case WM_LBUTTONUP:
    {
        static POINT pt;
        pt = { GET_X_LPARAM(lParam), GET_Y_LPARAM(lParam) };

        if (bSBLBtnDown)
        {
            bSBLBtnDown = FALSE;
            ReleaseCapture();
        }
        else if (bTBLBtnDown)
		{
			bTBLBtnDown = FALSE;
			ReleaseCapture();
			if (PtInRect(&rcTrackBar, pt))
			{
				BASS_ChannelSetPosition(
					g_hStream,
					BASS_ChannelSeconds2Bytes(
						g_hStream,
						(float)(pt.x - rcTrackBar.left) * (float)uTBMax / (float)(rcTrackBar.right - rcTrackBar.left) / 100.0f),
					BASS_POS_BYTE
				);// 设置位置
			}
		}
		else if (PtInRect(&m_rcBtmBK, pt) || iPushed != -1)
        {
            if (iPushed == -1)
                return 0;
            i = iPushed;
            iPushed = -1;
            iLastHot = -1;
            bLBTDown = FALSE;

            ReleaseCapture();

            WndProc_LeftBK(hWnd, LEFTBKM_TOOLBAR_REDRAW, TRUE, TRUE);
            if (i != HitTest_BtmBK(pt))
                return 0;

            ti.lpszText = NULL;
            SendMessageW(hToolTip, TTM_GETTOOLINFOW, 0, (LPARAM)&ti);
            SendMessageW(hToolTip, TTM_SETTOOLINFOW, 0, (LPARAM)&ti);
            switch (i)
            {
            case 0:// 上一曲
            BTOpe_Last:
            {
                if (g_iCurrFileIndex == -1)
                    return 0;
                Playing_PlayNext(TRUE);
            }
            break;

            case 1:// 播放/暂停
            BTOpe_Play:
            {
                if (g_iCurrFileIndex == -1)
                    return 0;
                Playing_PlayOrPause();
            }
			break;

			case 2:// 停止
			{
				if (g_iCurrFileIndex == -1)
					return 0;
				Playing_Stop();
			}
			break;

			case 3:// 下一曲
			BTOpe_Next:
			{
				if (g_iCurrFileIndex == -1)
					return 0;
				Playing_PlayNext();
			}
			break;

            case 4:// 歌词
            {
                HMENU hMenu = CreatePopupMenu();
                AppendMenuW(hMenu, IsWindow(g_hLrcWnd) ? MF_CHECKED : 0, IDMI_LRC_SHOW, L"桌面歌词");
                AppendMenuW(hMenu, m_bLrcShow ? MF_CHECKED : 0, IDMI_LRC_LRCSHOW, L"滚动歌词");
                AppendMenuW(hMenu, GS.bForceTwoLines ? MF_CHECKED : 0, IDMI_LRC_FORCETWOLINES, L"禁止自动换行");
                RECT rc;
                GetWindowRect(hWnd, &rc);
                int iRet = TrackPopupMenu(hMenu, TPM_RETURNCMD,
                    rc.left + (int)D2DRcTimeLabel.right + GC.cyBT * 4, rc.top + (int)D2DRcTimeLabel.bottom, 0, g_hMainWnd, NULL);
                DestroyMenu(hMenu);
                switch (iRet)
                {
                case IDMI_LRC_SHOW:
                {
                    LrcWnd_Show();
                }
                break;
                case IDMI_LRC_LRCSHOW:
                {
                    m_bLrcShow = !m_bLrcShow;
                    RECT rc;
                    GetClientRect(g_hBKLeft, &rc);
                    SendMessageW(g_hBKLeft, WM_SIZE, 0, MAKELONG(rc.right, rc.bottom));
                }
                break;
                case IDMI_LRC_FORCETWOLINES:
                {
                    if (GS.bForceTwoLines)
                    {
                        GS.bForceTwoLines = FALSE;
                        KillTimer(g_hMainWnd, IDT_ANIMATION);
                    }
                    else
                        GS.bForceTwoLines = TRUE;

                    m_IsDraw[2] = TRUE;
                    TimerProc(NULL, 0, IDT_DRAWING_LRC, 0);
                }
                break;
                }
            }
            break;

            case 5:// 循环方式
            {
                iRepeatMode++;
				if (iRepeatMode > 4)
					iRepeatMode %= 5;
				WndProc_LeftBK(hWnd, LEFTBKM_TOOLBAR_REDRAW, TRUE, TRUE);
				goto ShowToolTip;
			}
			break;

			case 6:// 播放设置
			{
				if (!IsWindow(hDlgEffect))
				{
					hDlgEffect = CreateDialogParamW(g_hInst, MAKEINTRESOURCEW(IDD_EFFECT), hWnd, DlgProc_Effect, 0);
					ShowWindow(hDlgEffect, SW_SHOW);
				}
				else
					SetFocus(hDlgEffect);
			}
			break;

			case 7:// 播放列表
			{
				HMENU hMenu = CreatePopupMenu();
				AppendMenuW(hMenu, g_bListSeped ? MF_CHECKED : 0, IDMI_PL_SEPARATE, L"将列表从主窗口拆离");
				AppendMenuW(hMenu, g_bListHidden ? 0 : MF_CHECKED, IDMI_PL_SHOW, L"显示播放列表");
				RECT rc;
				GetWindowRect(hWnd, &rc);
				int iRet = TrackPopupMenu(hMenu, TPM_RETURNCMD,
					rc.left + (int)D2DRcTimeLabel.right + GC.cyBT * 7, rc.top + (int)D2DRcTimeLabel.bottom, 0, g_hMainWnd, NULL);
				DestroyMenu(hMenu);
				switch (iRet)
				{
				case IDMI_PL_SEPARATE:
					UI_SeparateListWnd(!g_bListSeped);
					break;
				case IDMI_PL_SHOW:
					UI_ShowList(g_bListHidden);
					break;
				}
			}
			break;

			case 8:// 设置
				DialogBoxParamW(g_hInst, MAKEINTRESOURCEW(IDD_OPTIONS), g_hMainWnd, DlgProc_Options, 0);
				break;

			case 9:// 关于
				DialogBoxParamW(g_hInst, MAKEINTRESOURCEW(IDD_ABOUT), g_hMainWnd, DlgProc_About, 0);
				break;
            }
        }
    }
    return 0;

    case WM_MOUSEWHEEL:
    {
        if (!g_Lrc->iCount)
            return 0;

        iDelayTime = 5;
        if (m_iLrcSBPos == -1)
            m_iLrcSBPos = g_iCurrLrcIndex;

        int iDistance = -GET_WHEEL_DELTA_WPARAM(wParam) / WHEEL_DELTA;
        if (m_iLrcSBPos + iDistance < 0)
            m_iLrcSBPos = 0;
        else if (m_iLrcSBPos + iDistance > iSBMax)
            m_iLrcSBPos = iSBMax;
        else
            m_iLrcSBPos = m_iLrcSBPos + iDistance;
        WndProc_LeftBK(hWnd, LEFTBKM_LRCSHOW_REDRAWSB, FALSE, TRUE);
        m_IsDraw[2] = TRUE;
        UI_VEProcLrcShowing();
        SetTimer(hWnd, IDT_LRCSCROLL, TIMERELAPSE_LRCSCROLL, NULL);
    }
    return 0;

	case WM_TIMER:
	{
		switch (wParam)
		{
		case IDT_LRCSCROLL:
		{
			--iDelayTime;
			if (iDelayTime <= 0)
			{
				KillTimer(hWnd, IDT_LRCSCROLL);
				m_iLrcSBPos = -1;
				WndProc_LeftBK(hWnd, LEFTBKM_LRCSHOW_REDRAWSB, TRUE, TRUE);
			}
		}
        return 0;
        case IDT_PGS:
        {
			if (!g_hStream)
				return 0;

			if (BASS_ChannelIsActive(g_hStream) == BASS_ACTIVE_STOPPED && g_bPlaying)
			{
				SyncProc_End(NULL, 0, 0, NULL);
				return 0;
			}

            g_fTime = (float)BASS_ChannelBytes2Seconds(
                g_hStream,
                BASS_ChannelGetPosition(g_hStream, BASS_POS_BYTE));

            if (m_bWndMinimized)
            {
                UI_VEProcLrcShowing(FALSE, TRUE);
            }
            else
            {
                m_pD2DDCLeftBK->BeginDraw();
                //////////////////////////画进度提示
                static int iMinOld = -1, iMin2Old = -1;

                int iMin = (int)g_fTime / 60,
                    iMin2 = (int)(g_llLength / 1000 / 60);
                if (iMin != iMinOld || iMin2 != iMin2Old)
                {
                    wsprintfW(szTime, L"%02d:%02d/%02d:%02d",
                        iMin,
                        (int)g_fTime - iMin * 60,
                        iMin2,
                        (int)(g_llLength / 1000) - iMin2 * 60);

                    m_pD2DDCLeftBK->DrawBitmap(m_pD2DBmpLeftBK2, &D2DRcTimeLabel, 1.0f, D2D1_BITMAP_INTERPOLATION_MODE_LINEAR, &D2DRcTimeLabel);
                    ID2D1SolidColorBrush* pD2DBrush;
                    m_pD2DDCLeftBK->CreateSolidColorBrush(D2D1::ColorF(D2D1::ColorF::Black), &pD2DBrush);
                    g_pDWTFNormal->SetTextAlignment(DWRITE_TEXT_ALIGNMENT_TRAILING);
                    g_pDWTFNormal->SetParagraphAlignment(DWRITE_PARAGRAPH_ALIGNMENT_CENTER);
                    m_pD2DDCLeftBK->DrawTextW(szTime, lstrlenW(szTime), g_pDWTFNormal, &D2DRcTimeLabel, pD2DBrush, D2D1_DRAW_TEXT_OPTIONS_CLIP);
                    pD2DBrush->Release();
                }
                if (g_bShowAlbum)
                {
                    //////////////////////////画旋转封面
                    UI_VEDrawAlbum(FALSE, FALSE);
                }
                else
                {
                    //////////////////////////画波形
                    BOOL bWavesDraw = UI_VEDrawWaves(FALSE, FALSE);
                    //////////////////////////画频谱
                    UI_VEDrawSpe(FALSE, FALSE);
                }
                //////////////////////////画歌词
                BOOL bLrcDraw = UI_VEProcLrcShowing(FALSE, FALSE);
                //////////////////////////画进度条
                uTBPos = (UINT)g_fTime * 100;
                WndProc_LeftBK(hWnd, LEFTBKM_PROGBAR_REDRAW, FALSE, FALSE);
                m_pD2DDCLeftBK->EndDraw();
                m_pDXGIScLeftBK->Present(0, 0);
            }
			if (g_pITaskbarList)
				g_pITaskbarList->SetProgressValue(g_hMainWnd, (ULONGLONG)g_fTime * 100, (ULONGLONG)g_llLength / 10);
        }
        return 0;
		}
	}
	return 0;

    case WM_MOUSEMOVE:
    {
        POINT pt = { GET_X_LPARAM(lParam), GET_Y_LPARAM(lParam) };
        static int iLastPos = -1;
        if (m_iLrcFixedIndex != -1)
            return 0;
        if (bSBLBtnDown)
        {
            iDelayTime = 5;
            int i = (pt.y - m_rcLrcShow.top - iCursorOffest) * iSBMax / (rcLrcSB.bottom - rcLrcSB.top - iThumbSize);
            if (i < 0)
                i = 0;
            else if (i > iSBMax)
                i = iSBMax;
            if (iLastPos != i)
            {
                iLastPos = m_iLrcSBPos = i;
                WndProc_LeftBK(hWnd, LEFTBKM_LRCSHOW_REDRAWSB, FALSE, TRUE);
                UI_VEDrawLrc(m_rcLrcShow.top + m_cyLeftBK / 2);
            }
        }
        else if (m_iLrcCenter >= 0 && m_iLrcFixedIndex == -1)
        {
            if (PtInRect(&m_rcLrcShow, pt))
            {
                int i = HitTest_LrcShow(pt);
                if (i != iLastIndex)
                {
                    m_iLrcMouseHover = i;
                    if (iLastIndex >= 0 && iLastIndex < g_Lrc->iCount)
                        UI_DrawLrcItem(iLastIndex, -1, 0, TRUE, TRUE);

                    if (i != -1 && i < g_Lrc->iCount)
                        UI_DrawLrcItem(i, -1, 0, TRUE, TRUE);

                    iLastIndex = i;
                }
                iDelayTime = 5;
                TRACKMOUSEEVENT tme;
                tme.cbSize = sizeof(TRACKMOUSEEVENT);
                tme.dwFlags = TME_LEAVE;
                tme.hwndTrack = hWnd;
                tme.dwHoverTime = 0;
                TrackMouseEvent(&tme);
            }
            else
            {
                if (iLastIndex != -1)
                {
                    m_iLrcMouseHover = -1;
                    if (iLastIndex < g_Lrc->iCount)
                        UI_DrawLrcItem(iLastIndex, -1, 0, TRUE, TRUE);

                    iLastIndex = -1;
                }
            }
        }

        static BOOL bInBTBK = FALSE;
        if (PtInRect(&m_rcBtmBK, pt))
        {
            bInBTBK = TRUE;
            if (!bLBTDown)
            {
                POINT pt = { GET_X_LPARAM(lParam), GET_Y_LPARAM(lParam) };
                iHot = HitTest_BtmBK(pt);
                if (iLastHot != iHot)
                {
                    ti.lpszText = NULL;
                    SendMessageW(hToolTip, TTM_GETTOOLINFOW, 0, (LPARAM)&ti);
                    SendMessageW(hToolTip, TTM_SETTOOLINFOW, 0, (LPARAM)&ti);
                    iLastHot = iHot;
                    WndProc_LeftBK(hWnd, LEFTBKM_TOOLBAR_REDRAW, TRUE, TRUE);
                }
                TRACKMOUSEEVENT tme;
                tme.cbSize = sizeof(tme);
                tme.hwndTrack = hWnd;
                tme.dwFlags = TME_LEAVE | TME_HOVER;
                tme.dwHoverTime = 200;
                TrackMouseEvent(&tme);
            }
        }
        else
        {
            if (bInBTBK)
            {
                bInBTBK = FALSE;
                iHot = -1;
                WndProc_LeftBK(hWnd, LEFTBKM_TOOLBAR_REDRAW, TRUE, TRUE);
                SendMessageW(hToolTip, TTM_TRACKACTIVATE, FALSE, (LPARAM)&ti);
            }
        }
    }
    return 0;

    case WM_MOUSELEAVE:
    {
        if (iHot != -1)
        {
            ti.lpszText = NULL;
            SendMessageW(hToolTip, TTM_GETTOOLINFOW, 0, (LPARAM)&ti);
            SendMessageW(hToolTip, TTM_SETTOOLINFOW, 0, (LPARAM)&ti);
            iLastHot = iHot = -1;
            WndProc_LeftBK(hWnd, LEFTBKM_TOOLBAR_REDRAW, TRUE, TRUE);
            SendMessageW(hToolTip, TTM_TRACKACTIVATE, FALSE, (LPARAM)&ti);
        }

        if (m_iLrcFixedIndex != -1)
            return 0;
        iLastIndex = m_iLrcMouseHover = -1;
        m_IsDraw[2] = TRUE;
        TimerProc(NULL, 0, IDT_DRAWING_LRC, 0);
    }
    return 0;

    case WM_MOUSEHOVER:
    {
        if (iHot != -1 && iPushed == -1 && iLastOver != iHot)
        {
        ShowToolTip:
            POINT pt = { GET_X_LPARAM(lParam),GET_Y_LPARAM(lParam) };
            ClientToScreen(hWnd, &pt);
            iLastOver = iHot;
            ti.lpszText = NULL;

            if (iHot == 5)
                ti.lpszText = (LPWSTR)c_szBtmTip[BTMBKBTNCOUNT + iRepeatMode];
            else
                ti.lpszText = (LPWSTR)c_szBtmTip[iHot];

            SendMessageW(hToolTip, TTM_SETTOOLINFOW, 0, (LPARAM)&ti);
            SendMessageW(hToolTip, TTM_TRACKACTIVATE, TRUE, (LPARAM)&ti);
            SendMessageW(hToolTip, TTM_TRACKPOSITION, 0, MAKELPARAM(pt.x, pt.y));
        }
    }
    return 0;

    case WM_RBUTTONUP:
    {
        POINT pt = { GET_X_LPARAM(lParam), GET_Y_LPARAM(lParam) };
        if (PtInRect(&m_rcLrcShow, pt))
        {
            int i = HitTest_LrcShow(pt);
            if (i != iLastIndex)
            {
                iLastIndex = m_iLrcMouseHover = i;
                m_IsDraw[2] = TRUE;
                TimerProc(NULL, 0, IDT_DRAWING_LRC, 0);
            }
            UINT uFlags = (i == -1) ? MF_GRAYED : 0;
            HMENU hMenu = CreatePopupMenu();
            AppendMenuW(hMenu, uFlags, IDMI_LS_PLAY, L"从此处播放");
            AppendMenuW(hMenu, uFlags, IDMI_LS_COPY, L"复制歌词");

            ClientToScreen(hWnd, &pt);
            m_iLrcFixedIndex = m_iLrcCenter;
            int iRet = TrackPopupMenu(hMenu, TPM_RETURNCMD, pt.x, pt.y, 0, hWnd, NULL);
            DestroyMenu(hMenu);
            switch (iRet)
            {
            case IDMI_LS_PLAY:// 从此处播放
            {
                BASS_ChannelSetPosition(
                    g_hStream,
                    BASS_ChannelSeconds2Bytes(
                        g_hStream,
                        ((LRCDATA*)QKAGet(g_Lrc, i))->fTime),
                    BASS_POS_BYTE
                );
            }
            break;
            case IDMI_LS_COPY:// 复制歌词
            {
                PWSTR pszLrc = ((LRCDATA*)QKAGet(g_Lrc, i))->pszLrc;
                HGLOBAL hGlobal = GlobalAlloc(GMEM_MOVEABLE, (lstrlenW(pszLrc) + 1) * sizeof(WCHAR));
                void* pGlobal = GlobalLock(hGlobal);
                lstrcpyW((PWSTR)pGlobal, pszLrc);
                GlobalUnlock(hGlobal);
                if (OpenClipboard(hWnd))
                {
                    EmptyClipboard();
                    SetClipboardData(CF_UNICODETEXT, hGlobal);
                    CloseClipboard();
                }
                else
                {
                    GlobalFree(hGlobal);
                    break;
                }
            }
            break;
            }
            m_iLrcFixedIndex = -1;
            m_IsDraw[2] = TRUE;
            TimerProc(NULL, 0, IDT_DRAWING_LRC, 0);
        }
    }
    return 0;

    case WM_KEYDOWN:
    {
        if (wParam == VK_ESCAPE)
        {
            KillTimer(hWnd, IDT_LRCSCROLL);
            m_iLrcSBPos = -1;
            WndProc_LeftBK(hWnd, LEFTBKM_LRCSHOW_REDRAWSB, TRUE, TRUE);
            return 0;
        }
    }
    break;

    case WM_DESTROY:
        DestroyWindow(hToolTip);
        return 0;

    //****************进度条****************//
    case LEFTBKM_PROGBAR_REDRAW:// 重画进度条
    {
        if (lParam)
            m_pD2DDCLeftBK->BeginDraw();
        m_pD2DDCLeftBK->DrawBitmap(m_pD2DBmpLeftBK2, D2DRcTrackBar, 1.0f, D2D1_BITMAP_INTERPOLATION_MODE_LINEAR, &D2DRcTrackBar);
        if (uTBMax)
        {
            D2D_RECT_F D2DRcF;
			D2DRcF.left = D2DRcTrackBar.left;
			D2DRcF.top = D2DRcTrackBar.top + (DPIS_CYPROGBAR - GC.DS_CYPROGBARCORE) / 2;
			D2DRcF.bottom = D2DRcF.top + GC.DS_CYPROGBARCORE;
			float fWidth = D2DRcTrackBar.right - D2DRcTrackBar.left;
			D2DRcF.right = D2DRcF.left + uTBPos * fWidth / uTBMax;
			if (D2DRcF.right - D2DRcF.left > fWidth)
				goto NoPgs;
			m_pD2DDCLeftBK->FillRectangle(&D2DRcF, m_pD2DBrMyBlue);
			D2DRcF.left = D2DRcF.right;
			D2DRcF.right = D2DRcTrackBar.right;
			ID2D1SolidColorBrush* pD2DBrush;
			m_pD2DDCLeftBK->CreateSolidColorBrush(D2D1::ColorF(D2D1::ColorF::Gray, 0.7f), &pD2DBrush);
			m_pD2DDCLeftBK->FillRectangle(&D2DRcF, pD2DBrush);
			pD2DBrush->Release();
		}
		else
        {
        NoPgs:// 进度条尺寸不正确时跳过来
            D2D_RECT_F D2DRcF;
            D2DRcF.left = D2DRcTrackBar.left;
            D2DRcF.top = D2DRcTrackBar.top + (DPIS_CYPROGBAR - GC.DS_CYPROGBARCORE) / 2;
            D2DRcF.bottom = D2DRcF.top + GC.DS_CYPROGBARCORE;
            D2DRcF.right = D2DRcTrackBar.right;

            ID2D1SolidColorBrush* pD2DBrush;
            m_pD2DDCLeftBK->CreateSolidColorBrush(D2D1::ColorF(D2D1::ColorF::Gray, 0.7f), &pD2DBrush);
            m_pD2DDCLeftBK->FillRectangle(&D2DRcF, pD2DBrush);
            pD2DBrush->Release();
        }

        if (lParam)
            m_pD2DDCLeftBK->EndDraw();
        if (wParam)
            m_pDXGIScLeftBK->Present(0, 0);
    }
    return 0;

    case LEFTBKM_PROGBAR_SETPOS:// 置进度条位置
    {
        uTBPos = wParam;
        if (lParam)
            WndProc_LeftBK(hWnd, LEFTBKM_PROGBAR_REDRAW, TRUE, TRUE);
    }
    return 0;

    case LEFTBKM_PROGBAR_GETPOS:// 取进度条位置
        return uTBPos;

    case LEFTBKM_PROGBAR_SETMAX:// 置进度条最大位置
    {
        uTBMax = wParam;
        if (lParam)
            WndProc_LeftBK(hWnd, LEFTBKM_PROGBAR_REDRAW, TRUE, TRUE);
    }
    return 0;

    case LEFTBKM_PROGBAR_GETMAX:// 取进度条最大位置
        return uTBMax;
    //****************滚动歌词****************//
    case LEFTBKM_LRCSHOW_REDRAWSB:// 重画滚动条
    {
        if (iSBMax)
        {
            rcLrcSBThumb.left = rcLrcSB.left;
            rcLrcSBThumb.top = m_rcLrcShow.top + m_iLrcSBPos * (rcLrcSB.bottom - rcLrcSB.top - iThumbSize) / iSBMax;
            rcLrcSBThumb.right = rcLrcSB.right;
            rcLrcSBThumb.bottom = rcLrcSBThumb.top + iThumbSize;

            if (lParam)
                m_pD2DDCLeftBK->BeginDraw();

            D2D_RECT_F D2DRectF1 = { (FLOAT)rcLrcSB.left, (FLOAT)rcLrcSB.top, (FLOAT)rcLrcSB.right, (FLOAT)rcLrcSB.bottom },
                D2DRectF2 = { (FLOAT)rcLrcSBThumb.left, (FLOAT)rcLrcSBThumb.top, (FLOAT)rcLrcSBThumb.right, (FLOAT)rcLrcSBThumb.bottom };
            m_pD2DDCLeftBK->DrawBitmap(m_pD2DBmpLeftBK2, &D2DRectF1, 1.0f, D2D1_BITMAP_INTERPOLATION_MODE_LINEAR, &D2DRectF1);

            if (m_iLrcSBPos != -1)
                m_pD2DDCLeftBK->FillRectangle(&D2DRectF2, m_pD2DBrMyBlue);

            if (lParam)
                m_pD2DDCLeftBK->EndDraw();
            if (wParam)
                m_pDXGIScLeftBK->Present(0, 0);
        }
    }
    return 0;

    case LEFTBKM_LRCSHOW_SETMAX:// 置歌词数量
    {
        iSBMax = wParam;
        WndProc_LeftBK(hWnd, LEFTBKM_LRCSHOW_REDRAWSB, TRUE, TRUE);
    }
    return 0;
    //****************工具栏****************//
    case LEFTBKM_TOOLBAR_INIT:// 初始化
    {
        hToolTip = CreateWindowExW(0, TOOLTIPS_CLASSW, NULL, TTS_NOPREFIX | TTS_ALWAYSTIP, 0, 0, 0, 0, hWnd, NULL, NULL, NULL);// 创建工具提示
        SendMessageW(hToolTip, TTM_ADDTOOLW, 0, (LPARAM)&ti);
    }
    return 0;

    case LEFTBKM_TOOLBAR_GETREPEATMODE:// 取循环模式
        return iRepeatMode;

	case LEFTBKM_TOOLBAR_DOBTOPE:// 执行按钮动作
	{
		switch (wParam)
		{
		case 0:goto BTOpe_Last;
		case 1:goto BTOpe_Play;
		case 2:goto BTOpe_Next;
		}
	}
	return 0;

    case LEFTBKM_TOOLBAR_REDRAW:// 重画工具栏
    {
        HDC hDC;
        if (lParam)
            m_pD2DDCLeftBK->BeginDraw();

        D2D_RECT_F D2DRectF;
        if (m_rcBtmBK.right - m_rcBtmBK.left > m_cxLeftBK)
            D2DRectF = { 0,(float)m_rcBtmBK.top,(float)m_cxLeftBK,(float)m_rcBtmBK.bottom };
        else
            D2DRectF = { (float)m_rcBtmBK.left,(float)m_rcBtmBK.top,(float)m_rcBtmBK.right,(float)m_rcBtmBK.bottom };
        m_pD2DDCLeftBK->DrawBitmap(m_pD2DBmpLeftBK2, &D2DRectF, 1.0f, D2D1_BITMAP_INTERPOLATION_MODE_LINEAR, &D2DRectF);
        ID2D1SolidColorBrush* pD2DBrush;
        m_pD2DDCLeftBK->CreateSolidColorBrush(D2D1::ColorF(D2D1::ColorF::Black), &pD2DBrush);

        g_pDWTFNormal->SetTextAlignment(DWRITE_TEXT_ALIGNMENT_TRAILING);
        g_pDWTFNormal->SetParagraphAlignment(DWRITE_PARAGRAPH_ALIGNMENT_CENTER);
        D2DRcTimeLabel = { (float)m_rcBtmBK.left,(float)m_rcBtmBK.top,(float)(m_rcBtmBK.left + DPIS_CXTIME),(float)m_rcBtmBK.bottom };
        m_pD2DDCLeftBK->DrawTextW(szTime, lstrlenW(szTime), g_pDWTFNormal, &D2DRcTimeLabel, pD2DBrush, D2D1_DRAW_TEXT_OPTIONS_CLIP);

        D2D_RECT_F D2DRectF2;

        int iIconOffest = (GC.cyBT - GC.iIconSize) / 2;
        int x = (int)D2DRcTimeLabel.right, y = m_rcBtmBK.top + iIconOffest;
        RECT rc = { 0,0,0,GC.cyBT };
        if (iHot != -1 || iPushed != -1)
        {
            int i;
            if (iPushed != -1)
            {
                i = iPushed;
                pD2DBrush->SetColor(D2D1::ColorF(QKGDIClrToCommonClr(MYCLR_BTPUSHED)));
            }
            else
            {
                i = iHot;
                pD2DBrush->SetColor(D2D1::ColorF(QKGDIClrToCommonClr(MYCLR_BTHOT)));
            }

            D2DRectF2.left = x + (FLOAT)(GC.cyBT * i);
            D2DRectF2.top = D2DRectF.top;
            D2DRectF2.right = D2DRectF2.left + GC.cyBT;
            D2DRectF2.bottom = D2DRectF.bottom;
            m_pD2DDCLeftBK->FillRectangle(&D2DRectF2, pD2DBrush);
        }
        pD2DBrush->Release();

        m_pD2DGdiInteropRTLeftBK->GetDC(D2D1_DC_INITIALIZE_MODE_COPY, &hDC);
        DrawIconEx(hDC, x + iIconOffest, y, GR.hiLast, 0, 0, 0, NULL, DI_NORMAL);// 1 上一曲
        x += GC.cyBT;
        DrawIconEx(hDC, x + iIconOffest, y, g_bPlayIcon ? GR.hiPlay : GR.hiPause, 0, 0, 0, NULL, DI_NORMAL);// 2 播放/暂停
        x += GC.cyBT;
        DrawIconEx(hDC, x + iIconOffest, y, GR.hiStop, 0, 0, 0, NULL, DI_NORMAL);// 3 停止
        x += GC.cyBT;
        DrawIconEx(hDC, x + iIconOffest, y, GR.hiNext, 0, 0, 0, NULL, DI_NORMAL);// 4 下一曲
        x += GC.cyBT;
        DrawIconEx(hDC, x + iIconOffest, y, GR.hiLrc, 0, 0, 0, NULL, DI_NORMAL);// 5 歌词
        x += GC.cyBT;

        HICON hi;
        switch (iRepeatMode)
        {
        case REPEATMODE_TOTALLOOP:  hi = GR.hiArrowCircle;      break;
        case REPEATMODE_RADOM:      hi = GR.hiArrowCross;       break;
        case REPEATMODE_SINGLE:     hi = GR.hiArrowRight;       break;
        case REPEATMODE_SINGLELOOP: hi = GR.hiArrowCircleOne;   break;
        case REPEATMODE_TOTAL:      hi = GR.hiArrowRightThree;  break;
        default:                    hi = GR.hiArrowCircle;      break;
        }
        DrawIconEx(hDC, x + iIconOffest, y, hi, 0, 0, 0, NULL, DI_NORMAL);// 6 循环方式
        x += GC.cyBT;
        DrawIconEx(hDC, x + iIconOffest, y, GR.hiPlaySetting, 0, 0, 0, NULL, DI_NORMAL);// 7 均衡器
        x += GC.cyBT;
        DrawIconEx(hDC, x + iIconOffest, y, GR.hiPlayList, 0, 0, 0, NULL, DI_NORMAL);// 8 显示播放列表
        x += GC.cyBT;
        DrawIconEx(hDC, x + iIconOffest, y, GR.hiSettings, 0, 0, 0, NULL, DI_NORMAL);// 9 设置
        x += GC.cyBT;
        DrawIconEx(hDC, x + iIconOffest, y, GR.hiInfo, 0, 0, 0, NULL, DI_NORMAL);// 10 关于
        x += GC.cyBT;
        if (wParam)
        {
            HDC hDCWnd = GetDC(g_hBKLeft);
            BitBlt(hDCWnd, m_rcBtmBK.left, m_rcBtmBK.top, GC.cxBKBtm, GC.cyBT, hDC, m_rcBtmBK.left, m_rcBtmBK.top, SRCCOPY);
            ReleaseDC(g_hBKLeft, hDCWnd);
        }

        if (m_rcBtmBK.right - m_rcBtmBK.left > m_cxLeftBK)// 左边太小会导致DC释放失败（矩形超界）
        {
            RECT rc = { 0,m_rcBtmBK.top,m_cxLeftBK,m_rcBtmBK.bottom };
            m_pD2DGdiInteropRTLeftBK->ReleaseDC(&rc);
        }
        else
            m_pD2DGdiInteropRTLeftBK->ReleaseDC(&m_rcBtmBK);

        if (lParam)
            m_pD2DDCLeftBK->EndDraw();
    }
    return 0;
    }
    return DefWindowProcW(hWnd, uMsg, wParam, lParam);
}

INT_PTR CALLBACK DlgProc_License(HWND hDlg, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
    switch (uMsg)
    {
    case WM_INITDIALOG:
	{
		HRSRC hResInfo = FindResourceW(g_hInst, MAKEINTRESOURCEW(IDR_LICENSE), L"BIN");
        if (hResInfo)
        {
            HGLOBAL hRes = LoadResource(g_hInst, hResInfo);
            if (hRes)
            {
                PCCH p = (PCCH)LockResource(hRes);
                DWORD dwLength = SizeofResource(g_hInst, hResInfo);
                int iBufferSize = MultiByteToWideChar(CP_UTF8, 0, p, dwLength, NULL, 0);
                if (!iBufferSize)
                    return NULL;
                PWSTR pBuffer = new WCHAR[iBufferSize + 1];
                MultiByteToWideChar(CP_UTF8, 0, p, dwLength, pBuffer, iBufferSize);
                *(pBuffer + iBufferSize) = L'\0';// 添加结尾NULL
                SetDlgItemTextW(hDlg, IDC_ED_LICENSE, pBuffer);
                delete[] pBuffer;
            }
        }
		RECT rc;
		GetClientRect(hDlg, &rc);
		SetWindowPos(GetDlgItem(hDlg, IDC_ED_LICENSE), NULL, 0, 0, rc.right, rc.bottom, SWP_NOZORDER);
	}
	return TRUE;

	case WM_SIZE:
		SetWindowPos(GetDlgItem(hDlg, IDC_ED_LICENSE), NULL, 0, 0, LOWORD(lParam), HIWORD(lParam), SWP_NOZORDER);
		return TRUE;

    case WM_CLOSE:
        EndDialog(hDlg, 0);
        return TRUE;
	}
	return FALSE;
}

INT_PTR CALLBACK DlgProc_About(HWND hDlg, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
    static HBITMAP hBitmap;
    static HDC hCDC;
    static int cx0, cy0, cx, cy;
    switch (uMsg)
    {
    case WM_INITDIALOG:
    {
        //////////////////////初始化图片
        hBitmap = (HBITMAP)LoadImageW(g_hInst, MAKEINTRESOURCEW(IDB_ABOUT), IMAGE_BITMAP, 0, 0, 0);
        BITMAP bmp;
        GetObjectW(hBitmap, sizeof(bmp), &bmp);
        RECT rc;
        GetClientRect(hDlg, &rc);

        cx0 = bmp.bmWidth;
        cy0 = bmp.bmHeight;
        cx = rc.right;
		cy = cx * cy0 / cx0;// 计算缩放尺寸

		hCDC = CreateCompatibleDC(NULL);
		SelectObject(hCDC, hBitmap);
        //////////////////////初始化对话框内容
        SetDlgItemTextW(hDlg, IDC_ED_MYQQ, L"639582106");// 我的QQ
        SetDlgItemTextW(hDlg, IDC_ED_BASSWEBSITE, L"www.un4seen.com");// Un4seen网址

		HRSRC hResInfo = FindResourceW(g_hInst, MAKEINTRESOURCEW(IDR_COMPILETIME), L"BIN");
        if (hResInfo)
        {
            HGLOBAL hRes = LoadResource(g_hInst, hResInfo);
            if (hRes)
                SetDlgItemTextW(hDlg, IDC_ST_COMPILETIME, (PCWSTR)LockResource(hRes));// 编译时间

            WCHAR szCompileCount[48];
            hResInfo = FindResourceW(g_hInst, MAKEINTRESOURCEW(IDR_COMPILECOUNT), L"BIN");
            hRes = LoadResource(g_hInst, hResInfo);
            if (hRes)
            {
                wsprintfW(szCompileCount, L"%d", *(INT32*)LockResource(hRes));
                SetDlgItemTextW(hDlg, IDC_ST_COMPILECOUNT, szCompileCount);// 累计编译次数
            }
        }

        SetDlgItemTextW(hDlg, IDC_ST_PROGVER, CURRVER);// 当前版本
	}
	return FALSE;

	case WM_CLOSE:
		EndDialog(hDlg, NULL);
		return TRUE;

	case WM_DESTROY:
		DeleteDC(hCDC);
		DeleteObject(hBitmap);
		return TRUE;

	case WM_COMMAND:
	{
        switch (LOWORD(wParam))
        {
        case IDC_BT_OK:
            EndDialog(hDlg, NULL);
            return TRUE;
        case IDC_BT_LICENSE:
            DialogBoxParamW(g_hInst, MAKEINTRESOURCEW(IDD_LICENSE), hDlg, DlgProc_License, 0);
            return TRUE;
        case IDC_BT_GITHUB:
            ShellExecuteW(NULL, L"open", L"https://github.com/QingKong-s/Player/", NULL, NULL, SW_SHOW);
            return TRUE;
        }
    }
    return FALSE;

    case WM_PAINT:
    {
        PAINTSTRUCT ps;
        HDC hDC = BeginPaint(hDlg, &ps);
        SetStretchBltMode(hDC, HALFTONE);// 置拉伸模式
        SetBrushOrgEx(hDC, 0, 0, NULL);
        StretchBlt(hDC, 0, 0, cx, cy, hCDC, 0, 0, cx0, cy0, SRCCOPY);
        EndPaint(hDlg, &ps);
    }
    return TRUE;
    }
    return FALSE;
}

void CALLBACK TimerProc(HWND hWnd, UINT uMsg, UINT_PTR idEvent, DWORD dwTime)
{
	switch (idEvent)
	{
	case IDT_ANIMATION:
    {
		if (m_LrcHScrollInfo.iIndex == -1 || g_iCurrLrcIndex < 0)
            return;
        LRCDATA* p = (LRCDATA*)QKAGet(g_Lrc, g_iCurrLrcIndex);
        float fLastTime = g_fTime - p->fTime;
        static int iLastx1 = 0, iLastx2 = 0;// 上次左边，如果跟上次一样就不要再画了
        int ii;
        BOOL bRedraw = FALSE;
        if (m_LrcHScrollInfo.cx1 != -1)
        {
            if (fLastTime > m_LrcHScrollInfo.fNoScrollingTime1)
            {
                if (fLastTime < p->fDelay - m_LrcHScrollInfo.fNoScrollingTime1)
                {
                    ii = m_cxLrcShow / 2 - (int)(fLastTime * m_LrcHScrollInfo.cx1 / p->fDelay);
                    if (ii != iLastx1)
                    {
                        iLastx1 = m_LrcHScrollInfo.x1 = ii;
                        bRedraw = TRUE;
                    }
                }
                else
                {
                    ii = m_cxLrcShow - m_LrcHScrollInfo.cx1;
                    if (ii != iLastx1)
                    {
                        iLastx1 = m_LrcHScrollInfo.x1 = ii;
                        bRedraw = TRUE;
                    }
                }
            }
            else
            {
                if (iLastx1 != 0)
                {
                    iLastx1 = m_LrcHScrollInfo.x1 = 0;
                    bRedraw = TRUE;
                }
            }
        }

        if (m_LrcHScrollInfo.cx2 != -1)
        {
            if (fLastTime > m_LrcHScrollInfo.fNoScrollingTime2)
            {
                if (fLastTime < p->fDelay - m_LrcHScrollInfo.fNoScrollingTime2)
                {
                    ii = m_cxLrcShow / 2 - (int)(fLastTime * m_LrcHScrollInfo.cx2 / p->fDelay);
                    if (ii != iLastx2)
                    {
                        iLastx2 = m_LrcHScrollInfo.x2 = ii;
                        bRedraw = TRUE;
                    }
                }
                else
                {
                    ii = m_cxLrcShow - m_LrcHScrollInfo.cx2;
                    if (ii != iLastx2)
                    {
                        iLastx2 = m_LrcHScrollInfo.x2 = ii;
                        bRedraw = TRUE;
                    }
                }
            }
            else
            {
                if (iLastx2 != 0)
                {
                    iLastx2 = m_LrcHScrollInfo.x2 = 0;
                    bRedraw = TRUE;
                }
            }
        }

        if (bRedraw)
            UI_DrawLrcItem(g_iCurrLrcIndex, -1, 0, TRUE, TRUE);
    }
    return;
    case IDT_ANIMATION2:
    {
        if (m_iLrcSBPos != -1 || m_iLrcCenter == -1)
        {
            m_IsDraw[2] = TRUE;
            UI_VEProcLrcShowing();
            KillTimer(g_hMainWnd, IDT_ANIMATION2);
            return;
        }
        int iTop;
        static int iLastTop = 0x80000000;
        float fLastTime = g_fTime - m_LrcVScrollInfo.fTime;
        if (m_LrcVScrollInfo.bDirection)
        {
            iTop = m_LrcVScrollInfo.iSrcTop - (int)(fLastTime * m_LrcVScrollInfo.iDistance / m_LrcVScrollInfo.fDelay);
            if (iTop <= m_LrcVScrollInfo.iDestTop)
            {
                m_IsDraw[2] = TRUE;
                UI_VEProcLrcShowing();
                KillTimer(g_hMainWnd, IDT_ANIMATION2);
                return;
            }
            else if (iTop != iLastTop)
            {
                iLastTop = iTop;
            }
            else
                return;
        }
        else
        {
            iTop = m_LrcVScrollInfo.iSrcTop + (int)(fLastTime * m_LrcVScrollInfo.iDistance / m_LrcVScrollInfo.fDelay);
            if (iTop >= m_LrcVScrollInfo.iDestTop)
            {
                m_IsDraw[2] = TRUE;
                UI_VEProcLrcShowing();
                KillTimer(g_hMainWnd, IDT_ANIMATION2);
                return;
            }
            else if (iTop != iLastTop)
            {
                iLastTop = iTop;
            }
            else
                return;
        }

        UI_VEDrawLrc(iTop);
    }
    return;
    }
}

LRESULT CALLBACK QKCProc_SEB(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
    if (uMsg == QKCSEPN_DRAGEND)
    {
        RECT rc;
        GetClientRect(g_hMainWnd, &rc);
        g_cxBKList = rc.right - wParam - DPIS_GAP;
        UI_SetRitCtrlPos();
        SendMessageW(g_hMainWnd, WM_SIZE, 0, MAKELONG(rc.right, rc.bottom));
    }
    return 0;
}

int HitTest_BtmBK(POINT pt)
{
    if (!PtInRect(&m_rcBtmBK,pt))
        return -1;

    int iLeft = m_rcBtmBK.left + DPIS_CXTIME;
	for (int i = 0; i < BTMBKBTNCOUNT; i++)
    {
        if (pt.x > iLeft + i * GC.cyBT && 
            pt.x < iLeft + (i + 1) * GC.cyBT)
            return i;
    }
    return -1;
}

int HitTest_LrcShow(POINT pt)
{
    if (!g_Lrc->iCount || !g_hStream)
        return -1;

    LRCDATA* p = (LRCDATA*)QKAGet(g_Lrc, m_iLrcCenter);
    int i = m_iLrcCenter;
    if (pt.y >= p->rcItem.top)// 落在下半部分（包括中间一项）
    {
        while (p->iDrawID == m_iDrawingID)
        {
            if (PtInRect(&p->rcItem, pt))
                return i;
            ++i;
            if (i >= g_Lrc->iCount)
                break;
            p = (LRCDATA*)QKAGet(g_Lrc, i);
        }
    }
    else// 落在上半部分
    {
        --i;// 中间一项就不判断了，跳过
        if (i < 0)
            return -1;
        p = (LRCDATA*)QKAGet(g_Lrc, i);
        while (p->iDrawID == m_iDrawingID)
        {
            if (PtInRect(&p->rcItem, pt))
                return i;
            --i;
            if (i < 0)
                break;
            p = (LRCDATA*)QKAGet(g_Lrc, i);
        }
    }

    return -1;
}

void SettingsUpd_WndMain()
{
    /////////////////////歌词字体
	if (GS.iVisualMode == 0 || GS.iVisualMode == 1)
	{
		g_bShowAlbum = FALSE;
        m_cyAlbum = DPI(GS.iAlbumPicSize1);
	}
	else if (GS.iVisualMode == 2 || GS.iVisualMode == 3)
	{
		g_bShowAlbum = TRUE;
        m_cyAlbum = DPI(GS.iAlbumPicSize2);
	}

    DWRITE_FONT_WEIGHT DWFontWeight;
    if (GS.iSCLrcFontWeight >= 1 || GS.iSCLrcFontWeight <= 999)
        DWFontWeight = (DWRITE_FONT_WEIGHT)GS.iSCLrcFontWeight;
    else
        DWFontWeight = DWRITE_FONT_WEIGHT_NORMAL;

    SAFE_RELEASE(m_pDWTFLrc);
    g_pDWFactory->CreateTextFormat(GS.pszSCLrcFontName, NULL, DWFontWeight, DWRITE_FONT_STYLE_NORMAL, DWRITE_FONT_STRETCH_NORMAL,
        DPIF(GS.iSCLrcFontSize), L"zh-cn", &m_pDWTFLrc);
    /////////////////////歌词颜色
    SAFE_RELEASE(m_pD2DBrLrc1);
    SAFE_RELEASE(m_pD2DBrLrc2);
    D2D1_COLOR_F D2DColor;
    QKGDIColorToD2DColor(GS.crSCLrc1, &D2DColor);
    m_pD2DDCLeftBK->CreateSolidColorBrush(D2DColor, &m_pD2DBrLrc1);
    QKGDIColorToD2DColor(GS.crSCLrc2, &D2DColor);
    m_pD2DDCLeftBK->CreateSolidColorBrush(D2DColor, &m_pD2DBrLrc2);

	RECT rc;
	GetClientRect(g_hBKLeft, &rc);
	SendMessageW(g_hBKLeft, WM_SIZE, 0, MAKELPARAM(rc.right, rc.bottom));
}