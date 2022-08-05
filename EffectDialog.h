/*
* EffectDialog.h
* ��������Ч���Ի�����̼�����Ч����غ����Ķ���
*/
#pragma once
#include <Windows.h>

#include "bass.h"

struct TPARAM_FILLTIMECLM
{
	BOOL bJudgeItem;
};

struct GLOBALEFFECT
{
	HFX hFXChorus;
	BASS_DX8_CHORUS Chorus;
	HFX hFXCompressor;
	BASS_DX8_COMPRESSOR Compressor;
	HFX hFXDistortion;
	BASS_DX8_DISTORTION Distortion;
	HFX hFXEcho;
	BASS_DX8_ECHO Echo;
	HFX hFXFlanger;
	BASS_DX8_FLANGER Flanger;
	HFX hFXGargle;
	BASS_DX8_GARGLE Gargle;
	HFX hFXI3DL2Reverb;
	BASS_DX8_I3DL2REVERB I3DL2Reverb;
	HFX hFXReverb;
	BASS_DX8_REVERB Reverb;
	HFX hFXEQ[10];
	BASS_DX8_PARAMEQ EQ[10];
};

struct
{
	PCWSTR pszText;
	int Setting[10];
}
const c_EQSetting[23] =
{
	{L"��",			{0,0,0,0,0,0,0,0,0,0}},
	{L"ACG",		{4,6,3,0,0,2,5,1,1,4}},
	{L"��ҥ",		{0,3,0,0,1,4,5,3,0,2}},
	{L"����",		{6,4,6,2,0,0,0,0,0,0}},
	{L"����&����",	{6,5,6,1,0,0,1,3,4,0}},
	{L"����",		{2,6,4,0,-2,-1,2,2,1,3}},
	{L"�ŷ�",		{4,2,2,0,-1,3,4,1,1,3}},
	{L"�ŵ�",		{4,4,3,2,-1,-1,0,1,3,4}},
	{L"���",		{0,2,3,0,0,2,3,1,0,0}},
	{L"����",		{4,5,7,0,1,3,4,4,3,0}},
	{L"����",		{2,0,0,0,0,0,0,0,-1,-1}},
	{L"����",		{5,6,5,0,-1,1,0,1,4,3}},
	{L"�ؽ���",		{-2,5,4,-2,-2,-1,2,3,1,4}},
	{L"˵��",		{5,5,4,0,-2,1,3,0,3,4}},
	{L"��ʿ",		{3,3,1,2,-1,-1,0,1,2,4}},
	{L"�ֳ�",		{5,5,6,0,-1,0,3,4,6,5}},
	{L"����",		{-2,-3,-3,0,1,4,3,2,-1,-2}},
	{L"����",		{-1,-1,0,1,4,3,1,0,-1,1}},
	{L"ҡ��",		{4,3,3,1,0,-1,0,1,2,4}},
	{L"���",		{0,0,0,0,0,0,0,-2,-2,0}},
	{L"��͵���",	{3,2,1,0,0,0,0,-2,-2,-2}},
	{L"��͸���",	{-3,-1,0,0,0,0,0,-1,3,2}},
	{L"���ص���",	{6,5,8,2,0,0,0,0,0,0}},
};

const float c_EQCenter[10] = { 31,62,125,250,500,1000,2000,4000,8000,16000 };

#define EFFECTWNDTABCOUNT		10
#define SBV_INVALIDVALUE		-2

#define EFFECT_CHORUS		0x00000001
#define EFFECT_COMPRESSOR	0x00000002
#define EFFECT_DISTORTION	0x00000004
#define EFFECT_ECHO			0x00000008
#define EFFECT_FLANGER		0x00000010
#define EFFECT_GARGLE		0x00000020
#define EFFECT_I3DL2REVERB	0x00000040
#define EFFECT_REVERB		0x00000080
#define EFFECT_EQ			0x00000100
#define EFFECT_ALL			0xFFFFFFFF

void GlobalEffect_ResetToDefault(UINT i);

INT_PTR CALLBACK DlgProc_EQ(HWND hDlg, UINT message, WPARAM wParam, LPARAM lParam);
INT_PTR CALLBACK DlgProc_SBV(HWND hDlg, UINT message, WPARAM wParam, LPARAM lParam);
INT_PTR CALLBACK DlgProc_Chorus(HWND hDlg, UINT message, WPARAM wParam, LPARAM lParam);
INT_PTR CALLBACK DlgProc_Compressor(HWND hDlg, UINT message, WPARAM wParam, LPARAM lParam);
INT_PTR CALLBACK DlgProc_Distortion(HWND hDlg, UINT message, WPARAM wParam, LPARAM lParam);
INT_PTR CALLBACK DlgProc_Echo(HWND hDlg, UINT message, WPARAM wParam, LPARAM lParam);
INT_PTR CALLBACK DlgProc_Flanger(HWND hDlg, UINT message, WPARAM wParam, LPARAM lParam);
INT_PTR CALLBACK DlgProc_Gargle(HWND hDlg, UINT message, WPARAM wParam, LPARAM lParam);
INT_PTR CALLBACK DlgProc_I3DL2Reverb(HWND hDlg, UINT message, WPARAM wParam, LPARAM lParam);
INT_PTR CALLBACK DlgProc_Reverb(HWND hDlg, UINT message, WPARAM wParam, LPARAM lParam);
INT_PTR CALLBACK DlgProc_Effect(HWND hDlg, UINT message, WPARAM wParam, LPARAM lParam);