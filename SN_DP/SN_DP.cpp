// SN_DP.cpp : 定义应用程序的入口点。
//

#include "stdafx.h"
#include "SN_DP.h"
#include "AgoraManager.h"
#include "PlayerHooker.h"
#include "HttpServer.h"
#include <atlstr.h>

#include "AGEventDef.h"

#define MAX_LOADSTRING 100

class IPlayerHooker;
class IAudioCaptureCallback;

bool bIsDebugMode;
bool bIsSaveDumpPcm;

CString s2cs(const std::string &str)
{
	return CString(str.c_str());
}

std::string cs2s(const CString &str)
{
	CString sTemp(str);
	return CStringA(sTemp.GetBuffer()).GetBuffer();
}

int str2int(const std::string &str)
{
	return atoi(str.data());
}

int CS2int(const CString &csStr)
{
	std::string str = cs2s(csStr);
	return str2int(str);
}

std::string int2str(int nNum)
{
	char str[512] = { 0 };
	_itoa_s(nNum, str, 10);
	return str;
}

bool IsDebugMode(HINSTANCE HModule)
{
	TCHAR szFilePath[MAX_PATH];
	CString DebugMode(_T(""));

	::GetModuleFileName(HModule, szFilePath, MAX_PATH);
	LPTSTR lpLastSlash = _tcsrchr(szFilePath, _T('\\'));

	if (lpLastSlash == NULL)
		return FALSE;

	SIZE_T nNameLen = MAX_PATH - (lpLastSlash - szFilePath + 1);
	_tcscpy_s(lpLastSlash + 1, nNameLen, _T("DebugMode.ini"));

	if (::GetFileAttributes(szFilePath) == INVALID_FILE_ATTRIBUTES) {
		CreateFile(szFilePath, GENERIC_READ | GENERIC_WRITE, FILE_SHARE_READ | FILE_SHARE_WRITE, NULL,
			CREATE_NEW, FILE_ATTRIBUTE_NORMAL, NULL);

		::WritePrivateProfileString(_T("DebugMode"), _T("DebugMode"), _T("1"), szFilePath);
		::WritePrivateProfileString(_T("DebugMode"), _T("SaveDumpPcm"), _T("0"), szFilePath);

		return FALSE;
	}

	CString strResolution;
	//Default 1
	::GetPrivateProfileString(_T("DebugMode"), _T("DebugMode"), NULL, DebugMode.GetBuffer(MAX_PATH), MAX_PATH, szFilePath);
	if (_T("") == DebugMode) {
		DebugMode = _T("1");
		::WritePrivateProfileString(_T("DebugMode"), _T("DebugMode"), DebugMode, szFilePath);
	}

	DebugMode.ReleaseBuffer();

	return CS2int(DebugMode);
}


bool IsSaveDumpPcm(HINSTANCE HModule)
{
	TCHAR szFilePath[MAX_PATH];
	CString DebugMode(_T(""));

	::GetModuleFileName(HModule, szFilePath, MAX_PATH);
	LPTSTR lpLastSlash = _tcsrchr(szFilePath, _T('\\'));

	if (lpLastSlash == NULL)
		return FALSE;

	SIZE_T nNameLen = MAX_PATH - (lpLastSlash - szFilePath + 1);
	_tcscpy_s(lpLastSlash + 1, nNameLen, _T("DebugMode.ini"));

	if (::GetFileAttributes(szFilePath) == INVALID_FILE_ATTRIBUTES) {
		CreateFile(szFilePath, GENERIC_READ | GENERIC_WRITE, FILE_SHARE_READ | FILE_SHARE_WRITE, NULL,
			CREATE_NEW, FILE_ATTRIBUTE_NORMAL, NULL);

		::WritePrivateProfileString(_T("DebugMode"), _T("DebugMode"), _T("1"), szFilePath);
		::WritePrivateProfileString(_T("DebugMode"), _T("SaveDumpPcm"), _T("0"), szFilePath);

		return FALSE;
	}

	CString strResolution;
	//Default 0
	::GetPrivateProfileString(_T("DebugMode"), _T("SaveDumpPcm"), NULL, DebugMode.GetBuffer(MAX_PATH), MAX_PATH, szFilePath);
	if (_T("") == DebugMode){
		DebugMode = _T("0");
		::WritePrivateProfileString(_T("DebugMode"),_T("SaveDumpPcm"),DebugMode,szFilePath);
	}

	DebugMode.ReleaseBuffer();

	return CS2int(DebugMode);
}



AgoraManager*	pAgoraManager = NULL;
SDHttpServer*   pSDHttpServer = NULL;
HWND			hRenderWnd = NULL;
HWND			hRenderChildL = NULL;
HWND			hRenderChildR = NULL;
HWND			hButtonMode = NULL;
HWND			hButtonMin = NULL;
extern uint32_t nLastErrorCode;
extern uint32_t nActCode;
BOOL bTopMost;

// 全局变量: 
HINSTANCE hInst;                                // 当前实例
WCHAR szTitle[MAX_LOADSTRING];                  // 标题栏文本
WCHAR szWindowClass[MAX_LOADSTRING];            // 主窗口类名

// 此代码模块中包含的函数的前向声明: 
ATOM                MyRegisterClass(HINSTANCE hInstance);
BOOL                InitInstance(HINSTANCE, int);
LRESULT CALLBACK    WndProc(HWND, UINT, WPARAM, LPARAM);
INT_PTR CALLBACK    About(HWND, UINT, WPARAM, LPARAM);

void writeFormatLog(const char* fmt, ...)
{
	char tempname[128] = { 0 };
	FILE* ffIle = _fsopen("c:/debug.dat", "r", _SH_DENYRW);
	if (ffIle != NULL)
	{
		FILE* fIn = _fsopen("c:/6RoomsLog/agoravideo.log", "ab+", _SH_DENYWR);
		if (!fIn)
		{
			fclose(ffIle);
			return;
		}

		va_list params;
		va_start(params, fmt);
		SYSTEMTIME sys;
		GetLocalTime(&sys);
		fprintf(fIn, "%04d-%02d-%02d %02d:%02d:%02d:%03d | ", sys.wYear, sys.wMonth, sys.wDay, sys.wHour, sys.wMinute, sys.wSecond, sys.wMilliseconds);
		vfprintf(fIn, fmt, params);
		fprintf(fIn, "\n");
		va_end(params);
		fclose(fIn);
		fclose(ffIle);
	}
}

#include <Dbghelp.h>
#pragma comment(lib, "Dbghelp.lib")
LONG WINAPI SNDP_UnhandledExceptionFilter(PEXCEPTION_POINTERS pExceptionInfo)
{
	char s[256];
	sprintf_s(s, "C:\\6RoomsLog\\SNDP_%010d.dmp", (DWORD)time(NULL));
	HANDLE hDumpFile = CreateFileA(s, GENERIC_WRITE, 0, NULL, CREATE_ALWAYS, FILE_ATTRIBUTE_NORMAL, NULL);
	if (hDumpFile != INVALID_HANDLE_VALUE)
	{
		MINIDUMP_EXCEPTION_INFORMATION dumpInfo;
		dumpInfo.ExceptionPointers = pExceptionInfo;
		dumpInfo.ThreadId = GetCurrentThreadId();
		dumpInfo.ClientPointers = TRUE;

		MiniDumpWriteDump(GetCurrentProcess(), GetCurrentProcessId(), hDumpFile, MiniDumpNormal, &dumpInfo, NULL, NULL);
		CloseHandle(hDumpFile);
	}
	return EXCEPTION_EXECUTE_HANDLER;
}

int APIENTRY wWinMain(_In_ HINSTANCE hInstance,
	_In_opt_ HINSTANCE hPrevInstance,
	_In_ LPWSTR    lpCmdLine,
	_In_ int       nCmdShow)
{
	UNREFERENCED_PARAMETER(hPrevInstance);
	UNREFERENCED_PARAMETER(lpCmdLine);

	HANDLE hS = CreateSemaphoreA(NULL, 0, 1, APPVERSION);
	if (GetLastError() == ERROR_ALREADY_EXISTS) return 0;
	//   // TODO: 在此放置代码。
	bTopMost = FALSE;
	if (!initSystem())
	{
		return FALSE;
	}

	// 初始化全局字符串
	LoadStringW(hInstance, IDS_APP_TITLE, szTitle, MAX_LOADSTRING);
	LoadStringW(hInstance, IDC_SN_DP, szWindowClass, MAX_LOADSTRING);
	MyRegisterClass(hInstance);

	ULONG_PTR gditoken;
	GdiplusStartupInput gdiinput;
	GdiplusStartup(&gditoken, &gdiinput, NULL);

	SetUnhandledExceptionFilter(SNDP_UnhandledExceptionFilter);
	writelog("VideoDP up");
	// 执行应用程序初始化: 
	if (!InitInstance(hInstance, nCmdShow))
	{
		return FALSE;
	}

	bIsDebugMode = IsDebugMode(NULL);
	bIsSaveDumpPcm = IsSaveDumpPcm(NULL);
	//DeleteFile(_T("./V6room/MusicDest.pcm"));
	//DeleteFile(_T("./V6room/FrameMix.pcm"));

	HACCEL hAccelTable = LoadAccelerators(hInstance, MAKEINTRESOURCE(IDI_ICON1));

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
	GdiplusShutdown(gditoken);
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

	wcex.style = CS_DROPSHADOW;//CS_HREDRAW | CS_VREDRAW;
	wcex.lpfnWndProc = WndProc;
	wcex.cbClsExtra = 0;
	wcex.cbWndExtra = 0;
	wcex.hInstance = hInstance;
	wcex.hIcon = LoadIcon(hInstance, MAKEINTRESOURCE(IDI_ICON1));
	wcex.hCursor = LoadCursor(nullptr, IDC_ARROW);
	wcex.hbrBackground = (HBRUSH)GetStockObject(WHITE_BRUSH);//(COLOR_WINDOW+1);//CreateSolidBrush(RGB(255, 0, 0));
	wcex.lpszMenuName = NULL;// MAKEINTRESOURCEW(IDC_SN_DP);
	wcex.lpszClassName = szWindowClass;
	wcex.hIconSm = NULL;// LoadIcon(wcex.hInstance, MAKEINTRESOURCE(IDI_SMALL));

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
int nClientWidth = 0;
int nClientHeight = 0;
int nChildHeight = 0;
int nLeftWidth = 0;
int nRightWidth = 0;
BOOL InitInstance(HINSTANCE hInstance, int nCmdShow)
{
	hInst = hInstance; // 将实例句柄存储在全局变量中

	RECT RenderRect;
	SetRect(&RenderRect, 0, 0, NCLIENT_WIDTH, NCLIENT_HEIGHT);
	AdjustWindowRectEx(&RenderRect, WS_POPUP | WS_MINIMIZEBOX, FALSE, WS_EX_DLGMODALFRAME);
	nClientWidth = RenderRect.right - RenderRect.left;
	nClientHeight = RenderRect.bottom - RenderRect.top;
	hRenderWnd = CreateWindowEx(WS_EX_DLGMODALFRAME,
								szTitle,
								szTitle,
								WS_POPUP | WS_MINIMIZEBOX,
								262,
								35,
								nClientWidth,
								nClientHeight,
								NULL,
								NULL,
								hInstance,
								NULL);
// 	DWORD exStyle = ::GetWindowLong(hRenderWnd, GWL_EXSTYLE);
// 	exStyle |= WS_EX_LAYERED;
// 	::SetWindowLong(hRenderWnd, GWL_EXSTYLE, exStyle);
// 	::SetLayeredWindowAttributes(hRenderWnd, RGB(0, 0, 0), 128, LWA_ALPHA);

	if (!hRenderWnd)
	{
		return FALSE;
	}

//	ShowWindow(hRenderWnd, nCmdShow);
	UpdateWindow(hRenderWnd);

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
RECT DesckTopRect;
void initEngine()
{
	pAgoraManager->initEngine(APP_ID);
	//char versions[256] = { 0 };
	int buildn;
	pAgoraManager->SDK_Version = pAgoraManager->pRTCEngine->getVersion(&buildn);
	//set hwnd for message post by event_handler
	pAgoraManager->setEventHandler(hRenderWnd);
	//open log
	RtcEngineParameters rep(pAgoraManager->pRTCEngine);
	rep.setLogFile("C:\\6RoomsLog\\agoraSDk.log");

	pAgoraManager->initParam();//方便调试
}

BOOL    bSetTimer = FALSE;
BOOL	bSetModeDone = TRUE;
HBITMAP hBmpJianrong;//位图句柄  
HBITMAP hBmpJisu;//位图句柄
HBITMAP hBmpMin;//位图句柄 

LRESULT CALLBACK WndProc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam)
{
	RECT curRect;
	RECT rectt;
	POINT p_mouse;
	LPVOID lpData;
	int32_t errcode;
	int result;
	bool bQuit = FALSE;
	switch (message)
	{
	case WM_CREATE:
		hRenderChildL = CreateWindowW(L"static", L"LW", WS_VISIBLE | WS_CHILD | SS_BLACKFRAME,
			0, NTITLE_HEIGHT, nClientWidth / 2, nClientHeight - NTITLE_HEIGHT, hWnd, (HMENU)IDB_LEFTWINDOW, ((LPCREATESTRUCT)lParam)->hInstance, NULL);
		hRenderChildR = CreateWindow(L"static", L"RW", WS_VISIBLE | WS_CHILD | SS_BLACKFRAME,
			nClientWidth / 2, NTITLE_HEIGHT, nClientWidth / 2, nClientHeight - NTITLE_HEIGHT, hWnd, (HMENU)IDB_RIGHTWINDOW, ((LPCREATESTRUCT)lParam)->hInstance, NULL);

		hButtonMin = CreateWindow(L"button", L"min", WS_CHILD | WS_VISIBLE,
			nClientWidth - 50, 0, 50, NTITLE_HEIGHT, hWnd, (HMENU)IDB_MINBUTTON, ((LPCREATESTRUCT)lParam)->hInstance, NULL);
		hBmpMin = LoadBitmap(((LPCREATESTRUCT)lParam)->hInstance, MAKEINTRESOURCE(IDB_BITMAP3));
		SetWindowLong(hButtonMin, GWL_STYLE, GetWindowLong(hButtonMin, GWL_STYLE) + BS_BITMAP);
		SendMessage(hButtonMin, BM_SETIMAGE, IMAGE_BITMAP, (LPARAM)hBmpMin);
#ifdef BUTTON_CHOICE
		hButtonMode = CreateWindow(L"button", L"mode", WS_CHILD | WS_VISIBLE,
			nClientWidth - 100, 0, 50, NTITLE_HEIGHT, hWnd, (HMENU)IDB_MODEBUTTON, ((LPCREATESTRUCT)lParam)->hInstance, NULL);
		
 		hBmpJianrong = LoadBitmap(((LPCREATESTRUCT)lParam)->hInstance, MAKEINTRESOURCE(IDB_BITMAP1));//兼容
 		hBmpJisu = LoadBitmap(((LPCREATESTRUCT)lParam)->hInstance, MAKEINTRESOURCE(IDB_BITMAP2));//极速

		SetWindowLong(hButtonMode, GWL_STYLE, GetWindowLong(hButtonMode, GWL_STYLE) + BS_BITMAP);


// 		if (pAgoraManager->readConfigFile(&pAgoraManager->nDisplayMode) != 0)
// 			pAgoraManager->nDisplayMode = DISPLAY_D3D;
		pAgoraManager->nDisplayMode = DISPLAY_GDI;
		if (pAgoraManager->nDisplayMode == DISPLAY_GDI)
			SendMessage(hButtonMode, BM_SETIMAGE, IMAGE_BITMAP, (LPARAM)hBmpJianrong);
		else if (pAgoraManager->nDisplayMode == DISPLAY_D3D)
			SendMessage(hButtonMode, BM_SETIMAGE, IMAGE_BITMAP, (LPARAM)hBmpJisu);
		writelog("default display mode:%d", pAgoraManager->nDisplayMode);
#endif
		pAgoraManager->setClientHwnd(hWnd, hRenderChildL, hRenderChildR);
		initEngine();
		break;
	case AGORA_WINDOW_START:
		pAgoraManager->nStartState = pAgoraManager->start();
		SetEvent(pAgoraManager->mevent_start);
		break;
	case WM_COMMAND:
	{
		int wmId = LOWORD(wParam);
		// 分析菜单选择:
		switch (wmId)
		{
			//             case IDM_ABOUT:
			//                 DialogBox(hInst, MAKEINTRESOURCE(IDD_ABOUTBOX), hWnd, About);
			//                 break;
// 		case IDM_EXIT:
// 			DestroyWindow(hWnd);
// 			break;
#ifdef BUTTON_CHOICE
 		case IDB_MODEBUTTON:
 			if (bSetModeDone)
 			{
 				if (pAgoraManager->nDisplayMode == DISPLAY_D3D)
 				{
 					result = MessageBox(hWnd, L"从“极速”切到“兼容”模式，在您不能正常看到自己视频的时候可用，切换后需要重新发起连麦，您确定要切换吗？", L"提示", MB_ICONINFORMATION | MB_YESNO);// MB_OKCANCEL);
 					switch (result)
 					{
 					case IDYES:
 						bQuit = true;
 						writelog("DISPLAY_D3D:user choose yes");
 						break;
 					case IDNO:
 						bQuit = false;
 						writelog("DISPLAY_D3D:user choose no");
 						break;
 					}
 				}
 				else if (pAgoraManager->nDisplayMode == DISPLAY_GDI)
 				{
 					result = MessageBox(hWnd, L"从“兼容”切到”极速“模式，您确认需要切换吗？", L"提示", MB_ICONINFORMATION | MB_YESNO);// MB_OKCANCEL);
 					switch (result)
 					{
 					case IDYES:
 						bQuit = true;
 						writelog("DISPLAY_GDI:user choose yes");
 						break;
 					case IDNO:
 						bQuit = false;
 						writelog("DISPLAY_GDI:user choose no");
 						break;
 					}
 				}
 
 				if (bQuit)
 				{
 					bSetModeDone = FALSE;
 					pAgoraManager->nDisplayMode++;
 					if (pAgoraManager->nDisplayMode == DISPLAY_D2D)
 						pAgoraManager->nDisplayMode = DISPLAY_GDI;
 
 					if (pAgoraManager->nDisplayMode == DISPLAY_GDI)
 						SendMessage(hButtonMode, BM_SETIMAGE, IMAGE_BITMAP, (LPARAM)hBmpJianrong);
 					else if (pAgoraManager->nDisplayMode == DISPLAY_D3D)
 						SendMessage(hButtonMode, BM_SETIMAGE, IMAGE_BITMAP, (LPARAM)hBmpJisu);
 					errcode = pAgoraManager->writeConfigFile(pAgoraManager->nDisplayMode);
 					if (errcode != 0)
 						writelog("write config err:%d mode[%d]", errcode, pAgoraManager->nDisplayMode);
 					//pAgoraManager->restart();
 					bSetModeDone = TRUE;
 					PostQuitMessage(0);
 				}
 			}
 			break;
#endif
		case IDB_MINBUTTON:
			ShowWindow(hWnd, SW_MINIMIZE);
			break;
		default:
			return DefWindowProc(hWnd, message, wParam, lParam);
		}
	}
	break;
	case WM_MOUSEMOVE:
		if (!bSetTimer)
		{
			SetTimer(hWnd, TimerID_Min, 100, NULL);
			bSetTimer = TRUE;
		}
		GetCursorPos(&p_mouse);
		RECT curRect_s;
		GetWindowRect(hWnd, &curRect_s);
		if (PtInRect(&curRect_s, p_mouse))
		{
			ShowWindow(hButtonMode, SW_SHOW);
			ShowWindow(hButtonMin, SW_SHOW);
		}
// 		else
// 		{
// 			ShowWindow(hButtonMode, SW_HIDE);
// 			ShowWindow(hButtonMin, SW_HIDE);
// 			InvalidateRect(hRenderWnd, NULL, TRUE);
// 		}
		break;
	case WM_ERASEBKGND:
		return TRUE;
	case WM_PAINT:
	{
		PAINTSTRUCT ps;
		RECT rect;
		HDC hdc = BeginPaint(hWnd, &ps);
		GetClientRect(hWnd, &rect);
		SelectObject(hdc, (HBRUSH)GetStockObject(DKGRAY_BRUSH));//一个有颜色的HBRUSH
		Rectangle(hdc, rect.left, rect.top, rect.right, NTITLE_HEIGHT);

// 		hdc = GetDC(hWnd);
// 		SetTextColor(hdc, RGB(0, 0, 255));
// 		DWORD err = GetLastError();

		// 			Graphics graphics(hdc);
		// 
		// 			BitmapData bitmapData;
		// 			bitmap->LockBits(NULL, 0, PixelFormat24bppRGB, &bitmapData);
		// 			if (pAgoraManager->pVideoPlayManager->pdisplay)
		// 				memcpy(bitmapData.Scan0, pAgoraManager->pVideoPlayManager->pdisplay, 320 * 240 * 3);
		// 			bitmap->UnlockBits(&bitmapData);
		// 			bitmap->RotateFlip(RotateFlipType::Rotate180FlipX);
		// 			graphics.DrawImage(bitmap, 0, 0, 640, 480);
		EndPaint(hWnd, &ps);
	}
	break;
	case  WINDOW_HIDESHOW_SHOW:
		MoveWindow(hButtonMin, nClientWidth - 50, 0, 50, NTITLE_HEIGHT, TRUE);
		MoveWindow(hRenderChildL, 0, NTITLE_HEIGHT, nLeftWidth, nChildHeight, TRUE);
		MoveWindow(hRenderChildR, nLeftWidth, NTITLE_HEIGHT, nLeftWidth, nChildHeight, TRUE);
		MoveWindow(hWnd, 300, 200, nClientWidth, nClientHeight, TRUE);
		writelog("c[%d][%d] childw[%d]", nClientWidth, nClientHeight, nLeftWidth);
		ShowWindow(hWnd, SW_SHOW);
		if (!bTopMost)
		{
			SetWindowPos(hWnd, HWND_TOPMOST, 300, 200, nClientWidth, nClientHeight, SWP_NOSIZE);
			bTopMost = TRUE;
		}
		break;
	case WINDOW_HIDESHOW_HIDE:
		ShowWindow(hWnd, SW_HIDE);
		break;
	case WM_SIZE:
// 		GetWindowRect(hWnd, &curRect);
// 		if ((curRect.right - curRect.left) != NCLIENT_WIDTH || (curRect.bottom - curRect.top) != NCLIENT_HEIGHT)
// 		{
// 			MoveWindow(hRenderChildL, 0, NTITLE_HEIGHT, (curRect.right - curRect.left) / 2, curRect.bottom - curRect.top - NTITLE_HEIGHT, TRUE);
// 			MoveWindow(hRenderChildR, (curRect.right - curRect.left) / 2, NTITLE_HEIGHT, (curRect.right - curRect.left) / 2, curRect.bottom - curRect.top - NTITLE_HEIGHT, TRUE);
// 			//ShowWindow(hWnd, SW_SHOW);
// 			writelog("");
// 		}
		break;
	case WM_MSGID(EID_JOINCHANNEL_SUCCESS):
		lpData = (LPAGE_JOINCHANNEL_SUCCESS)wParam;
		//pAgoraManager->setEventHandler(hRenderWnd);
		delete lpData;
		break;
	case WM_MSGID(EID_LEAVE_CHANNEL):
		lpData = (LPAGE_LEAVE_CHANNEL)wParam;

		delete lpData;
		break;
	case WM_MSGID(EID_ERROR):
		lpData = (LPAGE_ERROR)wParam;
		delete lpData;
		break;
	case WM_MSGID(EID_FIRST_LOCAL_VIDEO_FRAME):
	{
		lpData = (LPAGE_FIRST_LOCAL_VIDEO_FRAME)wParam;
		//pAgoraManager->JoinChannel("test66", 233, NULL);
		delete lpData;
		break;
	}
	case WM_MSGID(EID_APICALL_EXECUTED):
		lpData = (LPAGE_APICALL_EXECUTED)wParam;
		delete lpData;
		break;
	case WM_MSGID(EID_LOCAL_VIDEO_STAT):
		/*LPAGE_LOCAL_VIDEO_STAT*/ lpData = (LPAGE_LOCAL_VIDEO_STAT)wParam;
		delete lpData;
		break;
	case WM_MSGID(EID_USER_JOINED)://remote 
	{
		/*LPAGE_FIRST_REMOTE_VIDEO_DECODED*/ lpData = (LPAGE_FIRST_REMOTE_VIDEO_DECODED)wParam;
		uid_t uid = ((LPAGE_FIRST_REMOTE_VIDEO_DECODED)lpData)->uid;
		if (pAgoraManager->ChatRoomInfo.bLeft)
			pAgoraManager->setRemoteCanvas(uid, hRenderChildR);
		else
			pAgoraManager->setRemoteCanvas(uid, hRenderChildL);
		if (pAgoraManager->RtmpPushInfo.brtmppush)
		{
			pAgoraManager->setCompositingLayout(pAgoraManager->ChatRoomInfo.nUID, uid);
		}
		delete lpData;
		break;
	}
	case WM_MSGID(EID_USER_OFFLINE)://remote
		/*LPAGE_USER_OFFLINE*/ lpData = (LPAGE_USER_OFFLINE)wParam;
		delete lpData;
		break;
	case WM_LBUTTONUP:
// 		GetCursorPos(&p_mouse);
// 		GetWindowRect(hWnd, &curRect);
// 		rectt.top = curRect.top;
// 		rectt.bottom = curRect.top + 30;
// 		rectt.right = curRect.right;
// 		rectt.left = curRect.right - 30;
// 		if (PtInRect(&rectt, p_mouse))
// 		{
// 			ShowWindow(hWnd, SW_MINIMIZE);
// 		}
		break;
	case WM_LBUTTONDOWN:
		GetCursorPos(&p_mouse);
		GetWindowRect(hWnd, &curRect);
		rectt;
		rectt.top = curRect.top;
		rectt.bottom = curRect.top + 30;
		rectt.right = curRect.right;
		rectt.left = curRect.right - 30;
		if (!PtInRect(&rectt, p_mouse))
		{
			SendMessageA(hWnd, WM_NCLBUTTONDOWN, HTCAPTION, 0);
			GetWindowRect(hWnd, &curRect);
			SetWindowPos(hWnd, HWND_TOPMOST, curRect.left, curRect.top, nClientWidth, nClientHeight - NTITLE_HEIGHT, SWP_NOSIZE);
		}
		break;
	case WM_TIMER:
		if (wParam == TimerID_SDPlay)
		{
#ifdef NEED_PING
			KillTimer(hWnd, TimerID_SDPlay);
			pAgoraManager->stop();
			writelog("flash time out");
			ShowWindow(hWnd, SW_HIDE);
			nLastErrorCode = 10011;
#endif
		}
		if (wParam == TimerID_Min)
		{
			GetCursorPos(&p_mouse);
			RECT curRect_s;
			GetWindowRect(hWnd, &curRect_s);

			if (!PtInRect(&curRect_s, p_mouse))
			{
				if (bSetTimer)
				{
					KillTimer(hWnd, TimerID_Min);
					ShowWindow(hButtonMode, SW_HIDE);
					ShowWindow(hButtonMin, SW_HIDE);
					InvalidateRect(hRenderWnd, NULL, TRUE);
					bSetTimer = FALSE;
				}
			}
			break;
		}
		break;
	case WM_DESTROY:
		writelog("wm_destroy");
		OutputDebugStringA("[ Exit ]..WM_DESTROY\n");
		pAgoraManager->bStopKugou = TRUE;
		pAgoraManager->stopHook();
		pAgoraManager->stop();
		pAgoraManager->releaseEngine(TRUE);
		quitSystem();
		WSACleanup();
		writelog("wm_destroy done");
		//PostQuitMessage(0);
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

USHORT getAvailablePort(USHORT pstart, USHORT pend)
{
	SOCKET s = socket(AF_INET, SOCK_STREAM, 0);
	SOCKADDR_IN addr;
	addr.sin_family = AF_INET;
	addr.sin_addr.s_addr = inet_addr("127.0.0.1");

	for (; pstart <= pend; pstart++)
	{
		addr.sin_port = htons(pstart);
		if (::bind(s, (struct sockaddr*)&addr, sizeof(addr)) == 0) break;
	}

	closesocket(s);
	if (pstart > pend) return 0;
	return pstart;
}

BOOL quitSystem()
{
// 	if (pAgoraManager->pRTCEngine)
// 	{
// 		delete pAgoraManager->pRTCEngine;
// 	}

	if (pAgoraManager->pVideoCaptureManager)
	{
		delete pAgoraManager->pVideoCaptureManager;
	}

	if (pAgoraManager->pVideoPlayManager)
	{
		delete pAgoraManager->pVideoPlayManager;
	}

// 	if (pAgoraManager->m_devAudioin)
// 	{
// 		delete pAgoraManager->m_devAudioin;
// 	}

// 	if (pAgoraManager->m_devCamera)
// 	{
// 		delete pAgoraManager->m_devCamera;
// 	}

// 	if (pAgoraManager->m_devPlayout)
// 	{
// 		delete pAgoraManager->m_devPlayout;
// 	}

// 	if (pAgoraManager->m_EngineEventHandler)
// 	{
// 		delete pAgoraManager->m_EngineEventHandler;
// 	}

// 	if (pAgoraManager->m_CapVideoFrameObserver)
// 	{
// 		delete pAgoraManager->m_CapVideoFrameObserver;
// 	}
// 
// 	if (pAgoraManager->m_CapAudioFrameObserver)
// 	{
// 		delete pAgoraManager->m_CapAudioFrameObserver;
// 	}

	if (pAgoraManager->pPlayerCaptureManager)
	{
		delete pAgoraManager->pPlayerCaptureManager;
	}

// 	if (pAgoraManager->mpAudioCaptureCallback)
// 	{
// 		delete pAgoraManager->mpAudioCaptureCallback;
// 	}

	if (pAgoraManager)
	{
		delete pAgoraManager;
	}

	if (pSDHttpServer)
	{
		pSDHttpServer->stop();
		delete pSDHttpServer;
	}
#ifdef NEED_PING
	KillTimer(hRenderWnd, TimerID_SDPlay);
#endif
	return TRUE;
}

BOOL initSystem()
{
	int nstep = 0;
	nLastErrorCode = 0;
	nActCode = 1002;
	pAgoraManager = new AgoraManager();
	if (!pAgoraManager)
	{
		goto InitError;
	}

	nstep = 1;
	pAgoraManager->pRTCEngine = (IRtcEngine *)createAgoraRtcEngine();
	if (!pAgoraManager->pRTCEngine)
	{
		goto InitError;
	}

	// 	nstep = 2;
	pAgoraManager->pVideoCaptureManager = new VideoCaptureManager();
	if (!pAgoraManager->pVideoCaptureManager)
	{
		goto InitError;
	}

	nstep = 3;
	pAgoraManager->pVideoPlayManager = new VideoPlayManager();
	if (!pAgoraManager->pVideoPlayManager)
	{
		goto InitError;
	}

	nstep = 4;
	pAgoraManager->m_devAudioin = new CAgoraAudInputManager();
	if (!pAgoraManager->m_devAudioin)
	{
		goto InitError;
	}

	nstep = 5;
	pAgoraManager->m_devCamera = new CAgoraCameraManager();
	if (!pAgoraManager->m_devCamera)
	{
		goto InitError;
	}

	nstep = 6;
	pAgoraManager->m_devPlayout = new CAgoraPlayoutManager();
	if (!pAgoraManager->m_devPlayout)
	{
		goto InitError;
	}

	nstep = 7;
	pAgoraManager->m_EngineEventHandler = new CAGEngineEventHandler();
	if (!pAgoraManager->m_EngineEventHandler)
	{
		goto InitError;
	}

	nstep = 8;
	pAgoraManager->m_CapVideoFrameObserver = new CExtendVideoFrameObserver();
	if (!pAgoraManager->m_CapVideoFrameObserver)
	{
		goto InitError;
	}

	nstep = 9;
	pAgoraManager->m_CapAudioFrameObserver = new CExtendAudioFrameObserver();
	if (!pAgoraManager->m_CapAudioFrameObserver)
	{
		goto InitError;
	}

	nstep = 10;
	pAgoraManager->pPlayerCaptureManager = new AgoraPlayerManager();
	if (!pAgoraManager->pPlayerCaptureManager)
	{
		goto InitError;
	}

	nstep = 11;
	pAgoraManager->mpAudioCaptureCallback = new CAudioCaptureCallback();
	if (!pAgoraManager->mpAudioCaptureCallback)
	{
		goto InitError;
	}
	nstep = 10;
	WSADATA wsa;
	WSAStartup(MAKEWORD(2, 2), &wsa);
	USHORT pPortOk = getAvailablePort(7200, 7210);
	pSDHttpServer = new SDHttpServer("127.0.0.1", pPortOk);
	if (!pSDHttpServer)
	{
		goto InitError;
	}
	pSDHttpServer->start();

	return TRUE;
InitError:
	writelog("exe start err[%d]", nstep);
	quitSystem();
	WSACleanup();
	return FALSE;
}