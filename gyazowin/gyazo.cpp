/*
* gyazo.cpp
* 
* Entry point of gyazowin application
* 
*/

// System headers
#include <windows.h>
#include <gdiplus.h>
using namespace Gdiplus;
#include <ShlObj.h>
#include <Shlwapi.h>
#include <tchar.h>
#include <WinInet.h>

// STL headers
#include <fstream>
#include <string>
#include <sstream>

// Project headers
#include "resource.h"
#include "util.h"
using namespace Gyazo;

// Globals
HINSTANCE hInstance;							// Application instance
TCHAR* szTitle			= _T("Gyazo");		// Text in the title bar
TCHAR* szWindowClass	= _T("GYAZOWIN");	// Main window class name
TCHAR* szWindowClassL	= _T("GYAZOWINL");	// Layer window class name
HWND hLayerWnd;

int screenOffsetX, screenOffsetY;	// virtual screen offset

// Declarations
ATOM				MyRegisterClass(HINSTANCE hInstance);
BOOL				InitInstance(HINSTANCE, int);
LRESULT CALLBACK	WndProc(HWND, UINT, WPARAM, LPARAM);
LRESULT CALLBACK	LayerWndProc(HWND, UINT, WPARAM, LPARAM);

int					GetEncoderClsid(const WCHAR* format, CLSID* pClsid);

BOOL				isPng(LPCTSTR fileName);
VOID				drawRubberband(HDC hdc, LPRECT newRect, BOOL erase);
VOID				execUrl(const TCHAR* url);
VOID				setClipBoardText(const char* str);
BOOL				convertPNG(LPCTSTR destFile, LPCTSTR srcFile);
BOOL				savePNG(LPCTSTR fileName, HBITMAP newBMP);
BOOL				uploadFile(HWND hwnd, LPCTSTR fileName);
string			getId();
BOOL				saveId(const TCHAR* str);

// Entry point
int APIENTRY _tWinMain(HINSTANCE hInstance,
					   HINSTANCE hPrevInstance,
					   LPTSTR    lpCmdLine,
					   int       nCmdShow)
{
	UNREFERENCED_PARAMETER(hPrevInstance);
	UNREFERENCED_PARAMETER(lpCmdLine);

	MSG msg;


	// Get app directory
	TCHAR szThisPath[MAX_PATH];
	GetModuleFileName(NULL, szThisPath, MAX_PATH);
	PathRemoveFileSpec(szThisPath);

	// Set the current directory to app directory
	SetCurrentDirectory(szThisPath);

	// Upload file if it specified as an argument
	if (__argc == 2)
	{
		// Exit by file upload
		LPCTSTR fileArg = __targv[1];
		if (isPng(fileArg))
		{
			// PNG to upload directly
			uploadFile(NULL, fileArg);
		}
		else
		{
			// Convert to PNG format
			TCHAR tmpDir[MAX_PATH], tmpFile[MAX_PATH];
			GetTempPath(MAX_PATH, tmpDir);
			GetTempFileName(tmpDir, _T("gya"), 0, tmpFile);

			if (convertPNG(tmpFile, fileArg))
			{
				//Upload
				uploadFile(NULL, tmpFile);
			}
			else
			{
				// Can not be converted into PNG ...
				MessageBox(NULL, _T("Cannot convert this image"), szTitle, 
					MB_OK | MB_ICONERROR);
			}
			DeleteFile(tmpFile);
		}
		return TRUE;
	}

	// To register a window class
	MyRegisterClass(hInstance);

	// I run the application initialization :
	if (!InitInstance (hInstance, nCmdShow))
	{
		return FALSE;
	}

	// Main message loop :
	while (GetMessage(&msg, NULL, 0, 0))
	{
		TranslateMessage(&msg);
		DispatchMessage(&msg);
	}

	return (int) msg.wParam;
}

// Look at the header whether PNG image check (once)
BOOL isPng(LPCTSTR fileName)
{
	unsigned char pngHead[] = { 0x89, 0x50, 0x4E, 0x47, 0x0D, 0x0A, 0x1A, 0x0A };
	unsigned char readHead[8];

	FILE *fp = NULL;

	if (_tfopen_s(&fp, fileName, _T("rb")) != 0
		|| fread(readHead, 1, 8, fp) != 8)
	{
		// Can not read the file
		return FALSE;
	}
	fclose(fp);

	// compare
	if (memcmp(pngHead, readHead, sizeof(pngHead)) != 0)
	{
		return FALSE;
	}

	return TRUE;

}

// To register a window class
ATOM MyRegisterClass(HINSTANCE hInstance)
{
	WNDCLASS wc;

	// Main window
	wc.style         = 0;							// Do not send a WM_PAINT
	wc.lpfnWndProc   = WndProc;
	wc.cbClsExtra    = 0;
	wc.cbWndExtra    = 0;
	wc.hInstance     = hInstance;
	wc.hIcon         = LoadIcon(hInstance, MAKEINTRESOURCE(IDI_ICON1));
	wc.hCursor       = LoadCursor(NULL, IDC_CROSS);	// + Cursor
	wc.hbrBackground = 0;							// Background
	wc.lpszMenuName  = 0;
	wc.lpszClassName = szWindowClass;

	RegisterClass(&wc);

	// Layer window
	wc.style         = CS_HREDRAW | CS_VREDRAW;
	wc.lpfnWndProc   = LayerWndProc;
	wc.cbClsExtra    = 0;
	wc.cbWndExtra    = 0;
	wc.hInstance     = hInstance;
	wc.hIcon         = LoadIcon(hInstance, MAKEINTRESOURCE(IDI_ICON1));
	wc.hCursor       = LoadCursor(NULL, IDC_CROSS);	// + Cursor
	wc.hbrBackground = (HBRUSH) GetStockObject(WHITE_BRUSH);
	wc.lpszMenuName  = 0;
	wc.lpszClassName = szWindowClassL;

	return RegisterClass(&wc);
}

// (I covered with a window full screen) initialization of instance
BOOL InitInstance(HINSTANCE hInst, int nCmdShow)
{
	HWND hWnd;
	hInstance = hInst; // I will store instances processing in a global variable .

	// It covers the entire virtual screen
	int screenOffsetX = GetSystemMetrics(SM_XVIRTUALSCREEN);
	int screenOffsetY = GetSystemMetrics(SM_YVIRTUALSCREEN);
	int screenWidth = GetSystemMetrics(SM_CXVIRTUALSCREEN);
	int screenHeight = GetSystemMetrics(SM_CYVIRTUALSCREEN);

	// I remember the offset value of x, y
	screenOffsetX = screenOffsetX;
	screenOffsetY = screenOffsetY;

	// Completely ni shi taウィthrough clothウをmake ru nn
	hWnd = CreateWindowEx(
		WS_EX_TRANSPARENT | WS_EX_TOOLWINDOW | WS_EX_TOPMOST
#if (_WIN32_WINNT >= 0x0500)
		| WS_EX_NOACTIVATE
#endif
		,
		szWindowClass, NULL, WS_POPUP,
		0, 0, 0, 0,
		NULL, NULL, hInst, NULL);

	// Did not make ... ?
	if (!hWnd)
	{
		return FALSE;
	}

	// I cover the entire screen
	MoveWindow(hWnd, screenOffsetX, screenOffsetY, screenWidth, screenHeight, FALSE);

	// (I troubled and is combed SW_MAXIMIZE) ignore the nCmdShow
	ShowWindow(hWnd, SW_SHOW);
	UpdateWindow(hWnd);

	// ESC key detection timer
	SetTimer(hWnd, 1, 100, NULL);

	// You can layer window
	hLayerWnd = CreateWindowEx(
		WS_EX_TOOLWINDOW
#if (_WIN32_WINNT >= 0x0500)
		| WS_EX_LAYERED | WS_EX_NOACTIVATE
#endif
		,
		szWindowClassL, NULL, WS_POPUP,
		100, 100, 300, 300,
		hWnd, NULL, hInst, NULL);

	SetLayeredWindowAttributes(hLayerWnd, RGB(255, 0, 0), 100, LWA_COLORKEY|LWA_ALPHA);

	return TRUE;
}

// I get the CLSID of the Encoder corresponding to the specified format
// Cited from MSDN Library: Retrieving the Class Identifier for an Encoder
int GetEncoderClsid(const WCHAR* format, CLSID* pClsid)
{
	unsigned num = 0;		// number of image encoders
	unsigned size = 0;		// size of the image encoder array in bytes

	GetImageEncodersSize(&num, &size);
	if (size == 0)
	{
		return -1;
	}

	//ImageCodecInfo* pImageCodecInfo = new ImageCodecInfo[num];
	ImageCodecInfo* pImageCodecInfo = reinterpret_cast<ImageCodecInfo*>(new char[size]);

	GetImageEncoders(num, size, pImageCodecInfo);

	for (unsigned i = 0; i < num; ++i)
	{
		if (wcscmp(pImageCodecInfo[i].MimeType, format) == 0)
		{
			*pClsid = pImageCodecInfo[i].Clsid;
			delete pImageCodecInfo;
			return i;
		}    
	}

	delete[] pImageCodecInfo;
	return -1;
}

// Drawing rubber band .
VOID drawRubberband(HDC hdc, LPRECT newRect, BOOL erase)
{
	static BOOL firstDraw = TRUE;	// First does not perform to erase the previous band
	static RECT clipRect;			// Band that was drawn last

	if (firstDraw)
	{
		// View layer window
		ShowWindow(hLayerWnd, SW_SHOW);
		UpdateWindow(hLayerWnd);

		firstDraw = FALSE;
	}

	if (erase)
	{
		// I hide the layer window
		ShowWindow(hLayerWnd, SW_HIDE);
	}

	// Check coordinate
	clipRect = *newRect;
	if (clipRect.right < clipRect.left)
	{
		swap(clipRect.right, clipRect.left);
	}
	if (clipRect.bottom < clipRect.top)
	{
		swap(clipRect.bottom, clipRect.top);
	}

	MoveWindow(hLayerWnd, 
		clipRect.left, 
		clipRect.top, 
		clipRect.right - clipRect.left + 1, 
		clipRect.bottom - clipRect.top + 1, 
		TRUE);
}

// Convert to PNG format
BOOL convertPNG(LPCTSTR destFile, LPCTSTR srcFile)
{
	BOOL				result = FALSE;

	GdiplusStartupInput	gdiplusStartupInput;
	ULONG_PTR			gdiplusToken;
	CLSID				clsidEncoder;

	// Full initialization GDI +
	GdiplusStartup(&gdiplusToken, &gdiplusStartupInput, NULL);

	Image *b = new Image(srcFile, 0);

	if (b->GetLastStatus() == 0)
	{
		if (GetEncoderClsid(L"image/png", &clsidEncoder))
		{
			// save!
			if (b->Save(destFile, &clsidEncoder, 0) == 0)
			{
				// Saveでki ta
				result = TRUE;
			}
		}
	}

	// Clean up
	delete b;
	GdiplusShutdown(gdiplusToken);

	return result;
}

// PNG formatでsave (GDI + Use)
BOOL savePNG(LPCTSTR fileName, HBITMAP newBMP)
{
	BOOL				result = FALSE;

	GdiplusStartupInput	gdiplusStartupInput;
	ULONG_PTR			gdiplusToken;
	CLSID				clsidEncoder;

	// Full initialization GDI +
	GdiplusStartup(&gdiplusToken, &gdiplusStartupInput, NULL);

	// Create a Bitmap from HBITMAP
	Bitmap *b = new Bitmap(newBMP, NULL);

	if (GetEncoderClsid(L"image/png", &clsidEncoder))
	{
		if (b->Save(fileName, &clsidEncoder, 0) == 0)
		{
			result = TRUE;
		}
	}

	// Clean up
	delete b;
	GdiplusShutdown(gdiplusToken);

	return result;
}

// Layer window procedure
LRESULT CALLBACK LayerWndProc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam)
{
	HDC		hdc;
	RECT	clipRect	= {0, 0, 500, 500};
	HBRUSH	hBrush;
	HPEN	hPen;
	HFONT	hFont;

	switch (message)
	{
	case WM_ERASEBKGND:
		{
			GetClientRect(hWnd, &clipRect);

			hdc = GetDC(hWnd);
			hBrush = CreateSolidBrush(RGB(100, 100, 100));
			SelectObject(hdc, hBrush);
			hPen = CreatePen(PS_DASH, 1, RGB(255, 255, 255));
			SelectObject(hdc, hPen);
			Rectangle(hdc, 0, 0, clipRect.right, clipRect.bottom);

			//The output size of the rectangle
			int fontHeight = MulDiv(8, GetDeviceCaps(hdc, LOGPIXELSY), 72);
			hFont = CreateFont(
				(-1) * fontHeight,		// Font height
				0,					// Text parcels
				0,					// Angle of text
				0,					// Angle of the x-axis and the baseline
				FW_REGULAR,			// The font weight (thickness)
				FALSE,				// Italic
				FALSE,				// Underline
				FALSE,				// Strike through
				ANSI_CHARSET,		// Character set
				OUT_DEFAULT_PRECIS,	// Output Accuracy
				CLIP_DEFAULT_PRECIS,// Clipping accuracy
				PROOF_QUALITY,		// Output Quality
				FIXED_PITCH | FF_MODERN, // Family pitch
				_T("Tahoma")		// Face name
				);

			SelectObject(hdc, hFont);
			// show size
			int iWidth  = clipRect.right  - clipRect.left;
			int iHeight = clipRect.bottom - clipRect.top;

			TCHAR sWidth[200], sHeight[200];
			_stprintf_s(sWidth, _T("%d"), iWidth);
			_stprintf_s(sHeight, _T("%d"), iHeight);

			int width = fontHeight * 5 / 2 + 8;
			int heightTop = fontHeight * 2 + 8;
			int heightBot = heightTop - fontHeight;
			size_t nWidth = _tcslen(sWidth);
			size_t nHeight = _tcslen(sHeight);

			SetBkMode(hdc, TRANSPARENT);
			SetTextColor(hdc, RGB(0, 0, 0));
			TextOut(hdc, clipRect.right - width + 1, clipRect.bottom - heightTop + 1, sWidth, nWidth);
			TextOut(hdc, clipRect.right - width + 1, clipRect.bottom - heightBot + 1, sHeight, nHeight);
			SetTextColor(hdc, RGB(255, 255, 255));
			TextOut(hdc, clipRect.right - width, clipRect.bottom - heightTop, sWidth, nWidth);
			TextOut(hdc, clipRect.right - width, clipRect.bottom - heightBot, sHeight, nHeight);

			DeleteObject(hPen);
			DeleteObject(hBrush);
			DeleteObject(hFont);
			ReleaseDC(hWnd, hdc);

			return TRUE;
		}
		break;

	default:
		return DefWindowProc(hWnd, message, wParam, lParam);
	}

	return FALSE;
}

// Window procedure
LRESULT CALLBACK WndProc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam)
{
	HDC hdc;

	static BOOL onClip		= FALSE;
	static BOOL firstDraw	= TRUE;
	static RECT clipRect	= {0, 0, 0, 0};

	switch (message)
	{
	case WM_RBUTTONDOWN:
		// Cancellation
		DestroyWindow(hWnd);
		return DefWindowProc(hWnd, message, wParam, lParam);
		break;

	case WM_TIMER:
		// Detection of the ESC key is pressed
		if (GetKeyState(VK_ESCAPE) & 0x8000)
		{
			DestroyWindow(hWnd);
			return DefWindowProc(hWnd, message, wParam, lParam);
		}
		break;

	case WM_MOUSEMOVE:
		if (onClip)
		{
			// Set the new coordinate
			clipRect.right  = LOWORD(lParam) + screenOffsetX;
			clipRect.bottom = HIWORD(lParam) + screenOffsetY;

			hdc = GetDC(NULL);
			drawRubberband(hdc, &clipRect, FALSE);

			ReleaseDC(NULL, hdc);
		}
		break;


	case WM_LBUTTONDOWN:
		// Clip start
		onClip = TRUE;

		// Set the initial position
		clipRect.left = LOWORD(lParam) + screenOffsetX;
		clipRect.top  = HIWORD(lParam) + screenOffsetY;

		// Capture the mouse
		SetCapture(hWnd);
		break;

	case WM_LBUTTONUP:
		{
			// Clip end
			onClip = FALSE;

			// And release the mouse capture
			ReleaseCapture();

			// Set the new coordinate
			clipRect.right  = LOWORD(lParam) + screenOffsetX;
			clipRect.bottom = HIWORD(lParam) + screenOffsetY;

			// Screen ni direct drawing,っte -shaped
			HDC hdc = GetDC(NULL);

			// I turn off the line
			drawRubberband(hdc, &clipRect, TRUE);

			// Check coordinate
			if (clipRect.right < clipRect.left)
			{
				swap(clipRect.right, clipRect.left);
			}
			if (clipRect.bottom < clipRect.top)
			{
				swap(clipRect.bottom, clipRect.top);
			}

			// Image capture
			int iWidth	= clipRect.right  - clipRect.left + 1;
			int iHeight	= clipRect.bottom - clipRect.top  + 1;

			if (iWidth == 0 || iHeight == 0)
			{
				// It is not already in the image , it is not nothing
				ReleaseDC(NULL, hdc);
				DestroyWindow(hWnd);
				break;
			}

			// Create a bitmap buffer
			HBITMAP newBMP = CreateCompatibleBitmap(hdc, iWidth, iHeight);
			HDC	    newDC  = CreateCompatibleDC(hdc);

			// Associated
			SelectObject(newDC, newBMP);

			// Portraitをobtain
			BitBlt(newDC, 0, 0, iWidth, iHeight, 
				hdc, clipRect.left, clipRect.top, SRCCOPY);

			// I hide the window !
			ShowWindow(hWnd, SW_HIDE);

			// The determination of the temporary file name
			TCHAR tmpDir[MAX_PATH], tmpFile[MAX_PATH];
			GetTempPath(MAX_PATH, tmpDir);
			GetTempFileName(tmpDir, _T("gya"), 0, tmpFile);

			if (savePNG(tmpFile, newBMP))
			{
				uploadFile(hWnd, tmpFile);
			}
			else
			{
				// PNG save failed
				MessageBox(hWnd, _T("Cannot save png image"), szTitle, 
					MB_OK | MB_ICONERROR);
			}

			// Clean up
			DeleteFile(tmpFile);

			DeleteDC(newDC);
			DeleteObject(newBMP);

			ReleaseDC(NULL, hdc);
			DestroyWindow(hWnd);
			PostQuitMessage(0);
		}
		break;

	case WM_DESTROY:
		PostQuitMessage(0);
		break;

	default:
		return DefWindowProc(hWnd, message, wParam, lParam);
	}

	return 0;
}

// Copy text to clipboard
VOID setClipBoardText(const TCHAR* str)
{
	HGLOBAL	hText;
	LPTSTR	pText;
	size_t	slen;

	slen = _tcslen(str) + 1; // NULL

	hText = GlobalAlloc(GMEM_DDESHARE | GMEM_MOVEABLE, slen * sizeof(TCHAR));

	pText = (LPTSTR) GlobalLock(hText);
	_tcsncpy_s(pText, slen, str, slen);
	GlobalUnlock(hText);

	// I open the clipboard
	OpenClipboard(NULL);
	EmptyClipboard();
	SetClipboardData(
#ifdef UNICODE
		CF_UNICODETEXT
#else
		CF_TEXT
#endif
		, 
		hText);
	CloseClipboard();

	// Liberation
	GlobalFree(hText);
}

// I open a browser (char *) URL that is specified
VOID execUrl(const TCHAR* url)
{
	// Run the open command
	SHELLEXECUTEINFO lsw = {};
	lsw.cbSize = sizeof(SHELLEXECUTEINFO);
	lsw.lpVerb = _T("open");
	lsw.lpFile = url;

	ShellExecuteEx(&lsw);
}

// I generate load the ID
string getId()
{
	TCHAR idFile[MAX_PATH];
	TCHAR idDir[MAX_PATH];

	SHGetSpecialFolderPath(NULL, idFile, CSIDL_APPDATA, FALSE);

	_tcscat_s(idFile, _T("\\Gyazo"));
	_tcscpy_s(idDir, idFile);
	_tcscat_s(idFile, _T("\\id.txt"));

	const TCHAR* idOldFile = _T("id.txt");
	BOOL oldFileExist = FALSE;

	string idStr;

	// First Load ID from the file
	ifstream ifs;

	ifs.open(idFile);
	if (!ifs.fail())
	{
		// I read the ID
		ifs >> idStr;
		ifs.close();
	}
	else
	{
		ifstream ifsold;
		ifsold.open(idOldFile);
		if (!ifsold.fail())
		{
			// (Compatibility with older versions) to read the ID from the same directory
			ifsold >> idStr;
			ifsold.close();
		}
	}

	return idStr;
}

// Save ID
BOOL saveId(const TCHAR* str)
{
	TCHAR idFile[MAX_PATH];
	TCHAR idDir[MAX_PATH];

	SHGetSpecialFolderPath(NULL, idFile, CSIDL_APPDATA, FALSE);

	_tcscat_s(idFile, _T("\\Gyazo"));
	_tcscpy_s(idDir, idFile);
	_tcscat_s(idFile, _T("\\id.txt"));

	const TCHAR* idOldFile = _T("id.txt");

	// I want to save the ID
	CreateDirectory(idDir, NULL);
	ofstream ofs;
	ofs.open(idFile);
	if (!ofs.fail())
	{
		ofs << str;
		ofs.close();

		// Delete the old configuration file
		if (PathFileExists(idOldFile))
		{
			DeleteFile(idOldFile);
		}
	}
	else
	{
		return FALSE;
	}

	return TRUE;
}

// I want to upload a PNG file .
BOOL uploadFile(HWND hwnd, LPCTSTR fileName)
{
	const TCHAR* UPLOAD_SERVER	= _T("gyazo.com");
	const TCHAR* UPLOAD_PATH	= _T("/upload.cgi");

	const char*  sBoundary = "----BOUNDARYBOUNDARY----";		// boundary
	const char   sCrLf[]   = { 0xd, 0xa, 0x0 };					// Diverted (CR + LF)
	const TCHAR* szHeader  = 
		_T("Content-type: multipart/form-data; boundary=----BOUNDARYBOUNDARY----");

	std::ostringstream	buf;	// Outgoing messages

	// Get an ID
	string idTStr = getId();
	std::string idStr(idTStr.begin(), idTStr.end());

	// Configuring Message
	// -- "id" part
	buf << "--";
	buf << sBoundary;
	buf << sCrLf;
	buf << "content-disposition: form-data; name=\"id\"";
	buf << sCrLf;
	buf << sCrLf;
	buf << idStr;
	buf << sCrLf;

	// - " ImageData " part
	buf << "--";
	buf << sBoundary;
	buf << sCrLf;
	buf << "content-disposition: form-data; name=\"imagedata\"; filename=\"gyazo.com\"";
	buf << sCrLf;
	//buf << "Content-type: image/png";	// 一応
	//buf << sCrLf;
	buf << sCrLf;

	// I read a PNG file : text
	std::ifstream png;
	png.open(fileName, std::ios::binary);
	if (png.fail())
	{
		MessageBox(hwnd, _T("PNG open failed"), szTitle, MB_ICONERROR | MB_OK);
		png.close();
		return FALSE;
	}
	buf << png.rdbuf();		// read all & append to buffer
	png.close();

	// Last
	buf << sCrLf;
	buf << "--";
	buf << sBoundary;
	buf << "--";
	buf << sCrLf;

	// Message completion
	std::string oMsg(buf.str());

	// WinInetをpreparation (proxy required Full setをはuse)
	HINTERNET hSession = InternetOpen(szTitle, 
		INTERNET_OPEN_TYPE_PRECONFIG, NULL, NULL, 0);
	if (hSession == NULL)
	{
		MessageBox(hwnd, _T("Cannot configure wininet"),
			szTitle, MB_ICONERROR | MB_OK);
		return FALSE;
	}

	// Access point
	HINTERNET hConnection = InternetConnect(hSession, 
		UPLOAD_SERVER, INTERNET_DEFAULT_HTTP_PORT,
		NULL, NULL, INTERNET_SERVICE_HTTP, 0, NULL);
	if (hSession == NULL)
	{
		MessageBox(hwnd, _T("Cannot initiate connection"),
			szTitle, MB_ICONERROR | MB_OK);
		return FALSE;
	}

	// Full set requirements before
	HINTERNET hRequest = HttpOpenRequest(hConnection,
		_T("POST"), UPLOAD_PATH, NULL,
		NULL, NULL, INTERNET_FLAG_DONT_CACHE | INTERNET_FLAG_RELOAD, NULL);
	if (hSession == NULL)
	{
		MessageBox(hwnd, _T("Cannot compose post request"),
			szTitle, MB_ICONERROR | MB_OK);
		return FALSE;
	}

	// User Agentを 指定
	const TCHAR* ua = _T("User-Agent: Gyazowin/1.0\r\n");
	BOOL bResult = HttpAddRequestHeaders(
		hRequest, ua, _tcslen(ua), 
		HTTP_ADDREQ_FLAG_ADD | HTTP_ADDREQ_FLAG_REPLACE);
	if (bResult == FALSE)
	{
		MessageBox(hwnd, _T("Cannot set user agent"),
			szTitle, MB_ICONERROR | MB_OK);
		return FALSE;
	}

	// Requirementsをmessenger
	if (HttpSendRequest(hRequest,
		szHeader,
		lstrlen(szHeader),
		(LPVOID)oMsg.c_str(),
		(DWORD) oMsg.length()))
	{
		// Success requiresは

		DWORD resLen = 8;
		TCHAR resCode[8];

		// state codeを 取得
		HttpQueryInfo(hRequest, HTTP_QUERY_STATUS_CODE, resCode, &resLen, 0);
		if (_ttoi(resCode) != 200)
		{
			// upload 失敗 (status error)
			MessageBox(hwnd, _T("Failed to upload (unexpected result code, under maintainance?)"),
				szTitle, MB_ICONERROR | MB_OK);
		}
		else
		{
			// upload succeeded

			// get new id
			DWORD idLen = 100;
			TCHAR newid[100];

			memset(newid, 0, idLen*sizeof(TCHAR));	
			_tcscpy_s(newid, _T("X-Gyazo-Id"));

			HttpQueryInfo(hRequest, HTTP_QUERY_CUSTOM, newid, &idLen, 0);
			if (GetLastError() != ERROR_HTTP_HEADER_NOT_FOUND && idLen != 0)
			{
				//save new id
				saveId(newid);
			}

			// I read (URL) Results
			DWORD len;
			char  resbuf[1024];
			std::string result;

			// Never so long , but once well
			while (InternetReadFile(hRequest, (LPVOID) resbuf, 1024, &len) 
				&& len != 0)
			{
				result.append(resbuf, len);
			}

			std::string id = result.substr(result.find_last_of("/") + 1);

			stringstream url;
			url << _T("http://ffoxin.github.io/gyazo.htm?id=") << id.c_str();

			// Copy the URL to the clipboard
			setClipBoardText(url.str().c_str());

			// Launch an URL
			execUrl(url.str().c_str()); 

			return TRUE;
		}
	}
	else
	{
		// Upload failed ...
		MessageBox(hwnd, _T("Failed to upload"), szTitle, MB_ICONERROR | MB_OK);
	}

	return FALSE;
}
