/*
* GlobalVar.h
* 定义全局变量
*/
#pragma once
#include <Windows.h>
#include <Shobjidl.h>

#include "bass.h"

#include "MyProject.h"
#include "Function.h"
#include "WndEffect.h"
// 窗口句柄类
extern HWND				g_hMainWnd;             // 主窗口
extern HWND				g_hLV;                  // 列表ListView
extern HWND				g_hTBProgess;           // 进度条
extern HWND				g_hLrcWnd;              // 桌面歌词
extern HWND				g_hBKLeft;              // 左侧背景
extern HWND				g_hBKBtm;               // 下部按钮背景
extern HWND				g_hBKRight;             // 列表工具背景
extern HWND				g_hSEB;                 // 分隔条
extern HWND				g_hBKList;              // 列表背景
// 全局类
extern HINSTANCE		g_hInst;                // 全局实例句柄
extern GLOBALRES		GR;                     // 全局资源
extern GLOBALCONTEXT	GC;                     // 全局上下文（尺寸等）
extern SETTINGS			GS;                     // 全局设置
extern HFONT            g_hFontDrawing;         // 绘制用大字体
extern HFONT			g_hFont;                // 界面字体
extern HFONT			g_hFontCenterLrc;		// 中间歌词字体
extern int				WM_TASKBARBUTTONCREATED;// 任务栏按钮创建完毕
extern UINT				g_uMyClipBoardFmt;      // 内部拖放剪贴板格式
extern ITaskbarList4*	g_pITaskbarList;        // ITaskbarList接口指针，负责任务栏操作
extern pFuncGetDpiForSystem pGetDpiForSystem;   // GetDpiForSystem函数指针
extern pFuncGetDpiForWindow pGetDpiForWindow;   // GetDpiForWindow函数指针
extern int				g_iDPI;                 // 当前DPI
extern PWSTR			g_pszDefPic;            // 路径：默认图片
extern PWSTR			g_pszDataDir;           // 路径：程序数据目录
extern PWSTR			g_pszListDir;           // 路径：列表目录
extern PWSTR			g_pszCurrDir;           // 路径：程序所在目录
extern PWSTR			g_pszProfie;            // 路径：配置文件
// Bass与播放类
extern QKARRAY			g_Lrc;                  // 全局歌词数据
extern int				g_iLrcState;            // 桌面歌词标志
extern QKARRAY			g_ItemData;             // 全局列表数据

extern HSTREAM			g_hStream;              // 全局播放流
extern BOOL				g_bHMUSIC;              // 是否为MOD音乐
extern BOOL				g_bPlaying;				// 是否正在播放，用来判断是否播完

extern ULONGLONG        g_llLength;             // 总长度，单位毫秒，进度条单位为百毫秒
extern float			g_fTime;                // 当前进度

extern PWSTR			g_pszFile;				// 当前文件路径
extern int				g_iCurrFileIndex;       // 正在播放的文件（LV索引）
extern int				g_iCurrLrcIndex;        // 现行歌词索引，-1：第一句但不高亮
extern int				g_iLaterPlay;           // 稍后播放（LV索引）

extern GLOBALEFFECT		g_GlobalEffect;         // 全局音效
extern BOOL				g_bSlient;              // 是否静音
extern float            g_fDefSpeed,            // 默认速度
						g_fSpeedChanged,        // 速度，-1~5
						g_fBlanceChanged,       // 平衡，-1~1
						g_fVolChanged;          // 音量，0~1
// 通信与标志类
extern BOOL				g_bListSeped;           // 列表是否拆离
extern BOOL				g_bListHidden;          // 列表是否隐藏
extern int              g_cxBKList;             // 列表背景宽度
extern int              g_iSearchResult;        // 搜索结果数
extern int              g_bSort;                // 是否处于排序模式

extern BOOL				g_bPlayIcon;            // 是否为播放图标（桌面歌词和主窗口用）