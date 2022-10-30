/*
* EffectDialog.cpp
* 包含声音效果对话框过程及声音效果相关函数的实现
*/
#include "WndEffect.h"

#include <Windows.h>
#include <CommCtrl.h>

#include <stdio.h>

#include "bass.h"
#include "bass_fx.h"

#include "resource.h"
#include "GlobalVar.h"

void GlobalEffect_ResetToDefault(UINT i)
{
    if (i & EFFECT_CHORUS)
    {
        g_GlobalEffect.Chorus =
        {
            50.0f,
            10.0f,
            25.0f,
            1.1f,
            1,
            16.0f,
            BASS_DX8_PHASE_90
        };
    }

    if (i & EFFECT_COMPRESSOR)
    {
        g_GlobalEffect.Compressor =
        {
            0.0f,
            10.0f,
            200.0f,
            -20.0f,
            3.0f,
            4.0f
        };
    }

    if (i & EFFECT_DISTORTION)
    {
        g_GlobalEffect.Distortion =
        {
            -18.0f,
            15.0f,
            2400.0f,
            2400.0f,
            8000.0f
        };
    }

    if (i & EFFECT_ECHO)
    {
        g_GlobalEffect.Echo =
        {
            50.0f,
            50.0f,
            500.0f,
            500.0f,
            FALSE
        };
    }

    if (i & EFFECT_FLANGER)
    {
        g_GlobalEffect.Flanger =
        {
            50.0f,
            100.0f,
            -50.0f,
            0.25f,
            1,
            2.0f,
            BASS_DX8_PHASE_ZERO
        };
    }

    if (i & EFFECT_GARGLE)
    {
        g_GlobalEffect.Gargle =
        {
            20,
            0
        };
    }

    if (i & EFFECT_I3DL2REVERB)
    {
        g_GlobalEffect.I3DL2Reverb =
        {
            -1000,
            -100,
            0.0f,
            1.49f,
            0.83f,
            -2602,
            0.007f,
            200,
            0.011f,
            100.0f,
            100.0f,
            5000.0f
        };
    }

    if (i & EFFECT_REVERB)
    {
        g_GlobalEffect.Reverb =
        {
            0.0f,
            0.0f,
            1000.0f,
            0.001f
        };
    }

    if (i & EFFECT_EQ)
    {
        for (int i = 0; i < 10; ++i)
        {
            g_GlobalEffect.EQ[i].fCenter = c_EQCenter[i];
            g_GlobalEffect.EQ[i].fBandwidth = 12;
            g_GlobalEffect.EQ[i].fGain = 0;
        }
    }

    if (i & EFFECT_ROTATE)
    {
        g_GlobalEffect.Rotate.lChannel = -1;
        g_GlobalEffect.Rotate.fRate = 0;
    }
}
void GlobalEffect_SetPriority(int* i)
{
    //g_GlobalEffect.iPriorityChorus = i[0];
    //g_GlobalEffect.iPriorityCompressor = i[1];
    //g_GlobalEffect.iPriorityDistortion = i[2];
    //g_GlobalEffect.iPriorityEcho = i[3];
    //g_GlobalEffect.iPriorityFlanger = i[4];
    //g_GlobalEffect.iPriorityGargle = i[5];
    //g_GlobalEffect.iPriorityI3DL2Reverb = i[6];
    //g_GlobalEffect.iPriorityReverb = i[7];
    //g_GlobalEffect.iPriorityRotate = i[8];
}

INT_PTR CALLBACK DlgProc_EQ(HWND hDlg, UINT message, WPARAM wParam, LPARAM lParam)
{
    static HWND hTB[10];
    static BOOL bApply = FALSE;
    static int iCBSel = 0;
    switch (message)
    {
    case WM_INITDIALOG:
    {
        SendDlgItemMessageW(hDlg, IDC_CB_ENABLE, BM_SETCHECK, bApply ? BST_CHECKED : BST_UNCHECKED, 0);

		while (SendDlgItemMessageW(hDlg, IDC_CB_EQ, CB_DELETESTRING, 0, 0) != CB_ERR);// 清空

        for (int i = 0; i < 23; i++)
        {
            SendDlgItemMessageW(hDlg, IDC_CB_EQ, CB_INSERTSTRING, -1, (LPARAM)c_EQSetting[i].pszText);
        }
        SendDlgItemMessageW(hDlg, IDC_CB_EQ, CB_SETCURSEL, iCBSel, 0);

        SendDlgItemMessageW(hDlg, IDC_TB_BANDWIDTH, TBM_SETRANGE, FALSE, MAKELPARAM(10, 360));
        SendDlgItemMessageW(hDlg, IDC_TB_BANDWIDTH, TBM_SETPOS, TRUE, (LPARAM)(g_GlobalEffect.EQ[0].fBandwidth * 10));// 缩小10倍
        SendDlgItemMessageW(hDlg, IDC_TB_BANDWIDTH, TBM_SETPAGESIZE, 0, 10);

        hTB[0] = GetDlgItem(hDlg, IDC_TB1);
        hTB[1] = GetDlgItem(hDlg, IDC_TB2);
        hTB[2] = GetDlgItem(hDlg, IDC_TB3);
        hTB[3] = GetDlgItem(hDlg, IDC_TB4);
        hTB[4] = GetDlgItem(hDlg, IDC_TB5);
        hTB[5] = GetDlgItem(hDlg, IDC_TB6);
        hTB[6] = GetDlgItem(hDlg, IDC_TB7);
        hTB[7] = GetDlgItem(hDlg, IDC_TB8);
        hTB[8] = GetDlgItem(hDlg, IDC_TB9);
        hTB[9] = GetDlgItem(hDlg, IDC_TB10);
        for (int i = 0; i < 10; i++)
        {
            SendMessageW(hTB[i], TBM_SETRANGE, FALSE, MAKELPARAM(0, 300));// 减去150再缩小10倍
            SendMessageW(hTB[i], TBM_SETPOS, TRUE, (LPARAM)(g_GlobalEffect.EQ[i].fGain * 10) + 150);
            SendMessageW(hTB[i], TBM_SETPAGESIZE, 0, 10);
        }
    }
    return FALSE;
    case WM_COMMAND:
    {
        switch (HIWORD(wParam))
        {
        case CBN_SELCHANGE:
        {
            iCBSel = SendMessageW((HWND)lParam, CB_GETCURSEL, 0, 0);
            for (int i = 0; i < 10; i++)
            {
                g_GlobalEffect.EQ[i].fGain = c_EQSetting[iCBSel].Setting[i];
                SendMessageW(hTB[i], TBM_SETPOS, TRUE, (LPARAM)(g_GlobalEffect.EQ[i].fGain * 10 + 150));
            }
            goto SetEQ;
        }
        return TRUE;
        case BN_CLICKED:
        {
            if (LOWORD(wParam) == IDC_CB_ENABLE)//启用
            {
                bApply = (SendMessageW((HWND)lParam, BM_GETCHECK, 0, 0) == BST_CHECKED);
                if (!g_hStream)
                    return TRUE;
                if (bApply)
                {
                    for (int i = 0; i < 10; ++i)
                    {
                        if (g_GlobalEffect.hFXEQ[i])
                            BASS_ChannelRemoveFX(g_hStream, g_GlobalEffect.hFXEQ[i]);
                        g_GlobalEffect.hFXEQ[i] = BASS_ChannelSetFX(g_hStream, BASS_FX_DX8_PARAMEQ, 1);
                    }

                    for (int i = 0; i < 10; i++)
                    {
                        g_GlobalEffect.EQ[i].fGain = (float)SendMessageW(hTB[i], TBM_GETPOS, 0, 0) / 10.0f - 15.0f;
                        g_GlobalEffect.EQ[i].fBandwidth = (float)SendDlgItemMessageW(hDlg, IDC_TB_BANDWIDTH, TBM_GETPOS, 0, 0) / 10.0f;
                    }

                    goto SetEQ;
                }
                else
                {
                    for (int i = 0; i < 10; ++i)
                    {
                        if (g_GlobalEffect.hFXEQ[i])
                            BASS_ChannelRemoveFX(g_hStream, g_GlobalEffect.hFXEQ[i]);
                        g_GlobalEffect.hFXEQ[i] = NULL;
                    }
                }
            }
            else if (LOWORD(wParam) == IDC_BT_RESET)//重置
            {
                GlobalEffect_ResetToDefault(EFFECT_EQ);
                DlgProc_EQ(hDlg, WM_INITDIALOG, 0, 0);
                goto SetEQ;
            }
        }
        return TRUE;
        }
    }
    return FALSE;
    case WM_VSCROLL:
    {
        for (int i = 0; i < 10; i++)
        {
            if (hTB[i] == (HWND)lParam)
            {
                g_GlobalEffect.EQ[i].fGain = (float)SendMessageW((HWND)lParam, TBM_GETPOS, 0, 0) / 10.0f - 15.0f;
                break;
            }

            g_GlobalEffect.EQ[i].fBandwidth = (float)SendDlgItemMessageW(hDlg, IDC_TB_BANDWIDTH, TBM_GETPOS, 0, 0) / 10.0f;
        }

    SetEQ:
        if (!bApply && g_hStream)
            return TRUE;
        for (int i = 0; i < 10; i++)
        {
            BASS_FXSetParameters(g_GlobalEffect.hFXEQ[i], &g_GlobalEffect.EQ[i]);
        }
    }
    return TRUE;
    }
    return FALSE;
}
INT_PTR CALLBACK DlgProc_SBV(HWND hDlg, UINT message, WPARAM wParam, LPARAM lParam)
{
    static HWND hTBSpeed,
        hTBBlance,
        hTBVol,
        hTBTempo,
        hSTSpeed,
        hSTBlance,
        hSTVol,
        hSTVU,
        hSTTempo;

    static WCHAR szValue[10];
    static HDC hCDC;
    static HBITMAP hBitmap;
    static int cxVU, cyVU;
    static DRAWING_TIME DrawingTime[2] = { 0 };
    switch (message)
    {
    case WM_INITDIALOG:
    {
        hTBSpeed = GetDlgItem(hDlg, IDC_TB_SPEED);
        hTBBlance = GetDlgItem(hDlg, IDC_TB_BLANCE);
        hTBVol = GetDlgItem(hDlg, IDC_TB_VOL);
        hTBTempo = GetDlgItem(hDlg, IDC_TB_TEMPO);

        hSTSpeed = GetDlgItem(hDlg, IDC_ST_SPEED2);
        hSTBlance = GetDlgItem(hDlg, IDC_ST_BLANCE2);
        hSTVol = GetDlgItem(hDlg, IDC_ST_VOL2);
        hSTVU = GetDlgItem(hDlg, IDC_ST_VU);
        hSTTempo = GetDlgItem(hDlg, IDC_ST_TEMPO2);

        float f;
        HWND hST;
        hST = GetDlgItem(hDlg, IDC_ST_SPEED);
        SetWindowLongPtrW(hST, GWL_STYLE, GetWindowLongPtrW(hST, GWL_STYLE) | SS_REALSIZEIMAGE | SS_ICON);
        SendMessageW(hST, STM_SETICON, (WPARAM)GR.hiSpeed, 0);

        hST = GetDlgItem(hDlg, IDC_ST_TEMPO);
        SetWindowLongPtrW(hST, GWL_STYLE, GetWindowLongPtrW(hST, GWL_STYLE) | SS_REALSIZEIMAGE | SS_ICON);
        SendMessageW(hST, STM_SETICON, (WPARAM)GR.hiTempo, 0);

        hST = GetDlgItem(hDlg, IDC_ST_BLANCE);
        SetWindowLongPtrW(hST, GWL_STYLE, GetWindowLongPtrW(hST, GWL_STYLE) | SS_REALSIZEIMAGE | SS_ICON);
        SendMessageW(hST, STM_SETICON, (WPARAM)GR.hiBlance, 0);
        ////////////////////////////////速度
        SendMessageW(hTBSpeed, TBM_SETRANGEMAX, TRUE, 500);
        SendMessageW(hTBSpeed, TBM_SETPAGESIZE, 0, 1);
        if (g_fDefSpeed != 0)
        {
            f = 0;
            BASS_ChannelGetAttribute(g_hStream, BASS_ATTRIB_FREQ, &f);
            f = f / g_fDefSpeed;
            SendMessageW(hTBSpeed, TBM_SETPOS, TRUE, (LPARAM)(f * 100));
            swprintf(szValue, 10, L"x%.2f", f);
            SetWindowTextW(hSTSpeed, szValue);
        }
        else
            SetWindowTextW(hSTSpeed, L"x0.00");
        ////////////////////////////////平衡
        f = 0;
        BASS_ChannelGetAttribute(g_hStream, BASS_ATTRIB_PAN, &f);
        SendMessageW(hTBBlance, TBM_SETRANGEMAX, TRUE, 100);
        SendMessageW(hTBBlance, TBM_SETRANGEMIN, TRUE, -100);
        SendMessageW(hTBBlance, TBM_SETPAGESIZE, 0, 1);
        f = f * 100;
        SendMessageW(hTBBlance, TBM_SETPOS, TRUE, (LPARAM)f);
        wsprintfW(szValue, L"%2d", (int)f);
        SetWindowTextW(hSTBlance, szValue);
        ////////////////////////////////音量
        f = 0;
        BASS_ChannelGetAttribute(g_hStream, BASS_ATTRIB_VOL, &f);
        SendMessageW(hTBVol, TBM_SETRANGEMAX, FALSE, 200);
        SendMessageW(hTBVol, TBM_SETPAGESIZE, 0, 1);
        if (g_bSlient)
        {
            SendDlgItemMessageW(hDlg, IDC_BT_VOL, BM_SETIMAGE, IMAGE_ICON, (LPARAM)GR.hiSlient);
            EnableWindow(hTBVol, FALSE);
            f = g_fVolChanged;
        }
        else
        {
            SendDlgItemMessageW(hDlg, IDC_BT_VOL, BM_SETIMAGE, IMAGE_ICON, (LPARAM)GR.hiVol);
            f = f * 100;
        }
        SendMessageW(hTBVol, TBM_SETPOS, TRUE, (LPARAM)f);
        wsprintfW(szValue, L"%3d", (int)f);
        SetWindowTextW(hSTVol, szValue);
        ////////////////////////////////节拍(-95~5000)
        f = 0;
        BASS_ChannelGetAttribute(g_hStream, BASS_ATTRIB_TEMPO, &f);
        SendMessageW(hTBTempo, TBM_SETRANGEMAX, TRUE, 400);
        SendMessageW(hTBTempo, TBM_SETRANGEMIN, TRUE, -95);
        SendMessageW(hTBTempo, TBM_SETPAGESIZE, 0, 1);
        SendMessageW(hTBTempo, TBM_SETPOS, TRUE, (LPARAM)(f * 100.0f));
		swprintf(szValue, 10, L"%.0f%%", f * 100.0f);
        SetWindowTextW(hSTTempo, szValue);

        RECT rc;
        GetClientRect(hSTVU, &rc);
        cxVU = rc.right;
        cyVU = rc.bottom;

        HDC hDC = GetDC(hDlg);
        hCDC = CreateCompatibleDC(NULL);
        hBitmap = CreateCompatibleBitmap(hDC, cxVU, DPIS_CYSPE);
        SelectObject(hCDC, hBitmap);
		ReleaseDC(hDlg, hDC);

        SetTimer(hDlg, IDT_DRAWING_VU, TIMERELAPSE_VU_SPE, NULL);
    }
    return FALSE;
    case WM_COMMAND:
	{
		switch (LOWORD(wParam))
		{
		case IDC_BT_VOL:
		{
			g_bSlient = !g_bSlient;
			if (g_bSlient)
			{
                SendMessageW((HWND)lParam, BM_SETIMAGE, IMAGE_ICON, (LPARAM)GR.hiSlient);
				BASS_ChannelGetAttribute(g_hStream, BASS_ATTRIB_VOL, &g_fVolChanged);
				BASS_ChannelSetAttribute(g_hStream, BASS_ATTRIB_VOL, 0);
				EnableWindow(hTBVol, FALSE);
			}
			else
			{
                SendMessageW((HWND)lParam, BM_SETIMAGE, IMAGE_ICON, (LPARAM)GR.hiVol);
				BASS_ChannelSetAttribute(g_hStream, BASS_ATTRIB_VOL, g_fVolChanged);
				EnableWindow(hTBVol, TRUE);
			}
		}
		return TRUE;
        case IDC_BT_RESET:
        {
            SendMessageW(hTBSpeed, TBM_SETPOS, TRUE, 100);
            SendMessageW(hTBBlance, TBM_SETPOS, TRUE, 0);
            SendMessageW(hTBVol, TBM_SETPOS, TRUE, 100);
            SendMessageW(hTBTempo, TBM_SETPOS, TRUE, 0);
            DlgProc_SBV(hDlg, WM_HSCROLL, 0, (LPARAM)hTBSpeed);
            DlgProc_SBV(hDlg, WM_HSCROLL, 0, (LPARAM)hTBBlance);
            DlgProc_SBV(hDlg, WM_HSCROLL, 0, (LPARAM)hTBVol);
            DlgProc_SBV(hDlg, WM_HSCROLL, 0, (LPARAM)hTBTempo);
        }
        return TRUE;
		}
    }
    return FALSE;
    case WM_TIMER:
    {
        switch (wParam)
        {
        case IDT_DRAWING_VU:
        {
            static RECT rc = { 0,0,cxVU,cyVU };
            HDC hDC = GetDC(hSTVU);
            QKGradientFill(
                hCDC,
                &rc,
                RGB(0, (int)(255.0f * 0.7f), 0),
                RGB((int)(255.0f * 0.7f), 0, 0)
            );//画暗色背景

            if (g_hStream)
            {
                DrawingTime[0].uTime += TIMERELAPSE_VU_SPE;
                if (DrawingTime[0].uTime >= 500)
                {
                    DrawingTime[0].bbool = TRUE;
                    DrawingTime[0].uTime = 0;
                }
                DrawingTime[1].uTime += TIMERELAPSE_VU_SPE;
                if (DrawingTime[1].uTime >= 500)
                {
                    DrawingTime[1].bbool = TRUE;
                    DrawingTime[1].uTime = 0;
                }

                //仍然是右上左下
                static int cxRightOld, cxLeftOld;
                int cxRight, cxLeft;
                HRGN hRgn1, hRgn2, hRgn3;

                int iStep = 10, iStep2 = 15;
                int dwLevel = BASS_ChannelGetLevel(g_hStream);//取电平
                cxRight = (int)((float)cxVU * (float)HIWORD(dwLevel) / 32768.0f);//右声道
                if (cxRight > cxRightOld)
                    cxRightOld = cxRight;
                else
                    cxRightOld -= iStep;
                //------------
                if (cxRightOld < 3)
                    cxRightOld = 3;
                //------------
                if (DrawingTime[0].bbool)
                    DrawingTime[0].i -= iStep2;
                //------------
                if (cxRightOld > DrawingTime[0].i)
                {
                    DrawingTime[0].i = cxRightOld;
                    DrawingTime[0].bbool = FALSE;
                    DrawingTime[0].uTime = 0;
                }
                //------------
                if (DrawingTime[0].i < 3)
                {
                    DrawingTime[0].i = 3;
                }
                hRgn1 = CreateRectRgn(0, 0, cxRightOld, cyVU / 2);//上区域
                hRgn2 = CreateRectRgn(DrawingTime[0].i - 3, 0, DrawingTime[0].i, cyVU / 2);//峰值
                CombineRgn(hRgn1, hRgn1, hRgn2, RGN_OR);//合并上区域和上峰值
                DeleteObject(hRgn2);

                cxLeft = (int)((float)cxVU * (float)LOWORD(dwLevel) / 32768.0f);//左声道
                if (cxLeft > cxLeftOld)
                    cxLeftOld = cxLeft;
                else
                    cxLeftOld -= iStep;
                //------------
                if (cxLeftOld < 3)
                    cxLeftOld = 3;
                //------------
                if (DrawingTime[1].bbool)
                    DrawingTime[1].i -= iStep2;
                //------------
                if (cxLeftOld > DrawingTime[1].i)
                {
                    DrawingTime[1].i = cxLeftOld;
                    DrawingTime[1].bbool = FALSE;
                    DrawingTime[1].uTime = 0;
                }
                //------------
                if (DrawingTime[1].i < 3)
                {
                    DrawingTime[1].i = 3;
                }
                hRgn2 = CreateRectRgn(0, cyVU / 2, cxLeftOld, cyVU);//下区域
                hRgn3 = CreateRectRgn(DrawingTime[1].i - 3, cyVU / 2, DrawingTime[1].i, cyVU);//峰值
                CombineRgn(hRgn2, hRgn2, hRgn3, RGN_OR);//下区域和下峰值
                DeleteObject(hRgn3);
                CombineRgn(hRgn1, hRgn1, hRgn2, RGN_OR);//合并上下
                DeleteObject(hRgn2);
                SelectClipRgn(hCDC, hRgn1);//设置剪辑区
                DeleteObject(hRgn1);//删除，GDI会为剪辑区保存一个备份
                QKGradientFill(hCDC, &rc, QKCOLOR_GREEN, QKCOLOR_RED);//再次填充
                SelectClipRgn(hCDC, NULL);//清除剪辑区
            }
            BitBlt(hDC, 0, 0, cxVU, cyVU, hCDC, 0, 0, SRCCOPY);//显示
            ReleaseDC(hSTVU, hDC);
        }
        }
    }
    return TRUE;
    case WM_DESTROY:
        DeleteDC(hCDC);
        DeleteObject(hBitmap);
        KillTimer(hDlg, IDT_VU_SPE_TIME);
        KillTimer(hDlg, IDT_DRAWING_VU);
        return TRUE;
    case WM_HSCROLL:
    {
        DWORD dwPos = SendMessageW((HWND)lParam, TBM_GETPOS, 0, 0);

        if ((HWND)lParam == hTBSpeed)
        {
            g_fSpeedChanged = (float)dwPos / 100.0f;
            if (g_hStream)
                BASS_ChannelSetAttribute(g_hStream, BASS_ATTRIB_FREQ, g_fSpeedChanged * g_fDefSpeed);
            swprintf(szValue, 6, L"x%.2f", (float)dwPos / 100.0f);
            SetWindowTextW(hSTSpeed, szValue);
            return 0;
        }
        else if ((HWND)lParam == hTBBlance)
        {
            g_fBlanceChanged = (float)((int)dwPos) / 100.0f;
            if (g_hStream)
                BASS_ChannelSetAttribute(g_hStream, BASS_ATTRIB_PAN, g_fBlanceChanged);
            wsprintfW(szValue, L"%2d", dwPos - 100);
            SetWindowTextW(hSTBlance, szValue);
            return 0;
        }
        else if ((HWND)lParam == hTBVol)
        {
            g_fVolChanged = (float)dwPos / 100.0f;
            if (g_hStream)
                BASS_ChannelSetAttribute(g_hStream, BASS_ATTRIB_VOL, g_fVolChanged);
            wsprintfW(szValue, L"%3d", dwPos);
            SetWindowTextW(hSTVol, szValue);
            return 0;
        }
        else if ((HWND)lParam == hTBTempo)
        {
            g_GlobalEffect.fTempo = (float)((int)dwPos);
            if (g_hStream)
                BASS_ChannelSetAttribute(g_hStream, BASS_ATTRIB_TEMPO, g_GlobalEffect.fTempo);
            swprintf(szValue, 10, L"%.0f%%", g_GlobalEffect.fTempo);
            SetWindowTextW(hSTTempo, szValue);
        }
    }
    return TRUE;
    }
    return FALSE;
}
INT_PTR CALLBACK DlgProc_Chorus(HWND hDlg, UINT message, WPARAM wParam, LPARAM lParam)
{
    static BOOL bApply = FALSE;
    switch (message)
    {
    case WM_INITDIALOG:
    {
        SendDlgItemMessageW(hDlg, IDC_CB_ENABLE, BM_SETCHECK, bApply ? BST_CHECKED : BST_UNCHECKED, 0);

        SendDlgItemMessageW(hDlg, IDC_TB_WETDRYMIX, TBM_SETRANGE, FALSE, MAKELPARAM(0, 100));
        SendDlgItemMessageW(hDlg, IDC_TB_WETDRYMIX, TBM_SETPOS, TRUE, (LPARAM)g_GlobalEffect.Chorus.fWetDryMix);

        SendDlgItemMessageW(hDlg, IDC_TB_LFODEPTH, TBM_SETRANGE, FALSE, MAKELPARAM(0, 100));
        SendDlgItemMessageW(hDlg, IDC_TB_LFODEPTH, TBM_SETPOS, TRUE, (LPARAM)g_GlobalEffect.Chorus.fDepth);

        SendDlgItemMessageW(hDlg, IDC_TB_FEEDBACK, TBM_SETRANGE, FALSE, MAKELPARAM(0, 99 * 2));
        SendDlgItemMessageW(hDlg, IDC_TB_FEEDBACK, TBM_SETPOS, TRUE, (LPARAM)(g_GlobalEffect.Chorus.fFeedback + 99));// 减去99

        SendDlgItemMessageW(hDlg, IDC_TB_LFOFREQUENCY, TBM_SETRANGE, FALSE, MAKELPARAM(0, 100));
        SendDlgItemMessageW(hDlg, IDC_TB_LFOFREQUENCY, TBM_SETPOS, TRUE, (LPARAM)(g_GlobalEffect.Chorus.fFrequency * 10));// 缩小十倍

        SendDlgItemMessageW(hDlg, IDC_RB_SINEWAVE, BM_SETCHECK, BST_UNCHECKED, 0);
        SendDlgItemMessageW(hDlg, IDC_RB_TRIANGLEWAVE, BM_SETCHECK, BST_UNCHECKED, 0);
        SendDlgItemMessageW(hDlg, g_GlobalEffect.Chorus.lWaveform == 1 ? IDC_RB_SINEWAVE : IDC_RB_TRIANGLEWAVE, BM_SETCHECK, BST_CHECKED, 0);

        SendDlgItemMessageW(hDlg, IDC_TB_DELAY, TBM_SETRANGE, FALSE, MAKELPARAM(0, 40));
        SendDlgItemMessageW(hDlg, IDC_TB_DELAY, TBM_SETPOS, TRUE, (LPARAM)g_GlobalEffect.Chorus.fDelay);

        SendDlgItemMessageW(hDlg, IDC_CB_LFOPHASE, CB_INSERTSTRING, -1, (LPARAM)L"-180");
        SendDlgItemMessageW(hDlg, IDC_CB_LFOPHASE, CB_INSERTSTRING, -1, (LPARAM)L"-90");
        SendDlgItemMessageW(hDlg, IDC_CB_LFOPHASE, CB_INSERTSTRING, -1, (LPARAM)L"0");
        SendDlgItemMessageW(hDlg, IDC_CB_LFOPHASE, CB_INSERTSTRING, -1, (LPARAM)L"90");
        SendDlgItemMessageW(hDlg, IDC_CB_LFOPHASE, CB_INSERTSTRING, -1, (LPARAM)L"180");

        int iIndex = BASS_DX8_PHASE_90;
        switch (g_GlobalEffect.Chorus.lPhase)
        {
        case BASS_DX8_PHASE_NEG_180:iIndex = 0; break;
        case BASS_DX8_PHASE_NEG_90:iIndex = 1; break;
        case BASS_DX8_PHASE_ZERO:iIndex = 2; break;
        case BASS_DX8_PHASE_90:iIndex = 3; break;
        case BASS_DX8_PHASE_180:iIndex = 4; break;
        }

        SendDlgItemMessageW(hDlg, IDC_CB_LFOPHASE, CB_SETCURSEL, iIndex, 0);
    }
    return FALSE;
    case WM_COMMAND:
    {
        switch (HIWORD(wParam))
        {
        case CBN_SELCHANGE:
            goto SetChorus;
            return TRUE;
        case BN_CLICKED:
        {
            switch (LOWORD(wParam))
            {
            case IDC_CB_ENABLE:
                bApply = (SendMessageW((HWND)lParam, BM_GETCHECK, 0, 0) == BST_CHECKED);
                if (!g_hStream)
                    return TRUE;
                if (bApply)
                {
                    if (g_GlobalEffect.hFXChorus)
                        BASS_ChannelRemoveFX(g_hStream, g_GlobalEffect.hFXChorus);
                    g_GlobalEffect.hFXChorus = BASS_ChannelSetFX(g_hStream, BASS_FX_DX8_CHORUS, 1);
                    goto SetChorus;
                }
                else
                {
                    if (g_GlobalEffect.hFXChorus)
                        BASS_ChannelRemoveFX(g_hStream, g_GlobalEffect.hFXChorus);
                    g_GlobalEffect.hFXChorus = NULL;
                }

                return TRUE;
            case IDC_BT_RESET:
                GlobalEffect_ResetToDefault(EFFECT_CHORUS);
                DlgProc_Chorus(hDlg, WM_INITDIALOG, 0, 0);
                goto SetChorus;
                return TRUE;
            case IDC_RB_SINEWAVE:
            case IDC_RB_TRIANGLEWAVE:
                goto SetChorus;
                return TRUE;
            }
        }
        return TRUE;
        }
    }
    return FALSE;
    case WM_HSCROLL:
    {
    SetChorus:
        DWORD dwPhase = BASS_DX8_PHASE_90;
        switch (SendDlgItemMessageW(hDlg, IDC_CB_LFOPHASE, CB_GETCURSEL, 0, 0))
        {
        case 0:dwPhase = BASS_DX8_PHASE_NEG_180; break;
        case 1:dwPhase = BASS_DX8_PHASE_NEG_90; break;
        case 2:dwPhase = BASS_DX8_PHASE_ZERO; break;
        case 3:dwPhase = BASS_DX8_PHASE_90; break;
        case 4:dwPhase = BASS_DX8_PHASE_180; break;
        }

        g_GlobalEffect.Chorus =
        {
            (float)SendDlgItemMessageW(hDlg, IDC_TB_WETDRYMIX, TBM_GETPOS, 0, 0),
            (float)SendDlgItemMessageW(hDlg, IDC_TB_LFODEPTH, TBM_GETPOS, 0, 0),
            (float)SendDlgItemMessageW(hDlg, IDC_TB_FEEDBACK, TBM_GETPOS, 0, 0) - 99,
            (float)SendDlgItemMessageW(hDlg, IDC_TB_LFOFREQUENCY, TBM_GETPOS, 0, 0) / 10,
            (SendDlgItemMessageW(hDlg, IDC_RB_SINEWAVE, BM_GETCHECK, 0, 0) == BST_CHECKED ? 1ul : 0ul),
            (float)SendDlgItemMessageW(hDlg, IDC_TB_DELAY, TBM_GETPOS, 0, 0),
            dwPhase
        };
        if (!bApply || !g_hStream)
            return TRUE;
        BASS_FXSetParameters(g_GlobalEffect.hFXChorus, &g_GlobalEffect.Chorus);
    }
    return TRUE;
    }
    return FALSE;
}
INT_PTR CALLBACK DlgProc_Compressor(HWND hDlg, UINT message, WPARAM wParam, LPARAM lParam)
{
    static BOOL bApply = FALSE;
    switch (message)
    {
    case WM_INITDIALOG:
    {
        SendDlgItemMessageW(hDlg, IDC_CB_ENABLE, BM_SETCHECK, bApply ? BST_CHECKED : BST_UNCHECKED, 0);

        SendDlgItemMessageW(hDlg, IDC_TB_GAIN, TBM_SETRANGE, FALSE, MAKELPARAM(0, 60 * 2));
        SendDlgItemMessageW(hDlg, IDC_TB_GAIN, TBM_SETPOS, TRUE, (LPARAM)g_GlobalEffect.Compressor.fGain);

        SendDlgItemMessageW(hDlg, IDC_TB_ATTACK, TBM_SETRANGEMIN, FALSE, 1);
        SendDlgItemMessageW(hDlg, IDC_TB_ATTACK, TBM_SETRANGEMAX, FALSE, 50000);
        SendDlgItemMessageW(hDlg, IDC_TB_ATTACK, TBM_SETPOS, TRUE, (LPARAM)(g_GlobalEffect.Compressor.fAttack * 100));// 缩小100倍

        SendDlgItemMessageW(hDlg, IDC_TB_RELEASE, TBM_SETRANGE, FALSE, MAKELPARAM(50, 3000));
        SendDlgItemMessageW(hDlg, IDC_TB_RELEASE, TBM_SETPOS, TRUE, (LPARAM)g_GlobalEffect.Compressor.fRelease);

        SendDlgItemMessageW(hDlg, IDC_TB_THRESHOLD, TBM_SETRANGE, FALSE, MAKELPARAM(0, 60));
        SendDlgItemMessageW(hDlg, IDC_TB_THRESHOLD, TBM_SETPOS, TRUE, (LPARAM)(g_GlobalEffect.Compressor.fThreshold + 60));// 减去60

        SendDlgItemMessageW(hDlg, IDC_TB_RATIO, TBM_SETRANGE, FALSE, MAKELPARAM(1, 100));
        SendDlgItemMessageW(hDlg, IDC_TB_RATIO, TBM_SETPOS, TRUE, (LPARAM)g_GlobalEffect.Compressor.fRatio);

        SendDlgItemMessageW(hDlg, IDC_TB_PREDELAY, TBM_SETRANGE, FALSE, MAKELPARAM(0, 40));
        SendDlgItemMessageW(hDlg, IDC_TB_PREDELAY, TBM_SETPOS, TRUE, (LPARAM)(g_GlobalEffect.Compressor.fPredelay * 10));// 缩小10倍
    }
    return FALSE;
    case WM_COMMAND:
    {
        if (HIWORD(wParam) == BN_CLICKED)
        {
            if (LOWORD(wParam) == IDC_CB_ENABLE)// 启用
            {
                bApply = (SendMessageW((HWND)lParam, BM_GETCHECK, 0, 0) == BST_CHECKED);
                if (!g_hStream)
                    return TRUE;
                if (bApply)
                {
                    if (g_GlobalEffect.hFXCompressor)
                        BASS_ChannelRemoveFX(g_hStream, g_GlobalEffect.hFXCompressor);
                    g_GlobalEffect.hFXCompressor = BASS_ChannelSetFX(g_hStream, BASS_FX_DX8_COMPRESSOR, 1);
                    goto SetCompressor;
                }
                else
                {
                    if (g_GlobalEffect.hFXCompressor)
                        BASS_ChannelRemoveFX(g_hStream, g_GlobalEffect.hFXCompressor);
                    g_GlobalEffect.hFXCompressor = NULL;
                }
            }
            else if (LOWORD(wParam) == IDC_BT_RESET)//重置
            {
                GlobalEffect_ResetToDefault(EFFECT_COMPRESSOR);
                DlgProc_Compressor(hDlg, WM_INITDIALOG, 0, 0);
                goto SetCompressor;
            }
        }
    }
    return TRUE;
    case WM_HSCROLL:
    {
    SetCompressor:
        g_GlobalEffect.Compressor =
        {
            (float)SendDlgItemMessageW(hDlg, IDC_TB_GAIN, TBM_GETPOS, 0, 0),
            (float)SendDlgItemMessageW(hDlg, IDC_TB_ATTACK, TBM_GETPOS, 0, 0) / 100,
            (float)SendDlgItemMessageW(hDlg, IDC_TB_RELEASE, TBM_GETPOS, 0, 0),
            (float)SendDlgItemMessageW(hDlg, IDC_TB_THRESHOLD, TBM_GETPOS, 0, 0) - 60,
            (float)SendDlgItemMessageW(hDlg, IDC_TB_RATIO, TBM_GETPOS, 0, 0),
            (float)SendDlgItemMessageW(hDlg, IDC_TB_PREDELAY, TBM_GETPOS, 0, 0) / 10,
        };
        if (!bApply || !g_hStream)
            return TRUE;
        BASS_FXSetParameters(g_GlobalEffect.hFXCompressor, &g_GlobalEffect.Compressor);
    }
    }
    return FALSE;
}
INT_PTR CALLBACK DlgProc_Distortion(HWND hDlg, UINT message, WPARAM wParam, LPARAM lParam)
{
    static BOOL bApply = FALSE;
    switch (message)
    {
    case WM_INITDIALOG:
    {
        SendDlgItemMessageW(hDlg, IDC_CB_ENABLE, BM_SETCHECK, bApply ? BST_CHECKED : BST_UNCHECKED, 0);

        SendDlgItemMessageW(hDlg, IDC_TB_GAIN, TBM_SETRANGE, FALSE, MAKELPARAM(0, 60));
        SendDlgItemMessageW(hDlg, IDC_TB_GAIN, TBM_SETPOS, TRUE, (LPARAM)(g_GlobalEffect.Distortion.fGain + 60));// 减去60

        SendDlgItemMessageW(hDlg, IDC_TB_EDGE, TBM_SETRANGE, FALSE, MAKELPARAM(0, 100));
        SendDlgItemMessageW(hDlg, IDC_TB_EDGE, TBM_SETPOS, TRUE, (LPARAM)g_GlobalEffect.Distortion.fEdge);

        SendDlgItemMessageW(hDlg, IDC_TB_CENTERFREQUENCY, TBM_SETRANGE, FALSE, MAKELPARAM(100, 8000));
        SendDlgItemMessageW(hDlg, IDC_TB_CENTERFREQUENCY, TBM_SETPOS, TRUE, (LPARAM)g_GlobalEffect.Distortion.fPostEQCenterFrequency);

        SendDlgItemMessageW(hDlg, IDC_TB_BANDWIDTH, TBM_SETRANGE, FALSE, MAKELPARAM(100, 8000));
        SendDlgItemMessageW(hDlg, IDC_TB_BANDWIDTH, TBM_SETPOS, TRUE, (LPARAM)g_GlobalEffect.Distortion.fPostEQBandwidth);

        SendDlgItemMessageW(hDlg, IDC_TB_PRELOWPASSCUTOFF, TBM_SETRANGE, FALSE, MAKELPARAM(100, 8000));
        SendDlgItemMessageW(hDlg, IDC_TB_PRELOWPASSCUTOFF, TBM_SETPOS, TRUE, (LPARAM)g_GlobalEffect.Distortion.fPreLowpassCutoff);
    }
    return FALSE;
    case WM_COMMAND:
    {
        if (HIWORD(wParam) == BN_CLICKED)
        {
            if (LOWORD(wParam) == IDC_CB_ENABLE)// 启用
            {
                bApply = (SendMessageW((HWND)lParam, BM_GETCHECK, 0, 0) == BST_CHECKED);
                if (!g_hStream)
                    return TRUE;
                if (bApply)
                {
                    if (g_GlobalEffect.hFXDistortion)
                        BASS_ChannelRemoveFX(g_hStream, g_GlobalEffect.hFXDistortion);
                    g_GlobalEffect.hFXDistortion = BASS_ChannelSetFX(g_hStream, BASS_FX_DX8_DISTORTION, 1);
                    goto SetDistortion;
                }
                else
                {
                    if (g_GlobalEffect.hFXDistortion)
                        BASS_ChannelRemoveFX(g_hStream, g_GlobalEffect.hFXDistortion);
                    g_GlobalEffect.hFXDistortion = NULL;
                }
            }
            else if (LOWORD(wParam) == IDC_BT_RESET)//重置
            {
                GlobalEffect_ResetToDefault(EFFECT_DISTORTION);
                DlgProc_Distortion(hDlg, WM_INITDIALOG, 0, 0);
                goto SetDistortion;
            }
        }
    }
    return TRUE;
    case WM_HSCROLL:
    {
    SetDistortion:
        g_GlobalEffect.Distortion =
        {
            (float)SendDlgItemMessageW(hDlg, IDC_TB_GAIN, TBM_GETPOS, 0, 0) - 60,
            (float)SendDlgItemMessageW(hDlg, IDC_TB_EDGE, TBM_GETPOS, 0, 0),
            (float)SendDlgItemMessageW(hDlg, IDC_TB_CENTERFREQUENCY, TBM_GETPOS, 0, 0),
            (float)SendDlgItemMessageW(hDlg, IDC_TB_BANDWIDTH, TBM_GETPOS, 0, 0),
            (float)SendDlgItemMessageW(hDlg, IDC_TB_PRELOWPASSCUTOFF, TBM_GETPOS, 0, 0)
        };
        if (!bApply || !g_hStream)
            return TRUE;
        BASS_FXSetParameters(g_GlobalEffect.hFXDistortion, &g_GlobalEffect.Distortion);
    }
    return TRUE;
    }
    return FALSE;
}
INT_PTR CALLBACK DlgProc_Echo(HWND hDlg, UINT message, WPARAM wParam, LPARAM lParam)
{
    static BOOL bApply = FALSE;
    switch (message)
    {
    case WM_INITDIALOG:
    {
        SendDlgItemMessageW(hDlg, IDC_CB_ENABLE, BM_SETCHECK, bApply ? BST_CHECKED : BST_UNCHECKED, 0);

        SendDlgItemMessageW(hDlg, IDC_TB_WETDRYMIX, TBM_SETRANGE, FALSE, MAKELPARAM(0, 100));
        SendDlgItemMessageW(hDlg, IDC_TB_WETDRYMIX, TBM_SETPOS, TRUE, (LPARAM)g_GlobalEffect.Echo.fWetDryMix);

        SendDlgItemMessageW(hDlg, IDC_TB_FEEDBACK, TBM_SETRANGE, FALSE, MAKELPARAM(0, 100));
        SendDlgItemMessageW(hDlg, IDC_TB_FEEDBACK, TBM_SETPOS, TRUE, (LPARAM)g_GlobalEffect.Echo.fFeedback);

        SendDlgItemMessageW(hDlg, IDC_TB_LEFTDELAY, TBM_SETRANGE, FALSE, MAKELPARAM(1, 2000));
        SendDlgItemMessageW(hDlg, IDC_TB_LEFTDELAY, TBM_SETPOS, TRUE, (LPARAM)g_GlobalEffect.Echo.fLeftDelay);

        SendDlgItemMessageW(hDlg, IDC_TB_RIGHTDELAY, TBM_SETRANGE, FALSE, MAKELPARAM(1, 2000));
        SendDlgItemMessageW(hDlg, IDC_TB_RIGHTDELAY, TBM_SETPOS, TRUE, (LPARAM)g_GlobalEffect.Echo.fRightDelay);

        SendDlgItemMessageW(hDlg, IDC_CB_PANDELAY, BM_SETCHECK, g_GlobalEffect.Echo.lPanDelay ? BST_CHECKED : BST_UNCHECKED, 0);
    }
    return FALSE;
    case WM_COMMAND:
    {
        if (HIWORD(wParam) == BN_CLICKED)
        {
            switch (LOWORD(wParam))
            {
            case IDC_CB_ENABLE:// 启用
                bApply = (SendMessageW((HWND)lParam, BM_GETCHECK, 0, 0) == BST_CHECKED);
                if (!g_hStream)
                    return TRUE;
                if (bApply)
                {
                    if (g_GlobalEffect.hFXEcho)
                        BASS_ChannelRemoveFX(g_hStream, g_GlobalEffect.hFXEcho);
                    g_GlobalEffect.hFXEcho = BASS_ChannelSetFX(g_hStream, BASS_FX_DX8_ECHO, 1);
                    goto SetEcho;
                }
                else
                {
                    if (g_GlobalEffect.hFXEcho)
                        BASS_ChannelRemoveFX(g_hStream, g_GlobalEffect.hFXEcho);
                    g_GlobalEffect.hFXEcho = NULL;
                }
                return TRUE;
            case IDC_BT_RESET: // 重置
                GlobalEffect_ResetToDefault(EFFECT_ECHO);
                DlgProc_Echo(hDlg, WM_INITDIALOG, 0, 0);
                goto SetEcho;
                return TRUE;
            case IDC_CB_PANDELAY:
                goto SetEcho;
                return TRUE;
            }
        }
    }
    return TRUE;
    case WM_HSCROLL:
    {
    SetEcho:
        g_GlobalEffect.Echo =
        {
            (float)SendDlgItemMessageW(hDlg, IDC_TB_WETDRYMIX, TBM_GETPOS, 0, 0),
            (float)SendDlgItemMessageW(hDlg, IDC_TB_FEEDBACK, TBM_GETPOS, 0, 0),
            (float)SendDlgItemMessageW(hDlg, IDC_TB_LEFTDELAY, TBM_GETPOS, 0, 0),
            (float)SendDlgItemMessageW(hDlg, IDC_TB_RIGHTDELAY, TBM_GETPOS, 0, 0),
            (SendDlgItemMessageW(hDlg, IDC_CB_PANDELAY, BM_GETCHECK, 0, 0) == BST_CHECKED ? TRUE : FALSE)
        };
        if (!bApply || !g_hStream)
            return TRUE;
        BASS_FXSetParameters(g_GlobalEffect.hFXEcho, &g_GlobalEffect.Echo);
    }
    return TRUE;
    }
    return FALSE;
}
INT_PTR CALLBACK DlgProc_Flanger(HWND hDlg, UINT message, WPARAM wParam, LPARAM lParam)
{
    static BOOL bApply = FALSE;
    switch (message)
    {
    case WM_INITDIALOG:
    {
        SendDlgItemMessageW(hDlg, IDC_CB_ENABLE, BM_SETCHECK, bApply ? BST_CHECKED : BST_UNCHECKED, 0);

        SendDlgItemMessageW(hDlg, IDC_TB_WETDRYMIX, TBM_SETRANGE, FALSE, MAKELPARAM(0, 100));
        SendDlgItemMessageW(hDlg, IDC_TB_WETDRYMIX, TBM_SETPOS, TRUE, (LPARAM)g_GlobalEffect.Flanger.fWetDryMix);

        SendDlgItemMessageW(hDlg, IDC_TB_LFODEPTH, TBM_SETRANGE, FALSE, MAKELPARAM(0, 100));
        SendDlgItemMessageW(hDlg, IDC_TB_LFODEPTH, TBM_SETPOS, TRUE, (LPARAM)g_GlobalEffect.Flanger.fDepth);

        SendDlgItemMessageW(hDlg, IDC_TB_FEEDBACK, TBM_SETRANGE, FALSE, MAKELPARAM(0, 99 * 2));
        SendDlgItemMessageW(hDlg, IDC_TB_FEEDBACK, TBM_SETPOS, TRUE, (LPARAM)(g_GlobalEffect.Flanger.fFeedback + 99));// 减去99

        SendDlgItemMessageW(hDlg, IDC_TB_LFOFREQUENCY, TBM_SETRANGE, FALSE, MAKELPARAM(0, 100));
        SendDlgItemMessageW(hDlg, IDC_TB_LFOFREQUENCY, TBM_SETPOS, TRUE, (LPARAM)(g_GlobalEffect.Flanger.fFrequency * 10));// 缩小十倍

        SendDlgItemMessageW(hDlg, IDC_RB_SINEWAVE, BM_SETCHECK, BST_UNCHECKED, 0);
        SendDlgItemMessageW(hDlg, IDC_RB_TRIANGLEWAVE, BM_SETCHECK, BST_UNCHECKED, 0);
        SendDlgItemMessageW(hDlg, g_GlobalEffect.Flanger.lWaveform == 1 ? IDC_RB_SINEWAVE : IDC_RB_TRIANGLEWAVE, BM_SETCHECK, BST_CHECKED, 0);

        SendDlgItemMessageW(hDlg, IDC_TB_DELAY, TBM_SETRANGE, FALSE, MAKELPARAM(0, 40));
        SendDlgItemMessageW(hDlg, IDC_TB_DELAY, TBM_SETPOS, TRUE, (LPARAM)g_GlobalEffect.Flanger.fDelay);

        SendDlgItemMessageW(hDlg, IDC_CB_LFOPHASE, CB_INSERTSTRING, -1, (LPARAM)L"-180");
        SendDlgItemMessageW(hDlg, IDC_CB_LFOPHASE, CB_INSERTSTRING, -1, (LPARAM)L"-90");
        SendDlgItemMessageW(hDlg, IDC_CB_LFOPHASE, CB_INSERTSTRING, -1, (LPARAM)L"0");
        SendDlgItemMessageW(hDlg, IDC_CB_LFOPHASE, CB_INSERTSTRING, -1, (LPARAM)L"90");
        SendDlgItemMessageW(hDlg, IDC_CB_LFOPHASE, CB_INSERTSTRING, -1, (LPARAM)L"180");

        int iIndex = BASS_DX8_PHASE_90;
        switch (g_GlobalEffect.Flanger.lPhase)
        {
        case BASS_DX8_PHASE_NEG_180:iIndex = 0; break;
        case BASS_DX8_PHASE_NEG_90:iIndex = 1; break;
        case BASS_DX8_PHASE_ZERO:iIndex = 2; break;
        case BASS_DX8_PHASE_90:iIndex = 3; break;
        case BASS_DX8_PHASE_180:iIndex = 4; break;
        }

        SendDlgItemMessageW(hDlg, IDC_CB_LFOPHASE, CB_SETCURSEL, iIndex, 0);
    }
    return FALSE;
    case WM_COMMAND:
    {
        switch (HIWORD(wParam))
        {
        case CBN_SELCHANGE:
            goto SetFlanger;
            return TRUE;
        case BN_CLICKED:
        {
            switch (LOWORD(wParam))
            {
            case IDC_CB_ENABLE:
                bApply = (SendMessageW((HWND)lParam, BM_GETCHECK, 0, 0) == BST_CHECKED);
                if (!g_hStream)
                    return TRUE;
                if (bApply)
                {
                    if (g_GlobalEffect.hFXFlanger)
                        BASS_ChannelRemoveFX(g_hStream, g_GlobalEffect.hFXFlanger);
                    g_GlobalEffect.hFXFlanger = BASS_ChannelSetFX(g_hStream, BASS_FX_DX8_FLANGER, 1);
                    goto SetFlanger;
                }
                else
                {
                    if (g_GlobalEffect.hFXFlanger)
                        BASS_ChannelRemoveFX(g_hStream, g_GlobalEffect.hFXFlanger);
                    g_GlobalEffect.hFXFlanger = NULL;
                }

                return TRUE;
            case IDC_BT_RESET:
                GlobalEffect_ResetToDefault(EFFECT_FLANGER);
                DlgProc_Flanger(hDlg, WM_INITDIALOG, 0, 0);
                goto SetFlanger;
                return TRUE;
            case IDC_RB_SINEWAVE:
            case IDC_RB_TRIANGLEWAVE:
                goto SetFlanger;
                return TRUE;
            }
        }
        return TRUE;
        }
    }
    return FALSE;
    case WM_HSCROLL:
    {
    SetFlanger:
        DWORD dwPhase = BASS_DX8_PHASE_90;
        switch (SendDlgItemMessageW(hDlg, IDC_CB_LFOPHASE, CB_GETCURSEL, 0, 0))
        {
        case 0:dwPhase = BASS_DX8_PHASE_NEG_180; break;
        case 1:dwPhase = BASS_DX8_PHASE_NEG_90; break;
        case 2:dwPhase = BASS_DX8_PHASE_ZERO; break;
        case 3:dwPhase = BASS_DX8_PHASE_90; break;
        case 4:dwPhase = BASS_DX8_PHASE_180; break;
        }

        g_GlobalEffect.Flanger =
        {
            (float)SendDlgItemMessageW(hDlg, IDC_TB_WETDRYMIX, TBM_GETPOS, 0, 0),
            (float)SendDlgItemMessageW(hDlg, IDC_TB_LFODEPTH, TBM_GETPOS, 0, 0),
            (float)SendDlgItemMessageW(hDlg, IDC_TB_FEEDBACK, TBM_GETPOS, 0, 0) - 99,
            (float)SendDlgItemMessageW(hDlg, IDC_TB_LFOFREQUENCY, TBM_GETPOS, 0, 0) / 10,
            (SendDlgItemMessageW(hDlg, IDC_RB_SINEWAVE, BM_GETCHECK, 0, 0) == BST_CHECKED ? 1ul : 0ul),
            (float)SendDlgItemMessageW(hDlg, IDC_TB_DELAY, TBM_GETPOS, 0, 0),
            dwPhase
        };
        if (!bApply || !g_hStream)
            return TRUE;
        BASS_FXSetParameters(g_GlobalEffect.hFXFlanger, &g_GlobalEffect.Flanger);
    }
    return TRUE;
    }
    return FALSE;
}
INT_PTR CALLBACK DlgProc_Gargle(HWND hDlg, UINT message, WPARAM wParam, LPARAM lParam)
{
    static BOOL bApply = FALSE;
    switch (message)
    {
    case WM_INITDIALOG:
    {
        SendDlgItemMessageW(hDlg, IDC_CB_ENABLE, BM_SETCHECK, bApply ? BST_CHECKED : BST_UNCHECKED, 0);

        SendDlgItemMessageW(hDlg, IDC_TB_RATEHZ, TBM_SETRANGE, FALSE, MAKELPARAM(1, 1000));
        SendDlgItemMessageW(hDlg, IDC_TB_RATEHZ, TBM_SETPOS, TRUE, (LPARAM)g_GlobalEffect.Gargle.dwRateHz);

        SendDlgItemMessageW(hDlg, IDC_RB_SINEWAVE, BM_SETCHECK, BST_UNCHECKED, 0);
        SendDlgItemMessageW(hDlg, IDC_RB_TRIANGLEWAVE, BM_SETCHECK, BST_UNCHECKED, 0);
        SendDlgItemMessageW(hDlg, g_GlobalEffect.Gargle.dwWaveShape == 1 ? IDC_RB_SINEWAVE : IDC_RB_TRIANGLEWAVE, BM_SETCHECK, BST_CHECKED, 0);
    }
    return FALSE;
    case WM_COMMAND:
    {
        if (HIWORD(wParam) == BN_CLICKED)
        {
            switch (LOWORD(wParam))
            {
            case IDC_CB_ENABLE:
                bApply = (SendMessageW((HWND)lParam, BM_GETCHECK, 0, 0) == BST_CHECKED);
                if (!g_hStream)
                    return TRUE;
                if (bApply)
                {
                    if (g_GlobalEffect.hFXGargle)
                        BASS_ChannelRemoveFX(g_hStream, g_GlobalEffect.hFXGargle);
                    g_GlobalEffect.hFXGargle = BASS_ChannelSetFX(g_hStream, BASS_FX_DX8_GARGLE, 1);
                    goto SetGargle;
                }
                else
                {
                    if (g_GlobalEffect.hFXGargle)
                        BASS_ChannelRemoveFX(g_hStream, g_GlobalEffect.hFXGargle);
                    g_GlobalEffect.hFXGargle = NULL;
                }

                return TRUE;
            case IDC_BT_RESET:
                GlobalEffect_ResetToDefault(EFFECT_GARGLE);
                DlgProc_Gargle(hDlg, WM_INITDIALOG, 0, 0);
                goto SetGargle;
                return TRUE;
            case IDC_RB_SINEWAVE:
            case IDC_RB_TRIANGLEWAVE:
                goto SetGargle;
                return TRUE;
            }
        }
    }
    return TRUE;
    case WM_HSCROLL:
    {
    SetGargle:
        g_GlobalEffect.Gargle =
        {
            (DWORD)SendDlgItemMessageW(hDlg, IDC_TB_RATEHZ, TBM_GETPOS, 0, 0),
            (SendDlgItemMessageW(hDlg, IDC_RB_SINEWAVE, BM_GETCHECK, 0, 0) == BST_CHECKED ? 1ul : 0ul)
        };
        if (!bApply || !g_hStream)
            return TRUE;
        BASS_FXSetParameters(g_GlobalEffect.hFXGargle, &g_GlobalEffect.Gargle);
    }
    return TRUE;
    }
    return FALSE;
}
INT_PTR CALLBACK DlgProc_I3DL2Reverb(HWND hDlg, UINT message, WPARAM wParam, LPARAM lParam)
{
    static BOOL bApply = FALSE;
    switch (message)
    {
    case WM_INITDIALOG:
    {
        SendDlgItemMessageW(hDlg, IDC_CB_ENABLE, BM_SETCHECK, bApply ? BST_CHECKED : BST_UNCHECKED, 0);

        SendDlgItemMessageW(hDlg, IDC_TB_ROOM, TBM_SETRANGE, FALSE, MAKELPARAM(0, 10000));
        SendDlgItemMessageW(hDlg, IDC_TB_ROOM, TBM_SETPOS, TRUE, (LPARAM)(g_GlobalEffect.I3DL2Reverb.lRoom + 10000));// 减去10000

        SendDlgItemMessageW(hDlg, IDC_TB_ROOMHF, TBM_SETRANGE, FALSE, MAKELPARAM(0, 10000));
        SendDlgItemMessageW(hDlg, IDC_TB_ROOMHF, TBM_SETPOS, TRUE, (LPARAM)(g_GlobalEffect.I3DL2Reverb.lRoomHF + 10000));// 减去10000

        SendDlgItemMessageW(hDlg, IDC_TB_ROOMROLLOFFFACTOR, TBM_SETRANGE, FALSE, MAKELPARAM(0, 100));
        SendDlgItemMessageW(hDlg, IDC_TB_ROOMROLLOFFFACTOR, TBM_SETPOS, TRUE, (LPARAM)(g_GlobalEffect.I3DL2Reverb.flRoomRolloffFactor * 10));// 缩小10倍

        SendDlgItemMessageW(hDlg, IDC_TB_DECAYTIME, TBM_SETRANGE, FALSE, MAKELPARAM(10, 2000));
        SendDlgItemMessageW(hDlg, IDC_TB_DECAYTIME, TBM_SETPOS, TRUE, (LPARAM)(g_GlobalEffect.I3DL2Reverb.flDecayTime * 100));// 缩小100倍

        SendDlgItemMessageW(hDlg, IDC_TB_DECAYHFRADIO, TBM_SETRANGE, FALSE, MAKELPARAM(10, 2000));
        SendDlgItemMessageW(hDlg, IDC_TB_DECAYHFRADIO, TBM_SETPOS, TRUE, (LPARAM)(g_GlobalEffect.I3DL2Reverb.flDecayHFRatio * 100));// 缩小100倍

        SendDlgItemMessageW(hDlg, IDC_TB_REFLECTIONS, TBM_SETRANGE, FALSE, MAKELPARAM(0, 11000));
        SendDlgItemMessageW(hDlg, IDC_TB_REFLECTIONS, TBM_SETPOS, TRUE, (LPARAM)(g_GlobalEffect.I3DL2Reverb.lReflections + 10000));// 减去10000

        SendDlgItemMessageW(hDlg, IDC_TB_REFLECTIONSDELAY, TBM_SETRANGE, FALSE, MAKELPARAM(0, 300));
        SendDlgItemMessageW(hDlg, IDC_TB_REFLECTIONSDELAY, TBM_SETPOS, TRUE, (LPARAM)(g_GlobalEffect.I3DL2Reverb.flReflectionsDelay * 1000));// 缩小1000倍

        SendDlgItemMessageW(hDlg, IDC_TB_REVERB, TBM_SETRANGE, FALSE, MAKELPARAM(0, 12000));
        SendDlgItemMessageW(hDlg, IDC_TB_REVERB, TBM_SETPOS, TRUE, (LPARAM)(g_GlobalEffect.I3DL2Reverb.lReverb + 10000));// 减去10000

        SendDlgItemMessageW(hDlg, IDC_TB_REVERBDELAY, TBM_SETRANGE, FALSE, MAKELPARAM(0, 100));
        SendDlgItemMessageW(hDlg, IDC_TB_REVERBDELAY, TBM_SETPOS, TRUE, (LPARAM)(g_GlobalEffect.I3DL2Reverb.flReverbDelay * 1000));// 缩小1000倍

        SendDlgItemMessageW(hDlg, IDC_TB_DIFFUSION, TBM_SETRANGE, FALSE, MAKELPARAM(0, 100));
        SendDlgItemMessageW(hDlg, IDC_TB_DIFFUSION, TBM_SETPOS, TRUE, (LPARAM)g_GlobalEffect.I3DL2Reverb.flDiffusion);

        SendDlgItemMessageW(hDlg, IDC_TB_DENSITY, TBM_SETRANGE, FALSE, MAKELPARAM(1, 100));
        SendDlgItemMessageW(hDlg, IDC_TB_DENSITY, TBM_SETPOS, TRUE, (LPARAM)g_GlobalEffect.I3DL2Reverb.flDensity);

        SendDlgItemMessageW(hDlg, IDC_TB_HFREFERENCE, TBM_SETRANGE, FALSE, MAKELPARAM(20, 20000));
        SendDlgItemMessageW(hDlg, IDC_TB_HFREFERENCE, TBM_SETPOS, TRUE, (LPARAM)g_GlobalEffect.I3DL2Reverb.flHFReference);
    }
    return FALSE;
    case WM_COMMAND:
    {
        if (HIWORD(wParam) == BN_CLICKED)
        {
            if (LOWORD(wParam) == IDC_CB_ENABLE)// 启用
            {
                bApply = (SendMessageW((HWND)lParam, BM_GETCHECK, 0, 0) == BST_CHECKED);
                if (!g_hStream)
                    return TRUE;
                if (bApply)
                {
                    if (g_GlobalEffect.hFXI3DL2Reverb)
                        BASS_ChannelRemoveFX(g_hStream, g_GlobalEffect.hFXI3DL2Reverb);
                    g_GlobalEffect.hFXI3DL2Reverb = BASS_ChannelSetFX(g_hStream, BASS_FX_DX8_I3DL2REVERB, 1);
                    goto SetI3DL2Reverb;
                }
                else
                {
                    if (g_GlobalEffect.hFXI3DL2Reverb)
                        BASS_ChannelRemoveFX(g_hStream, g_GlobalEffect.hFXI3DL2Reverb);
                    g_GlobalEffect.hFXI3DL2Reverb = NULL;
                }
            }
            else if (LOWORD(wParam) == IDC_BT_RESET)//重置
            {
                GlobalEffect_ResetToDefault(EFFECT_I3DL2REVERB);
                DlgProc_I3DL2Reverb(hDlg, WM_INITDIALOG, 0, 0);
                goto SetI3DL2Reverb;
            }
        }
    }
    return TRUE;
    case WM_HSCROLL:
    {
    SetI3DL2Reverb:
        g_GlobalEffect.I3DL2Reverb =
        {
            SendDlgItemMessageW(hDlg, IDC_TB_ROOM, TBM_GETPOS, 0, 0) - 10000,
            SendDlgItemMessageW(hDlg, IDC_TB_ROOMHF, TBM_GETPOS, 0, 0) - 10000,
            (float)SendDlgItemMessageW(hDlg, IDC_TB_ROOMROLLOFFFACTOR, TBM_GETPOS, 0, 0) / 10,
            (float)SendDlgItemMessageW(hDlg, IDC_TB_DECAYTIME, TBM_GETPOS, 0, 0) / 100,
            (float)SendDlgItemMessageW(hDlg, IDC_TB_DECAYHFRADIO, TBM_GETPOS, 0, 0) / 100,
            SendDlgItemMessageW(hDlg, IDC_TB_REFLECTIONS, TBM_GETPOS, 0, 0) - 10000,
            (float)SendDlgItemMessageW(hDlg, IDC_TB_REFLECTIONSDELAY, TBM_GETPOS, 0, 0) / 1000,
            SendDlgItemMessageW(hDlg, IDC_TB_REVERB, TBM_GETPOS, 0, 0) - 10000,
            (float)SendDlgItemMessageW(hDlg, IDC_TB_REVERBDELAY, TBM_GETPOS, 0, 0) / 1000,
            (float)SendDlgItemMessageW(hDlg, IDC_TB_DIFFUSION, TBM_GETPOS, 0, 0),
            (float)SendDlgItemMessageW(hDlg, IDC_TB_DENSITY, TBM_GETPOS, 0, 0) ,
            (float)SendDlgItemMessageW(hDlg, IDC_TB_HFREFERENCE, TBM_GETPOS, 0, 0)
        };
        if (!bApply || !g_hStream)
            return TRUE;
        BASS_FXSetParameters(g_GlobalEffect.hFXI3DL2Reverb, &g_GlobalEffect.I3DL2Reverb);
    }
    return TRUE;
    }
    return FALSE;
}
INT_PTR CALLBACK DlgProc_Reverb(HWND hDlg, UINT message, WPARAM wParam, LPARAM lParam)
{
    static BOOL bApply = FALSE;
    switch (message)
    {
    case WM_INITDIALOG:
    {
        SendDlgItemMessageW(hDlg, IDC_CB_ENABLE, BM_SETCHECK, bApply ? BST_CHECKED : BST_UNCHECKED, 0);

        SendDlgItemMessageW(hDlg, IDC_TB_GAIN, TBM_SETRANGE, FALSE, MAKELPARAM(0, 96));
        SendDlgItemMessageW(hDlg, IDC_TB_GAIN, TBM_SETPOS, TRUE, (LPARAM)(g_GlobalEffect.Reverb.fInGain + 96));// 减去96

        SendDlgItemMessageW(hDlg, IDC_TB_REVERBMIX, TBM_SETRANGE, FALSE, MAKELPARAM(0, 96));
        SendDlgItemMessageW(hDlg, IDC_TB_REVERBMIX, TBM_SETPOS, TRUE, (LPARAM)(g_GlobalEffect.Reverb.fReverbMix + 96));// 减去96

        SendDlgItemMessageW(hDlg, IDC_TB_REVERBTIME, TBM_SETRANGEMIN, FALSE, 1);
        SendDlgItemMessageW(hDlg, IDC_TB_REVERBTIME, TBM_SETRANGEMAX, FALSE, 3000000);
        SendDlgItemMessageW(hDlg, IDC_TB_REVERBTIME, TBM_SETPOS, TRUE, (LPARAM)(g_GlobalEffect.Reverb.fReverbTime * 1000));// 缩小1000

        SendDlgItemMessageW(hDlg, IDC_TB_HIGHFREQRTRATIO, TBM_SETRANGE, FALSE, MAKELPARAM(1, 999));
        SendDlgItemMessageW(hDlg, IDC_TB_HIGHFREQRTRATIO, TBM_SETPOS, TRUE, (LPARAM)(g_GlobalEffect.Reverb.fHighFreqRTRatio * 1000));// 缩小1000倍
    }
    return FALSE;
    case WM_COMMAND:
    {
        if (HIWORD(wParam) == BN_CLICKED)
        {
            if (LOWORD(wParam) == IDC_CB_ENABLE)// 启用
            {
                bApply = (SendMessageW((HWND)lParam, BM_GETCHECK, 0, 0) == BST_CHECKED);
                if (!g_hStream)
                    return TRUE;
                if (bApply)
                {
                    if (g_GlobalEffect.hFXReverb)
                        BASS_ChannelRemoveFX(g_hStream, g_GlobalEffect.hFXReverb);
                    g_GlobalEffect.hFXReverb = BASS_ChannelSetFX(g_hStream, BASS_FX_DX8_REVERB, 1);
                    goto SetReverb;
                }
                else
                {
                    if (g_GlobalEffect.hFXReverb)
                        BASS_ChannelRemoveFX(g_hStream, g_GlobalEffect.hFXReverb);
                    g_GlobalEffect.hFXReverb = NULL;
                }
            }
            else if (LOWORD(wParam) == IDC_BT_RESET)//重置
            {
                GlobalEffect_ResetToDefault(EFFECT_REVERB);
                DlgProc_Reverb(hDlg, WM_INITDIALOG, 0, 0);
                goto SetReverb;
            }
        }
    }
    return TRUE;
    case WM_HSCROLL:
    {
    SetReverb:
        g_GlobalEffect.Reverb =
        {
            (float)SendDlgItemMessageW(hDlg, IDC_TB_GAIN, TBM_GETPOS, 0, 0) - 96,
            (float)SendDlgItemMessageW(hDlg, IDC_TB_REVERBMIX, TBM_GETPOS, 0, 0) - 96,
            (float)SendDlgItemMessageW(hDlg, IDC_TB_REVERBTIME, TBM_GETPOS, 0, 0) / 1000,
            (float)SendDlgItemMessageW(hDlg, IDC_TB_HIGHFREQRTRATIO, TBM_GETPOS, 0, 0) / 1000
        };
        if (!bApply || !g_hStream)
            return TRUE;
        BASS_FXSetParameters(g_GlobalEffect.hFXReverb, &g_GlobalEffect.Reverb);
    }
    return TRUE;
    }
    return FALSE;
}
INT_PTR CALLBACK DlgProc_Rotate(HWND hDlg, UINT message, WPARAM wParam, LPARAM lParam)
{
    static BOOL bApply = FALSE;
    switch (message)
    {
    case WM_INITDIALOG:
    {
        SendDlgItemMessageW(hDlg, IDC_TB_SPEED, TBM_SETRANGEMIN, TRUE, -250);// 扩大100倍
        SendDlgItemMessageW(hDlg, IDC_TB_SPEED, TBM_SETRANGEMAX, TRUE, 250);
        SendDlgItemMessageW(hDlg, IDC_TB_SPEED, TBM_SETPOS, TRUE, (LPARAM)(g_GlobalEffect.Rotate.fRate * 100));
    }
    return FALSE;
    case WM_COMMAND:
    {
        if (HIWORD(wParam) == BN_CLICKED)
        {
            if (LOWORD(wParam) == IDC_CB_ENABLE)// 启用
            {
                bApply = (SendMessageW((HWND)lParam, BM_GETCHECK, 0, 0) == BST_CHECKED);
                if (!g_hStream)
                    return TRUE;
                if (bApply)
                {
                    if (g_GlobalEffect.hFXRotate)
                        BASS_ChannelRemoveFX(g_hStream, g_GlobalEffect.hFXRotate);
                    g_GlobalEffect.hFXRotate = BASS_ChannelSetFX(g_hStream, BASS_FX_BFX_ROTATE, 1);
                    goto SetRotate;
                }
                else
                {
                    if (g_GlobalEffect.hFXRotate)
                        BASS_ChannelRemoveFX(g_hStream, g_GlobalEffect.hFXRotate);
                    g_GlobalEffect.hFXRotate = NULL;
                }
            }
            else if (LOWORD(wParam) == IDC_BT_RESET)//重置
            {
                GlobalEffect_ResetToDefault(EFFECT_REVERB);
                DlgProc_Reverb(hDlg, WM_INITDIALOG, 0, 0);
                goto SetRotate;
            }
        }
    }
    return FALSE;
    case WM_HSCROLL:
    {
    SetRotate:
        g_GlobalEffect.Rotate.fRate = (float)((int)SendDlgItemMessageW(hDlg, IDC_TB_SPEED, TBM_GETPOS, 0, 0)) / 100.0f;
        if (!bApply || !g_hStream)
            return TRUE;
        BASS_FXSetParameters(g_GlobalEffect.hFXRotate, &g_GlobalEffect.Rotate);
    }
    return TRUE;
    }
    return FALSE;
}
INT_PTR CALLBACK DlgProc_Effect(HWND hDlg, UINT message, WPARAM wParam, LPARAM lParam)
{
    static HWND hTab;
    static HWND hChild[EFFECTWNDTABCOUNT];
    switch (message)
    {
    case WM_INITDIALOG:
    {
        RECT rc, rc2;
        GetWindowRect(hDlg, &rc);
        GetWindowRect(g_hMainWnd, &rc2);
        SetWindowPos(hDlg, NULL,
            ((rc2.right - rc2.left) - (rc.right - rc.left)) / 2 + rc2.left,
            ((rc2.bottom - rc2.top) - (rc.bottom - rc.top)) / 2 + rc2.top,
            0, 0, SWP_NOZORDER | SWP_NOSIZE);

        hTab = GetDlgItem(hDlg, IDC_TAB);
        SetWindowLongPtrW(hTab, GWL_STYLE, GetWindowLongPtrW(hTab, GWL_STYLE) | WS_CLIPCHILDREN);//剪辑子窗口
        TCITEMW tci = { 0 };
        tci.mask = TCIF_TEXT;

        tci.pszText = (LPWSTR)L"参数调节";
        SendMessageW(hTab, TCM_INSERTITEMW, 0, (LPARAM)&tci);

        tci.pszText = (LPWSTR)L"均衡器";
        SendMessageW(hTab, TCM_INSERTITEMW, 1, (LPARAM)&tci);

        tci.pszText = (LPWSTR)L"合唱";
        SendMessageW(hTab, TCM_INSERTITEMW, 2, (LPARAM)&tci);

        tci.pszText = (LPWSTR)L"压缩";
        SendMessageW(hTab, TCM_INSERTITEMW, 3, (LPARAM)&tci);

        tci.pszText = (LPWSTR)L"失真";
        SendMessageW(hTab, TCM_INSERTITEMW, 4, (LPARAM)&tci);

        tci.pszText = (LPWSTR)L"回声";
        SendMessageW(hTab, TCM_INSERTITEMW, 5, (LPARAM)&tci);

        tci.pszText = (LPWSTR)L"镶边";
        SendMessageW(hTab, TCM_INSERTITEMW, 6, (LPARAM)&tci);

        tci.pszText = (LPWSTR)L"漱口";
        SendMessageW(hTab, TCM_INSERTITEMW, 7, (LPARAM)&tci);

        tci.pszText = (LPWSTR)L"3D混响";
        SendMessageW(hTab, TCM_INSERTITEMW, 8, (LPARAM)&tci);

        tci.pszText = (LPWSTR)L"混响";
        SendMessageW(hTab, TCM_INSERTITEMW, 9, (LPARAM)&tci);

        tci.pszText = (LPWSTR)L"环绕";
        SendMessageW(hTab, TCM_INSERTITEMW, 10, (LPARAM)&tci);

        SendMessageW(hTab, TCM_SETCURSEL, 0, 0);

        SendMessageW(hTab, TCM_GETITEMRECT, 0, (LPARAM)&rc);
        GetClientRect(hTab, &rc2);

        PCWSTR DialogID[] =
        {
            MAKEINTRESOURCEW(IDD_SBV),
            MAKEINTRESOURCEW(IDD_EQ),
            MAKEINTRESOURCEW(IDD_CHORUS),
            MAKEINTRESOURCEW(IDD_COMPRESSOR),
            MAKEINTRESOURCEW(IDD_DISTORTION),
            MAKEINTRESOURCEW(IDD_ECHO),
            MAKEINTRESOURCEW(IDD_FLANGER),
            MAKEINTRESOURCEW(IDD_GARGLE),
            MAKEINTRESOURCEW(IDD_I3DL2REVERB),
            MAKEINTRESOURCEW(IDD_REVERB),
            MAKEINTRESOURCEW(IDD_ROTATE)
        };
        DLGPROC DialogProc[] =
        {
            DlgProc_SBV,
            DlgProc_EQ,
            DlgProc_Chorus,
            DlgProc_Compressor,
            DlgProc_Distortion,
            DlgProc_Echo,
            DlgProc_Flanger,
            DlgProc_Gargle,
            DlgProc_I3DL2Reverb,
            DlgProc_Reverb,
            DlgProc_Rotate
        };

        for (int i = 0; i < EFFECTWNDTABCOUNT; ++i)
        {
            hChild[i] = CreateDialogParamW(g_hInst, DialogID[i], hDlg, DialogProc[i], 0);

            SetWindowLongPtrW(hChild[i], GWL_STYLE, WS_CHILD);
            SetParent(hChild[i], hTab);
            SetWindowPos(hChild[i], NULL, 0, rc.bottom, rc2.right + 3, rc2.bottom - rc.bottom + 3, SWP_NOZORDER);
            ShowWindow(hChild[i], SW_HIDE);

            SendDlgItemMessageW(hChild[i], IDC_CB_ENABLE, BM_SETIMAGE, IMAGE_ICON, (LPARAM)GR.hiTick);
            SendDlgItemMessageW(hChild[i], IDC_BT_RESET, BM_SETIMAGE, IMAGE_ICON, (LPARAM)GR.hiArrowCircle);// 设置图标
        }
        ShowWindow(hChild[0], SW_SHOW);
    }
    return FALSE;
    case WM_CLOSE:
    {
        DestroyWindow(hDlg);
    }
    return TRUE;
    case WM_NOTIFY:
    {
        if (((NMHDR*)lParam)->idFrom == IDC_TAB)
        {
            if (((NMHDR*)lParam)->code == TCN_SELCHANGE)//返回值不使用
            {
                int j = SendMessageW(hTab, TCM_GETCURSEL, 0, 0);
                for (int i = 0; i < EFFECTWNDTABCOUNT; ++i)
                {
                    if (i == j)
                        ShowWindow(hChild[i], SW_SHOW);
                    else
                        ShowWindow(hChild[i], SW_HIDE);
                }
                return TRUE;
            }
        }
    }
    return FALSE;
    }
    return FALSE;
}