//EditBox.cpp: 定义应用程序的入口点。
//

#include "stdafx.h"
#include "EditBox.h"
#include <assert.h>

#define MAX_LOADSTRING 100

// 全局变量: 
HINSTANCE hInst;                                // 当前实例
WCHAR szTitle[MAX_LOADSTRING];                  // 标题栏文本
WCHAR szWindowClass[MAX_LOADSTRING];            // 主窗口类名

// 此代码模块中包含的函数的前向声明: 
ATOM                MyRegisterClass(HINSTANCE hInstance);
BOOL                InitInstance(HINSTANCE, int);
LRESULT CALLBACK    WndProc(HWND, UINT, WPARAM, LPARAM);
INT_PTR CALLBACK    About(HWND, UINT, WPARAM, LPARAM);

int APIENTRY wWinMain(_In_ HINSTANCE hInstance,
                     _In_opt_ HINSTANCE hPrevInstance,
                     _In_ LPWSTR    lpCmdLine,
                     _In_ int       nCmdShow)
{
    UNREFERENCED_PARAMETER(hPrevInstance);
    UNREFERENCED_PARAMETER(lpCmdLine);

    // TODO: 在此放置代码。

    // 初始化全局字符串
    LoadStringW(hInstance, IDS_APP_TITLE, szTitle, MAX_LOADSTRING);
    LoadStringW(hInstance, IDC_EDITBOX, szWindowClass, MAX_LOADSTRING);
    MyRegisterClass(hInstance);

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

    wcex.style          = CS_HREDRAW | CS_VREDRAW;
    wcex.lpfnWndProc    = WndProc;
    wcex.cbClsExtra     = 0;
    wcex.cbWndExtra     = 0;
    wcex.hInstance      = hInstance;
    wcex.hIcon          = LoadIcon(hInstance, MAKEINTRESOURCE(IDI_EDITBOX));
    wcex.hCursor        = LoadCursor(nullptr, IDC_ARROW);
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
	   WS_OVERLAPPEDWINDOW |
	   WS_HSCROLL |
	   WS_VSCROLL,
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
//  WM_COMMAND  - 处理应用程序菜单
//  WM_PAINT    - 绘制主窗口
//  WM_DESTROY  - 发送退出消息并返回
//
//
LRESULT CALLBACK WndProc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam)
{
	static int cxChar, cyChar;	// 字符大小

	static RECT rc;
	static int cxClient, cyClient;	// 显示区大小
	static int cxPageRow, cyPageLine;	// 页面大小

	static int cxCaret = 0, cyCaret = 0;	// 光标位置

	static int nNumLines = 1;	// 文本行数
	TEXTMETRIC tm;	// 字符信息
	static HFONT hFont;	// 字体
	static SCROLLINFO sInfo; // 滚动条

	static int cyCount[(1 << 5)];
	static TCHAR szBuffer[(1<<5)][(1 << 10)];

	static int cxText, cyText;
	static LPCWSTR lpString;
	static int cText;
	static bool isZH = false;

    switch (message)
    {
	case WM_CREATE:
		{
			HDC hdc = GetDC(hWnd);

			// 设置字体
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
			SelectObject(hdc, hFont);

			GetTextMetrics(hdc, &tm);
			cxChar = tm.tmAveCharWidth;
			cyChar = tm.tmHeight;

			// 获取客户区大小
			GetClientRect(hWnd, &rc);
			cxClient = rc.right - rc.left;
			cyClient = rc.bottom - rc.top;

			// 设置滚动条
			sInfo.cbSize = sizeof(sInfo);
			sInfo.fMask = SIF_ALL;
			sInfo.nMin = 0;
			sInfo.nMax = nNumLines - 1;
			sInfo.nPage = cyClient / cyChar;
			sInfo.nPos = 0;
			sInfo.nTrackPos = 0;
			SetScrollInfo(hWnd, SB_VERT, &sInfo, TRUE);

			ReleaseDC(hWnd, hdc);

			// 设置光标
			CreateCaret(hWnd, NULL, 1, cyChar);
			SetCaretPos(cxCaret, cyCaret);
			ShowCaret(hWnd);

			// 字符
			memset(cyCount, 0x0, _countof(cyCount));
			memset(szBuffer, 0x0, _countof(szBuffer));
		}
		break;
    case WM_COMMAND:
        {
            int wmId = LOWORD(wParam);
            // 分析菜单选择: 
            switch (wmId)
            {
            case IDM_ABOUT:
                DialogBox(hInst, MAKEINTRESOURCE(IDD_ABOUTBOX), hWnd, About);
                break;
            case IDM_EXIT:
                DestroyWindow(hWnd);
                break;
            default:
                return DefWindowProc(hWnd, message, wParam, lParam);
            }
        }
        break;
	case WM_SIZE:
		{
			if (message == WM_SIZE)
			{
				cxClient = LOWORD(lParam);
				cyClient = HIWORD(lParam);
			}

			cxPageRow = cxClient / cxChar;
			cyPageLine = cyClient / cyChar;

			if (hWnd == GetFocus())
			{
				SetCaretPos(cxCaret * cxChar,
					cyCaret * cyChar);
			}
		}
		break;
	case WM_SETFOCUS:
		{
			CreateCaret(hWnd, NULL, 1, cyChar);
			SetCaretPos(cxCaret * cxChar,
				cyCaret * cyChar);
			ShowCaret(hWnd);
		}
		break;
	case WM_KILLFOCUS:
		{
			HideCaret(hWnd);
			DestroyCaret();
		}
		break;
	case WM_KEYDOWN:
		{
			switch (wParam)
			{
				case VK_HOME:
					{
						cxCaret = 0;
					}
					break;
				case VK_END:
					{
						//
					}
					break;
				case VK_UP:
					{
						cyCaret = max(cyCaret - 1, 0);
					}
					break;
				case VK_DOWN:
					{
						cyCaret = min(cyCaret + 1, cyPageLine);
					}
					break;
				case VK_LEFT:
					{
						cxCaret = max(cxCaret - 1 - isZH, 0);
					}
					break;
				case VK_RIGHT:
					{
						cxCaret = min(cxCaret + 1 + isZH, cxPageRow);
					}
					break;
				case VK_DELETE:
					{
						// 光标不移动
					}
					break;
				default:
					break;
			}
			SetCaretPos(cxCaret * cxChar, cyCaret * cyChar);
		}
		break;
	case WM_CHAR:
		{
			//for (int i = 0; i < (int)LOWORD(lParam); ++i)
			//{
				switch (wParam)
				{
				case '\b':
					{
						if (cxCaret > 0)
						{
							--cxCaret;
							SendMessage(hWnd, WM_KEYDOWN,
								VK_DELETE, 1);
						}
					}
					break;
				case '\t':
					{
						do
						{
							SendMessage(hWnd, WM_CHAR,
								' ', 1);
						} while (cxCaret % 4 != 0);
					}
					break;
				case '\n':
					{
						if (cyCaret != cyPageLine - 2)
						{
							++cyCaret;
						}
						cxCaret = 0;
					}
					break;
				case '\r':
					{
						if (cyCaret != cyPageLine - 2)
						{
							++cyCaret;
						}
						cxCaret = 0;
					}
					break;
				default:
					{
						int len = wcslen(szBuffer[cyCaret]);
						for (int pos = len; pos > cxCaret; --pos)
							szBuffer[cyCaret][pos] = szBuffer[cyCaret][pos - 1];
						szBuffer[cyCaret][cxCaret] = (TCHAR)(wParam);
						isZH = (HIBYTE(LOWORD(wParam))) ? true : false;

						HDC hdc = GetDC(hWnd);
						SelectObject(hdc, hFont);

						cxText = cxCaret * cxChar;
						cyText = cyCaret * cyChar;
						lpString = szBuffer[cyCaret] + cxCaret;
						cText = len - cxCaret + 1;
						TextOut(hdc, cxText, cyText, lpString, cText);

						// 右移光标
						HideCaret(hWnd);
						cxCaret = min(cxCaret + 1 + isZH, cxPageRow);
						SetCaretPos(cxCaret, cyCaret);

						TCHAR sz[100];
						memset(sz, 0, _countof(sz));
						wsprintf(sz, TEXT("cText=%d cxCaret=%d"), cText, cxCaret);
						TextOut(hdc, 0, cyText + cyChar, sz, wcslen(sz));

						ReleaseDC(hWnd, hdc);
					}
					break;
				}

				SetCaretPos(cxCaret * cxChar, cyCaret * cyChar);
				ShowCaret(hWnd);
			//}
		}
		break;
	case WM_VSCROLL:
		{
			sInfo.fMask = SIF_ALL;
			GetScrollInfo(hWnd, SB_VERT, &sInfo);

			int iVertPos = sInfo.nPos;
			switch (LOWORD(wParam))
			{
			case SB_TOP:

				sInfo.nPos = sInfo.nMin;
				break;

			case SB_BOTTOM:
				sInfo.nPos = sInfo.nMax;
				break;

			case SB_LINEUP:
				sInfo.nPos -= 1;
				break;

			case SB_LINEDOWN:
				sInfo.nPos += 1;
				break;

			case SB_PAGEUP:
				sInfo.nPos -= sInfo.nPage;
				break;

			case SB_PAGEDOWN:
				sInfo.nPos += sInfo.nPage;

			case SB_THUMBTRACK:
				sInfo.nPos = sInfo.nTrackPos;
				break;
			default:
				break;
			}
			sInfo.fMask = SIF_POS;
			SetScrollInfo(hWnd, SB_VERT, &sInfo, TRUE);

			GetScrollInfo(hWnd, SB_VERT, &sInfo);
			if (sInfo.nPos != iVertPos)
			{
				ScrollWindow(hWnd, 0, cyChar * (iVertPos - sInfo.nPos),
					NULL, NULL);
				UpdateWindow(hWnd);
			}
		}
		break;
    case WM_PAINT:
        {
			ShowCaret(hWnd);

            PAINTSTRUCT ps;
            HDC hdc = BeginPaint(hWnd, &ps);
            // TODO: 在此处添加使用 hdc 的任何绘图代码...

            EndPaint(hWnd, &ps);
        }
        break;
    case WM_DESTROY:
		{
			DestroyCaret();
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
