/*
* GlobalVar.h
* ����ȫ�ֱ���
*/
#pragma once
#include <Windows.h>
#include <Shobjidl.h>
#include <d2d1.h>
#include <dwrite.h>
#include <d2d1_1.h>
#include <d3d11.h>
#include <wincodec.h>
#include <dxgi.h>
#include <dxgi1_2.h>

#include "bass.h"

#include "MyProject.h"
#include "Function.h"
#include "WndEffect.h"
// ���ھ����
extern HWND				g_hMainWnd;             // ������
extern HWND				g_hLV;                  // �б�ListView
extern HWND				g_hTBProgess;           // ������
extern HWND				g_hLrcWnd;              // ������
extern HWND				g_hBKLeft;              // ��౳��
//extern HWND				g_hBKBtm;               // �²���ť����
extern HWND				g_hBKRight;             // �б��߱���
extern HWND				g_hSEB;                 // �ָ���
extern HWND				g_hBKList;              // �б���
// ȫ����
extern HINSTANCE		g_hInst;                // ȫ��ʵ�����
extern GLOBALRES		GR;                     // ȫ����Դ
extern GLOBALCONTEXT	GC;                     // ȫ�������ģ��ߴ�ȣ�
extern SETTINGS			GS;                     // ȫ������
extern HFONT            g_hFontDrawing;         // �����ô�����
extern HFONT			g_hFont;                // ��������
extern HFONT			g_hFontCenterLrc;		// �м�������
extern int				WM_TASKBARBUTTONCREATED;// ��������ť�������
extern UINT				g_uMyClipBoardFmt;      // �ڲ��Ϸż������ʽ
extern ITaskbarList4*	g_pITaskbarList;        // ITaskbarList�ӿ�ָ�룬��������������
extern pFuncGetDpiForSystem pGetDpiForSystem;   // GetDpiForSystem����ָ��
extern pFuncGetDpiForWindow pGetDpiForWindow;   // GetDpiForWindow����ָ��
extern int				g_iDPI;                 // ��ǰDPI
extern PWSTR			g_pszDefPic;            // ·����Ĭ��ͼƬ
extern PWSTR			g_pszDataDir;           // ·������������Ŀ¼
extern PWSTR			g_pszListDir;           // ·�����б�Ŀ¼
extern PWSTR			g_pszCurrDir;           // ·������������Ŀ¼
extern PWSTR			g_pszProfie;            // ·���������ļ�
extern ID2D1Factory1*	g_pD2DFactory;
extern IDWriteFactory*	g_pDWFactory;
extern ID2D1Device*		g_pD2DDevice;
extern IDXGIDevice1*	g_pDXGIDevice;
extern IWICImagingFactory* g_pWICFactory;
extern IDXGIFactory2*	g_pDXGIFactory;
extern IDWriteTextFormat* g_pDWTFNormal;
extern IDWriteTextFormat* g_pDWTFBig;
extern IDWriteTextFormat* g_pDWTFBig2;
// Bass�벥����
extern QKARRAY			g_Lrc;                  // ȫ�ָ������
extern int				g_iLrcState;            // �����ʱ�־
extern QKARRAY			g_ItemData;             // ȫ���б�����

extern HSTREAM			g_hStream;              // ȫ�ֲ�����
extern BOOL				g_bHMUSIC;              // �Ƿ�ΪMOD����
extern BOOL				g_bPlaying;				// �Ƿ����ڲ��ţ������ж��Ƿ���

extern ULONGLONG        g_llLength;             // �ܳ��ȣ���λ���룬��������λΪ�ٺ���
extern float			g_fTime;                // ��ǰ����

extern PWSTR			g_pszFile;				// ��ǰ�ļ�·��
extern int				g_iCurrFileIndex;       // ���ڲ��ŵ��ļ���LV������
extern int				g_iCurrLrcIndex;        // ���и��������-1����һ�䵫������
extern int				g_iLaterPlay;           // �Ժ󲥷ţ�LV������

extern GLOBALEFFECT		g_GlobalEffect;         // ȫ����Ч
extern BOOL				g_bSlient;              // �Ƿ���
extern float            g_fDefSpeed,            // Ĭ���ٶ�
						g_fSpeedChanged,        // �ٶȣ�-1~5
						g_fBlanceChanged,       // ƽ�⣬-1~1
						g_fVolChanged;          // ������0~1
// ͨ�����־��
extern BOOL				g_bListSeped;           // �б��Ƿ����
extern BOOL				g_bListHidden;          // �б��Ƿ�����
extern int              g_cxBKList;             // �б������
extern int              g_iSearchResult;        // ���������
extern int              g_bSort;                // �Ƿ�������ģʽ

extern BOOL				g_bPlayIcon;            // �Ƿ�Ϊ����ͼ�꣨�����ʺ��������ã�