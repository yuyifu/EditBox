//EditBox.cpp: 定义应用程序的入口点。
//

#include "stdafx.h"
#include "editbox.h"
#include <functional>
#include <algorithm>
#include <cstring>
#include "CreateDump.h"

#pragma comment(lib, "Shell32.lib")

#define MAX_LOADSTRING 100

// 全局变量: 
HINSTANCE hInst;                                // 当前实例
WCHAR szTitle[MAX_LOADSTRING];                  // 标题栏文本
WCHAR szWindowClass[MAX_LOADSTRING];            // 主窗口类名
HTEXT hText;									// 文本对象

// 此代码模块中包含的函数的前向声明: 
ATOM                MyRegisterClass(HINSTANCE hInstance);
BOOL                InitInstance(HINSTANCE, int);
LRESULT CALLBACK    WndProc(HWND, UINT, WPARAM, LPARAM);
INT_PTR CALLBACK    About(HWND, UINT, WPARAM, LPARAM);
INT_PTR CALLBACK	Find(HWND, UINT, WPARAM, LPARAM);
BOOL				AdjustWindowSize(HWND, const POINT&, const POINT&, const POINT&);
BOOL				AdjustClientPos(HWND, POINT&, const POINT&, const POINT&, const POINT&);
BOOL				UpdateRect(HWND, const LONG&, const LONG&, const LONG&, const LONG&);

BOOL				MoveClient(HWND, const POINT&, POINT&, const POINT&, const POINT&, POINT&, POINT&);

int APIENTRY wWinMain(_In_ HINSTANCE hInstance,
                     _In_opt_ HINSTANCE hPrevInstance,
                     _In_ LPWSTR    lpCmdLine,
                     _In_ int       nCmdShow)
{
    UNREFERENCED_PARAMETER(hPrevInstance);
    UNREFERENCED_PARAMETER(lpCmdLine);

    // 初始化全局字符串
    LoadStringW(hInstance, IDS_APP_TITLE, szTitle, MAX_LOADSTRING);
    LoadStringW(hInstance, IDC_EDITBOX, szWindowClass, MAX_LOADSTRING);
    MyRegisterClass(hInstance);

	CCreateDump::Instance()->DeclarDumpFile("editbox");

    // 执行应用程序初始化: 
    if (!InitInstance (hInstance, nCmdShow))
    {
        return FALSE;
    }

    HACCEL hAccelTable = LoadAccelerators(hInstance, MAKEINTRESOURCE(IDC_EDITBOX));

    MSG msg;

    // 主消息循环: 
    while (GetMessage(&msg, nullptr, 0, 0))
    {
        if (!TranslateAccelerator(msg.hwnd, hAccelTable, &msg))
        {
            TranslateMessage(&msg);
            DispatchMessage(&msg);
        }
    }

    return (int) msg.wParam;
}



//
//  函数: MyRegisterClass()
//
//  目的: 注册窗口类。
//
ATOM MyRegisterClass(HINSTANCE hInstance)
{
    WNDCLASSEXW wcex;

    wcex.cbSize = sizeof(WNDCLASSEX);

    wcex.style          = NULL;
    wcex.lpfnWndProc    = WndProc;
    wcex.cbClsExtra     = 0;
    wcex.cbWndExtra     = 0;
    wcex.hInstance      = hInstance;
    wcex.hIcon          = LoadIcon(hInstance, MAKEINTRESOURCE(IDI_EDITBOX));
    wcex.hCursor        = LoadCursor(nullptr, IDC_IBEAM);
    wcex.hbrBackground  = (HBRUSH)(COLOR_WINDOW+1);
    wcex.lpszMenuName   = MAKEINTRESOURCEW(IDC_EDITBOX);
    wcex.lpszClassName  = szWindowClass;
    wcex.hIconSm        = LoadIcon(wcex.hInstance, MAKEINTRESOURCE(IDI_SMALL));

    return RegisterClassExW(&wcex);
}

//
//   函数: InitInstance(HINSTANCE, int)
//
//   目的: 保存实例句柄并创建主窗口
//
//   注释: 
//
//        在此函数中，我们在全局变量中保存实例句柄并
//        创建和显示主程序窗口。
//
BOOL InitInstance(HINSTANCE hInstance, int nCmdShow)
{
   hInst = hInstance; // 将实例句柄存储在全局变量中

   HWND hWnd = CreateWindowW(szWindowClass, szTitle,
	   WS_CAPTION |		// 标题栏
	   WS_SYSMENU |		// 窗口调整按钮
	   WS_MAXIMIZE |	// 初始最大化
	   WS_HSCROLL |		// 水平滚动条
	   WS_MINIMIZEBOX |	// 最小化按钮
	   WS_VSCROLL		// 竖直滚动条
	   ,		
      CW_USEDEFAULT, CW_USEDEFAULT, CW_USEDEFAULT, CW_USEDEFAULT, nullptr, nullptr, hInstance, nullptr);

   if (!hWnd)
   {
      return FALSE;
   }

   ShowWindow(hWnd, SW_SHOW);
   UpdateWindow(hWnd);

   return TRUE;
}

//
//  函数: WndProc(HWND, UINT, WPARAM, LPARAM)
//
//  目的:    处理主窗口的消息。
//
//	WM_CREATE			- 创建窗口时初始化
//	WM_SHOWWINDOW		- 清空窗口（文本也被清理）
//  WM_COMMAND			- 处理应用程序菜单
//	WN_CLOSE			- 处理应用程序关闭时的资源释放
//	WM_SIZE				- 用户调整大小时进行重绘处理
//	WM_SETFOCOUS		- 用户重新单击应用程序
//	WM_KILLFOCUS		- 用户放弃应用程序
//	WM_KEYDOWN			- 处理虚拟按键
//	WM_CHAR				- 处理字符按键
//	WM_VSCROLL			- 处理用户拖动竖直滚动条
//	WM_HSCROLL			- 处理用户拖动水平滚动条
//	WM_LBUTTONDOWM		- 处理用户单击鼠标左键
//	WM_NCLBUTTONDBLCLK	- 处理用户双击鼠标左键击中标题栏
//	WM_LBUTTONDBLCLK	- 处理用户双击鼠标左键击中显示区
//	WM_MOUSEMOVE		- 处理用户滑动鼠标
//  WM_PAINT			- 绘制主窗口
//  WM_DESTROY			- 发送退出消息并返回
//	
//
//
LRESULT CALLBACK WndProc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam)
{
	// 字符模块
	static	POINT		pCharSize;	// 字符大小
	static	HFONT		hFont;
	static	TEXTMETRIC	tm;
	// ...

	// 窗口模块
	static	POINT		pClientSize;	// 显示区大小
	static	POINT		pClientPos;		// 显示区坐标
	static	POINT		pPageSize;
	static	POINT		pWindowSize;	// 窗口大小
	// ...

	// 光标模块
	static	POINT		pCaretSize;		// 光标大小
	static	POINT		pCaretPos;		// 光标坐标
	static	POINT		pStartPos;		// 选段起点
	static	POINT		pEndPos;		// 终点
	// ...

	static	HDC			hMemDC;
	static	HBRUSH		hBrush;
	static	HBITMAP		hBitmap;
	static	HTIMER		hMouseTimer;
	static	HTIMER		hSaveTimer;

    switch (message)
    {
	case WM_CREATE:	// 创建窗口
		{
			HDC hdc = GetDC(hWnd);

			// 初始化字体相关信息
			hFont = CreateFont(
				16,		// 高度
				0,		// 宽度
				0,		// 水平
				0,		// 倾斜
				400,	// 粗度
				0,		// 不倾斜
				0,		// 无下划线
				0,		// 无中划线
				GB2312_CHARSET,					// 字符集
				OUT_DEFAULT_PRECIS,				// 输出精度
				CLIP_DEFAULT_PRECIS,			// 裁剪精度
				DEFAULT_QUALITY,				// 输出质量
				DEFAULT_PITCH | FF_DONTCARE,	// 间距
				TEXT("New Curier")				// 名称
			);
			// ...
			SelectObject(hdc, hFont);

			// 设置字符像素大小（英文）
			GetTextMetrics(hdc, &tm);
			pCharSize.x = tm.tmAveCharWidth;
			pCharSize.y = tm.tmHeight;
			// ...

			// 创建文本对象
			hText = CreateText(pCharSize.x, pCharSize.y);
			hMouseTimer = nullptr;
			// ...

			ReleaseDC(hWnd, hdc);
		}
		break;
	case WM_SHOWWINDOW:	// 显示窗口
		{
			RECT rc;
			GetClientRect(hWnd, &rc);
			pClientSize.x = rc.right - rc.left;
			pClientSize.y = rc.bottom - rc.top;

			HDC hdc = GetDC(hWnd);
			hMemDC = CreateCompatibleDC(hdc);
			hBitmap = CreateCompatibleBitmap(hdc, pClientSize.x, pClientSize.y);
			hBrush = CreateSolidBrush(RGB(0xFF, 0xFF, 0xFF));
			SelectObject(hMemDC, hBitmap);
			SelectObject(hMemDC, hFont);
			ReleaseDC(hWnd, hdc);


			// 设置页面栅格
			pPageSize.x = pClientSize.x / USWIDTH(pCharSize.x);
			pPageSize.y = pClientSize.y / USHEIGHT(pCharSize.y);
			// 设置显示区像素大小
			pClientSize.x = pPageSize.x * USWIDTH(pCharSize.x);
			pClientSize.y = pPageSize.y * USHEIGHT(pCharSize.y);
			// 设置显示区坐标
			pClientPos.x = 0;
			pClientPos.y = 0;
			// 设置窗口大小
			pWindowSize.x = 0;
			pWindowSize.y = 0;
			// ...

			// 设置滚动条
			SCROLLINFO sInfo;
			sInfo.cbSize = sizeof(SCROLLINFO);
			sInfo.fMask = SIF_ALL;
			sInfo.nMax = pPageSize.x - 1;
			sInfo.nMin = 0;
			sInfo.nPage = pPageSize.x;
			sInfo.nPos = 0;
			sInfo.nTrackPos = 0;
			SetScrollInfo(hWnd, SB_HORZ, &sInfo, FALSE);

			sInfo.cbSize = sizeof(SCROLLINFO);
			sInfo.fMask = SIF_ALL;
			sInfo.nMax = pPageSize.y - 1;
			sInfo.nMin = 0;
			sInfo.nPage = pPageSize.y;
			sInfo.nPos = 0;
			sInfo.nTrackPos = 0;
			SetScrollInfo(hWnd, SB_VERT, &sInfo, FALSE);
			// ...

			// 设置光标大小
			pCaretSize.x = 1;
			pCaretSize.y = USHEIGHT(pCharSize.y);
			// 设置光标位置
			pCaretPos.x = pClientPos.x;
			pCaretPos.y = pClientPos.y;
			// 创建光标
			CreateCaret(hWnd, NULL, pCaretSize.x, pCaretSize.y);
			// 重绘光标
			SetCaretPos(pCaretPos.x - pClientPos.x, pCaretPos.y - pClientPos.y);
			ShowCaret(hWnd);
			// ...

			// 更新选段
			pStartPos.x = pCaretPos.x; pStartPos.y = pCaretPos.y;
			pEndPos.x = pStartPos.x; pEndPos.y = pStartPos.y;
			// ...
		}
		break;
    case WM_COMMAND:
        {
            int wmId = LOWORD(wParam);
            // 分析菜单选择: 
            switch (wmId)
            {
			case IDM_NEW:
				{	// 新建文件
					RVALUE result;
					if ((result =
						UserMessageProc(hText, NULL, NULL, UM_NEW, NULL, NULL)) != UR_ERROR)
					{
						if (result == UR_NOTSAVED)
						{	// 未保存
							// 保存当前文件
							SendMessage(hWnd, WM_COMMAND, IDM_SAVE, NULL);
							if ((result =	// 新建文件
								UserMessageProc(hText, NULL, NULL, UM_NEW, NULL, NULL)) != UR_ERROR)
							{
								// 重绘显示区(初始化窗口)
								SetWindowText(hWnd, TEXT("未命名.txt"));
								SendMessage(hWnd, WM_SHOWWINDOW, NULL, NULL);
								UpdateRect(hWnd, 0, pClientSize.x, 0, pClientSize.y);
							}
						}
						else if (result == UR_SAVED)
						{	// 已保存
							// 重绘显示区(初始化窗口)
							SetWindowText(hWnd, TEXT("未命名.txt"));
							SendMessage(hWnd, WM_SHOWWINDOW, NULL, NULL);
							UpdateRect(hWnd, 0, pClientSize.x, 0, pClientSize.y);
						}
					}
				}
				break;
			case IDM_OPEN:
				{	// 打开文件
					// 弹出对话框
					TCHAR szFileName[MAX_PATH] = {0};	// 文件名记录
					OPENFILENAME ofn = { 0 };
					ofn.lStructSize = sizeof(OPENFILENAME);
					ofn.hwndOwner = hWnd;
					ofn.lpstrTitle = TEXT("打开文件");
					ofn.lpstrFile = szFileName;
					ofn.lpstrFilter = TEXT("txt文本文件(*.txt)");
					ofn.nMaxFile = MAX_PATH;
					ofn.nFilterIndex = 1;
					ofn.lpstrInitialDir = TEXT("./");	// 初始化路径
					ofn.Flags = OFN_FILEMUSTEXIST | OFN_EXPLORER | OFN_HIDEREADONLY | OFN_PATHMUSTEXIST;

					if (GetOpenFileName(&ofn) && lstrcmp(szFileName, TEXT("")) != 0)
					{	// 选中文件
						RVALUE result;
						if ((result =
							UserMessageProc(hText, NULL, NULL, UM_OPEN, NULL, (SPARAM)szFileName)) != UR_ERROR)
						{
							if (result == UR_NOTSAVED)
							{	// 未保存
								// 保存
								SendMessage(hWnd, WM_COMMAND, IDM_SAVE, NULL);
								if ((result =
									UserMessageProc(hText, NULL, NULL, UM_OPEN, NULL, (SPARAM)szFileName)) != UR_ERROR)
								{
									SetWindowText(hWnd, szFileName);
									pWindowSize.x = (LONG)LODWORD(result); pWindowSize.y = (LONG)HIDWORD(result);
									AdjustWindowSize(hWnd, pWindowSize, pClientSize, pCharSize);
									// 设置显示区位置
									pClientPos.x = 0; pClientPos.y = 0;
									pCaretPos.x = pClientPos.x; pCaretPos.y = pClientPos.y;
									SetCaretPos(pClientPos.x - pCaretPos.x, pClientPos.y - pCaretPos.y);
									// 重置选段
									pStartPos.x = pCaretPos.x; pStartPos.y = pCaretPos.y;
									pEndPos.x = pStartPos.x; pEndPos.y = pStartPos.y;
									// 重绘显示区
									UpdateRect(hWnd, 0, pClientSize.x, 0, pClientSize.y);
								}
							}
							else if (result == UR_SAVED)
							{
								SetWindowText(hWnd, szFileName);
								pWindowSize.x = (LONG)LODWORD(result); pWindowSize.y = (LONG)HIDWORD(result);
								AdjustWindowSize(hWnd, pWindowSize, pClientSize, pCharSize);

								// 设置显示区位置
								pClientPos.x = 0; pClientPos.y = 0;
								// 重绘光标
								pCaretPos.x = pClientPos.x; pCaretPos.y = pClientPos.y;
								SetCaretPos(pClientPos.x - pCaretPos.x, pClientPos.y - pCaretPos.y);
								// 重置选段
								pStartPos.x = pCaretPos.x; pStartPos.y = pCaretPos.y;
								pEndPos.x = pStartPos.x; pEndPos.y = pStartPos.y;
								// 重绘显示区
								UpdateRect(hWnd, 0, pClientSize.x, 0, pClientSize.y);
							}
						}
					}
				}
				break;
			case IDM_SAVE:
				{	// 保存文件
					RVALUE result;
					if ((result =
						UserMessageProc(hText, NULL, NULL, UM_SAVE, NULL, NULL)) != UR_ERROR)
					{
						if (result == UR_NOPATH)
							SendMessage(hWnd, WM_COMMAND, IDM_SAVEAS, NULL);
					}
				}
				break;
			case IDM_SAVEAS:
				{	// 另存为
					TCHAR szFileName[MAX_PATH] = {0};	// 路径记录
					OPENFILENAME ofn;
					ofn.lStructSize = sizeof(OPENFILENAME);
					ofn.hwndOwner = hWnd;
					ofn.hInstance = NULL;
					ofn.lpstrFilter = TEXT("txt文本文件(*.txt)");
					ofn.nFilterIndex = 1;
					ofn.lpstrCustomFilter = NULL;
					ofn.nMaxCustFilter = 0;
					ofn.lpstrFile = szFileName;
					ofn.nMaxFile = MAX_PATH;
					ofn.lpstrFileTitle = NULL;
					ofn.nMaxFileTitle = 0;
					ofn.lpstrInitialDir = NULL;
					ofn.lpstrTitle = TEXT("保存为指定的文件...");
					ofn.Flags = OFN_HIDEREADONLY;
					ofn.nFileOffset = 0;
					ofn.nFileExtension = 0;
					ofn.lpstrDefExt = TEXT("txt");
					ofn.lCustData = 0L;
					ofn.lpfnHook = NULL;
					ofn.lpTemplateName = NULL;

					if (GetSaveFileName(&ofn) && lstrcmp(szFileName, TEXT("")) != 0)
					{
						RVALUE result;
						if ((result =
							UserMessageProc(hText, NULL, NULL, UM_SAVE, NULL, (SPARAM)szFileName)) != UR_ERROR)
						{
							// 无界面操作
							UserMessageProc(hText, NULL, NULL, UM_OPEN, NULL, (SPARAM)szFileName);
						}
					}
				}
				break;
            case IDM_ABOUT:
                DialogBox(hInst, MAKEINTRESOURCE(IDD_ABOUTBOX), hWnd, About);
                break;
            case IDM_EXIT:
				SendMessage(hWnd, WM_CLOSE, NULL, NULL);
                break;
			case IDM_FIND:
				{
					DialogBox(hInst, MAKEINTRESOURCE(IDD_FINDDIALOG), hWnd, Find);
				}
				break;
			case IDM_CUT:
				{
					BYTE keyState[256] = { 0 };
					keyState[VK_CONTROL] = -1;
					SetKeyboardState(keyState);
					// ...

					SendMessage(hWnd, WM_COMMAND, IDM_COPY, NULL);
					SendMessage(hWnd, WM_CHAR, TEXT('\b'), NULL);

					// 恢复Ctrl按键
					keyState[VK_CONTROL] = 0;
					SetKeyboardState(keyState);
				}
				break;
			case IDM_COPY:
				{
					if (OpenClipboard(hWnd))
					{	// 打开剪切板
						if (EmptyClipboard())
						{	// 清空剪切板
							LPWSTR lpString = nullptr;
							RVALUE result;
							if ((result =
								UserMessageProc(hText, NULL, NULL,
									UM_COPY, NULL, (SPARAM)(&lpString))) != UR_ERROR)
							{	// 拷贝到剪切板
								SIZE_T   nSize  = LODWORD(result) + 1;
								HGLOBAL  hMem   = GlobalAlloc(GHND, nSize * 2);
								if (hMem != NULL)
								{
									LPWSTR   lpchText = (LPWSTR)GlobalLock(hMem);
									if (lpchText != NULL)
									{
										lstrcpyn(lpchText, lpString, nSize);
										if (!GlobalUnlock(hMem))
										{
											if (SetClipboardData(CF_UNICODETEXT, hMem) != NULL)
											{
												chDEBUGMESSAGEBOX(TEXT("start=(%d,%d) end=(%d,%d)\n串长度=%d\n拷贝的串=%ls"),
													pStartPos.x, pStartPos.y, pEndPos.x, pEndPos.y,
													nSize, lpString);
											}
											else
											{
												chDEBUGMESSAGEBOX(TEXT("LastError=%d\n"), GetLastError());
											}
										}
										else
											chDEBUGMESSAGEBOX(TEXT("Unlock Global Memory Failed!\n"));
									}
									else
										chDEBUGMESSAGEBOX(TEXT("Memory Allocation Failed!\n"));
								}
								else
									chDEBUGMESSAGEBOX(TEXT("Memory Allocation Failed!\n"));
							}
						}
						if (!CloseClipboard())
						{
							// 失败
						}
					}
				}
				break;
			case IDM_PASTE:
				{
					// 判断数据格式    
					if (IsClipboardFormatAvailable(CF_UNICODETEXT))
					{
						if (OpenClipboard(hWnd))
						{	// 从剪切板拷贝
							HGLOBAL hClipMemory = GetClipboardData(CF_UNICODETEXT);
							if (hClipMemory != NULL)
							{
								SIZE_T nSize = GlobalSize(hClipMemory) / 2;
								LPWSTR lpClipMemory = (LPWSTR)GlobalLock(hClipMemory);
								LPWSTR lpchText = new TCHAR[nSize + 1]{ 0 };

								lstrcpyn(lpchText, lpClipMemory, nSize);
								// 关闭剪切板
								GlobalUnlock(hClipMemory);
								CloseClipboard();

								RVALUE result;
								if ((result =
									UserMessageProc(hText, pCaretPos.x, pCaretPos.y, UM_PASTE, (FPARAM)(&pEndPos), (SPARAM)lpchText)) != UR_ERROR)
								{
									pWindowSize.x = (LONG)LODWORD(result); pWindowSize.y = (LONG)HIDWORD(result); // 获取窗口大小
									pStartPos.x = pCaretPos.x;  pStartPos.y = pCaretPos.y;
									pCaretPos.x = pEndPos.x; pCaretPos.y = pEndPos.y; // 设置光标位置

									chDEBUGMESSAGEBOX(TEXT("光标=(%d, %d)"), pCaretPos.x, pCaretPos.y);

									AdjustWindowSize(hWnd, pWindowSize, pClientSize, pCharSize);		// 调整显示区大小
									// 检测光标是否在显示区内并调整显示区的位置
									AdjustClientPos(hWnd, pClientPos, pCaretPos, pPageSize, pCharSize); // 滑动显示区
									SetCaretPos(pCaretPos.x - pClientPos.x, pCaretPos.y - pClientPos.y); // 重绘光标
									if (!INRANGE(pCaretPos.x, pClientPos.x, pClientPos.x + pClientSize.x) || !INRANGE(pCaretPos.y, pClientPos.y, pClientPos.y + pClientSize.y - USHEIGHT(pCharSize.y)))
										HideCaret(hWnd);
									else
										ShowCaret(hWnd);
									UpdateRect(hWnd, 0, pClientSize.x, pStartPos.y - pClientPos.y, pEndPos.y - pClientPos.y + USHEIGHT(pCharSize.y));
								}
								delete[] lpchText;	// 释放临时内存
							}
							else
								CloseClipboard();
						}
					}
				}
				break;
			case IDM_CANCEL:
				{
					RVALUE result;
					if ((result =
						UserMessageProc(hText, NULL, NULL, UM_CANCEL, (FPARAM)&pStartPos, (SPARAM)&pEndPos)) != UR_ERROR)
					{
						// 获取窗口大小
						pWindowSize.x = (LONG)LODWORD(result); pWindowSize.y = (LONG)HIDWORD(result);
						pCaretPos.x = pEndPos.x; pCaretPos.y = pEndPos.y;

						AdjustWindowSize(hWnd, pWindowSize, pClientSize, pCharSize);		// 调整显示区大小
						// 检测光标是否在显示区内并调整显示区的位置
						AdjustClientPos(hWnd, pClientPos, pCaretPos, pPageSize, pCharSize); // 滑动显示区
						SetCaretPos(pCaretPos.x - pClientPos.x, pCaretPos.y - pClientPos.y); ; // 重绘光标
						if (!INRANGE(pCaretPos.x, pClientPos.x, pClientPos.x + pClientSize.x) || !INRANGE(pCaretPos.y, pClientPos.y, pClientPos.y + pClientSize.y - USHEIGHT(pCharSize.y)))
							HideCaret(hWnd);
						else
							ShowCaret(hWnd);
						UpdateRect(hWnd, 0, pClientSize.x, pStartPos.y - pClientPos.y, pEndPos.y - pClientPos.y);
					}
				}
				break;
			case IDM_ALL:
				{
					RVALUE result;
					if ((result =
						UserMessageProc(hText, NULL, NULL, UM_ALL, NULL, NULL)) != UR_ERROR)
					{
						pCaretPos.x = (LONG)LODWORD(result); pCaretPos.y = (LONG)HIDWORD(result); // 设置光标位置

						// 检测光标是否在显示区内并调整显示区的位置
						AdjustClientPos(hWnd, pClientPos, pCaretPos, pPageSize, pCharSize); // 滑动显示区
						SetCaretPos(pCaretPos.x - pClientPos.x, pCaretPos.y - pClientPos.y);  ; // 重绘光标
						if (!INRANGE(pCaretPos.x, pClientPos.x, pClientPos.x + pClientSize.x) || !INRANGE(pCaretPos.y, pClientPos.y, pClientPos.y + pClientSize.y - USHEIGHT(pCharSize.y)))
							HideCaret(hWnd);
						else
							ShowCaret(hWnd);
						UpdateRect(hWnd, 0, pClientSize.x, pStartPos.y - pClientPos.y, pEndPos.y - pClientPos.y);

						// 重置选段
						pStartPos.x = 0; pStartPos.y = 0;
						pEndPos.x = pCaretPos.x; pEndPos.y = pCaretPos.y;
					}
				}
            default:
                return DefWindowProc(hWnd, message, wParam, lParam);
            }
        }
        break;
	case WM_CLOSE:
		{
			RVALUE result;
			if ((result =
				UserMessageProc(hText, 0, 0, UM_CLOSE, NULL, NULL)) != UR_ERROR)
			{
				if (result == UR_NOTSAVED)
				{	// 未保存
					// 保存
					SendMessage(hWnd, WM_COMMAND, IDM_SAVE, NULL);
				}
				{
					// 无操作
				}
			}
			else
			{	// 发生异常
				assert(!(result == UR_ERROR));
			}
			// 销毁文本对象
			DestroyText(hText);
			// ...

			DestroyWindow(hWnd);
		}
		break;
	case WM_SIZE:
		{
			// 永不被调用
		}
		break;
	case WM_SETFOCUS:	// 获得焦点
		{
			// 新建光标
			CreateCaret(hWnd, NULL, pCaretSize.x, pCaretSize.y);
			// 重绘光标
			SetCaretPos(pClientPos.x - pCaretPos.x, pClientPos.y - pCaretPos.y);
			if (!INRANGE(pCaretPos.x, pClientPos.x, pClientPos.x + pClientSize.x) || !INRANGE(pCaretPos.y, pClientPos.y, pClientPos.y + pClientSize.y - USHEIGHT(pCharSize.y)))
				HideCaret(hWnd);
			else
				ShowCaret(hWnd);
			ShowCaret(hWnd);
			// ...
		}
		break;
	case WM_KILLFOCUS:	// 释放焦点
		{
			// 隐藏光标
			HideCaret(hWnd);
			DestroyCaret();
			// ...
		}
		break;
	case WM_KEYDOWN:
		{
			switch (wParam)
			{
				case VK_HOME:
					{	// HOME键
						RVALUE result;
						if ((result =
							UserMessageProc(hText, NULL, NULL, UM_HOME, NULL, NULL)) != UR_ERROR)
						{
							pCaretPos.x = (LONG)LODWORD(result); pCaretPos.y = (LONG)HIDWORD(result);// 设置光标位置
							MoveClient(hWnd, pCaretPos, pClientPos, pPageSize, pCharSize, pStartPos, pEndPos);
						}
						else
						{	// 发生异常
							assert(!(result == UR_ERROR));
						}
					}
					break;
				case VK_END:
					{	// END键
						RVALUE result;
						if ((result = 
							UserMessageProc(hText, pCaretPos.x, pCaretPos.y, UM_END, NULL, NULL)) != UR_ERROR)
						{
							pCaretPos.x = (LONG)LODWORD(result); pCaretPos.y = (LONG)HIDWORD(result);// 设置光标位置
							MoveClient(hWnd, pCaretPos, pClientPos, pPageSize, pCharSize, pStartPos, pEndPos);
						}
						else
						{	// 异常
							assert(!(result == UR_ERROR));
						}
					}
					break;
				case VK_UP:
					{
						RVALUE result;
						if ((result = 
							UserMessageProc(hText, pCaretPos.x, pCaretPos.y, UM_UP, NULL, NULL)) != UR_ERROR)
						{
							pCaretPos.x = (LONG)LODWORD(result); pCaretPos.y = (LONG)HIDWORD(result);// 设置光标位置
							MoveClient(hWnd, pCaretPos, pClientPos, pPageSize, pCharSize, pStartPos, pEndPos);
						}
						else
						{
							assert(!(result == UR_ERROR));
						}
					}
					break;
				case VK_DOWN:
					{
						RVALUE result;
						if ((result = 
							UserMessageProc(hText, pCaretPos.x, pCaretPos.y, UM_DOWN, NULL, NULL)) != UR_ERROR)
						{
							pCaretPos.x = (LONG)LODWORD(result); pCaretPos.y = (LONG)HIDWORD(result);// 设置光标位置
							MoveClient(hWnd, pCaretPos, pClientPos, pPageSize, pCharSize, pStartPos, pEndPos);
						}
						else
						{	// 光标处于底层
							assert(!(result == UR_ERROR));
						}
					}
					break;
				case VK_RIGHT:
					{
						RVALUE result;
						if ((result =
							UserMessageProc(hText, pCaretPos.x, pCaretPos.y, UM_RIGHT, NULL, NULL)) != UR_ERROR)
						{
							pCaretPos.x = (LONG)LODWORD(result); pCaretPos.y = (LONG)HIDWORD(result);// 设置光标位置
							MoveClient(hWnd, pCaretPos, pClientPos, pPageSize, pCharSize, pStartPos, pEndPos);
						}
						else
						{	
							assert(!(result == UR_ERROR));
						}
					}
					break;
				case VK_LEFT:
					{
						RVALUE result;
						if ((result =
							UserMessageProc(hText, pCaretPos.x, pCaretPos.y, UM_LEFT, NULL, NULL)) != UR_ERROR)
						{
							pCaretPos.x = (LONG)LODWORD(result); pCaretPos.y = (LONG)HIDWORD(result);// 设置光标位置
							MoveClient(hWnd, pCaretPos, pClientPos, pPageSize, pCharSize, pStartPos, pEndPos);
						}
						else
						{	// 发生异常
							assert(!(result == UR_ERROR));
						}
					}
					break;
				case VK_DELETE:
					{
						RVALUE result;
						if ((result =
							UserMessageProc(hText, pStartPos.x, pStartPos.y, UM_DELETE, NULL, (SPARAM)((SPARAM)pEndPos.y << 32 | (SPARAM)pEndPos.x))) != UR_ERROR)
						{
							pWindowSize.x = (LONG)LODWORD(result); pWindowSize.y = (LONG)HIDWORD(result);// 获取窗口大小

							AdjustWindowSize(hWnd, pWindowSize, pClientSize, pCharSize);			// 调整显示区大小
							AdjustClientPos(hWnd, pClientPos, pCaretPos, pPageSize, pCharSize);		// 滑动显示区
							SetCaretPos(pCaretPos.x - pClientPos.x, pCaretPos.y - pClientPos.y);  ;	// 重绘光标
							if (!INRANGE(pCaretPos.x, pClientPos.x, pClientPos.x + pClientSize.x) || !INRANGE(pCaretPos.y, pClientPos.y, pClientPos.y + pClientSize.y - USHEIGHT(pCharSize.y)))
								HideCaret(hWnd);
							else
								ShowCaret(hWnd);

							if (pStartPos.y != pEndPos.y)
								UpdateRect(hWnd, 
									0, pClientSize.x + USWIDTH(pCharSize.x), 
									pStartPos.y - pClientPos.y, pClientSize.y + USHEIGHT(pCharSize.y));
							else
								UpdateRect(hWnd,
									pStartPos.x, pClientSize.x + USWIDTH(pCharSize.x), 
									pStartPos.y - pClientPos.y, pEndPos.y - pClientPos.y + USHEIGHT(pCharSize.y));

							// 更新选段
							pStartPos.x = pCaretPos.x; pStartPos.y = pCaretPos.y;
							pEndPos.x   = pStartPos.x; pEndPos.y   = pStartPos.y;
						}
						else
						{	// 发生异常
							assert(!(result == UR_ERROR));
						}
					}
					break;
				default:
					break;
			}
		}
		break;
	case WM_CHAR:
		{
			for (int i = 0; i < (int)LOWORD(lParam); ++i)
			{
				switch (wParam)
				{
				case '\b':
					{
						// 调整光标 -> 挪至上侧
						if (pEndPos.y < pStartPos.y || 
							(pEndPos.y == pEndPos.y && pEndPos.x < pStartPos.x))
						{
							std::swap(pStartPos.x, pEndPos.x);
							std::swap(pStartPos.y, pEndPos.y);
						}
						pCaretPos.x = pStartPos.x;
						pCaretPos.y = pStartPos.y;
						// ...

						// 判断选段是否为空
						if (pStartPos.x == pEndPos.x &&
							pStartPos.y == pEndPos.y)
						{	// 光标左移
							POINT temp;
							temp.x = pEndPos.x; temp.y = pEndPos.y;
							SendMessage(hWnd, WM_KEYDOWN, VK_LEFT, NULL);
							pEndPos.x = temp.x; pEndPos.y = temp.y;
						}
						// ...

						// 若光标未在显示区 滑动显示区 -> 将光标显示在中部
						AdjustClientPos(hWnd, pClientPos, pCaretPos, pPageSize, pCharSize); // 滑动显示区
						SetCaretPos(pCaretPos.x - pClientPos.x, pCaretPos.y - pClientPos.y); // 重绘光标
						if (!INRANGE(pCaretPos.x, pClientPos.x, pClientPos.x + pClientSize.x) || !INRANGE(pCaretPos.y, pClientPos.y, pClientPos.y + pClientSize.y - USHEIGHT(pCharSize.y)))
							HideCaret(hWnd);
						else
							ShowCaret(hWnd);

						if (pStartPos.x == 0 && pEndPos.x == 0 && pStartPos.y == 0 && pEndPos.y == 0)
							return DefWindowProc(hWnd, message, wParam, lParam);
						else // 执行删除
							SendMessage(hWnd, WM_KEYDOWN, VK_DELETE, NULL);
					}
					break;
				case '\t':
					{
						RVALUE result;
						// 字符处理
						if ((result =
							UserMessageProc(hText, pCaretPos.x, pCaretPos.y, UM_CHAR, NULL, (SPARAM)wParam)) != UR_ERROR)
						{
							pWindowSize.x = (LONG)LODWORD(result);
							pCaretPos.x = (LONG)HIDWORD(result);

							AdjustWindowSize(hWnd, pWindowSize, pClientSize, pCharSize); // 调整显示区大小
							AdjustClientPos(hWnd, pClientPos, pCaretPos, pPageSize, pCharSize);
							SetCaretPos(pCaretPos.x - pClientPos.x, pCaretPos.y - pClientPos.y);
							if (!INRANGE(pCaretPos.x, pClientPos.x, pClientPos.x + pClientSize.x) || !INRANGE(pCaretPos.y, pClientPos.y, pClientPos.y + pClientSize.y - USHEIGHT(pCharSize.y)))
								HideCaret(hWnd);
							else
								ShowCaret(hWnd);
							UpdateRect(hWnd,
								0, pClientSize.x,
								pCaretPos.y - pClientPos.y, pCaretPos.y - pClientPos.y + USHEIGHT(pCaretSize.y));

							// 重置选段
							pStartPos.x = pCaretPos.x; pStartPos.y = pCaretPos.y;
							pEndPos.x = pStartPos.x; pEndPos.y = pStartPos.y;
						}
						else
						{	// 发生异常
							assert(!(result == UR_ERROR));
						}
					}
					break;
				case '\n':
					{
					}
					break;
				case '\r':
					{
						RVALUE result;
						if ((result = 
							UserMessageProc(hText, pCaretPos.x, pCaretPos.y, UM_RETURN, NULL, NULL)) != UR_ERROR)
						{
							pWindowSize.x = (LONG)LODWORD(result); pWindowSize.y = (LONG)HIDWORD(result); // 获取窗口大小
							pCaretPos.x = 0; pCaretPos.y = pCaretPos.y + pCaretSize.y; // 调整光标位置

							AdjustWindowSize(hWnd, pWindowSize, pClientSize, pCharSize); // 调整显示区大小
							AdjustClientPos(hWnd, pClientPos, pCaretPos, pPageSize, pCharSize);
							SetCaretPos(pCaretPos.x - pClientPos.x, pCaretPos.y - pClientPos.y);
							if (!INRANGE(pCaretPos.x, pClientPos.x, pClientPos.x + pClientSize.x) || !INRANGE(pCaretPos.y, pClientPos.y, pClientPos.y + pClientSize.y - USHEIGHT(pCharSize.y)))
								HideCaret(hWnd);
							else
								ShowCaret(hWnd);
							UpdateRect(hWnd, 
								0, pClientSize.x, 
								pCaretPos.y - pClientPos.y - USHEIGHT(pCharSize.y), pClientSize.y + USHEIGHT(pCharSize.y));
							pStartPos.x = pCaretPos.x; pStartPos.y = pCaretPos.y;
							pEndPos.x = pStartPos.x; pEndPos.y = pStartPos.y;
						}
						else
						{	// 发生异常
							assert(!(result == UR_ERROR));
						}
					}
					break;
				default:
					{
						RVALUE result;
						if (GetAsyncKeyState(VK_CONTROL) < 0)
						{	// 非字符处理（控制命令/快捷键）
							if (wParam == 0x01)
							{	// Ctrl + A 全选
								SendMessage(hWnd, WM_COMMAND, IDM_ALL, NULL);
							}
							else if (wParam == 0x03)
							{	// Ctrl + C 拷贝
								SendMessage(hWnd, WM_COMMAND, IDM_COPY, NULL);
							}
							else if (wParam == 0x16)
							{	// Ctrl + V 粘贴
								SendMessage(hWnd, WM_COMMAND, IDM_PASTE, NULL);
							}
							else if (wParam == 0x18)
							{	// Ctrl + X 剪切
									// 设置Ctrl被按下
								SendMessage(hWnd, WM_COMMAND, IDM_CUT, NULL);
							}
							else if (wParam == 0x1A)
							{	// Ctrl + Z 撤销
								SendMessage(hWnd, WM_COMMAND, IDM_CANCEL, NULL);
							}
							else if (wParam == 0x06)
							{	// Ctrl + F 查找
								SendMessage(hWnd, WM_COMMAND, IDM_FIND, NULL);
							}
							else if (wParam == 0x13)
							{	// Ctrl + S 
								SendMessage(hWnd, WM_COMMAND, IDM_SAVE, NULL);
							}
							else
							{
								TCHAR out[100] = { 0 };
								wsprintf(out, TEXT("%x"), wParam);
								// chDEBUGMESSAGEBOX(out);
							}
						}
						else
						{
							// 字符处理
							if ((result =
								UserMessageProc(hText, pCaretPos.x, pCaretPos.y, UM_CHAR, NULL, (SPARAM)wParam)) != UR_ERROR)
							{
								pWindowSize.x = (LONG)LODWORD(result);
								pCaretPos.x   = (LONG)HIDWORD(result);

								AdjustWindowSize(hWnd, pWindowSize, pClientSize, pCharSize); // 调整显示区大小
								AdjustClientPos(hWnd, pClientPos, pCaretPos, pPageSize, pCharSize);
								SetCaretPos(pCaretPos.x - pClientPos.x, pCaretPos.y - pClientPos.y);
								if (!INRANGE(pCaretPos.x, pClientPos.x, pClientPos.x + pClientSize.x) || !INRANGE(pCaretPos.y, pClientPos.y, pClientPos.y + pClientSize.y - USHEIGHT(pCharSize.y)))
									HideCaret(hWnd);
								else
									ShowCaret(hWnd);
								UpdateRect(hWnd, 
									0, pClientSize.x,
									pCaretPos.y - pClientPos.y, pCaretPos.y - pClientPos.y + USHEIGHT(pCaretSize.y));

								// 重置选段
								pStartPos.x = pCaretPos.x; pStartPos.y = pCaretPos.y;
								pEndPos.x   = pStartPos.x; pEndPos.y   = pStartPos.y;
							}
							else
							{	// 发生异常
								assert(!(result == UR_ERROR));
							}
						}
					}
					break;
				}
			}
		}
		break;
	case WM_VSCROLL:
		{
			// 获取滚动条信息
			SCROLLINFO	sInfo;
			sInfo.cbSize = sizeof(SCROLLINFO);
			sInfo.fMask = SIF_ALL;
			GetScrollInfo(hWnd, SB_VERT, &sInfo);
			// ...

			int iVertPos = sInfo.nPos;	// 记录初始位置
			switch (LOWORD(wParam))
			{
			case SB_LINEUP:
				{
					sInfo.nPos = max(sInfo.nPos - 1, sInfo.nMin);
				}
				break;
			case SB_LINEDOWN:
				{
					sInfo.nPos = min(sInfo.nPos + 1, sInfo.nMax - sInfo.nPage + 1);
				}
				break;
			case SB_PAGEUP:
				{
					sInfo.nPos = max(sInfo.nPos - (int)sInfo.nPage, sInfo.nMin);
				}
				break;
			case SB_PAGEDOWN:
				{
					sInfo.nPos = min(sInfo.nPos + (int)sInfo.nPage, sInfo.nMax - sInfo.nPage + 1);
				}
				break;
			case SB_TOP:
				{
					sInfo.nPos = sInfo.nMin;
				}
				break;
			case SB_BOTTOM:
				{
					sInfo.nPos = sInfo.nMax - sInfo.nPage + 1;
				}
				break;
			case SB_THUMBTRACK:
				{
					if (sInfo.nTrackPos < sInfo.nMin)
						sInfo.nPos = sInfo.nMin;
					else if (sInfo.nTrackPos > sInfo.nMax - sInfo.nPage + 1)
						sInfo.nPos = sInfo.nMax - sInfo.nPage + 1;
					else
						sInfo.nPos = sInfo.nTrackPos;
				}
			default:
				break;
			}
			// 更新显示区位置纵坐标
			pClientPos.y = sInfo.nPos * USHEIGHT(pCharSize.y);
			// ...

			// 更新滚动条
			sInfo.cbSize = sizeof(SCROLLINFO);
			sInfo.fMask = SIF_POS;
			SetScrollInfo(hWnd, SB_VERT, &sInfo, TRUE);
			// ...
			SetCaretPos(pCaretPos.x - pClientPos.x, pCaretPos.y - pClientPos.y);
			if (!INRANGE(pCaretPos.x, pClientPos.x, pClientPos.x + pClientSize.x) || !INRANGE(pCaretPos.y, pClientPos.y, pClientPos.y + pClientSize.y - USHEIGHT(pCharSize.y)))
				HideCaret(hWnd);
			else
				ShowCaret(hWnd);
			// 重绘显示区
			RECT rcScroll;
			rcScroll.left = 0;
			rcScroll.right = pClientSize.x;
			rcScroll.top = 0;
			rcScroll.bottom = pClientSize.y;
			RECT rcPaint;
			ScrollWindowEx(hWnd, 0, (iVertPos - sInfo.nPos) * USHEIGHT(pCharSize.y), &rcScroll, NULL, NULL, &rcPaint, SW_INVALIDATE);

			GetDC(hWnd);
			RECT rcClient;
			GetClientRect(hWnd, &rcClient);
			HDC hdc = GetDC(hWnd);
			RECT rcRight;
			SetRect(&rcRight, pClientSize.x, 0, rcClient.right, rcClient.bottom);
			FillRect(hdc, &rcRight, hBrush);
			RECT rcBottom;
			SetRect(&rcBottom, 0, pClientSize.y, rcClient.right, rcClient.bottom);
			FillRect(hdc, &rcBottom, hBrush);
			ReleaseDC(hWnd, hdc);

			InvalidateRect(hWnd, &rcPaint, FALSE);
			UpdateWindow(hWnd);
		}
		break;
	case WM_HSCROLL:
		{
			// 获取滚动条信息
			SCROLLINFO sInfo;
			sInfo.cbSize = sizeof(SCROLLINFO);
			sInfo.fMask = SIF_ALL;
			GetScrollInfo(hWnd, SB_HORZ, &sInfo);
			// ...

			int iVertPos = sInfo.nPos;	// 记录初始位置
			switch (LOWORD(wParam))
			{
			case SB_LINELEFT:
				{
					sInfo.nPos = max(sInfo.nPos - 1, sInfo.nMin);
				}
				break;
			case SB_LINERIGHT:
				{
					sInfo.nPos = min(sInfo.nPos + 1, sInfo.nMax - sInfo.nPage + 1);
				}
				break;
			case SB_PAGELEFT:
				{
					sInfo.nPos = max(sInfo.nPos - (int)sInfo.nPage, sInfo.nMin);
				}
				break;
			case SB_PAGERIGHT:
				{
					sInfo.nPos = min(sInfo.nPos + (int)sInfo.nPage, sInfo.nMax - sInfo.nPage + 1);
				}
				break;
			case SB_LEFT:
				{
					sInfo.nPos = sInfo.nMin;
				}
				break;
			case SB_RIGHT:
				{
					sInfo.nPos = sInfo.nMax - sInfo.nPage + 1;
				}
				break;
			case SB_THUMBTRACK:
				{
					if (sInfo.nTrackPos < sInfo.nMin)
						sInfo.nPos = sInfo.nMin;
					else if (sInfo.nTrackPos > sInfo.nMax - sInfo.nPage + 1)
						sInfo.nPos = sInfo.nMax - sInfo.nPage + 1;
					else
						sInfo.nPos = sInfo.nTrackPos;
				}
				break;
			default:
				break;
			}
			// 更新显示区位置横坐标
			pClientPos.x = sInfo.nPos * USWIDTH(pCharSize.x);
			// ...

			// 更新滚动条
			sInfo.cbSize = sizeof(SCROLLINFO);
			sInfo.fMask = SIF_POS;
			SetScrollInfo(hWnd, SB_HORZ, &sInfo, TRUE);
			// ...
			SetCaretPos(pCaretPos.x - pClientPos.x, pCaretPos.y - pClientPos.y);
			if (!INRANGE(pCaretPos.x, pClientPos.x, pClientPos.x + pClientSize.x) || !INRANGE(pCaretPos.y, pClientPos.y, pClientPos.y + pClientSize.y - USHEIGHT(pCharSize.y)))
				HideCaret(hWnd);
			else
				ShowCaret(hWnd);
			// 重绘显示区
			RECT rcScroll;
			rcScroll.left = 0;
			rcScroll.right = pClientSize.x;
			rcScroll.top = 0;
			rcScroll.bottom = pClientSize.y;
			RECT rcPaint;
			ScrollWindowEx(hWnd, (iVertPos - sInfo.nPos) * USWIDTH(pCharSize.x), 0, &rcScroll, NULL, NULL, &rcPaint, SW_INVALIDATE);

			GetDC(hWnd);
			RECT rcClient;
			GetClientRect(hWnd, &rcClient);
			HDC hdc = GetDC(hWnd);
			RECT rcRight;
			SetRect(&rcRight, pClientSize.x, 0, rcClient.right, rcClient.bottom);
			FillRect(hdc, &rcRight, hBrush);
			RECT rcBottom;
			SetRect(&rcBottom, 0, pClientSize.y, rcClient.right, rcClient.bottom);
			FillRect(hdc, &rcBottom, hBrush);
			ReleaseDC(hWnd, hdc);

			InvalidateRect(hWnd, &rcPaint, FALSE);
			UpdateWindow(hWnd);
		}
		break;
	case WM_LBUTTONDOWN:
		{
			// 解析鼠标点击位置
			POINT pCursor;
			GetCursorPos(&pCursor);
			ScreenToClient(hWnd, &pCursor);

			RECT rcClient;
			GetClientRect(hWnd, &rcClient);
			if (!INRANGE(pCursor.x, rcClient.left, rcClient.right) ||
				!INRANGE(pCursor.y, rcClient.top, rcClient.bottom))
			{	// 未击中显示区
				return DefWindowProc(hWnd, message, wParam, lParam);
			}
			pCursor.x += pClientPos.x;
			pCursor.y += pClientPos.y;
			// ...

			RVALUE result;
			if ((result = 
				UserMessageProc(hText, pCursor.x, pCursor.y, UM_CURSOR, NULL, NULL)) != UR_ERROR)
			{	
				pCaretPos.x = (LONG)LODWORD(result); pCaretPos.y = (LONG)HIDWORD(result); // 设置光标位置
				AdjustClientPos(hWnd, pClientPos, pCaretPos, pPageSize, pCharSize);		  // 滑动显示区
				SetCaretPos(pCaretPos.x - pClientPos.x, pCaretPos.y - pClientPos.y);  ;	  // 重绘光标
				if (!INRANGE(pCaretPos.x, pClientPos.x, pClientPos.x + pClientSize.x) || !INRANGE(pCaretPos.y, pClientPos.y, pClientPos.y + pClientSize.y - USHEIGHT(pCharSize.y)))
					HideCaret(hWnd);
				else
					ShowCaret(hWnd);
				UpdateRect(hWnd, 0, pClientSize.x, pStartPos.y - pClientPos.y, pEndPos.y - pClientPos.y + USHEIGHT(pCharSize.y));

				// 更新选段
				pStartPos.x = pCaretPos.x; pStartPos.y = pCaretPos.y;
				pEndPos.x   = pStartPos.x; pEndPos.y   = pStartPos.y; 
			}
			else
			{	// 发生异常
				assert(!(result == UR_ERROR));
			}
		}
		break;
	case WM_LBUTTONUP:
		{
			POINT pCursor;	// 用户鼠标落点
			GetCursorPos(&pCursor);
			ScreenToClient(hWnd, &pCursor);
			// 用户鼠标落点转化为绝对坐标
			pCursor.x += pClientPos.x;
			pCursor.y += pClientPos.y;

			RVALUE result;
			if ((result =
				UserMessageProc(hText, pCursor.x, pCursor.y, UM_CHOOSE, (FPARAM)(((FPARAM)pStartPos.y) << 32 | (FPARAM)pStartPos.x), NULL)) != UR_ERROR)
			{
				pCaretPos.x = (LONG)LODWORD(result); pCaretPos.y = (LONG)HIDWORD(result); // 更新光标
				SetCaretPos(pCaretPos.x - pClientPos.x, pCaretPos.y - pClientPos.y);
				if (!INRANGE(pCaretPos.x, pClientPos.x, pClientPos.x + pClientSize.x) || !INRANGE(pCaretPos.y, pClientPos.y, pClientPos.y + pClientSize.y - USHEIGHT(pCharSize.y)))
					HideCaret(hWnd);
				else
					ShowCaret(hWnd);
				pEndPos.x = pCaretPos.x; pEndPos.y = pCaretPos.y; // 更新选段
				if (pStartPos.y > pEndPos.y || (pStartPos.y == pEndPos.y && pStartPos.x > pEndPos.x))
					std::swap(pStartPos, pEndPos);
				 // 重绘显示区
				InvalidateRect(hWnd, NULL, FALSE); UpdateWindow(hWnd);
			}
			else
			{	// 发生异常
				assert(!(result == UR_ERROR));
			}
		}
		break;
	case WM_NCLBUTTONDBLCLK:
		{
			if (HTCAPTION == wParam)
			{

			}
			else
			{
				return DefWindowProc(hWnd, message, wParam, lParam);
			}
		}
		break;
	case WM_NCLBUTTONDOWN:
		{
			if (HTCAPTION == wParam)
			{

			}
			else
			{
				return DefWindowProc(hWnd, message, wParam, lParam);
			}
		}
		break;
	case WM_MOUSEMOVE:
		{
			POINT pCursor;	// 用户鼠标落点
			GetCursorPos(&pCursor);
			ScreenToClient(hWnd, &pCursor);
			switch (wParam)
			{
			case MK_LBUTTON:
				{	// 按住左键
					if (hMouseTimer != nullptr && IsTimerStopped(hMouseTimer) == FALSE)
						return 0;

					if (hMouseTimer != nullptr)
						KillTimer(hMouseTimer), hMouseTimer = nullptr;
					else
						hMouseTimer = CreateTimer(100);

					if (pCursor.y > pClientSize.y)
					{	// 下移
						SendMessage(hWnd, WM_VSCROLL, SB_LINEDOWN, NULL);
					}
					else if (pCursor.x > pClientSize.x)
					{	// 右移
						SendMessage(hWnd, WM_HSCROLL, SB_LINERIGHT, NULL);
					}
					else if (pCursor.y < 0)
					{	// 上移
						SendMessage(hWnd, WM_VSCROLL, SB_LINEUP, NULL);
					}
					else if (pCursor.x < 0)
					{	// 下移
						SendMessage(hWnd, WM_HSCROLL, SB_LINELEFT, NULL);
					}
					// 用户鼠标落点转化为绝对坐标
					pCursor.x += pClientPos.x;
					pCursor.y += pClientPos.y;

					RVALUE result;
					if ((result =
						UserMessageProc(hText, pCursor.x, pCursor.y, UM_CHOOSE, (FPARAM)(((FPARAM)pStartPos.y) << 32 | (FPARAM)pStartPos.x), NULL)) != UR_ERROR)
					{
						pCaretPos.x = (LONG)LODWORD(result); pCaretPos.y = (LONG)HIDWORD(result); // 更新光标
						SetCaretPos(pCaretPos.x - pClientPos.x, pCaretPos.y - pClientPos.y);
						if (!INRANGE(pCaretPos.x, pClientPos.x, pClientPos.x + pClientSize.x) || !INRANGE(pCaretPos.y, pClientPos.y, pClientPos.y + pClientSize.y - USHEIGHT(pCharSize.y)))
							HideCaret(hWnd);
						else
							ShowCaret(hWnd);
						pEndPos.x = pCaretPos.x; pEndPos.y = pCaretPos.y; // 更新选段
					    // 重绘显示区
						InvalidateRect(hWnd, NULL, FALSE); UpdateWindow(hWnd);
					}
					else
					{	// 发生异常
						assert(!(result == UR_ERROR));
					}
				}
				break;
			default:
				return DefWindowProc(hWnd, message, wParam, lParam);
			}
		}
		break;
    case WM_PAINT:
        {
            PAINTSTRUCT ps;
            HDC hdc = BeginPaint(hWnd, &ps);

			// 显示区矩形区域
			RECT rcClient;
			GetClientRect(hWnd, &rcClient);
			FillRect(hMemDC, &rcClient, hBrush);

			RECT rcPaint;
			rcPaint.left = max(ps.rcPaint.left, 0);
			rcPaint.right = min(ps.rcPaint.right, pClientSize.x);
			rcPaint.top = max(ps.rcPaint.top, 0);
			rcPaint.bottom = min(ps.rcPaint.bottom, pClientSize.y);

			RVALUE result;
			LPWSTR lpchText = nullptr;
			for (LONG yOffset = rcClient.top; yOffset < rcClient.bottom; yOffset += USHEIGHT(pCharSize.y))
			{
				if ((result =
					UserMessageProc(hText, pClientPos.x + rcClient.left, pClientPos.y + yOffset,
						UM_TEXT, (FPARAM)((FPARAM)(pClientPos.y + yOffset) << 32 | (FPARAM)(pClientPos.x + rcClient.right + USWIDTH(pCharSize.x))),
						(SPARAM)(&lpchText))) != UR_ERROR)
				{
					int		iCount = (int)HIDWORD(result);
					short	iStart = (short)LOWORD(LODWORD(result));
					short	iEnd = (short)HIWORD(LODWORD(result));

					if (iCount > 0)
						MyTextOut(hMemDC, 0, yOffset, lpchText, iCount, iStart, iEnd, USWIDTH(pCharSize.x));
				}
			}

			BitBlt(hdc,
				rcPaint.left, rcPaint.top, rcPaint.right - rcPaint.left, rcPaint.bottom - rcPaint.top,
				hMemDC, rcPaint.left, rcPaint.top, SRCCOPY);

            EndPaint(hWnd, &ps);
        }
        break;
    case WM_DESTROY:
		{
			DestroyCaret();
			KillTimer(hMouseTimer);
			DeleteDC(hMemDC);
			DeleteObject(hBrush);
			DeleteObject(hBitmap);
			PostQuitMessage(0);
		}
        break;
    default:
		return DefWindowProc(hWnd, message, wParam, lParam);
    }
    return 0;
}

// “关于”框的消息处理程序。
INT_PTR CALLBACK About(HWND hDlg, UINT message, WPARAM wParam, LPARAM lParam)
{
    UNREFERENCED_PARAMETER(lParam);
    switch (message)
    {
    case WM_INITDIALOG:
        return (INT_PTR)TRUE;

    case WM_COMMAND:
        if (LOWORD(wParam) == IDOK || LOWORD(wParam) == IDCANCEL)
        {
            EndDialog(hDlg, LOWORD(wParam));
            return (INT_PTR)TRUE;
        }
        break;
    }
    return (INT_PTR)FALSE;
}

// “查找”框的消息处理程序。
INT_PTR CALLBACK Find(HWND hDlg, UINT message, WPARAM wParam, LPARAM lParam)
{
	UNREFERENCED_PARAMETER(lParam);
	switch (message)
	{
	case WM_INITDIALOG:
		return (INT_PTR)TRUE;

	case WM_COMMAND:
		{
			if (LOWORD(wParam) == IDOK)
			{
				EndDialog(hDlg, LOWORD(wParam));
				return (INT_PTR)TRUE;
			}
			else if (LOWORD(wParam) == IDCANCEL)
			{
				EndDialog(hDlg, LOWORD(wParam));
				return (INT_PTR)TRUE;
			}
		}
		break;
	}
	return (INT_PTR)FALSE;
}

BOOL AdjustWindowSize(HWND hWnd, const POINT& pWindowSize, const POINT& pClientSize, const POINT& pCharSize)
{
	// 调整显示区大小
	SCROLLINFO shInfo;
	shInfo.cbSize = sizeof(SCROLLINFO);
	shInfo.fMask = SIF_RANGE | SIF_PAGE;
	GetScrollInfo(hWnd, SB_HORZ, &shInfo);

	// 调整滚动条范围
	shInfo.nPage = pClientSize.x / USWIDTH(pCharSize.x);
	shInfo.nMin  = 0;
	shInfo.nMax  = max(pWindowSize.x / USWIDTH(pCharSize.x) - 1, shInfo.nPage - 1);

	shInfo.cbSize = sizeof(SCROLLINFO);
	shInfo.fMask = SIF_RANGE | SIF_PAGE;
	SetScrollInfo(hWnd, SB_HORZ, &shInfo, TRUE);

	SCROLLINFO svInfo;
	svInfo.cbSize = sizeof(SCROLLINFO);
	svInfo.fMask = SIF_RANGE | SIF_PAGE;
	GetScrollInfo(hWnd, SB_VERT, &svInfo);

	// 调整滚动条范围
	svInfo.nPage = pClientSize.y / USHEIGHT(pCharSize.y);
	svInfo.nMin  = 0;
	svInfo.nMax  = max(pWindowSize.y / USHEIGHT(pCharSize.y) - 1, svInfo.nPage - 1);

	svInfo.cbSize = sizeof(SCROLLINFO);
	svInfo.fMask = SIF_RANGE | SIF_PAGE;
	SetScrollInfo(hWnd, SB_VERT, &svInfo, TRUE);

	return (TRUE);
}

BOOL AdjustClientPos(HWND hWnd, POINT& pClientPos, const POINT& pCaretPos, const POINT& pPageSize, const POINT& pCharSize)
{
	if (!INRANGE(pCaretPos.x, pClientPos.x, pClientPos.x + pPageSize.x * USWIDTH(pCharSize.x)))
	{
		if (INRANGE(pCaretPos.y, pClientPos.y, pClientPos.y + (pPageSize.y - 1) * USHEIGHT(pCharSize.y)))
		{
			int xOffset =
				pCaretPos.x - pClientPos.x - (pPageSize.x >> 1) * USWIDTH(pCharSize.x);

			SCROLLINFO sInfo;
			sInfo.cbSize = sizeof(SCROLLINFO);
			sInfo.fMask = SIF_POS | SIF_RANGE | SIF_PAGE;
			GetScrollInfo(hWnd, SB_HORZ, &sInfo);

			int iOldHorzPos = sInfo.nPos;
			sInfo.nPos = sInfo.nPos + xOffset / USWIDTH(pCharSize.x);
			if (sInfo.nPos < sInfo.nMin)
				sInfo.nPos = sInfo.nMin;
			if (sInfo.nPos > sInfo.nMax - sInfo.nPage + 1)
				sInfo.nPos = sInfo.nMax - sInfo.nPage + 1;
			int iNowHorzPos = sInfo.nPos;

			sInfo.cbSize = sizeof(SCROLLINFO);
			sInfo.fMask = SIF_POS;
			SetScrollInfo(hWnd, SB_HORZ, &sInfo, TRUE);

			pClientPos.x = iNowHorzPos * USWIDTH(pCharSize.x);

			RECT rcScroll;
			rcScroll.left   = 0;
			rcScroll.right  = pPageSize.x * USWIDTH(pCharSize.x);
			rcScroll.top    = 0;
			rcScroll.bottom = pPageSize.y * USHEIGHT(pCharSize.y);
			RECT rcPaint;
			ScrollWindowEx(hWnd, (iOldHorzPos - iNowHorzPos) * USWIDTH(pCharSize.x), 0, &rcScroll, 
				NULL, NULL, &rcPaint, SW_INVALIDATE);
			InvalidateRect(hWnd, &rcPaint, FALSE);
		}
		else
		{
			int xOffset =
				pCaretPos.x - pClientPos.x - (pPageSize.x >> 1) * USWIDTH(pCharSize .x);
			int yOffset =
				pCaretPos.y - pClientPos.y - (pPageSize.y >> 1) * USHEIGHT(pCharSize.y);

			SCROLLINFO svInfo;
			svInfo.cbSize = sizeof(SCROLLINFO);
			svInfo.fMask = SIF_POS | SIF_RANGE | SIF_PAGE;
			GetScrollInfo(hWnd, SB_VERT, &svInfo);

			int iOldVertPos = svInfo.nPos;
			svInfo.nPos = svInfo.nPos + yOffset / USHEIGHT(pCharSize.y);
			if (svInfo.nPos < svInfo.nMin)
				svInfo.nPos = svInfo.nMin;
			if (svInfo.nPos > svInfo.nMax - svInfo.nPage + 1)
				svInfo.nPos = svInfo.nMax - svInfo.nPage + 1;
			int iNowVertPos = svInfo.nPos;

			svInfo.cbSize = sizeof(SCROLLINFO);
			svInfo.fMask = SIF_POS;
			SetScrollInfo(hWnd, SB_VERT, &svInfo, TRUE);

			SCROLLINFO shInfo;
			shInfo.cbSize = sizeof(SCROLLINFO);
			shInfo.fMask = SIF_POS | SIF_RANGE | SIF_PAGE;
			GetScrollInfo(hWnd, SB_HORZ, &shInfo);

			int iOldHorzPos = shInfo.nPos;
			shInfo.nPos = shInfo.nPos + xOffset / USWIDTH(pCharSize.x);
			if (shInfo.nPos < shInfo.nMin)
				shInfo.nPos = shInfo.nMin;
			if (shInfo.nPos > shInfo.nMax - shInfo.nPage + 1)
				shInfo.nPos = shInfo.nMax - shInfo.nPage + 1;
			int iNowHorzPos = shInfo.nPos;

			shInfo.cbSize = sizeof(SCROLLINFO);
			shInfo.fMask = SIF_POS;
			SetScrollInfo(hWnd, SB_HORZ, &shInfo, TRUE);

			pClientPos.x = iNowHorzPos * USWIDTH(pCharSize.x);
			pClientPos.y = iNowVertPos * USHEIGHT(pCharSize.y);

			RECT rcScroll;
			rcScroll.left   = 0;
			rcScroll.right  = pPageSize.x * USWIDTH(pCharSize.x);
			rcScroll.top    = 0;
			rcScroll.bottom = pPageSize.y * USHEIGHT(pCharSize.y);
			RECT rcPaint;
			ScrollWindowEx(hWnd, (iOldHorzPos - iNowHorzPos) * USWIDTH(pCharSize.x), (iOldVertPos - iNowVertPos) * USHEIGHT(pCharSize.y),
				&rcScroll, NULL, NULL, &rcPaint, SW_INVALIDATE);
			InvalidateRect(hWnd, &rcPaint, FALSE);
		}
	}
	else
	{
		if (!INRANGE(pCaretPos.y, pClientPos.y, pClientPos.y + (pPageSize.y - 1) * USHEIGHT(pCharSize.y)))
		{
			int yOffset =
				pCaretPos.y - pClientPos.y - (pPageSize.y >> 1) * USHEIGHT(pCharSize.y);

			SCROLLINFO sInfo;
			sInfo.cbSize = sizeof(SCROLLINFO);
			sInfo.fMask = SIF_POS | SIF_RANGE | SIF_PAGE;
			GetScrollInfo(hWnd, SB_VERT, &sInfo);

			int iOldVertPos = sInfo.nPos;
			sInfo.nPos = sInfo.nPos + yOffset / USHEIGHT(pCharSize.y);
			if (sInfo.nPos < sInfo.nMin)
				sInfo.nPos = sInfo.nMin;
			if (sInfo.nPos > sInfo.nMax - sInfo.nPage + 1)
				sInfo.nPos = sInfo.nMax - sInfo.nPage + 1;
			int iNowVertPos = sInfo.nPos;

			sInfo.cbSize = sizeof(SCROLLINFO);
			sInfo.fMask = SIF_POS;
			SetScrollInfo(hWnd, SB_VERT, &sInfo, TRUE);

			pClientPos.y = iNowVertPos * USHEIGHT(pCharSize.y);

			RECT rcScroll;
			rcScroll.left   = 0;
			rcScroll.right  = pPageSize.x * USWIDTH(pCharSize.x);
			rcScroll.top    = 0;
			rcScroll.bottom = pPageSize.y * USHEIGHT(pCharSize.y);
			RECT rcPaint;
			ScrollWindowEx(hWnd, 0,
				(iOldVertPos - iNowVertPos) * USHEIGHT(pCharSize.y), &rcScroll, NULL, NULL, &rcPaint, SW_INVALIDATE);
			InvalidateRect(hWnd, &rcPaint, FALSE);
		}
	}
	return (TRUE);
}

BOOL MoveClient(HWND hWnd, const POINT& pCaretPos, POINT& pClientPos, const POINT& pPageSize, const POINT& pCharSize, POINT& pStartPos, POINT& pEndPos)
{
	AdjustClientPos(hWnd, pClientPos, pCaretPos, pPageSize, pCharSize);		// 滑动显示区

	GetDC(hWnd);
	HBRUSH hBrush = CreateSolidBrush(RGB(0xFF, 0xFF, 0xFF));
	RECT rcClient;
	GetClientRect(hWnd, &rcClient);
	HDC hdc = GetDC(hWnd);
	RECT rcRight;
	SetRect(&rcRight, pPageSize.x * USWIDTH(pCharSize.x), 0, rcClient.right, rcClient.bottom);
	FillRect(hdc, &rcRight, hBrush);
	RECT rcBottom;
	SetRect(&rcBottom, 0, pPageSize.x * USWIDTH(pCharSize.x), rcClient.right, rcClient.bottom);
	FillRect(hdc, &rcBottom, hBrush);
	ReleaseDC(hWnd, hdc);

	SetCaretPos(pCaretPos.x - pClientPos.x, pCaretPos.y - pClientPos.y);	// 重绘光标
	if (!INRANGE(pCaretPos.x, pClientPos.x, pClientPos.x + pPageSize.x * USWIDTH(pCharSize.x)) || !INRANGE(pCaretPos.y, pClientPos.y, pClientPos.y + (pPageSize.y - 1) * USHEIGHT(pCharSize.y)))
		HideCaret(hWnd);
	else
		ShowCaret(hWnd);
	UpdateRect(hWnd,
		0, pPageSize.x * pCharSize.x + USWIDTH(pCharSize.x),
		pStartPos.y - pClientPos.y, pEndPos.y - pClientPos.y + USHEIGHT(pCharSize.y));

	// 更新选段
	pStartPos.x = pCaretPos.x; pStartPos.y = pCaretPos.y;
	pEndPos.x   = pStartPos.x; pEndPos.y   = pStartPos.y;
	DeleteObject(hBrush);
	return (TRUE);
}

BOOL UpdateRect(HWND hWnd, const LONG& left, const LONG& right, const LONG& top, const LONG& bottom)
{
	static RECT rcPaint;

	rcPaint.left    = left;
	rcPaint.right   = right;
	rcPaint.top     = top;
	rcPaint.bottom  = bottom;

	InvalidateRect(hWnd, &rcPaint, FALSE);
	UpdateWindow(hWnd);

	return (TRUE);
}
