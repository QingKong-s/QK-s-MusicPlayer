/*
*           晴空的自绘控件
*   (C) 2021-2022 晴空 保留所有权利
*/
#include "QKCtrl.h"

#include <Windows.h>
#include <windowsx.h>
#include <Commctrl.h>
#include <UxTheme.h>
#include <vsstyle.h>

#include "MyProject.h"
#include "Function.h"

LRESULT CALLBACK WndProc_QKCTrackBar(HWND, UINT, WPARAM, LPARAM);
LRESULT CALLBACK WndProc_QKCSeparateBar(HWND, UINT, WPARAM, LPARAM);

BOOL QKCtrlInit()
{
    WNDCLASSEXW wcex = { 0 };
    wcex.cbSize = sizeof(WNDCLASSEX);
    wcex.style = CS_HREDRAW | CS_VREDRAW;
    wcex.lpfnWndProc = WndProc_QKCTrackBar;
    wcex.hInstance = GetModuleHandleW(NULL);
    wcex.lpszClassName = QKCCN_TRACKBAR;
    wcex.hCursor = LoadCursorW(NULL, IDC_ARROW);
    wcex.cbWndExtra = sizeof(LPVOID);
    if (!RegisterClassExW(&wcex))
        return FALSE;

    wcex.style = CS_HREDRAW | CS_VREDRAW;
    wcex.lpfnWndProc = WndProc_QKCSeparateBar;
    wcex.lpszClassName = QKCCN_SEPARATEBAR;
    wcex.hCursor = LoadCursorW(NULL, IDC_SIZEWE);
    wcex.cbWndExtra = sizeof(LPVOID);
    if (!RegisterClassExW(&wcex))
        return FALSE;

    wcex.style = CS_HREDRAW | CS_VREDRAW;
    HMODULE hLib = LoadLibraryW(L"User32.dll");
    if (!hLib)
        return FALSE;
    wcex.lpfnWndProc = (WNDPROC)GetProcAddress(hLib, "DefWindowProcW");
    FreeLibrary(hLib);
    wcex.lpszClassName = QKCCN_SEPARATEBAR2;
    wcex.cbWndExtra = 0;
    if (!RegisterClassExW(&wcex))
        return FALSE;

    return TRUE;
}
LRESULT CALLBACK WndProc_QKCTrackBar(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
    QKCTBCONTEXT* pContext;
    pContext = (QKCTBCONTEXT*)GetWindowLongW(hWnd, 0);
    switch (uMsg)
    {
    case WM_CREATE:
        pContext = new QKCTBCONTEXT;
        ZeroMemory(pContext, sizeof(QKCTBCONTEXT));
        SetWindowLongPtrW(hWnd, 0, (LONG_PTR)pContext);
        return 0;
    case WM_PAINT:
    {
        ValidateRect(hWnd, NULL);
        RECT rc, rc2;//rc2：滑块区域
        GetClientRect(hWnd, &rc);
        HDC hDC = GetDC(hWnd);
        if (pContext->byPressed == 1)
            rc2.left = pContext->iDragPos - 5;
        else
            rc2.left = (LONG)((float)pContext->iPos / (float)pContext->iMax * (float)(rc.right - 12) + 6 - 5);
        rc2.top = rc.bottom / 2 - 10;
        rc2.right = rc2.left + 10;
        rc2.bottom = rc2.top + 20;
        pContext->rc = rc2;
        if (pContext->pCallBack)
        {
            QKCTBPAINT Pnt;
            Pnt.hDC = hDC;
            Pnt.rc = &rc2;
            Pnt.rcClient = &rc;
            Pnt.iCtrlID = GetDlgCtrlID(hWnd);
            Pnt.byHot = pContext->byPressed;
            Pnt.byPressed = pContext->byPressed;
            CallWindowProcW(pContext->pCallBack, hWnd, QKCTBN_PAINT, 0, (LPARAM)&Pnt);
        }
        else//简单写了写，配色很烂
        {
            RECT rc3;
            HBRUSH hBrush = CreateSolidBrush(0xFFFFFF);
            FillRect(hDC, &rc, hBrush);
            DeleteObject(hBrush);
            if (pContext->iMax > 0)
            {
                hBrush = CreateSolidBrush(0x7F7F6C);
                rc3.left = 6;
                rc3.top = (rc.bottom - 6) / 2;
                rc3.right = rc.right - 6;
                rc3.bottom = rc3.top + 6;
                FrameRect(hDC, &rc3, hBrush);
                DeleteObject(hBrush);
                HPEN hPen = CreatePen(PS_SOLID, 1, 0xA4A49D);
                COLORREF cr;
                if (pContext->byPressed == 1)
                    cr = 0x555554;
                else if (pContext->byHot)
                    cr = 0x6A6A66;
                else
                    cr = 0xDEDED3;
                hBrush = CreateSolidBrush(cr);
                HGDIOBJ hOld1 = SelectObject(hDC, hPen);
                HGDIOBJ hOld2 = SelectObject(hDC, hBrush);
                Rectangle(hDC, rc2.left, rc2.top, rc2.right, rc2.bottom);
                DeleteObject(SelectObject(hDC, hOld1));
                DeleteObject(SelectObject(hDC, hOld2));
            }
        }
        ReleaseDC(hWnd, hDC);
        return 0;
    }
    case QKCTBM_SETRANGE:
        pContext->iMax = lParam;
        if (wParam)
            InvalidateRect(hWnd, NULL, FALSE);
        return 0;
    case QKCTBM_SETPOS:
        pContext->iPos = lParam;
        if (wParam)
            InvalidateRect(hWnd, NULL, FALSE);
        return 0;
    case QKCTBM_GETPOS:
        return pContext->iPos;
    case QKCTBM_GETRANGE:
        return pContext->iMax;
    case QKCTBM_SETPROC:
        pContext->pCallBack = (QKCPROC)lParam;
        if (lParam)
            InvalidateRect(hWnd, NULL, FALSE);
        return 0;
    case QKCTBM_GETTRACKPOS:
        return pContext->iDragPos;
    case WM_MOUSEMOVE:
    {
        POINT pt;
        pt.x = GET_X_LPARAM(lParam);
        pt.y = GET_Y_LPARAM(lParam);
        if (pContext->byPressed == 1)
        {
            RECT rc;
            GetClientRect(hWnd, &rc);
            pContext->iDragPos = GET_X_LPARAM(lParam);
            if (pContext->iDragPos > rc.right - 6)
                pContext->iDragPos = rc.right - 6;
            if (pContext->iDragPos < 6)
                pContext->iDragPos = 6;
            SendMessageW(GetParent(hWnd), WM_HSCROLL, (WPARAM)MAKELONG(TB_THUMBTRACK, 0), 0);
            InvalidateRect(hWnd, NULL, FALSE);
        }
        else
        {
            if (PtInRect(&(pContext->rc), pt))
            {
                if (pContext->byHot == 0)
                {
                    pContext->byHot = 1;
                    InvalidateRect(hWnd, NULL, FALSE);
                }
            }
            else
            {
                if (pContext->byHot == 1)
                {
                    pContext->byHot = 0;
                    InvalidateRect(hWnd, NULL, FALSE);
                }
            }
        }
        return 0;
    }
    case WM_LBUTTONDOWN:
    {
        POINT pt;
        pt.x = GET_X_LPARAM(lParam);
        pt.y = GET_Y_LPARAM(lParam);
        RECT rc;
        GetClientRect(hWnd, &rc);
        rc.left += 6;
        rc.right -= 6;
        rc.top = rc.bottom / 2 - 8;
        rc.bottom = rc.top + 16;
        if (PtInRect(&(pContext->rc), pt))
        {
            SetCapture(hWnd);
            pContext->byPressed = 1;
            pContext->iDragPos = pt.x;
        }
        else if (PtInRect(&rc, pt))
        {
            SendMessageW(GetParent(hWnd), WM_HSCROLL, (WPARAM)MAKELONG(TB_THUMBTRACK, 0), 0);
            SendMessageW(hWnd, QKCTBM_SETPOS, FALSE, (LPARAM)((float)(pt.x - 6) / (float)(rc.right - 12) * SendMessageW(hWnd, QKCTBM_GETRANGE, 0, 0)));
            SendMessageW(GetParent(hWnd), WM_HSCROLL, (WPARAM)MAKELONG(TB_THUMBPOSITION, 0), 0);
        }
 
        return 0;
    }
    case WM_LBUTTONUP:
        if (pContext->byPressed == 1)
        {
            ReleaseCapture();
            RECT rc;
            GetClientRect(hWnd, &rc);
            POINT pt;
            pt.x = GET_X_LPARAM(lParam);
            pt.y = GET_Y_LPARAM(lParam);
            if (pt.x < 6)
                SendMessageW(hWnd, QKCTBM_SETPOS, FALSE, 0);
            else if (pt.x > rc.right - 6)
                SendMessageW(hWnd, QKCTBM_SETPOS, FALSE, (LPARAM)SendMessageW(hWnd, QKCTBM_GETRANGE, 0, 0));
            else
                SendMessageW(hWnd, QKCTBM_SETPOS, FALSE, (LPARAM)((float)(pt.x - 6) / (float)(rc.right - 12) * SendMessageW(hWnd, QKCTBM_GETRANGE, 0, 0)));
            SendMessageW(GetParent(hWnd), WM_HSCROLL, (WPARAM)MAKELONG(TB_THUMBPOSITION, 0), 0);
            pContext->byPressed = 0;
            InvalidateRect(hWnd, NULL, FALSE);
        }
        return 0;
    case WM_DESTROY:
        delete pContext;
        return 0;
    }
    return DefWindowProcW(hWnd, uMsg, wParam, lParam);
}
LRESULT CALLBACK WndProc_QKCSeparateBar(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
    QKCSEBCONTEXT* pContext;
    pContext = (QKCSEBCONTEXT*)GetWindowLongW(hWnd, 0);
    switch (uMsg)
    {
    case WM_CREATE:
	{
		pContext = new QKCSEBCONTEXT;
		ZeroMemory(pContext, sizeof(QKCSEBCONTEXT));
		SetWindowLongPtrW(hWnd, 0, (LONG_PTR)pContext);// 制控件上下文
		pContext->hChild = CreateWindowExW(WS_EX_LAYERED, QKCCN_SEPARATEBAR2, NULL,
			WS_POPUP | WS_DISABLED, 0, 0, 0, 0, NULL, NULL, GetModuleHandleW(NULL), NULL);
		SetWindowLongPtrW(pContext->hChild, GWLP_HWNDPARENT, (LONG_PTR)hWnd);// 置窗口所有者，保证拖动标记在基窗口上面
		pContext->iPos = -1;
	}
    return 0;
    case WM_SIZE:
    {
        int cx = LOWORD(lParam),
            cy = HIWORD(lParam);
        DeleteDC(pContext->hDC);
        DeleteObject(pContext->hBitmap);
        HDC hDC = GetDC(pContext->hChild);
        pContext->hDC = CreateCompatibleDC(hDC);
        pContext->hBitmap = CreateCompatibleBitmap(hDC, cx, cy);
        ReleaseDC(pContext->hChild, hDC);
        SelectObject(pContext->hDC, pContext->hBitmap);

        GpGraphics* pGdipGraphics;
        GpSolidFill* pGdipBrush;
        GdipCreateFromHDC(pContext->hDC, &pGdipGraphics);
        GdipCreateSolidFill(pContext->cr2, &pGdipBrush);
        GdipFillRectangle(pGdipGraphics, pGdipBrush, 0, 0, (REAL)cx, (REAL)cy);
        GdipDeleteBrush(pGdipBrush);
        GdipDeleteGraphics(pGdipGraphics);
    }
    return 0;
    case WM_PAINT:
    {
        PAINTSTRUCT ps;
        HDC hDC = BeginPaint(hWnd, &ps);
        HBRUSH hBrush = CreateSolidBrush(pContext->cr1);
        FillRect(hDC, &ps.rcPaint, hBrush);
        DeleteObject(hBrush);
        EndPaint(hWnd, &ps);
    }
    return 0;
    case WM_LBUTTONDOWN:
    {
        SetFocus(hWnd);
        SetCapture(hWnd);
        pContext->bLBTDown = TRUE;

		RECT rc;
		GetWindowRect(hWnd, &rc);
        pContext->y = rc.top;
		SIZEL s = { rc.right - rc.left,rc.bottom - rc.top };
		POINT ptDC = { 0 }, ptScr = { rc.left,rc.top };
		BLENDFUNCTION bf = { AC_SRC_OVER,0,255,AC_SRC_ALPHA };

        UpdateLayeredWindow(pContext->hChild, NULL, &ptScr, &s, pContext->hDC, &ptDC, 0, &bf, ULW_ALPHA);
        ShowWindow(pContext->hChild, SW_SHOWNOACTIVATE);

        ScreenToClient(GetParent(hWnd), &ptScr);
        pContext->iPos = ptScr.x;
        pContext->iOffest = GET_X_LPARAM(lParam);
    }
    return 0;
    case WM_LBUTTONUP:
    {
        ReleaseCapture();
        pContext->bLBTDown = FALSE;
        ShowWindow(pContext->hChild, SW_HIDE);
        if (pContext->pProc)
        {
            pContext->pProc(hWnd, QKCSEPN_DRAGEND, pContext->iPos, 0);
        }
        pContext->iPos = -1;
    }
    return 0;
    case WM_MOUSEMOVE:
    {
        if (pContext->bLBTDown)
        {
            POINT pt = { GET_X_LPARAM(lParam),GET_Y_LPARAM(lParam) }, pt1;
            ClientToScreen(hWnd, &pt);
            pt1.x = pt.x - pContext->iOffest;
            pt1.y = pContext->y;
            ScreenToClient(GetParent(hWnd), &pt);
            pContext->iPos = pt.x - pContext->iOffest;
            if (pt.x < pContext->iMin)
            {
				pt.x = pContext->iPos = pContext->iMin;
                ClientToScreen(GetParent(hWnd), &pt);
                pt1.x = pt.x;
            }
            else if (pt.x > pContext->iMax)
            {
                pt.x = pContext->iPos = pContext->iMax;
                ClientToScreen(GetParent(hWnd), &pt);
                pt1.x = pt.x;
            }

			BLENDFUNCTION bf = { AC_SRC_OVER,0,255,AC_SRC_ALPHA };
			UpdateLayeredWindow(pContext->hChild, NULL, &pt1, NULL, NULL, NULL, 0, &bf, ULW_ALPHA);
		}
    }
    return 0;
    case WM_KEYDOWN:
    {
        if (pContext->bLBTDown && wParam == VK_ESCAPE)// ESC键退出拖放是银河系的惯例
        {
			ReleaseCapture();
			pContext->bLBTDown = FALSE;
			ShowWindow(pContext->hChild, SW_HIDE);
		}
	}
	return 0;
	case WM_DESTROY:
		DestroyWindow(pContext->hChild);
		DeleteDC(pContext->hDC);
		DeleteObject(pContext->hBitmap);
		return 0;
	case QKCSEBM_SETCOLOR:
		pContext->cr1 = (COLORREF)wParam;
		if (lParam)
			InvalidateRect(hWnd, NULL, FALSE);
		return 0;
	case QKCSEBM_SETCOLOR2:
        pContext->cr2 = (ARGB)wParam;
        if (lParam)
        {
            RECT rc;
            GetClientRect(hWnd, &rc);
            SendMessageW(hWnd, WM_SIZE, 0, MAKELONG(rc.right, rc.bottom));
        }
        return 0;
    case QKCSEBM_SETPROC:
        pContext->pProc = (QKCPROC)wParam;
        return 0;
    case QKCSEBM_SETRANGE:
        pContext->iMin = wParam;
        pContext->iMax = lParam;
        return 0;
    case QKCSEBM_GETRANGE:
        return MAKELONG(pContext->iMin, pContext->iMax);
    }
    return DefWindowProcW(hWnd, uMsg, wParam, lParam);
}