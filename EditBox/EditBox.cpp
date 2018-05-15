//EditBox.cpp: 定义应用程序的入口点。
//

#include <algorithm>
#include <cstring>

#include "stdafx.h"
#include "editbox.h"
#include "debug.h"

#define MAX_LOADSTRING 100

// 全局变量: 
HINSTANCE hInst;                                // 当前实例
WCHAR     szTitle[MAX_LOADSTRING];              // 标题栏文本
WCHAR     szWindowClass[MAX_LOADSTRING];        // 主窗口类名
HTEXTINFO hTextInfo;							// 单个文本信息管理实例
WCHAR     szFindWhat[MAX_LOADSTRING];
WCHAR     szReplaceWhat[MAX_LOADSTRING];
ULONGLONG uiFindMode;

// 此代码模块中包含的函数的前向声明: 
ATOM                MyRegisterClass(HINSTANCE hInstance);
BOOL                InitInstance(HINSTANCE, int);
LRESULT CALLBACK    WndProc(HWND, UINT, WPARAM, LPARAM);
INT_PTR CALLBACK    About(HWND, UINT, WPARAM, LPARAM);
INT_PTR CALLBACK	Find(HWND, UINT, WPARAM, LPARAM);

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

	// 执行应用程序初始化: 
	if (!InitInstance(hInstance, nCmdShow))
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

	return (int)msg.wParam;
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

	wcex.style = NULL;
	wcex.lpfnWndProc = WndProc;
	wcex.cbClsExtra = 0;
	wcex.cbWndExtra = 0;
	wcex.hInstance = hInstance;
	wcex.hIcon = LoadIcon(hInstance, MAKEINTRESOURCE(IDI_EDITBOX));
	wcex.hCursor = LoadCursor(nullptr, IDC_IBEAM);
	wcex.hbrBackground = (HBRUSH)(COLOR_WINDOW + 1);
	wcex.lpszMenuName = MAKEINTRESOURCEW(IDC_EDITBOX);
	wcex.lpszClassName = szWindowClass;
	wcex.hIconSm = LoadIcon(wcex.hInstance, MAKEINTRESOURCE(IDI_SMALL));

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
		WS_HSCROLL |		// 水平滚动条
		WS_MINIMIZEBOX |	// 最小化按钮
		WS_VSCROLL		    // 竖直滚动条
		,
		CW_USEDEFAULT, CW_USEDEFAULT, CW_USEDEFAULT, CW_USEDEFAULT, nullptr, nullptr, hInstance, nullptr);

	if (!hWnd)
	{
		return FALSE;
	}

	ShowWindow(hWnd, nCmdShow);
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
	switch (message)
	{
	case WM_CREATE:	// 创建窗口
	{
#ifdef DEBUG
		assert(AllocConsole());
		FILE *stream;
		freopen_s(&stream, "CONIN$", "r+t", stdin);
		freopen_s(&stream, "CONOUT$", "w+t", stdout);

		wprintf_s(TEXT("Window created.\n"));
#endif // DEBUG

		HWND   hDesk;
		RECT   rc;
		hDesk = GetDesktopWindow();
		GetWindowRect(hDesk, &rc);
		//SetWindowLong(hWnd, GWL_STYLE, WS_CAPTION);
		//SetWindowPos(hWnd, HWND_TOPMOST, 0, 0, rc.right, rc.bottom, SWP_SHOWWINDOW);

		hTextInfo = CreateTextInfo(hWnd, hInst);

		SendMessage(hWnd, WM_COMMAND, IDM_NEW, NULL);
	}
	break;
	case WM_SHOWWINDOW:	// 显示窗口
	{

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
#ifdef DEBUG
			wprintf_s(TEXT("User want to create a new file.\n"));
#endif // DEBUG

			switch (UserMessageProc(HTEXT(hTextInfo),
				NULL, NULL, UM_NEW, NULL, NULL, NULL))
			{
			case UR_NOTSAVED:
			{	// 用户未保存当前文件
#ifdef DEBUG
				wprintf_s(TEXT("Current file has not been saved.\n"));
#endif // DEBUG

				SendMessage(hWnd, WM_COMMAND, IDM_SAVE, NULL);	// 提示用户保存
				switch (UserMessageProc(HTEXT(hTextInfo),
					NULL, NULL, UM_NEW, NULL, NULL, NULL))
				{	// 再次新建
				case UR_SUCCESS:
				{	// 修改文本标题
					SelectWindow(hWnd, hTextInfo, POINT{ 0, 0 }, NULL); // 默认文本窗口显示
#ifdef DEBUG
					wprintf_s(TEXT("Create new file: "));
					wprintf_s(TEXT("Current text information: "));
#endif // DEBUG
				}
				case UR_ERROR:
				{
				}
				default:
					break;
				}
			}
			case UR_SAVED:
			{	// 用户已保存当前文件
				SelectWindow(hWnd, hTextInfo, POINT{ 0, 0 }, NULL); // 默认文本窗口显示
#ifdef DEBUG
				wprintf_s(TEXT("Create new file: "));
				wprintf_s(TEXT("Current text information: "));
#endif // DEBUG
			}
			case UR_ERROR:
			{
			}
			default:
				break;
			}
		}
		break;
		case IDM_OPEN:
		{	// 打开文件
			// 弹出对话框
			TCHAR szFileName[MAX_PATH] = { 0 };	// 文件名记录
			OPENFILENAME ofn = { 0 };
			ofn.lStructSize = sizeof(OPENFILENAME);
			ofn.hwndOwner = hWnd;
			ofn.lpstrTitle = TEXT("打开文件");
			ofn.lpstrFile = szFileName;
			ofn.lpstrFilter = TEXT("文本文档(*.txt)");
			ofn.nMaxFile = MAX_PATH;
			ofn.nFilterIndex = 1;
			ofn.lpstrInitialDir = TEXT("./");	// 初始化路径
			ofn.Flags = OFN_FILEMUSTEXIST | OFN_EXPLORER | OFN_HIDEREADONLY | OFN_PATHMUSTEXIST;

			if (GetOpenFileName(&ofn) && lstrcmp(szFileName, TEXT("")) != 0)
			{	// 文件名合法
#ifdef DEBUG
				wprintf_s(TEXT("Open file: %ls\n"), szFileName);
#endif // DEBUG
				KERNELINFO kernelinfo;
				switch (UserMessageProc(HTEXT(hTextInfo),
					NULL, NULL, UM_OPEN, NULL, (SPARAM)szFileName, &kernelinfo))
				{
				case UR_NOTSAVED:
				{	// 用户未保存当前文件
					SendMessage(hWnd, WM_COMMAND, IDM_SAVE, NULL);	// 提示用户保存当前文件
					switch (UserMessageProc(HTEXT(hTextInfo),
						NULL, NULL, UM_OPEN, NULL, (SPARAM)szFileName, &kernelinfo))
					{
					case UR_SUCCESS:
					{
						SelectWindow(hWnd, hTextInfo, kernelinfo.m_pTextPixelSize, szFileName);
					}
					break;
					default:
						break;
					}
				}
				break;
				case UR_SUCCESS:
				{	// 用户已保存当前文件 直接打开文件
					SelectWindow(hWnd, hTextInfo, kernelinfo.m_pTextPixelSize, szFileName);
				}
				break;
				default:
				{
				}
				break;
				}
			}
		}
		break;
		case IDM_SAVE:
		{	// 保存文件
			switch (UserMessageProc(HTEXT(hTextInfo),
				NULL, NULL, UM_SAVE, NULL, NULL, NULL))
			{
			case UR_NOPATH:
			{	// 用户未设置保存路径
				SendMessage(hWnd, WM_COMMAND, IDM_SAVEAS, NULL);	// 提示用户进行另存为操作
			}
			break;
			case UR_ERROR:
			{
			}
			break;
			default:	// 保存成功
				break;
			}
		}
		break;
		case IDM_SAVEAS:
		{	// 另存为
			TCHAR szFileName[MAX_PATH] = { 0 };	// 路径记录
			OPENFILENAME ofn;
			ofn.lStructSize = sizeof(OPENFILENAME);
			ofn.hwndOwner = hWnd;
			ofn.hInstance = NULL;
			ofn.lpstrFilter = TEXT("文本文档(*.txt)");
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
				switch (UserMessageProc(HTEXT(hTextInfo),
					NULL, NULL, UM_SAVE, NULL, (SPARAM)szFileName, NULL))
				{
				case UR_SUCCESS:
					break;
				default:
					break;
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
			uiFindMode &= 0x0; uiFindMode |= FIND;
			DialogBox(hInst, MAKEINTRESOURCE(IDD_FINDDIALOG), hWnd, Find);

			chDEBUGMESSAGEBOX(szFindWhat);
		}
		break;
		case IDM_REPLACE:
		{
			uiFindMode &= 0x0; uiFindMode |= REPLACE;
			DialogBox(hInst, MAKEINTRESOURCE(IDD_FINDDIALOG), hWnd, Find);

			chDEBUGMESSAGEBOX(szReplaceWhat);
		}
		break;
		case IDM_CUT:
		{
			BYTE keyState[256] = { 0 };
			keyState[VK_CONTROL] = -1;
			SetKeyboardState(keyState);

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
					LPWSTR lpString = nullptr;	// 获取拷贝串(不负责释放资源 只获取地址)
					KERNELINFO kernelinfo;
					switch (UserMessageProc(HTEXT(hTextInfo),
						NULL, NULL, UM_COPY, NULL, (SPARAM)(&lpString), &kernelinfo))
					{
					case UR_SUCCESS:
					{
						SIZE_T   nSize = kernelinfo.m_uiCount + 1;
						HGLOBAL  hMem = GlobalAlloc(GHND, nSize * 2);	// UNICODE
						if (hMem != NULL)
						{
							LPWSTR   lpchText = (LPWSTR)GlobalLock(hMem);
							lstrcpyn(lpchText, lpString, (int)nSize);
							SetClipboardData(CF_UNICODETEXT, hMem);
						}
					}
					break;
					default:
						break;
					}
				}
				CloseClipboard();
			}
		}
		break;
		case IDM_PASTE:
		{
			if (IsClipboardFormatAvailable(CF_UNICODETEXT))
			{	// UNICODE数据
				if (OpenClipboard(hWnd))
				{	// 从剪切板拷贝
					HGLOBAL hClipMemory = GetClipboardData(CF_UNICODETEXT);
					if (hClipMemory != NULL)
					{
						SIZE_T nSize        = GlobalSize(hClipMemory) / 2;
						LPWSTR lpClipMemory = (LPWSTR)GlobalLock(hClipMemory);
						LPWSTR lpchText     = new TCHAR[nSize + 1]{ 0 };

						lstrcpyn(lpchText, lpClipMemory, (int)nSize); // 粘贴

						GlobalUnlock(hClipMemory);
						CloseClipboard(); // 关闭剪切板

						KERNELINFO kernelinfo;
						switch (UserMessageProc(HTEXT(hTextInfo),
							CARETPOS(hTextInfo).x, CARETPOS(hTextInfo).y,
							UM_PASTE,
							NULL, (SPARAM)lpchText, &kernelinfo))
						{
						case UR_SUCCESS:
						{
							SelectTextSize(hTextInfo, kernelinfo.m_pTextPixelSize); // 设置文本像素大小

							POINT pOldStartPixelPos = STARTPOS(hTextInfo);
							POINT pOldEndPixelPos   = ENDPOS(hTextInfo);
							MyInvalidateRect(hWnd,
								0, CLIENTSIZE(hTextInfo).x,
								STARTPOS(hTextInfo).y - PAINTPOS(hTextInfo).y,
								ENDPOS(hTextInfo).y - PAINTPOS(hTextInfo).y + CHARSIZE(hTextInfo).y
							); // 设定粘贴之前的高亮区域为无效区域

							SelectHighlight(hTextInfo,
								kernelinfo.m_pStartPixelPos, kernelinfo.m_pEndPixelPos
							); // 设置高亮部分

							MyInvalidateRect(hWnd,
								0, CLIENTSIZE(hTextInfo).x,
								STARTPOS(hTextInfo).y - PAINTPOS(hTextInfo).y,
								ENDPOS(hTextInfo).y - PAINTPOS(hTextInfo).y + CHARSIZE(hTextInfo).y
							); // 设定粘贴之后的高亮区域为无效区域

							SelectCaretPos(hTextInfo, ENDPOS(hTextInfo)); // 设置光标在高亮区域的尾部

							AdjustClientPos(hWnd, hTextInfo); // 滑动显示区 使光标落在显示区内部
						}
						break;
						default:
							break;
						}
						delete[] lpchText;	// 释放临时内存
					}
					CloseClipboard();
				}
			}
		}
		break;
		case IDM_CANCEL:
		{
			KERNELINFO kernelinfo;
			switch (UserMessageProc(HTEXT(hTextInfo), NULL, NULL, UM_CANCEL,
				NULL, NULL, &kernelinfo))
			{
			case UR_SUCCESS:
			{
				SelectTextSize(hTextInfo, kernelinfo.m_pTextPixelSize); // 设置文本像素大小

				MyInvalidateRect(hWnd,
					0, CLIENTSIZE(hTextInfo).x,
					STARTPOS(hTextInfo).y - PAINTPOS(hTextInfo).y,
					ENDPOS(hTextInfo).y - PAINTPOS(hTextInfo).y + CHARSIZE(hTextInfo).y
				); // 设定撤销之前的高亮区域为无效区域

				SelectHighlight(hTextInfo,
					kernelinfo.m_pStartPixelPos, kernelinfo.m_pEndPixelPos
				); // 设置高亮部分

				MyInvalidateRect(hWnd,
					0, CLIENTSIZE(hTextInfo).x,
					STARTPOS(hTextInfo).y - PAINTPOS(hTextInfo).y,
					ENDPOS(hTextInfo).y - PAINTPOS(hTextInfo).y + CHARSIZE(hTextInfo).y
				); // 设定撤销之后的高亮区域为无效区域

				SelectCaretPos(hTextInfo, ENDPOS(hTextInfo)); // 设置光标在高亮区域的尾部

				AdjustClientPos(hWnd, hTextInfo); // 滑动显示区 使光标落在显示区内部
			}
			break;
			default:
				break;
			}
		}
		break;
		case IDM_ALL:
		{
			KERNELINFO kernelinfo;
			switch (UserMessageProc(HTEXT(hTextInfo),
				NULL, NULL, UM_ALL, NULL, NULL, &kernelinfo))
			{
			case UR_SUCCESS:
			{
				SelectCaretPos(hTextInfo, kernelinfo.m_pEndPixelPos); // 设置光标在高亮部分的尾部
				SelectHighlight(hTextInfo,
					STARTPOS(hTextInfo), ENDPOS(hTextInfo)
				); // 设置高亮部分为整个文本
				MyInvalidateRect(hWnd,
					0, CLIENTSIZE(hTextInfo).x,
					STARTPOS(hTextInfo).y - PAINTPOS(hTextInfo).y,
					ENDPOS(hTextInfo).y - PAINTPOS(hTextInfo).y + CHARSIZE(hTextInfo).y
				); // 设定全选后的高亮区域为无效区域

				AdjustClientPos(hWnd, hTextInfo); // 滑动显示区 使光标落在显示区内部
			}
			break;
			default:
				break;
			}
		}
		default:
			return DefWindowProc(hWnd, message, wParam, lParam);
		}

		UpdateWindow(hWnd);
	}
	break;
	case WM_CLOSE:
	{
		switch (UserMessageProc(HTEXT(hTextInfo), NULL, NULL, UM_ISSAVED, NULL, NULL, NULL))
		{	// 查询是否已保存文件
		case UR_NOTSAVED:
		{	// 用户未保存当前文件
			switch (MessageBox(hWnd, TEXT("是否保存更改？"), NULL, MB_YESNOCANCEL))
			{
			case IDYES:
				SendMessage(hWnd, WM_COMMAND, IDM_SAVE, NULL);	// 提示用户保存
				UserMessageProc(HTEXT(hTextInfo), NULL, NULL, UM_CLOSE, NULL, NULL, NULL);
				DestroyWindow(hWnd);
				break;
			case IDNO:
				UserMessageProc(HTEXT(hTextInfo), NULL, NULL, UM_CLOSE, NULL, NULL, NULL);
				DestroyWindow(hWnd);
				break;
			case IDCANCEL:
				break;
			default:
				break;
			}
		}
		case UR_SAVED:
			UserMessageProc(HTEXT(hTextInfo), NULL, NULL, UM_CLOSE, NULL, NULL, NULL);
			DestroyWindow(hWnd);
			break;
		default:
			break;
		}
	}
	break;
	case WM_SIZE:
	{
		SendMessage(hTextInfo->m_hGDI->m_hStatus, message, wParam, lParam);

		RECT rect;
		GetClientRect(hWnd, &rect);
		SelectClientSize(hTextInfo, &rect);

		InvalidateRect(hWnd, NULL, FALSE); UpdateWindow(hWnd);
	}
	break;
	case WM_SETFOCUS:
	{	// 获得焦点
		CreateCaret(hWnd, NULL,
			CARETSIZE(hTextInfo).x, CARETSIZE(hTextInfo).y
		); // 新建光标
		SelectCaretPos(hTextInfo, CARETPOS(hTextInfo));	// 设置光标位置

		UpdateWindow(hWnd);
	}
	break;
	case WM_KILLFOCUS:
	{	// 释放焦点
		HideCaret(hWnd);	// 隐藏光标
		DestroyCaret();
	}
	break;
	case WM_KEYDOWN:
	{
		switch (wParam)
		{
		case VK_HOME:
		case VK_END:
		case VK_LEFT:
		case VK_RIGHT:
		case VK_UP:
		case VK_DOWN:
			MoveCaret(hWnd, (UINT)wParam, hTextInfo);
			break;
		case VK_DELETE:
		{
			KERNELINFO kernelinfo;
			switch (UserMessageProc(HTEXT(hTextInfo),
				STARTPOS(hTextInfo).x, STARTPOS(hTextInfo).y,
				UM_DELETE,
				NULL,
				(SPARAM)((SPARAM)ENDPOS(hTextInfo).y << 32 | (SPARAM)ENDPOS(hTextInfo).x), &kernelinfo))
			{
			case UR_SUCCESS:
			{
				MyInvalidateRect(hWnd,
					0, CLIENTSIZE(hTextInfo).x,
					STARTPOS(hTextInfo).y - PAINTPOS(hTextInfo).y,
					ENDPOS(hTextInfo).y - PAINTPOS(hTextInfo).y + CHARSIZE(hTextInfo).y
				); // 设置删除之前的高亮部分为无效区域

				if (STARTPOS(hTextInfo).y != ENDPOS(hTextInfo).y) // 换行符被删除
					MyInvalidateRect(hWnd,
						0, CLIENTSIZE(hTextInfo).x,
						CARETPOS(hTextInfo).y - PAINTPOS(hTextInfo).y - CHARSIZE(hTextInfo).y,
						PAINTSIZE(hTextInfo).y
					); // 设置重光标位置开始往下都为无效区域

				SelectTextSize(hTextInfo, kernelinfo.m_pTextPixelSize); // 设置删除高亮部分之后的文本像素大小

				AdjustClientPos(hWnd, hTextInfo); // 滑动显示区 使光标落在显示区内部
				SelectHighlight(hTextInfo, CARETPOS(hTextInfo), CARETPOS(hTextInfo)); // 设置无高亮部分
			}
			break;
			default:
				break;
			}
		}
		break;
		default:
			break;
		}
		UpdateWindow(hWnd);
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
				AdjustCaretPosBeforeBackspace(hWnd, hTextInfo); // 校准光标位置

				if (STARTPOS(hTextInfo) == POINT{ 0, 0 }
					&& ENDPOS(hTextInfo) == POINT{ 0, 0 })
					return DefWindowProc(hWnd, message, wParam, lParam);
				else // 执行删除
					SendMessage(hWnd, WM_KEYDOWN, VK_DELETE, NULL);
			}
			break;
			case '\n':
			{
			}
			break;
			case '\r':
			{
				KERNELINFO kernelinfo;
				switch (UserMessageProc(HTEXT(hTextInfo),
					CARETPOS(hTextInfo).x, CARETPOS(hTextInfo).y,
					UM_RETURN, NULL, NULL, &kernelinfo))
				{
				case UR_SUCCESS:
				{
					SelectTextSize(hTextInfo, kernelinfo.m_pTextPixelSize); // 设置文本的像素大小
					SelectCaretPos(hTextInfo, kernelinfo.m_pCaretPixelPos); // 设置光标的像素位置

					MyInvalidateRect(hWnd,
						0, CLIENTSIZE(hTextInfo).x,
						STARTPOS(hTextInfo).y - PAINTPOS(hTextInfo).y,
						ENDPOS(hTextInfo).y - PAINTPOS(hTextInfo).y + CHARSIZE(hTextInfo).y
					); // 设置插入换行符之前的高亮部分为无效区域
					SelectHighlight(hTextInfo, CARETPOS(hTextInfo), CARETPOS(hTextInfo)); // 设置无高亮部分
					MyInvalidateRect(hWnd,
						0, CLIENTSIZE(hTextInfo).x,
						CARETPOS(hTextInfo).y - PAINTPOS(hTextInfo).y - CHARSIZE(hTextInfo).y,
						PAINTSIZE(hTextInfo).y
					); // 设置插入换行符之后上一行至底部均为无效区域

					AdjustClientPos(hWnd, hTextInfo); // 滑动显示区 使光标落在显示区内
				}
				break;
				default:
					break;
				}
			}
			break;
			default:
			{
				if (GetAsyncKeyState(VK_CONTROL) < 0)
				{	// 非字符处理（控制命令/快捷键）
					if (wParam == 0x01) // Ctrl + A 全选
						SendMessage(hWnd, WM_COMMAND, IDM_ALL, NULL);
					else if (wParam == 0x03) // Ctrl + C 拷贝
						SendMessage(hWnd, WM_COMMAND, IDM_COPY, NULL);
					else if (wParam == 0x16) // Ctrl + V 粘贴
						SendMessage(hWnd, WM_COMMAND, IDM_PASTE, NULL);
					else if (wParam == 0x18) // Ctrl + X 剪切
						SendMessage(hWnd, WM_COMMAND, IDM_CUT, NULL);
					else if (wParam == 0x1A) // Ctrl + Z 撤销
						SendMessage(hWnd, WM_COMMAND, IDM_CANCEL, NULL);
					else if (wParam == 0x06) // Ctrl + F 查找
						SendMessage(hWnd, WM_COMMAND, IDM_FIND, NULL);
					else if (wParam == 0x13) // Ctrl + S 保存
						SendMessage(hWnd, WM_COMMAND, IDM_SAVE, NULL);
				}
				else
				{
					// 字符处理
#ifdef DEBUG
					wprintf_s(TEXT("Insert charactor: %c\n"), (TCHAR)wParam);
					wprintf_s(TEXT("Text information before insert: "));
#endif // DEBUG
					KERNELINFO kernelinfo;
					switch (UserMessageProc(HTEXT(hTextInfo),
						CARETPOS(hTextInfo).x, CARETPOS(hTextInfo).y,
						UM_CHAR,
						NULL, (SPARAM)wParam, &kernelinfo))
					{
					case UR_SUCCESS:
					{
						SelectTextSize(hTextInfo, kernelinfo.m_pTextPixelSize); // 设置文本的像素大小
						SelectCaretPos(hTextInfo, kernelinfo.m_pCaretPixelPos); // 设置光标的像素位置

						MyInvalidateRect(hWnd,
							0, CLIENTSIZE(hTextInfo).x,
							STARTPOS(hTextInfo).y - PAINTPOS(hTextInfo).y,
							ENDPOS(hTextInfo).y - PAINTPOS(hTextInfo).y + CHARSIZE(hTextInfo).y
						); // 设置插入换行符之前的高亮部分为无效区域

						if (STARTPOS(hTextInfo).y != ENDPOS(hTextInfo).y) // 换行符被替换
							MyInvalidateRect(hWnd,
								0, CLIENTSIZE(hTextInfo).x,
								CARETPOS(hTextInfo).y - PAINTPOS(hTextInfo).y - CHARSIZE(hTextInfo).y,
								PAINTSIZE(hTextInfo).y
							); // 设置插入换行符之后上一行至底部均为无效区域

						SelectHighlight(hTextInfo, CARETPOS(hTextInfo), CARETPOS(hTextInfo)); // 设置无高亮部分

						AdjustClientPos(hWnd, hTextInfo); // 滑动显示区 使光标落在显示区内
					}
					break;
					default:
						break;
					}
				}
#ifdef DEBUG
				wprintf_s(TEXT("Text information after insert: "));
#endif // DEBUG

			}
			break;
			}
		}
		UpdateWindow(hWnd);
	}
	break;
	case WM_VSCROLL:
	{
		// 获取滚动条信息
		SCROLLINFO	sInfo;
		sInfo.cbSize = sizeof(SCROLLINFO);
		sInfo.fMask = SIF_ALL;
		GetScrollInfo(hWnd, SB_VERT, &sInfo);

		int iVertPos = sInfo.nPos;	// 记录初始位置
		switch (LOWORD(wParam))
		{
		case SB_LINEUP:
			sInfo.nPos = max(sInfo.nPos - 1, sInfo.nMin);
			break;
		case SB_LINEDOWN:
			sInfo.nPos = min(sInfo.nPos + 1, sInfo.nMax - (int)sInfo.nPage + 1);
			break;
		case SB_PAGEUP:
			sInfo.nPos = max(sInfo.nPos - (int)sInfo.nPage, sInfo.nMin);
			break;
		case SB_PAGEDOWN:
			sInfo.nPos = min(sInfo.nPos + (int)sInfo.nPage, sInfo.nMax - (int)sInfo.nPage + 1);
			break;
		case SB_TOP:
			sInfo.nPos = sInfo.nMin;
			break;
		case SB_BOTTOM:
			sInfo.nPos = sInfo.nMax - sInfo.nPage + 1;
			break;
		case SB_THUMBTRACK:
			if (sInfo.nTrackPos < sInfo.nMin)
				sInfo.nPos = sInfo.nMin;
			else if (sInfo.nTrackPos > sInfo.nMax - (int)sInfo.nPage + 1)
				sInfo.nPos = sInfo.nMax - sInfo.nPage + 1;
			else
				sInfo.nPos = sInfo.nTrackPos;
			break;
		default:
			break;
		}
		SelectWindowPos(hTextInfo,
			POINT{ WINDOWPOS(hTextInfo).x, sInfo.nPos * USHEIGHT(CHARSIZE(hTextInfo).y) }
		);	// 设置窗口位置纵坐标

		// 更新滚动条
		sInfo.cbSize = sizeof(SCROLLINFO);
		sInfo.fMask = SIF_POS;
		SetScrollInfo(hWnd, SB_VERT, &sInfo, TRUE);

		SelectCaretPos(hTextInfo, CARETPOS(hTextInfo));	// 设置光标位置

		MyScrollWindow(hWnd, 0, (iVertPos - sInfo.nPos) * USHEIGHT(CHARSIZE(hTextInfo).y));	// 滑动窗口
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

		int iHorzPos = sInfo.nPos;	// 记录初始位置
		switch (LOWORD(wParam))
		{
		case SB_LINELEFT:
			sInfo.nPos = max(sInfo.nPos - 1, sInfo.nMin);
			break;
		case SB_LINERIGHT:
			sInfo.nPos = min(sInfo.nPos + 1, sInfo.nMax - (int)sInfo.nPage + 1);
			break;
		case SB_PAGELEFT:
			sInfo.nPos = max(sInfo.nPos - (int)sInfo.nPage, sInfo.nMin);
			break;
		case SB_PAGERIGHT:
			sInfo.nPos = min(sInfo.nPos + (int)sInfo.nPage, sInfo.nMax - (int)sInfo.nPage + 1);
			break;
		case SB_LEFT:
			sInfo.nPos = sInfo.nMin;
			break;
		case SB_RIGHT:
			sInfo.nPos = sInfo.nMax - sInfo.nPage + 1;
			break;
		case SB_THUMBTRACK:
			if (sInfo.nTrackPos < sInfo.nMin)
				sInfo.nPos = sInfo.nMin;
			else if (sInfo.nTrackPos > sInfo.nMax - (int)sInfo.nPage + 1)
				sInfo.nPos = sInfo.nMax - sInfo.nPage + 1;
			else
				sInfo.nPos = sInfo.nTrackPos;
			break;
		default:
			break;
		}
		SelectWindowPos(hTextInfo,
			POINT{ sInfo.nPos * USWIDTH(CHARSIZE(hTextInfo).x), WINDOWPOS(hTextInfo).y }
		); // 设置窗口位置横坐标

		// 更新滚动条
		sInfo.cbSize = sizeof(SCROLLINFO);
		sInfo.fMask = SIF_POS;
		SetScrollInfo(hWnd, SB_HORZ, &sInfo, TRUE);

		SelectCaretPos(hTextInfo, CARETPOS(hTextInfo));	// 设置光标位置

		MyScrollWindow(hWnd, (iHorzPos - sInfo.nPos) * USWIDTH(CHARSIZE(hTextInfo).x), 0);	// 滑动窗口
		UpdateWindow(hWnd);
	}
	break;
	case WM_LBUTTONDOWN:
	{
		// 解析鼠标点击位置
		POINT pCursor;
		GetCursorPos(&pCursor);
		ScreenToClient(hWnd, &pCursor);

		if (!INRANGEX(pCursor.x, 0, PAGESIZE(hTextInfo).x * USWIDTH(CHARSIZE(hTextInfo).x)) ||
			!INRANGEY(pCursor.y, 0, PAGESIZE(hTextInfo).y * USHEIGHT(CHARSIZE(hTextInfo).y)))
		{	// 未击中显示区
			return DefWindowProc(hWnd, message, wParam, lParam);
		}
		pCursor.x += WINDOWPOS(hTextInfo).x;
		pCursor.y += WINDOWPOS(hTextInfo).y;

		MyInvalidateRect(hWnd,
			0, WINDOWSIZE(hTextInfo).x,
			STARTPOS(hTextInfo).y - WINDOWPOS(hTextInfo).y,
			ENDPOS(hTextInfo).y - WINDOWPOS(hTextInfo).y + USHEIGHT(CHARSIZE(hTextInfo).y)
		);

#ifdef DEBUG
		wprintf_s(TEXT("Before left button down: ")); PRINTSTARTPOS(hTextInfo); PRINTENDPOS(hTextInfo); PRINTENDL(hTextInfo);
#endif // DEBUG


		RVALUE result;
		if ((result =
			UserMessageProc(HTEXT(hTextInfo),
				pCursor.x, pCursor.y, UM_CURSOR, NULL, NULL)) != UR_ERROR)
		{
			SelectCaretPos(hTextInfo,
				POINT{ (LONG)LODWORD(result), (LONG)HIDWORD(result) }
			);	// 设置光标位置
			SelectHighlight(hTextInfo,
				CARETPOS(hTextInfo),
				CARETPOS(hTextInfo)
			);	// 设置高亮部分(无)

#ifdef DEBUG
			wprintf_s(TEXT("After left button down: ")); PRINTSTARTPOS(hTextInfo); PRINTENDPOS(hTextInfo); PRINTENDL(hTextInfo);
#endif // DEBUG
		}
		UpdateWindow(hWnd);
	}
	break;
	case WM_LBUTTONUP:
	{
		// 解析鼠标点击位置
		POINT pCursor;
		GetCursorPos(&pCursor);
		ScreenToClient(hWnd, &pCursor);

		pCursor.x += WINDOWPOS(hTextInfo).x;
		pCursor.y += WINDOWPOS(hTextInfo).y;

		bool bIgnoreRepaint = false;
		if (STARTPOS(hTextInfo) == ENDPOS(hTextInfo))
			bIgnoreRepaint = true;

#ifdef DEBUG
		wprintf_s(TEXT("Before left button up: ")); PRINTSTARTPOS(hTextInfo); PRINTENDPOS(hTextInfo); PRINTENDL(hTextInfo);
#endif // DEBUG

		RVALUE result;
		if ((result =
			UserMessageProc(HTEXT(hTextInfo),
				pCursor.x, pCursor.y,
				UM_CHOOSE,
				(FPARAM)(((FPARAM)STARTPOS(hTextInfo).y) << 32 | (FPARAM)STARTPOS(hTextInfo).x), NULL)) != UR_ERROR)
		{
			SelectCaretPos(hTextInfo,
				POINT{ (LONG)LODWORD(result), (LONG)HIDWORD(result) }
			);	// 设置光标位置
			SelectHighlight(hTextInfo,
				STARTPOS(hTextInfo),
				CARETPOS(hTextInfo)
			);	// 设置高亮部分(尾部)
			// AdjustWindowPos(hWnd, hTextInfo);	// 滑动窗口

			if (STARTPOS(hTextInfo).y > ENDPOS(hTextInfo).y
				|| (STARTPOS(hTextInfo).y == ENDPOS(hTextInfo).y
					&& STARTPOS(hTextInfo).x > ENDPOS(hTextInfo).x))
				std::swap(STARTPOS(hTextInfo), ENDPOS(hTextInfo));
			if (!bIgnoreRepaint)
			{
				MyInvalidateRect(hWnd,
					0, WINDOWSIZE(hTextInfo).x,
					STARTPOS(hTextInfo).y - WINDOWPOS(hTextInfo).y,
					ENDPOS(hTextInfo).y - WINDOWPOS(hTextInfo).y + USHEIGHT(CHARSIZE(hTextInfo).y)
				);
				UpdateWindow(hWnd);
			}
#ifdef DEBUG
			wprintf_s(TEXT("After left button up: ")); PRINTSTARTPOS(hTextInfo); PRINTENDPOS(hTextInfo); PRINTENDL(hTextInfo);
#endif // DEBUG
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
		// 解析鼠标点击位置
		POINT pCursor;
		GetCursorPos(&pCursor);
		ScreenToClient(hWnd, &pCursor);
		switch (wParam)
		{
		case MK_LBUTTON:
		{	// 按住左键
			if (MTIMER(hTextInfo) != nullptr && IsTimerStopped(MTIMER(hTextInfo)) == FALSE)
				return 0;

			if (MTIMER(hTextInfo) != nullptr)
				KillTimer(MTIMER(hTextInfo)), MTIMER(hTextInfo) = nullptr;
			else
				MTIMER(hTextInfo) = CreateTimer(100);

			if (pCursor.y > WINDOWSIZE(hTextInfo).y) // 下移
				SendMessage(hWnd, WM_VSCROLL, SB_LINEDOWN, NULL);
			else if (pCursor.x > WINDOWSIZE(hTextInfo).x) // 右移
				SendMessage(hWnd, WM_HSCROLL, SB_LINERIGHT, NULL);
			else if (pCursor.y < 0)	// 上移
				SendMessage(hWnd, WM_VSCROLL, SB_LINEUP, NULL);
			else if (pCursor.x < 0) // 下移
				SendMessage(hWnd, WM_HSCROLL, SB_LINELEFT, NULL);
			pCursor.x += WINDOWPOS(hTextInfo).x;
			pCursor.y += WINDOWPOS(hTextInfo).y;

			RVALUE result;
			if ((result =
				UserMessageProc(HTEXT(hTextInfo),
					pCursor.x, pCursor.y,
					UM_CHOOSE,
					(FPARAM)(((FPARAM)STARTPOS(hTextInfo).y) << 32 | (FPARAM)STARTPOS(hTextInfo).x), NULL)) != UR_ERROR)
			{
				SelectCaretPos(hTextInfo,
					POINT{ (LONG)LODWORD(result), (LONG)HIDWORD(result) }
				);	// 设置光标位置
				SelectHighlight(hTextInfo,
					STARTPOS(hTextInfo),
					CARETPOS(hTextInfo)
				);

				if (STARTPOS(hTextInfo).y > ENDPOS(hTextInfo).y
					|| (STARTPOS(hTextInfo).y == ENDPOS(hTextInfo).y
						&& STARTPOS(hTextInfo).x > ENDPOS(hTextInfo).x))
					MyInvalidateRect(hWnd,
						0, WINDOWSIZE(hTextInfo).x,
						ENDPOS(hTextInfo).y - WINDOWPOS(hTextInfo).y,
						STARTPOS(hTextInfo).y - WINDOWPOS(hTextInfo).y + USHEIGHT(CHARSIZE(hTextInfo).y)
					);
				else
					MyInvalidateRect(hWnd,
						0, WINDOWSIZE(hTextInfo).x,
						STARTPOS(hTextInfo).y - WINDOWPOS(hTextInfo).y,
						ENDPOS(hTextInfo).y - WINDOWPOS(hTextInfo).y + USHEIGHT(CHARSIZE(hTextInfo).y)
					);
				UpdateWindow(hWnd);
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

		PaintWindow(&ps, hTextInfo);

		RECT rcPaint;
		rcPaint.left = max(ps.rcPaint.left, 0);
		rcPaint.right = min(ps.rcPaint.right, WINDOWSIZE(hTextInfo).x);
		rcPaint.top = max(ps.rcPaint.top, 0);
		rcPaint.bottom = min(ps.rcPaint.bottom, WINDOWSIZE(hTextInfo).y);

#ifdef DEBUG
		wprintf_s(TEXT("Repaint rectangle: (%d, %d, %d, %d)\n"), rcPaint.left, rcPaint.top, rcPaint.right, rcPaint.bottom);
#endif // DEBUG

		BitBlt(hdc,
			rcPaint.left, rcPaint.top, rcPaint.right - rcPaint.left, rcPaint.bottom - rcPaint.top,
			MEMDC(hTextInfo), rcPaint.left, rcPaint.top, SRCCOPY);

		DefaultFill(hWnd, hTextInfo);

		EndPaint(hWnd, &ps);
	}
	break;
	case WM_DESTROY:
	{
#ifdef DEBUG
		FreeConsole();
#endif // DEBUG

		DestroyCaret();
		ReleaseTextInfo(hWnd, hTextInfo);
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
	{
		if (uiFindMode & FIND)
		{
			SetWindowText(hDlg, TEXT("请输入待查找的字符串"));
			Edit_Enable(GetDlgItem(hDlg, IDC_REPLACEWHAT), FALSE);
		}
		else if (uiFindMode & REPLACE)
			SetWindowText(hDlg, TEXT("请输入待替换的字符串"));

		Edit_LimitText(GetDlgItem(hDlg, IDC_FINDWHAT), MAX_LOADSTRING);
		Edit_LimitText(GetDlgItem(hDlg, IDC_REPLACEWHAT), MAX_LOADSTRING);

		TCHAR szTypeItem[MAX_LOADSTRING];
		wsprintf(szTypeItem, TEXT("从光标处向后查找"));
		SendDlgItemMessage(hDlg, IDC_TYPEHINT, CB_ADDSTRING, NULL, (LPARAM)szTypeItem);
		wsprintf(szTypeItem, TEXT("从光标处向前查找"));
		SendDlgItemMessage(hDlg, IDC_TYPEHINT, CB_ADDSTRING, NULL, (LPARAM)szTypeItem);
		wsprintf(szTypeItem, TEXT("当前文档"));
		SendDlgItemMessage(hDlg, IDC_TYPEHINT, CB_ADDSTRING, NULL, (LPARAM)szTypeItem);
		SendDlgItemMessage(hDlg, IDC_TYPEHINT, CB_SETCURSEL, 0, 0);
	}
	return (INT_PTR)TRUE;

	case WM_COMMAND:
	{
		switch (LOWORD(wParam))
		{
		case IDOK:
		{
			GetDlgItemText(hDlg, IDC_FINDWHAT, szFindWhat, MAX_LOADSTRING);

			if (IsDlgButtonChecked(hDlg, IDC_COMPLETEMATCH) == BST_CHECKED)
				uiFindMode |= FIND_COMPELTE;
			if (IsDlgButtonChecked(hDlg, IDC_LHCASE) == BST_CHECKED)
				uiFindMode |= FIND_LHCASE;

			switch (SendDlgItemMessage(hDlg, IDC_TYPEHINT, CB_GETCURSEL, NULL, NULL))
			{
			case 0:	// 光标后查找
				uiFindMode |= AFTER_CARET;
				break;
			case 1:	// 光标前查找
				uiFindMode |= BEFORE_CARET;
				break;
			case 2:	// 当前文档查找
				uiFindMode |= WHOLE_TEXT;
				break;
			default:
				break;
			}

			RVALUE result;
			if ((result =
				UserMessageProc(HTEXT(hTextInfo),
					CARETPOS(hTextInfo).x, CARETPOS(hTextInfo).y,
					UM_FIND, (FPARAM)szFindWhat, (SPARAM)&uiFindMode)) != UR_ERROR)
			{
				SelectCaretPos(hTextInfo,
					POINT{ (LONG)LODWORD(result), (LONG)HIDWORD(result) }
				);	// 设置光标位置
				SelectHighlight(hTextInfo,
					CARETPOS(hTextInfo),
					POINT{ ((POINT*)(&uiFindMode))->x, ((POINT*)(&uiFindMode))->y }
				);	// 设置高亮部分
				AdjustWindowPos(hTextInfo->m_hWnd, hTextInfo);	// 滑动窗口
			}
			else
			{
				MessageBox(hTextInfo->m_hWnd, TEXT("无当前查找项"), NULL, MB_OK);
			}

			if (uiFindMode & REPLACE)
			{
				if (MessageBox(hTextInfo->m_hWnd,
					TEXT("确定要替换该查找项"), NULL, MB_OKCANCEL) == IDOK)
				{
					if ((result =
						UserMessageProc(HTEXT(hTextInfo),
							CARETPOS(hTextInfo).x, CARETPOS(hTextInfo).y,
							UM_REPLACE, (FPARAM)szFindWhat, (SPARAM)szReplaceWhat)) != UR_ERROR)
					{
						SelectTextSize(hTextInfo,
							POINT{ (LONG)LODWORD(result), TEXTSIZE(hTextInfo).y }
						);	// 设置文本范围
						SelectHighlight(hTextInfo,
							CARETPOS(hTextInfo),
							POINT{ (LONG)HIDWORD(result), CARETPOS(hTextInfo).y }
						);	// 设置高亮
					}
				}
			}
			uiFindMode &= (FIND | REPLACE);
		}
		return (INT_PTR)TRUE;

		case IDCANCEL:
		{
			EndDialog(hDlg, LOWORD(wParam));
		}
		return (INT_PTR)TRUE;
		}
	}
	break;
	}
	return (INT_PTR)FALSE;
}