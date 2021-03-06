/*--------------------------------------------------------------------------------------------------
// File : debug.cpp
//
// Description:
//             提供控制台调试和对话框调试的功能函数实现
//
// Author: 程鑫
//
// Date: 2018-04-29
//
--------------------------------------------------------------------------------------------------*/

#include "stdafx.h"
#include "miniword.h"

void chDEBUGPRINTBOX(LPCWSTR msg)
{
	MessageBox(NULL, msg, TEXT("DEBUG"), MB_OKCANCEL);
}

void chDEBUGTEXTOUT(HWND hWnd, LPCWSTR lpString, int x, int y)
{
	HDC hdc = GetDC(hWnd);
	TextOut(hdc, x, y, lpString, lstrlen(lpString));
	ReleaseDC(hWnd, hdc);
}

void chDEBUGMESSAGEBOX(LPCWSTR lpszFmt, ...)
{
	TCHAR out[1000] = { 0 };
	va_list arg_ptr;
	va_start(arg_ptr, lpszFmt);
	_vsnwprintf_s(out, _countof(out), lpszFmt, arg_ptr);
	MessageBox(NULL, out, TEXT("DEBUG"), MB_OKCANCEL);
}