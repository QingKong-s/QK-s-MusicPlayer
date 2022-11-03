#pragma once
#include <Windows.h>
#include <Shobjidl.h>
#include <ole2.h>
#include <shlwapi.h>
#include <wincodec.h>
#include <d2d1.h>
#include <d2d1_1.h>

#include "bass.h"
#define CURRVER L"0.3"
////////////////////GDI+Flat����
enum GpStatus 
{
	Ok,
	GenericError,
	InvalidParameter,
	OutOfMemory,
	ObjectBusy,
	InsufficientBuffer,
	NotImplemented,
	Win32Error,
	WrongState,
	Aborted,
	FileNotFound,
	ValueOverflow,
	AccessDenied,
	UnknownImageFormat,
	FontFamilyNotFound,
	FontStyleNotFound,
	NotTrueTypeFont,
	UnsupportedGdiplusVersion,
	GdiplusNotInitialized,
	PropertyNotFound,
	PropertyNotSupported,
	ProfileNotFound
};
enum SmoothingMode 
{
	SmoothingModeDefault,
	SmoothingModeHighSpeed,
	SmoothingModeHighQuality,
	SmoothingModeNone,
};
enum TextRenderingHint 
{
	TextRenderingHintSystemDefault,
	TextRenderingHintSingleBitPerPixelGridFit,
	TextRenderingHintSingleBitPerPixel,
	TextRenderingHintAntiAliasGridFit,
	TextRenderingHintAntiAlias,
	TextRenderingHintClearTypeGridFit
};
struct GdiplusStartupInput 
{
	UINT32         GdiplusVersion;
	void*		   DebugEventCallback;
	BOOL           SuppressBackgroundThread;
	BOOL           SuppressExternalCodecs;
};
enum Unit 
{
	UnitWorld,
	UnitDisplay,
	UnitPixel,
	UnitPoint,
	UnitInch,
	UnitDocument,
	UnitMillimeter
};
enum StringFormatFlags 
{
	StringFormatFlagsDirectionRightToLeft		= 0x00000001,
	StringFormatFlagsDirectionVertical			= 0x00000002,
	StringFormatFlagsNoFitBlackBox				= 0x00000004,
	StringFormatFlagsDisplayFormatControl		= 0x00000020,
	StringFormatFlagsNoFontFallback				= 0x00000400,
	StringFormatFlagsMeasureTrailingSpaces		= 0x00000800,
	StringFormatFlagsNoWrap						= 0x00001000,
	StringFormatFlagsLineLimit					= 0x00002000,
	StringFormatFlagsNoClip						= 0x00004000,
	StringFormatFlagsBypassGDI					= 0x80000000
};
struct RectF 
{
	float Left;
	float Top;
	float Width;
	float Height;
};
struct PointF 
{
	float x;
	float y;
};
struct BlurParams 
{
	float radius;
	BOOL  expandEdge;
};
enum FillMode 
{
	FillModeAlternate,
	FillModeWinding
};
enum WrapMode 
{
	WrapModeTile,
	WrapModeTileFlipX,
	WrapModeTileFlipY,
	WrapModeTileFlipXY,
	WrapModeClamp
};
enum FontStyle 
{
	FontStyleRegular,
	FontStyleBold,
	FontStyleItalic,
	FontStyleBoldItalic,
	FontStyleUnderline,
	FontStyleStrikeout
};
enum StringAlignment 
{
	StringAlignmentNear,
	StringAlignmentCenter,
	StringAlignmentFar
};
typedef void GpGraphics;
typedef void GpFontCollection;
typedef void GpFontFamily;
typedef void GpFont;
typedef void GpStringFormat;
typedef void GpPen;
typedef void GpPath;
typedef void GpSolidFill;
typedef void GpBrush;
typedef void GpLineGradient;
typedef void GpBrush;
typedef void GpBrush;
typedef void GpImage;
typedef void GpImageAttributes;
typedef void GpEffect;
typedef void GpBitmap;
typedef void* DrawImageAbort;
typedef float REAL;
typedef DWORD ARGB;
typedef int PixelFormat;//Gdipluspixelformats.h
#define WINGDIPAPI WINAPI
#define GDIPCONST const
#define PixelFormat32bppRGB 139273
#define PixelFormat32bppArgb 2498570
extern "C" {//extern "C"��ʹ��C�������ӷ�ʽ����ֹ���Ʒ��飬�����˴�gdiplus.lib
	GpStatus WINGDIPAPI	GdiplusStartup(ULONG_PTR* token, const GdiplusStartupInput* input, void* output);
	void	 WINGDIPAPI	GdiplusShutdown(ULONG_PTR token);
	GpStatus WINGDIPAPI	GdipCreateFromHDC(HDC hdc, GpGraphics** graphics);
	GpStatus WINGDIPAPI	GdipSetSmoothingMode(GpGraphics* graphics, SmoothingMode smoothingMode);
	GpStatus WINGDIPAPI GdipSetTextRenderingHint(GpGraphics* graphics, TextRenderingHint mode);
	GpStatus WINGDIPAPI	GdipCreateFontFamilyFromName(GDIPCONST WCHAR* name, GpFontCollection* fontCollection, GpFontFamily** FontFamily);
	//��������һ��FontStyleö��ֵ��λ�����:
	GpStatus WINGDIPAPI GdipCreateFont(GDIPCONST GpFontFamily* fontFamily, REAL emSize, INT style, Unit unit, GpFont** font);
	//����һ��һ��StringFormatFlagsö�٣�Ĭ��ֵΪ0��������������ID��Ĭ��ֵΪLANG_NEUTRAL:
	GpStatus WINGDIPAPI GdipCreateStringFormat(INT formatAttributes, LANGID language, GpStringFormat** format);
	GpStatus WINGDIPAPI GdipSetStringFormatAlign(GpStringFormat* format, StringAlignment align);
	GpStatus WINGDIPAPI GdipCreatePen1(ARGB color, REAL width, Unit unit, GpPen** pen);
	GpStatus WINGDIPAPI GdipMeasureString(GpGraphics* graphics, GDIPCONST WCHAR* string, INT length, GDIPCONST GpFont* font,
		GDIPCONST RectF* layoutRect, GDIPCONST GpStringFormat* stringFormat, RectF* boundingBox, INT* codepointsFitted, INT* linesFilled);
	GpStatus WINGDIPAPI GdipCreatePath(FillMode brushMode, GpPath** path);
	//��������һ��FontStyleö��ֵ��λ�����:
	GpStatus WINGDIPAPI GdipAddPathString(GpPath* path, GDIPCONST WCHAR* string, INT length, GDIPCONST GpFontFamily* family,
		INT style, REAL emSize, GDIPCONST RectF* layoutRect, GDIPCONST GpStringFormat* format);
	GpStatus WINGDIPAPI GdipCreateSolidFill(ARGB color, GpSolidFill** brush);
	GpStatus WINGDIPAPI GdipFillPath(GpGraphics* graphics, GpBrush* brush, GpPath* path);
	GpStatus WINGDIPAPI GdipDrawPath(GpGraphics* graphics, GpPen* pen, GpPath* path);
	GpStatus WINGDIPAPI GdipCreateLineBrush(GDIPCONST PointF* point1, GDIPCONST PointF* point2, ARGB color1, ARGB color2,
		WrapMode wrapMode, GpLineGradient** lineGradient);
	GpStatus WINGDIPAPI GdipFillRectangle(GpGraphics* graphics, GpBrush* brush, REAL x, REAL y, REAL width, REAL height);
	GpStatus WINGDIPAPI GdipGraphicsClear(GpGraphics* graphics, ARGB color);
	GpStatus WINGDIPAPI GdipDrawRectangle(GpGraphics* graphics, GpPen* pen, REAL x, REAL y, REAL width, REAL height);
	GpStatus WINGDIPAPI GdipResetPath(GpPath* path);
	GpStatus WINGDIPAPI GdipLoadImageFromStream(IStream* stream, GpImage** image);
	GpStatus WINGDIPAPI GdipGetImageHeight(GpImage* image, UINT* height);
	GpStatus WINGDIPAPI GdipGetImageWidth(GpImage* image, UINT* width);
	GpStatus WINGDIPAPI GdipDrawImageRectRect(GpGraphics* graphics, GpImage* image, REAL dstx, REAL dsty, REAL dstwidth, REAL dstheight,
		REAL srcx, REAL srcy, REAL srcwidth, REAL srcheight, Unit srcUnit, GDIPCONST GpImageAttributes* imageAttributes,
		DrawImageAbort callback, VOID* callbackData);
	GpStatus WINGDIPAPI GdipDisposeImage(GpImage* image);
	GpStatus WINGDIPAPI GdipDeletePath(GpPath* path);
	GpStatus WINGDIPAPI GdipDeletePen(GpPen* pen);
	GpStatus WINGDIPAPI GdipDeleteBrush(GpBrush* brush);
	GpStatus WINGDIPAPI GdipDeleteGraphics(GpGraphics* graphics);
	GpStatus WINGDIPAPI GdipDeleteFont(GpFont* font);
	GpStatus WINGDIPAPI GdipDeleteStringFormat(GpStringFormat* format);
	GpStatus WINGDIPAPI GdipDeleteFontFamily(GpFontFamily* fontFamily);
	GpStatus WINGDIPAPI GdipCreateEffect(const GUID guid, GpEffect** effect);
	GpStatus WINGDIPAPI GdipDeleteEffect(GpEffect* effect);
	GpStatus WINGDIPAPI GdipCreateBitmapFromHBITMAP(HBITMAP hbm, HPALETTE hpal, GpBitmap** bitmap);
	GpStatus WINGDIPAPI GdipSetEffectParameters(GpEffect* effect, const VOID* params, const UINT size);
	GpStatus WINGDIPAPI GdipCreateBitmapFromGraphics(INT width, INT height, GpGraphics* target, GpBitmap** bitmap);
	GpStatus WINGDIPAPI GdipBitmapApplyEffect(GpBitmap* bitmap, GpEffect* effect, RECT* roi, BOOL useAuxData, VOID** auxData, INT* auxDataSize);
	GpStatus WINGDIPAPI GdipCreateBitmapFromStream(IStream* stream, GpBitmap** bitmap);
	GpStatus WINGDIPAPI GdipCloneBitmapArea(REAL x, REAL y, REAL width, REAL height, PixelFormat format, GpBitmap* srcBitmap, GpBitmap** dstBitmap);
	GpStatus WINGDIPAPI GdipGetImageGraphicsContext(GpImage* image, GpGraphics** graphics);
	GpStatus WINGDIPAPI GdipCreateBitmapFromScan0(INT width, INT height, INT stride, PixelFormat format, BYTE* scan0, GpBitmap** bitmap);
	GpStatus WINGDIPAPI GdipDrawRectangle(GpGraphics* graphics, GpPen* pen, REAL x, REAL y, REAL width, REAL height);
	GpStatus WINGDIPAPI GdipLoadImageFromFile(GDIPCONST WCHAR* filename, GpImage** image);
	GpStatus WINGDIPAPI GdipCreateHBITMAPFromBitmap(GpBitmap* bitmap, HBITMAP* hbmReturn, ARGB background);
	GpStatus WINGDIPAPI GdipDrawImage(GpGraphics* graphics, GpImage* image, REAL x, REAL y);
	GpStatus WINGDIPAPI GdipCreateHICONFromBitmap(GpBitmap* bitmap, HICON* hbmReturn);
	GpStatus WINGDIPAPI GdipCreateBitmapFromHICON(HICON hicon, GpBitmap** bitmap);
	GpStatus WINGDIPAPI GdipStringFormatGetGenericDefault(GpStringFormat** format);
	GpStatus WINGDIPAPI GdipDrawString(GpGraphics* graphics, GDIPCONST WCHAR* string, INT length, GDIPCONST GpFont* font,
		GDIPCONST RectF* layoutRect, GDIPCONST GpStringFormat* stringFormat, GDIPCONST GpBrush* brush);
	GpStatus WINGDIPAPI GdipSetStringFormatLineAlign(GpStringFormat* format, StringAlignment align);
}
////////////////////�ṹ


struct DRAWING_TIME		//�ӳ�����
{
	int i;				// �߶�
	BOOL bbool;			// ʱ���Ƿ�������
	ULONG uTime;		// ʱ��
};

struct DLGRESULT_LIST
{
	WCHAR szFileName[MAX_PATH];
};

struct MUSICINFO
{
	PWSTR pszTitle;
	PWSTR pszArtist;
	PWSTR pszAlbum;
	PWSTR pszComment;
	IWICBitmap* pWICBitmap;
	PWSTR pszLrc;
};

struct CURRMUSICINFO
{
	PWSTR pszName;
	ID2D1Bitmap1* pD2DBmpOrgAlbum;
	ID2D1BitmapBrush* pD2DBrushOrgAlbum;
	MUSICINFO mi;
};


//////////////////////////////����
////////////////////SetPropW
#define PROP_WNDPROC			L"QKProp.WndProc"

#define PROP_DTLRCFONTSIZE		L"QKProp.Settings.DTLrcFontSize"
#define PROP_DTLRCFONTWEIGHT	L"QKProp.Settings.DTLrcFontWeight"
#define PROP_DTLRCCLR1			L"QKProp.Settings.DTLrcColor1"
#define PROP_DTLRCCLR2			L"QKProp.Settings.DTLrcColor2"

#define PROP_SCLRCFONTSIZE		L"QKProp.Settings.SCLrcFontSize"
#define PROP_SCLRCFONTWEIGHT	L"QKProp.Settings.SCLrcFontWeight"
#define PROP_SCLRCCLR1			L"QKProp.Settings.SCLrcColor1"
#define PROP_SCLRCCLR2			L"QKProp.Settings.SCLrcColor2"
#define PROP_DTLRCBORDERCLR		L"QKProp.Settings.DTLrcBorderColor"
////////////////////�̹߳�����־
#define THREADFLAG_STOP			1
#define THREADFLAG_STOPED		2
#define THREADFLAG_WORKING		3
#define THREADFLAG_ERROR		4
////////////////////Ƶ������
#define SPECOUNT				40
////////////////////��ʱ�־�������ʴ����ã�
#define LRCSTATE_STOP			1
#define LRCSTATE_LOADING		2
#define LRCSTATE_NOLRC			3
#define LRCSTATE_NORMAL			4
////////////////////��ɫ
#define MYCLR_LISTGRAY			0xF3F3F3
#define MYCLR_LISTPLAYING		0xE6E8B1
#define MYCLR_LISTGROUP			0xBDFAFF
#define MYCLR_BTHOT				0xE6E8B1
#define MYCLR_BTPUSHED			0xE6E88C
#define MYCLR_TBTRACK			0xCECECE
////////////////////��������
#define MAINWNDCLASS			L"QKPlayer.WndClass.Main"
#define BKWNDCLASS				L"QKPlayer.WndClass.BK"
#define LRCWNDCLASS				L"QKPlayer.WndClass.Lrc"
#define TBGHOSTWNDCLASS			L"QKPlayer.WndClass.TaskbarGhost"
#define WNDCLASS_LIST			L"QKPlayer.WndClass.List"
////////////////////��������
#define ECODESRC_NONE			0
#define ECODESRC_BASS			1
#define ECODESRC_WINSDK			2
#define ECODESRC_OTHERS			3
////////////////////��ʱ��
#define IDT_PGS						101
#define IDT_DRAWING_WAVES			102
#define IDT_DRAWING_VU     			103
#define IDT_DRAWING_LRC				104
#define IDT_VU_SPE_TIME				105
#define IDT_LRC						106
#define IDT_DRAWING_SPE				107
#define IDT_LRCSCROLL				108
#define IDT_ANIMATION				109
#define IDT_ANIMATION2				110
#define IDT_LISTBKDRAG				111

#define TIMERELAPSE_PGS				20
#define TIMERELAPSE_WAVES			20
#define TIMERELAPSE_VU_SPE			50
#define TIMERELAPSE_LRC				200
#define TIMERELAPSE_VU_SPE_TIME		500
#define TIMERELAPSE_LRCWND			600
#define TIMERELAPSE_TIME			500
#define TIMERELAPSE_LRCSCROLL		1000
#define TIMERELAPSE_ANIMATION		60
#define TIMERELAPSE_ANIMATION2		30
#define TIMERELAPSE_LISTBKDRAG		800
////////////////////���ú�������Χ�ļ�
#define MAXPROFILEBUFFER			48
#define PROFILENAME					L"\\Data\\QKPlayerConfig.ini"
#define DEFPICFILENAME				L"\\Data\\DefPic.png"
#define DATADIR						L"\\Data\\"
#define LISTDIR						L"\\List\\"

#define PPF_SECTIONLRC				L"Lyric"
#define PPF_SECTIONVISUAL			L"Visual"
#define PPF_SECTIONPLAYING			L"Playing"

#define PPF_KEY_DEFTEXTCODE			L"DefTextCode"
#define PPF_KEY_LRCDIR				L"LyricsDir"
#define PPF_KEY_DISABLEVANIMATION	L"DisableVAnimation"
#define PPF_KEY_DISABLEWORDBREAK	L"DisableWordBreak"
#define PPF_KEY_DISABLEDTLRCSHANDOW L"DisableDTLrcShandow"
#define PPF_KEY_DTFONTNAME			L"DTLrcFontName"
#define PPF_KEY_DTFONTSIZE			L"DTLrcFontSize"
#define PPF_KEY_DTFONTWEIGHT		L"DTLrcFontWeight"
#define PPF_KEY_DTLRCCLR1			L"DTLrcClr1"
#define PPF_KEY_DTLRCCLR2			L"DTLrcClr2"
#define PPF_KEY_DTLRCTRANSPARENT	L"DTLrcTransparent"
#define PPF_KEY_DTLRCSPACELINE		L"DTLrcSpaceLine"
#define PPF_KEY_SCFONTNAME			L"SCLrcFontName"
#define PPF_KEY_SCFONTSIZE			L"SCLrcFontSize"
#define PPF_KEY_SCFONTWEIGHT		L"SCLrcFontWeight"
#define PPF_KEY_SCLRCCLR1			L"SCLrcClr1"
#define PPF_KEY_SCLRCCLR2			L"SCLrcClr2"
#define PPF_KEY_SCLINEGAP			L"SCLrcLineGap"
#define PPF_KEY_SCALIGN				L"SCLrcAlign"
#define PPF_KEY_SCOFFSET			L"SCLrcOffset"
#define PPF_KEY_VISUALMODE			L"VisualMode"
#define PPF_KEY_HSCROLLTEXT			L"HScrollText"
#define PPF_KEY_ALBUMPICSIZE1		L"AlbumPicSize1"
#define PPF_KEY_ALBUMPICSIZE2		L"AlbumPicSize2"
#define PPF_KEY_DTLRCBORDERCLR		L"DTLrcBorderClr"
#define PPF_KEY_ENABLEDTLRCBORDER	L"EnableDTLrcBorder"
#define PPF_KEY_SOUNDFONT			L"MIDISoundFont"
////////////////////��Ƶ�ļ�����
#define MUSICTYPE_NORMAL			0
#define MUSICTYPE_MOD				1
#define MUSICTYPE_MIDI				2

struct GLOBALRES
{
	HBITMAP hbmpNewItem;
	HICON hiLocate;			// ��λ
	HICON hiPlus;			// ���
	HICON hiReadFile;		// ��ȡ�б�
	HICON hiSaveFile;		// �����б�
	HICON hiCross;			// ���
	HICON hiSearch;			// ����
	HICON hiPlaySetting;	// ��������
	HICON hiPlayList;		// ��ʾ/���ز����б�
	HICON hiArrowCross;		// �������
	HICON hiArrowRight;		// ��������
	HICON hiArrowRightThree;// ���岥��
	HICON hiArrowCircleOne;	// ����ѭ��
	HICON hiArrowCircle;	// ����ѭ��
	HICON hiLast;			// ��һ��
	HICON hiPlay;			// ����
	HICON hiPause;			// ��ͣ
	HICON hiStop;			// ֹͣ
	HICON hiNext;			// ��һ��
	HICON hiLrc;			// ���
	HICON hiSettings;		// ����
	HICON hiInfo;			// ��Ȧס��"i"
	HICON hiListManaging;	// �����б����
	HICON hiLast2;
	HICON hiPlay2;
	HICON hiPause2;
	HICON hiNext2;
	HICON hiCross2;
	HICON hiTick;
	HICON hiTick2;
	HICON hiSpeed;
	HICON hiBlance;
	HICON hiVol;
	HICON hiSlient;
	HICON hiTempo;
};

struct GLOBALCONTEXT
{
	HBRUSH hbrCyanDeeper;	// ������ˢ
	HBRUSH hbrMyBule;
	UINT uLrcDTFlags;		// ǿ��˫�б�־
	int DS_CYPROGBAR;		// �������߶�
	int DS_CYPROGBARCORE;

	int DS_CYBTBK;			// 
	int DS_CYSPE;			// 
	int DS_CYSPEHALF;
	int DS_CXSPEBAR;		// 
	int DS_CXSPEBARDIV;		//
	int DS_CXSPE;
	int DS_CXSPEHALF;
	int DS_CXBTMBTBK;

	int DS_CXPIC;			// 
	int DS_EDGE;			// �����
	int DS_CYTOPBK;			// ������Ϣ�߶�

	int DS_BT;
	int DS_CXRITBT;			// �Ҳఴť���
	int DS_GAP;				// ������


	int DS_LARGEIMAGE;		// ����ͼƬ�ߴ�

	int DS_CYRITBK;			

	int DS_CYSTLISTNAME;	// �Ҳ��б����ƾ�̬�߶�
	int DS_CXTIME;			// ������ʾ���
	int DS_CXWAVESLINE;		// ÿ��λ���ο��
	int DS_CYTOPTITLE;	
	int DS_CXTOPTIP;
	int DS_CYTOPTIP;
	int DS_GAPTOPTIP;
	int DS_CXABOUTPIC;
	int DS_CYABOUTPIC;
	int DS_LVTEXTSPACE;
	int DS_CXLRCTB;
	int DS_DEFCXLV;
	int DS_DTLRCEDGE;
	int DS_STDICON;
	int DS_DTLRCFRAME;
	int DS_CXDTLRCBTNRGN;
	int DS_CYLVITEM;
	int DS_CXDRAGDROPICON;
	int DS_CYDRAGDROPICON;
	int DS_LVDRAGEDGE;
	int DS_ALBUMLEVEL;

	int cyBT;			// �ײ������߶�
	int cxBKBtm;			// �ײ��������
	int iIconSize;			// ͼ���С
};
#define SIZE_CYPROGBAR			16
#define SIZE_CYPROGBARCORE		6
#define SIZE_CYBTBK				40
#define SIZE_CYSPE				80
#define SIZE_CYSPEHALF			SIZE_CYSPE / 2
#define SIZE_CXSPEBAR			5
#define SIZE_CXSPEBARDIV		1
#define SIZE_CXSPE			    200
#define SIZE_CXSPEHALF			SIZE_CXSPE / 2
#define SIZE_CXBTMBTBK			430
#define SIZE_CXPIC				200//��ʾ�����ʱ
#define SIZE_EDGE				20
#define SIZE_CYTOPBK			90
#define SIZE_BT					35
#define SIZE_CXRITBT			58
#define SIZE_GAP				4
#define SIZE_LARGEIMAGE			2000
#define SIZE_CYRITBK			110
#define SIZE_CYSTLISTNAME		22
#define SIZE_CXTIME				100
#define SIZE_CXWAVESLINE		2
#define SIZE_CYTOPTITLE			30
#define SIZE_CXTOPTIP			50
#define SIZE_CYTOPTIP			18
#define SIZE_GAPTOPTIP			6
#define SIZE_CXABOUTPIC			300
#define SIZE_CYABOUTPIC			148
#define SIZE_LVTEXTSPACE		5
#define SIZE_CXLRCTB			20
#define SIZE_DEFCXLV			372
#define SIZE_DTLRCEDGE			2
#define SIZE_STDICON			29
#define SIZE_DTLRCFRAME			8
#define SIZE_CYLVITEM			22
#define SIZE_CXDRAGDROPICON		200
#define SIZE_CYDRAGDROPICON		150
#define SIZE_LVDRAGEDGE			80
#define SIZE_ALBUMLEVEL			15

#define DPIS_CYPROGBAR GC.DS_CYPROGBAR
#define DPIS_CYBTBK GC.DS_CYBTBK
#define DPIS_CYSPE GC.DS_CYSPE
#define DPIS_CYSPEHALF GC.DS_CYSPEHALF
#define DPIS_CXSPEBAR GC.DS_CXSPEBAR
#define DPIS_CXSPEBARDIV GC.DS_CXSPEBARDIV
#define DPIS_CXSPE GC.DS_CXSPE
#define DPIS_CXSPEHALF GC.DS_CXSPEHALF
#define DPIS_CXBTMBTBK GC.DS_CXBTMBTBK
#define DPIS_CXPIC GC.DS_CXPIC
#define DPIS_EDGE GC.DS_EDGE
#define DPIS_CYTOPBK GC.DS_CYTOPBK
#define DPIS_BT GC.DS_BT
#define DPIS_CXRITBT GC.DS_CXRITBT
#define DPIS_GAP GC.DS_GAP
#define DPIS_LARGEIMAGE GC.DS_LARGEIMAGE
#define DPIS_CYRITBK GC.DS_CYRITBK
#define DPIS_CYSTLISTNAME GC.DS_CYSTLISTNAME
#define DPIS_CXTIME GC.DS_CXTIME
#define DPIS_CXWAVESLINE GC.DS_CXWAVESLINE
#define DPIS_CYTOPTITLE GC.DS_CYTOPTITLE
#define DPIS_CXTOPTIP GC.DS_CXTOPTIP
#define DPIS_CYTOPTIP GC.DS_CYTOPTIP
#define DPIS_GAPTOPTIP GC.DS_GAPTOPTIP
#define DPIS_CXABOUTPIC GC.DS_CXABOUTPIC
#define DPIS_CYABOUTPIC GC.DS_CYABOUTPIC
#define DPIS_LVTEXTSPACE GC.DS_LVTEXTSPACE
#define DPIS_CXLRCTB GC.DS_CXLRCTB
#define DPIS_DEFCXLV GC.DS_DEFCXLV
#define DPIS_DTLRCEDGE GC.DS_DTLRCEDGE
#define DPIS_STDICON GC.DS_STDICON
#define DPIS_DTLRCFRAME GC.DS_DTLRCFRAME
#define DPIS_CXDTLRCBTNRGN GC.DS_CXDTLRCBTNRGN
#define DPIS_CYLVITEM GC.DS_CYLVITEM
#define DPIS_CXDRAGDROPICON GC.DS_CXDRAGDROPICON
#define DPIS_CYDRAGDROPICON GC.DS_CYDRAGDROPICON
struct SETTINGS
{
	BOOL bForceTwoLines;
	BOOL bAscending;
	BOOL bNoBookMarkWhenSort;
	BOOL bLrcAnimation;
	BOOL bDTLrcShandow;
	UINT uDefTextCode;
	PWSTR pszLrcDir;
	PWSTR pszDTLrcFontName;
	UINT uDTLrcFontSize;
	UINT uDTLrcFontWeight;
	UINT uDTLrcTransparent;
	UINT crDTLrc1;
	UINT crDTLrc2;
	PWSTR pszDTLrcSpaceLine;

	PWSTR pszSCLrcFontName;
	UINT uSCLrcFontSize;
	UINT uSCLrcFontWeight;
	UINT uSCLrcTransparent;
	UINT crSCLrc1;
	UINT crSCLrc2;
	UINT uSCLrcAlign;
	UINT uSCLrcLineGap;
	UINT uSCLrcOffset;

	UINT uVisualMode;
	BOOL bHScrollText;

	UINT uAlbumPicSize1;
	UINT uAlbumPicSize2;

	UINT crDTLrcBorder;
	BOOL bEnableDTLrcBorder;

	PWSTR pszSoundFont;
};

const D2D_COLOR_F c_D2DClrCyanDeeper = { 0.f,0.3764f,0.7529f,1.f };// ����������

#define DPI(i) (i * g_iDPI / USER_DEFAULT_SCREEN_DPI)					// ���ߴ簴DPI�Ŵ�
#define DPIF(f) (f * (float)g_iDPI / (float)USER_DEFAULT_SCREEN_DPI)	// ���ߴ簴DPI�Ŵ󣨸���棩
#define RDPI(i) (i * USER_DEFAULT_SCREEN_DPI / g_iDPI)					// ���ߴ簴DPI��С
#define SAFE_RELEASE(p) if(p){p->Release();p=NULL;}						// ��ȫ�ͷŽӿ�

ULONG_PTR BASS_OpenMusic(PWSTR pszFile, DWORD dwFlagsHS = 0, DWORD dwFlagsHM = 0, DWORD dwFlagsHMIDI = 0);
BOOL BASS_FreeMusic(ULONG_PTR h);
void UI_UpdateDPISize();
void Res_Free();
void Res_Load(int iSize);

void Global_ShowError(PCWSTR pszTitle, PCWSTR pszContent, int iErrCodeSrc = ECODESRC_NONE, HWND hParent = NULL, DWORD dwOtherErrCode = 0);

typedef UINT(__stdcall* pFuncGetDpiForSystem)(void);
typedef UINT(__stdcall* pFuncGetDpiForWindow)(HWND);