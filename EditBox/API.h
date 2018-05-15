#pragma once

#include <Windows.h>

#include "Cursor.h"
#include "Record.h"

typedef CText* HTEXT;
HTEXT	_stdcall	CreateText(int iCharWidth, int iCharHeight);
BOOL	_stdcall	DestroyText(HTEXT &hText);

typedef ULONGLONG FPARAM;	// _INT64
typedef ULONGLONG SPARAM;	// _INT64
typedef UINT      RVALUE;   // _INT64 UserMessageProc处理返回的结果码
// UserMessageProc函数返回的结构体类型
typedef struct _USER_RESULT 
{
	POINT m_pCaretPixelPos;  // 光标的像素位置
	POINT m_pStartPixelPos;  // 起点的像素位置
	POINT m_pEndPixelPos;    // 终点的像素位置
	POINT m_pTextPixelSize;  // 文本的像素大小
	POINT m_pCaretCoord;     // 光标的逻辑坐标(X行 Y列)

	// 仅UM_TEXT使用
	UINT  m_uiCount;         // 文本字符数目
	UINT  m_uiStart;         // 高亮部分起点
	UINT  m_uiEnd;           // 高亮部分终点
	BOOL  m_bInside;         // 起点位于中文字符中间(true or false)
} KERNELINFO, *LPKERNELINFO;

#define LODWORD(l) ((UINT)(((ULONGLONG)(l)) & 0xffffffff))			// 低双字节
#define HIDWORD(l) ((UINT)(((ULONGLONG)(l) >> 32) & 0xffffffff))	// 高双字节

// +++ Move Caret Message Value +++ //
#define UM_HOME		0x00000024
#define UM_END		0x00000023
#define UM_UP		0x00000026
#define UM_DOWN		0x00000028
#define UM_LEFT		0x00000025
#define UM_RIGHT	0x00000027
// ++++++++++++ END +++++++++++++++ //

// +++ About Character Message Value +++ //
#define UM_RETURN	0x00000001
#define UM_CHAR		0x00000002
#define UM_TEXT		0x00000003
#define UM_DELETE	0x00000004
// ++++++++++++++ END ++++++++++++++++++ //

// +++ About Hightlight Message Value +++ //
#define UM_CHOOSE	0x00000005
#define UM_CURSOR	0x00000006
// +++++++++++++++ END ++++++++++++++++++ //

// +++ Advanced Operation Message Value +++ //
#define UM_NEW		0x00000007
#define UM_OPEN		0x00000008
#define UM_SAVE		0x00000009
#define UM_CLOSE	0x0000000A
#define UM_ALL		0x0000000B
#define UM_COPY		0x0000000C
#define UM_PASTE	0x0000000D
#define UM_CANCEL	0x0000000E
#define UM_FIND		0x0000000F
#define UM_REPLACE  0x00000010
// +++++++++++++++++ END ++++++++++++++++++ //

// ++++++++ About Query ++++++++ //
#define UM_ISSAVED 0x000000011
// +++++++++++++ END +++++++++++ //

// ++++++++++ Error Information +++++++++++ //
#define UR_NOPATH		0xFFFFFFFF
#define UR_NOTSAVED		0xFFFFFFFE
#define UR_NOTCANCEL	0xFFFFFFFD
#define UR_ERROR		0xFFFFFFFC
#define UR_SAVED		0xFFFFFFFB
#define UR_SUCCESS		0x00000000
// ++++++++++++++++++ END +++++++++++++++++ //

/*
  @Description: 接口函数，负责内部数据处理和图形界面绘制的沟通
*/
RVALUE _stdcall UserMessageProc(_In_ HTEXT hText,
	_In_opt_ int x, _In_opt_ int y,
	_In_ UINT message,
	_In_opt_ FPARAM fParam, _In_opt_ SPARAM sParam,
	_Out_opt_ LPKERNELINFO lpKernelInfo
);

#define FIND_COMPELTE 0x0000000000000001
#define FIND_LHCASE   0x0000000000000002

#define TEXT_SIZE	300				//显示文本长度
#define TAB_SIZE	4				//Tab字符为4空格

/*-------------------------------------------------------------------
  @Suggestion : Move the following statement to other header file
-------------------------------------------------------------------*/ 
Cursor* Initialize_Cursor(CText* p, int Width, int Height);
void    Alloc_Buffer(wchar_t* &p, size_t &Old_Size, size_t New_Size);
void    Free_Buffer(wchar_t* &p);
void    Set_Height_Light(int LineNumber, Position ps, Position pe, short int& start, short int& end);
