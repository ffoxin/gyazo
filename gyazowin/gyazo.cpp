/*
* gyazo.cpp
* 
* Entry point of gyazowin application
* 
*/

// System headers
#include <windows.h>
#include <Shlwapi.h>

// Project headers
#include "font.h"
#include "gyazolib.h"
#include "resource.h"
#include "stringconstants.h"

const GyazoSize cursorWinOffset(5, 5);
const GyazoSize clipWinTextBorder(5, 5);

// Globals
HINSTANCE hInstance;                // Application instance
HWND hClipWnd;
HWND hCursorWnd;
GyazoRect cursorWinRect;            // mouse coordinates window rect
GyazoSize cursorTextSize;           // mouse coordinates text size
GyazoSize cursorPos;                // mouse cursor position

GyazoSize screenOffset;             // virtual screen offset
GyazoSize screenSize;               // virtual screen size

// Declarations
ATOM                RegisterGyazoClass(HINSTANCE);
BOOL                InitInstance(HINSTANCE, int);
LRESULT CALLBACK    WndProcMain(HWND, UINT, WPARAM, LPARAM);
LRESULT CALLBACK    WndProcClip(HWND, UINT, WPARAM, LPARAM);
LRESULT CALLBACK    WndProcCursor(HWND, UINT, WPARAM, LPARAM);

void                DrawRubberband(HDC hdc, LPRECT newRect);
void                DrawCoordinates(HDC hdc, LPRECT newRect);
int                 DrawLabel(HDC hdc, const GyazoSize& textPos, LPCTSTR sText, int nText);

// Entry point
int APIENTRY wWinMain(HINSTANCE hInstance,
                       HINSTANCE,
                       LPTSTR,
                       int nCmdShow) {
    MSG msg;

    // Change working directory to app directory
    wchar_t appPath[MAX_PATH];
    GetModuleFileNameW(NULL, appPath, MAX_PATH);
    PathRemoveFileSpecW(appPath);
    SetCurrentDirectoryW(appPath);

    // Upload file if it specified as an argument
    if (__argc == 2) {
        // Exit by file upload
        string fileArg = __wargv[1];
        if (!IsPngFiles(fileArg)) {

        }
        if (IsPngFiles(fileArg)) {
            // PNG to upload directly
            UploadFile(fileArg);
        }
        else {
            // Convert to PNG format
            wchar_t tmpDir[MAX_PATH], tmpFile[MAX_PATH];
            GetTempPathW(MAX_PATH, tmpDir);
            GetTempFileNameW(tmpDir, GYAZO_PREFIX, 0, tmpFile);

            if (FileToPng(tmpFile, fileArg)) {
                //Upload
                UploadFile(tmpFile);
            }
            else {
                // Can not be converted into PNG
                ErrorMessage(ERROR_CONVERT_IMAGE);
            }
            DeleteFileW(tmpFile);
        }
        return TRUE;
    }

    // To register a window class
    RegisterGyazoClass(hInstance);

    // I run the application initialization :
    if (!InitInstance (hInstance, nCmdShow)) {
        return FALSE;
    }

    // Main message loop :
    while (GetMessageW(&msg, NULL, 0, 0)) {
        TranslateMessage(&msg);
        DispatchMessageW(&msg);
    }

    return (int) msg.wParam;
}

// To register a window class
ATOM RegisterGyazoClass(HINSTANCE hInstance) {
    WNDCLASS wc;
    ATOM result;

    // Main window
    wc.style            = 0;                            // Do not send a WM_PAINT
    wc.lpfnWndProc      = WndProcMain;
    wc.cbClsExtra       = 0;
    wc.cbWndExtra       = 0;
    wc.hInstance        = hInstance;
    wc.hIcon            = LoadIconW(hInstance, MAKEINTRESOURCE(IDI_ICON1));
    wc.hCursor          = LoadCursorW(NULL, IDC_CROSS);    // + Cursor
    wc.hbrBackground    = 0;                            // Background
    wc.lpszMenuName     = 0;
    wc.lpszClassName    = sWindowMainClass;

    result = RegisterClassW(&wc);
    if (result == 0) {
        return 0;
    }

    // Clip window
    wc.style            = CS_HREDRAW | CS_VREDRAW;
    wc.lpfnWndProc      = WndProcClip;
    wc.cbClsExtra       = 0;
    wc.cbWndExtra       = 0;
    wc.hInstance        = hInstance;
    wc.hIcon            = LoadIconW(hInstance, MAKEINTRESOURCE(IDI_ICON1));
    wc.hCursor          = LoadCursorW(NULL, IDC_CROSS);    // + Cursor
    wc.hbrBackground    = (HBRUSH) GetStockObject(WHITE_BRUSH);
    wc.lpszMenuName     = 0;
    wc.lpszClassName    = sWindowLayerClass;

    result = RegisterClassW(&wc);
    if (result == 0) {
        return 0;
    }

    // Cursor window
    wc.style            = CS_HREDRAW | CS_VREDRAW;
    wc.lpfnWndProc      = WndProcCursor;
    wc.cbClsExtra       = 0;
    wc.cbWndExtra       = 0;
    wc.hInstance        = hInstance;
    wc.hIcon            = LoadIconW(hInstance, MAKEINTRESOURCE(IDI_ICON1));
    wc.hCursor          = LoadCursorW(NULL, IDC_CROSS);    // + Cursor
    wc.hbrBackground    = (HBRUSH) GetStockObject(BLACK_BRUSH);
    wc.lpszMenuName     = 0;
    wc.lpszClassName    = sWindowCursorClass;

    result = RegisterClassW(&wc);
    if (result == 0) {
        return 0;
    }

    return result;
}

BOOL InitInstance(HINSTANCE hInst, int nCmdShow) {
    HWND hWnd;
    hInstance = hInst;

    screenOffset.cx = GetSystemMetrics(SM_XVIRTUALSCREEN);
    screenOffset.cy = GetSystemMetrics(SM_YVIRTUALSCREEN);
    screenSize.cx = GetSystemMetrics(SM_CXVIRTUALSCREEN);
    screenSize.cy = GetSystemMetrics(SM_CYVIRTUALSCREEN);

    hWnd = CreateWindowExW(
        WS_EX_TRANSPARENT | WS_EX_TOOLWINDOW | WS_EX_TOPMOST
#if (_WIN32_WINNT >= 0x0500)
        | WS_EX_NOACTIVATE
#endif
        ,
        sWindowMainClass, NULL, WS_POPUP,
        0, 0, 0, 0,
        NULL, NULL, hInst, NULL);

    // Did not make ... ?
    if (!hWnd) {
        return FALSE;
    }

    // I cover the entire screen
    MoveWindow(hWnd, screenOffset.cx, screenOffset.cy, screenSize.cx, screenSize.cy, FALSE);

    // (I troubled and is combed SW_MAXIMIZE) ignore the nCmdShow
    ShowWindow(hWnd, SW_SHOW);
    UpdateWindow(hWnd);

    // ESC key detection timer
    SetTimer(hWnd, 1, 100, NULL);

    // You can layer window
    hClipWnd = CreateWindowExW(
        WS_EX_TOOLWINDOW
#if (_WIN32_WINNT >= 0x0500)
        | WS_EX_LAYERED | WS_EX_NOACTIVATE
#endif
        ,
        sWindowLayerClass, NULL, WS_POPUP,
        0, 0, 0, 0,
        hWnd, NULL, hInst, NULL);

    SetLayeredWindowAttributes(hClipWnd, RGB(255, 0, 0), 100, LWA_COLORKEY | LWA_ALPHA);

    hCursorWnd = CreateWindowExW(
        WS_EX_TOOLWINDOW
#if (_WIN32_WINNT >= 0x0500)
        | WS_EX_LAYERED | WS_EX_NOACTIVATE
#endif
        ,
        sWindowCursorClass, NULL, WS_POPUP,
        0, 0, 0, 0,
        hWnd, NULL, hInst, NULL);

    SetLayeredWindowAttributes(hCursorWnd, RGB(255, 0, 0), 100, LWA_COLORKEY | LWA_ALPHA);

    return TRUE;
}

// Drawing rubber band
void DrawRubberband(HDC hdc, LPRECT newRect) {
    static bool firstDraw = true;   // First does not perform to erase the previous band

    if (firstDraw) {
        // View clip window
        ShowWindow(hClipWnd, SW_SHOW);
        UpdateWindow(hClipWnd);

        firstDraw = false;
    }

    if (newRect == NULL) {
        // Hide clip window
        ShowWindow(hClipWnd, SW_HIDE);
        return;
    }

    // Check coordinate
    GyazoRect clipRect = *newRect;
    if (clipRect.right < clipRect.left) {
        std::swap(clipRect.right, clipRect.left);
    }
    if (clipRect.bottom < clipRect.top) {
        std::swap(clipRect.bottom, clipRect.top);
    }

    MoveWindow(hClipWnd, 
        clipRect.left, 
        clipRect.top, 
        clipRect.right - clipRect.left + 1, 
        clipRect.bottom - clipRect.top + 1, 
        TRUE);
}

// Drawing rubber band
void DrawCoordinates(HDC hdc, LPRECT newRect) {
    static bool firstDraw = true;   // First does not perform to erase the previous band

    if (firstDraw) {
        // View coordinate window
        ShowWindow(hCursorWnd, SW_SHOW);
        UpdateWindow(hCursorWnd);

        firstDraw = false;
    }

    if (newRect == NULL) {
        // Hide the coordinate window
        ShowWindow(hCursorWnd, SW_HIDE);
        return;
    }

    GyazoRect coordRect = *newRect;
    if (coordRect.left + coordRect.right > screenSize.cx) {
        coordRect.left = screenSize.cx - coordRect.right;
    }
    if (coordRect.top + coordRect.bottom > screenSize.cy) {
        coordRect.top = screenSize.cy - coordRect.bottom;
    }

    MoveWindow(hCursorWnd, 
        coordRect.left, 
        coordRect.top, 
        coordRect.right, 
        coordRect.bottom, 
        TRUE);
}

int DrawLabel(HDC hdc, const GyazoSize& textPos, LPCTSTR sText, int nText) {
    int result;

    SetTextColor(hdc, RGB(0, 0, 0));
    result = TextOutW(hdc, textPos.cx + 1, textPos.cy + 1, sText, nText);
    if (result == 0) {
        return result;
    }

    SetTextColor(hdc, RGB(255, 255, 255));
    result = TextOutW(hdc, textPos.cx, textPos.cy, sText, nText);
    if (result == 0) {
        return result;
    }

    return result;
}

void EraseBackgroundLayer(const HWND& hWnd)
{
    GyazoRect winRect;
    GetWindowRect(hWnd, winRect);
    GyazoSize clipSize = winRect;

    HDC hdc = GetDC(hWnd);
    HBRUSH hBrush = CreateSolidBrush(RGB(100, 100, 100));
    SelectObject(hdc, hBrush);
    HPEN hPen = CreatePen(PS_DASH, 1, RGB(255, 255, 255));
    SelectObject(hdc, hPen);
    Rectangle(hdc, 0, 0, clipSize.cx, clipSize.cy);

    // The output size of the rectangle
    int fontHeight = MulDiv(8, GetDeviceCaps(hdc, LOGPIXELSY), 72);
    SelectObject(hdc, GyazoFont::GetFont(fontHeight));

    SetBkMode(hdc, TRANSPARENT);

    GyazoSize textSize, textPos;
    wchar_t sText[100];
    size_t nText;

    // Draw top left coordinates (top left corner)
    swprintf_s(sText, GYAZO_POINT_FORMAT, winRect.left, winRect.top);
    nText = wcslen(sText);
    textPos = clipWinTextBorder;

    DrawLabel(hdc, textPos, sText, nText);

    // Draw bottom right coordinates (bottom right corner)
    swprintf_s(sText, GYAZO_POINT_FORMAT, winRect.right, winRect.bottom);
    nText = wcslen(sText);
    GetTextExtentPointW(hdc, sText, nText, textSize);
    textPos = clipSize - textSize - clipWinTextBorder;

    DrawLabel(hdc, textPos, sText, nText);

    // Draw crop size (center)
    swprintf_s(sText, GYAZO_POINT_FORMAT, clipSize.cx, clipSize.cy);
    nText = wcslen(sText);
    GetTextExtentPointW(hdc, sText, nText, textSize);
    textPos.cx = (clipSize.cx - textSize.cx) / 2;
    textPos.cy = (clipSize.cy - textSize.cy) / 2;

    DrawLabel(hdc, textPos, sText, nText);

    // Release resources
    DeleteObject(hPen);
    DeleteObject(hBrush);
    GyazoFont::Release();
    ReleaseDC(hWnd, hdc);
}

// Layer window procedure
LRESULT CALLBACK WndProcClip(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam) {
    switch (message) {
    case WM_ERASEBKGND:
        EraseBackgroundLayer(hWnd);
        return TRUE;

    default:
        return DefWindowProc(hWnd, message, wParam, lParam);
    }

    return FALSE;
}

void EraseBackgroundCursor(const HWND& hWnd) {
    HDC hdc = GetDC(hWnd);
    HBRUSH hBrush = CreateSolidBrush(RGB(100, 100, 100));
    SelectObject(hdc, hBrush);
    HPEN hPen = CreatePen(PS_DASH, 1, RGB(255, 255, 255));
    SelectObject(hdc, hPen);
    Rectangle(hdc, 0, 0, cursorTextSize.cx + 4, cursorTextSize.cy);

    // The output size of the rectangle
    int fontHeight = MulDiv(8, GetDeviceCaps(hdc, LOGPIXELSY), 72);
    SelectObject(hdc, GyazoFont::GetFont(fontHeight));

    SetBkMode(hdc, TRANSPARENT);

    GyazoSize textPos;
    wchar_t sText[100];

    // Draw mouse coordinates
    swprintf(sText, sizeof(sText), GYAZO_POINT_FORMAT, cursorPos.cx, cursorPos.cy);
    size_t nText = wcslen(sText);
    GetTextExtentPointW(hdc, sText, nText, cursorTextSize);
    textPos.cx = 2;
    textPos.cy = 0;

    DrawLabel(hdc, textPos, sText, nText);

    // Release resources
    DeleteObject(hPen);
    DeleteObject(hBrush);
    GyazoFont::Release();
    ReleaseDC(hWnd, hdc);
}

// Cursor window procedure
LRESULT CALLBACK WndProcCursor(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam) {
    switch (message) {
    case WM_ERASEBKGND:
        EraseBackgroundCursor(hWnd);
        return TRUE;

    default:
        return DefWindowProcW(hWnd, message, wParam, lParam);
    }

    return FALSE;
}

void SaveAndUpload(const HWND& hWnd, GyazoRect& clipRect)
{
    HDC hdc = GetDC(NULL);

    // I turn off the line
    DrawRubberband(hdc, NULL);

    // Check coordinate
    if (clipRect.right < clipRect.left) {
        std::swap(clipRect.right, clipRect.left);
    }
    if (clipRect.bottom < clipRect.top) {
        std::swap(clipRect.bottom, clipRect.top);
    }

    // Image capture
    GyazoSize imageSize(
        clipRect.right - clipRect.left + 1, 
        clipRect.bottom - clipRect.top + 1);

    if (imageSize.cx == 0 || imageSize.cy == 0) {
        // It is not already in the image , it is not nothing
        ReleaseDC(NULL, hdc);
        DestroyWindow(hWnd);
        return;
    }

    // Create a bitmap buffer
    HBITMAP    newBMP    = CreateCompatibleBitmap(hdc, imageSize.cx, imageSize.cy);
    HDC        newDC    = CreateCompatibleDC(hdc);

    // Associated
    SelectObject(newDC, newBMP);

    // Portraitをobtain
    BitBlt(newDC, 0, 0, imageSize.cx, imageSize.cy, 
        hdc, clipRect.left, clipRect.top, SRCCOPY);

    // Hide the window
    ShowWindow(hWnd, SW_HIDE);

    // The determination of the temporary file name
    wchar_t tmpDir[MAX_PATH], tmpFile[MAX_PATH];
    GetTempPathW(MAX_PATH, tmpDir);
    GetTempFileNameW(tmpDir, GYAZO_PREFIX, 0, tmpFile);

    if (BitmapToPng(newBMP, tmpFile)) {
        UploadFile(tmpFile);
    }
    else {
        // PNG save failed
        ErrorMessage(ERROR_CONVERT_IMAGE);
    }

    // Clean up
    DeleteFileW(tmpFile);

    DeleteDC(newDC);
    DeleteObject(newBMP);

    ReleaseDC(NULL, hdc);
}

// Window procedure
LRESULT CALLBACK WndProcMain(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam) {
    HDC hdc;

    static bool isClip = false;
    static GyazoRect clipRect;

    switch (message) {
    case WM_RBUTTONDOWN:
        // Cancellation
        DestroyWindow(hWnd);
        return DefWindowProcW(hWnd, message, wParam, lParam);

    case WM_TIMER:
        // Detection of the ESC key is pressed
        if (GetKeyState(VK_ESCAPE) & 0x8000) {
            DestroyWindow(hWnd);
            return DefWindowProcW(hWnd, message, wParam, lParam);
        }
        break;

    case WM_MOUSEMOVE:
        cursorPos.cx = LOWORD(lParam) + screenOffset.cx;
        cursorPos.cy = HIWORD(lParam) + screenOffset.cy;

        hdc = GetDC(NULL);

        if (isClip) {
            // Set the new coordinate
            clipRect.right  = cursorPos.cx;
            clipRect.bottom = cursorPos.cy;

            DrawRubberband(hdc, clipRect);
        }
        else {
            cursorWinRect.left   = cursorPos.cx + cursorWinOffset.cx;
            cursorWinRect.top    = cursorPos.cy + cursorWinOffset.cy;
            cursorWinRect.right  = cursorTextSize.cx + 4;
            cursorWinRect.bottom = cursorTextSize.cy;

            DrawCoordinates(hdc, cursorWinRect);
            SendMessageW(hCursorWnd, WM_ERASEBKGND, NULL, NULL);
        }

        ReleaseDC(NULL, hdc);

        SetCapture(hWnd);
        break;


    case WM_LBUTTONDOWN:
        HDC hdc;
        // Clip start
        isClip = true;

        ReleaseCapture();

        // hide windows with mouse coordinates
        hdc = GetDC(NULL);
        DrawCoordinates(hdc, NULL);
        ReleaseDC(NULL, hdc);

        // Set the initial position
        clipRect.left = LOWORD(lParam) + screenOffset.cx;
        clipRect.top  = HIWORD(lParam) + screenOffset.cy;

        // Capture the mouse
        SetCapture(hWnd);
        break;

    case WM_LBUTTONUP:
        // Clip end
        isClip = false;

        // And release the mouse capture
        ReleaseCapture();

        // Set the new coordinate
        clipRect.right  = LOWORD(lParam) + screenOffset.cx;
        clipRect.bottom = HIWORD(lParam) + screenOffset.cy;

        // Screen ni direct drawing,っte -shaped
        SaveAndUpload(hWnd, clipRect);

        DestroyWindow(hWnd);
        PostQuitMessage(0);
        break;

    case WM_DESTROY:
        PostQuitMessage(0);
        break;

    default:
        return DefWindowProcW(hWnd, message, wParam, lParam);
    }

    return 0;
}
