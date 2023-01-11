#pragma once
#include <Windows.h>
#include <dwrite.h>

#include "MyProject.h"

//*****************************结构*****************************//
struct LRCDATA			// 当前歌曲的歌词信息，在g_Lrc中存储
{
	float fTime;		// 对应时间
	float fDelay;		// 延时，这一句歌词到下一句歌词的时间
	PWSTR pszLrc;		// 歌词
	int iOrgLength;		// 原文的字符数，用于横向滚动
	int cy;				// 歌词总高度，用于命中测试
	int iLastTop;		// 上次绘制时歌词矩形的顶边，用于滚动歌词绘制
	int iDrawID;		// 绘制ID，用于区分项目是否显示
	RECT rcItem;		// 歌词矩形，用于命中测试
};

struct LRCHSCROLLINFO				// 歌词横向滚动信息，成员中后面标有1/2的表示为第一句和第二句 
{
	int iIndex;						// 当前项
	int cx1;						// 宽度1
	int cx2;						// 宽度2
	int x1;							// 滚动坐标1
	int x2;							// 滚动坐标2
	float fNoScrollingTime1;		// 停滞时间1
	float fNoScrollingTime2;		// 停滞时间2
	BOOL bWndSizeChangedFlag;		// 窗口已被改变标志
	BOOL bShowCurr;					// 指示现行项目是否可见
	DWRITE_TEXT_METRICS Metrics1;	// 上次测高结果1
	DWRITE_TEXT_METRICS Metrics2;	// 上次测高结果2
};

struct LRCVSCROLLINFO	// 歌词垂直滚动信息
{
	int iSrcTop;		// 起始顶边
	int iDestTop;		// 目标顶边
	int iDistance;		// 距离
	BOOL bDirection;	// 方向，TRUE=向下，FALSE=向上
	float fDelay;		// 滚动过程延迟
	float fTime;		// 滚动开始时的播放进度
};

//*****************************常量*****************************//

//************工具栏工具提示字符串
const PCWSTR c_szBtmTip[] =
{
	L"上一曲",
	L"播放/暂停",
	L"停止",
	L"下一曲",
	L"歌词",
	L"循环方式",
	L"播放设置",
	L"显示/隐藏播放列表",
	L"设置",
	L"关于",
	L"循环方式：整体循环",
	L"循环方式：随机播放",
	L"循环方式：单曲播放",
	L"循环方式：单曲循环",
	L"循环方式：整体播放"
};
//************控件ID
#define IDC_BK_LEFT					100// 左背景
#define IDC_BK_RIGHTBTBK			101// 右按钮容器
#define IDC_BK_LIST					102// 右列表容器

#define IDC_ST_LISTNAME				110// 列表名称静态

#define IDC_LV_PLAY					120// 列表ListView

#define IDC_BT_JUMP					200
#define IDC_BT_OPEN					201
#define IDC_BT_LOADLIST				202
#define IDC_BT_SAVELIST				203
#define IDC_BT_EMPTY				204
#define IDC_BT_MORE					205
#define IDC_BT_SEARCH				206
#define IDC_BT_MANAGING				207

#define IDC_ED_SEARCH				130

#define IDC_SEB						140// 分隔条
//************任务栏工具条按钮
#define IDTBB_LAST					1000
#define IDTBB_PLAY					1001
#define IDTBB_NEXT					1002
//************菜单项ID
//****** "添加"
#define IDMI_OPEN_FILE				101
#define IDMI_OPEN_FOLDER			102
//****** "管理"
#define IDMI_TL_PLAY				110
#define IDMI_TL_DELETE_FROM_LIST	111
#define IDMI_TL_DELETE				112
#define IDMI_TL_OPEN_IN_EXPLORER	113
#define IDMI_TL_RENAME				114
#define IDMI_TL_INFO				115
#define IDMI_TL_ADDBOOKMARK			116
#define IDMI_TL_REMOVEBOOKMARK		117
#define IDMI_TL_NEXTBOOKMARK		118
#define IDMI_TL_LASTBOOKMARK		119
#define IDMI_TL_IGNORE				120
#define IDMI_TL_PLAYLATER			121
//******工具栏歌词按钮
#define IDMI_LRC_SHOW				120
#define IDMI_LRC_LRCSHOW			121
#define IDMI_LRC_FORCETWOLINES		122
//******滚动歌词右键菜单
#define IDMI_LS_PLAY				140
#define IDMI_LS_COPY				141
//****** "管理"
#define IDMI_LM_SORT_DEF			150
#define IDMI_LM_SORT_FILENAME		151
#define IDMI_LM_SORT_NAME			152
#define IDMI_LM_SORT_CTIME			153
#define IDMI_LM_SORT_MTIME			154
#define IDMI_LM_SORT_ASCENDING		155
#define IDMI_LM_SORT_DESCENDING		156
#define IDMI_LM_FIXSORT				157
#define IDMI_LM_DETAIL				158
#define IDMI_LM_BOOKMARK			159
#define IDMI_LM_SORT_REVERSE		160
#define IDMI_LM_SETLVDEFWIDTH		161
#define IDMI_LM_UPDATETIME			162
#define IDMI_LM_NOBOOKMARK			163
//******工具栏列表按钮
#define IDMI_PL_SEPARATE			170
#define IDMI_PL_SHOW				171
//************常量

//******频谱绘图
#define SPESTEP_BAR					7
#define SPESTEP_MAX					11
//******左背景消息
#define LEFTBKM_LRCSHOW_SETMAX			WM_USER + 1// (Max, 0)
#define LEFTBKM_LRCSHOW_REDRAWSB		WM_USER + 2// (bImmdShow, bIndependlyDrawing)
#define LEFTBKM_TOOLBAR_REDRAW			WM_USER + 3// (bImmdShow, bIndependlyDrawing)
#define LEFTBKM_TOOLBAR_INIT			WM_USER + 4// (0, 0)
#define LEFTBKM_TOOLBAR_GETREPEATMODE	WM_USER + 5// (0, 0)
#define LEFTBKM_TOOLBAR_DOBTOPE			WM_USER + 6// (0, 0)
#define LEFTBKM_PROGBAR_REDRAW			WM_USER + 7// (bImmdShow, bIndependlyDrawing)
#define LEFTBKM_PROGBAR_SETPOS			WM_USER + 8// (uPos, bRedraw)
#define LEFTBKM_PROGBAR_GETPOS			WM_USER + 9// (0, 0)
#define LEFTBKM_PROGBAR_SETMAX			WM_USER + 10// (uMax, bRedraw)
#define LEFTBKM_PROGBAR_GETMAX			WM_USER + 11// (0, 0)
//******主窗口消息
#define MAINWNDM_AUTONEXT				WM_USER + 1// (0, 0)
//******循环模式
#define REPEATMODE_TOTALLOOP	0// 整体循环
#define REPEATMODE_RADOM		1// 随机播放
#define REPEATMODE_SINGLE		2// 单曲播放
#define REPEATMODE_SINGLELOOP	3// 单曲循环
#define REPEATMODE_TOTAL		4// 整体播放
//******工具栏按钮状态
#define BTMBT_NORMAL			0
#define BTMBT_HOT				1
#define BTMBT_PUSHED			2
//******工具栏按钮数目
#define BTMBKBTNCOUNT			10
//******播放/暂停标志
#define POPF_PLAYICON			1// 应显示播放图标
#define POPF_PAUSEICON			2// 应显示暂停图标
#define POPF_AUTOSWITCH			3// 自动转换状态
//*****************************函数*****************************//
/*
* 目标：清除当前信息
*
* 参数：
*
* 返回值：
* 备注：
*/
void MainWnd_ReleaseCurrInfo();
/*
* 目标：播放完成同步过程
*
* 参数：
*
* 返回值：
* 备注：回调
*/
void CALLBACK SyncProc_End(HSYNC handle, DWORD channel, DWORD data, void* user);
/*
 * 目标：画一行歌词
 *
 * 参数：
 * iIndex 歌词索引
 * y 起始y坐标（顶边或底边），设为-1以使用上次绘画时的顶边，此时bTop参数无效
 * bTop 是否为顶边
 * bClearBK 是否擦除背景
 * bImmdShow 立即显示，若为FALSE，则需另外将后台位图拷贝到前台，设为TRUE，则自动剪辑歌词区域
 *
 * 返回值：返回已绘制的歌词高度
 * 备注：必须保证索引合法；自动处理强制双行设置；自动处理热点和选中项目；擦除背景时仅擦除歌词矩形
 */
int UI_DrawLrcItem(int iIndex, int y, BOOL bTop, BOOL bClearBK, BOOL bImmdShow, BOOL bCenterLine = FALSE, int* yOut = NULL);
/*
 * 目标：按索引播放列表中的文件
 *
 * 参数：
 * iIndex LV索引
 *
 * 返回值：
 * 操作简述：
 * 备注：
 */
void Playing_PlayFile(int iIndex);
void Playing_Stop(BOOL bNoGap = FALSE);
void Playing_PlayOrPause(UINT uFlag = POPF_AUTOSWITCH);
void Playing_PlayNext(BOOL bReverse = FALSE);
void Playing_AutoNext();
void StopThread_Waves();
UINT __stdcall Thread_GetWavesData(void* p);
/*
 * 目标：更新左侧内存位图并显示
 *
 * 参数：
 *
 * 返回值：
 * 操作简述：
 * 备注：
 */
void UI_UpdateLeftBK();
void UI_SeparateListWnd(BOOL b);
void UI_ShowList(BOOL b);
LRESULT CALLBACK WndProc_TBGhost(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam);
LRESULT CALLBACK WndProc(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam);
LRESULT CALLBACK WndProc_LeftBK(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam);
INT_PTR CALLBACK DlgProc_About(HWND hDlg, UINT uMsg, WPARAM wParam, LPARAM lParam);
void CALLBACK TimerProc(HWND hWnd, UINT uMsg, UINT_PTR idEvent, DWORD dwTime);
LRESULT CALLBACK QKCProc_SEB(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam);
int HitTest_LrcShow(POINT pt);
int HitTest_BtmBK(POINT pt);
void UI_VEDrawAlbum(BOOL bImmdShow = TRUE, BOOL bIndependlyDrawing = TRUE);
BOOL UI_VEDrawWaves(BOOL bImmdShow = TRUE, BOOL bIndependlyDrawing = TRUE);
void UI_VEDrawSpe(BOOL bImmdShow = TRUE, BOOL bIndependlyDrawing = TRUE);
void UI_VEDrawLrc(int yCenter, BOOL bImmdShow = TRUE, BOOL bIndependlyDrawing = TRUE);
BOOL UI_VEProcLrcShowing(BOOL bImmdShow = TRUE, BOOL bIndependlyDrawing = TRUE, BOOL bOnlyDrawDTLrc = FALSE);
void UI_RefreshBmpBrush();
void SettingsUpd_WndMain();