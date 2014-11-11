//
// Gyazo for Windows
//
// 2013-05-25
//

#include "font.h"
#include "gyazolib.h"
#include "resource.h"
#include "stringconstants.h"

#include <windows.h>
#include <Shlwapi.h>

Gyazo::Size const cursorWinOffset(5, 5);
Gyazo::Size const clipWinTextBorder(5, 5);

namespace Windows
{

LONG const CURSOR_OFFSET = 4L;

// Globals
HWND hClipWnd;
HWND hCursorWnd;
Gyazo::Rect cursorWinRect;            // mouse coordinates window rect
Gyazo::Size cursorTextSize;           // mouse coordinates text size
Gyazo::Size cursorPos;                // mouse cursor position

Gyazo::Size screenOffset;             // virtual screen offset
Gyazo::Size screenSize;               // virtual screen size

// Declarations
void                RegisterGyazoClass(HINSTANCE);
BOOL                InitInstance(HINSTANCE, int);
LRESULT CALLBACK    WndProcMain(HWND, UINT, WPARAM, LPARAM);
LRESULT CALLBACK    WndProcClip(HWND, UINT, WPARAM, LPARAM);
LRESULT CALLBACK    WndProcCursor(HWND, UINT, WPARAM, LPARAM);

void DrawRubberband(LPRECT newRect);
void DrawCoordinates(LPRECT newRect);
int DrawLabel(HDC hdc, Gyazo::Size const& textPos, LPCWSTR sText, std::size_t nText);

class TempFile
{
public:
    TempFile() : m_isInit(false)
    {
    }
    ~TempFile()
    {
        Release();
    }

    operator wchar_t const*()
    {
        Init();
        return m_tempFile;
    }

private:
    void Init()
    {
        if (!m_isInit)
        {
            wchar_t tempDir[MAX_PATH];
            ::GetTempPathW(MAX_PATH, tempDir);
            ::GetTempFileNameW(tempDir, Gyazo::Common::PREFIX, 0, m_tempFile);
            m_isInit = true;
        }
    }
    void Release()
    {
        if (m_isInit)
        {
            ::DeleteFileW(m_tempFile);
        }
    }

    wchar_t m_tempFile[MAX_PATH];
    bool m_isInit;
};

// Entry point
int WINAPI GyazoMain(HINSTANCE hInstance, int nShowCmd)
{
    MSG msg;

    // Change working directory to app directory
    wchar_t appPath[MAX_PATH];
    if (::GetModuleFileNameW(NULL, appPath, MAX_PATH) == 0)
    {
        Gyazo::ReportError();
    }
    ::PathRemoveFileSpecW(appPath);
    if (::SetCurrentDirectoryW(appPath) == 0)
    {
        Gyazo::ReportError();
    }

    // Upload file if it specified as an argument
    if (__argc == 2)
    {
        TempFile tmpFile;
        wchar_t const* fileArg = __wargv[1];
        if (!Gyazo::IsPngFile(fileArg))
        {
            // Convert to PNG format
            if (!Gyazo::FileToPng(fileArg, tmpFile))
            {
                // Can not be converted into PNG
                Gyazo::ErrorMessage(Gyazo::Windows::ERROR_CONVERT_IMAGE);
                Gyazo::ReportError();
            }
            fileArg = tmpFile;
        }

        // Upload PNG
        Gyazo::UploadFile(fileArg);

        return TRUE;
    }

    // Register window classes
    RegisterGyazoClass(hInstance);

    // I run the application initialization :
    if (!InitInstance(hInstance, nShowCmd))
    {
        return FALSE;
    }

    // Main message loop :
    while (::GetMessageW(&msg, NULL, 0, 0))
    {
        ::TranslateMessage(&msg);
        ::DispatchMessageW(&msg);
    }

    return (int)msg.wParam;
}

// To register a window class
void RegisterGyazoClass(HINSTANCE hInstance)
{
    WNDCLASS wc;

    // Main window
    wc.style = 0;                            // Do not send a WM_PAINT
    wc.lpfnWndProc = WndProcMain;
    wc.cbClsExtra = 0;
    wc.cbWndExtra = 0;
    wc.hInstance = hInstance;
    wc.hIcon = ::LoadIconW(hInstance, MAKEINTRESOURCE(IDI_ICON1));
    wc.hCursor = ::LoadCursorW(NULL, IDC_CROSS);    // + Cursor
    wc.hbrBackground = 0;                            // Background
    wc.lpszMenuName = 0;
    wc.lpszClassName = Gyazo::Windows::sWindowMainClass;

    if (::RegisterClassW(&wc) == 0)
    {
        Gyazo::ReportError();
    }

    // Clip window
    wc.style = CS_HREDRAW | CS_VREDRAW;
    wc.lpfnWndProc = WndProcClip;
    wc.cbClsExtra = 0;
    wc.cbWndExtra = 0;
    wc.hInstance = hInstance;
    wc.hIcon = ::LoadIconW(hInstance, MAKEINTRESOURCE(IDI_ICON1));
    wc.hCursor = ::LoadCursorW(NULL, IDC_CROSS);    // + Cursor
    wc.hbrBackground = (HBRUSH)::GetStockObject(WHITE_BRUSH);
    wc.lpszMenuName = 0;
    wc.lpszClassName = Gyazo::Windows::sWindowLayerClass;

    if (::RegisterClassW(&wc) == 0)
    {
        Gyazo::ReportError();
    }

    // Cursor window
    wc.style = CS_HREDRAW | CS_VREDRAW;
    wc.lpfnWndProc = WndProcCursor;
    wc.cbClsExtra = 0;
    wc.cbWndExtra = 0;
    wc.hInstance = hInstance;
    wc.hIcon = ::LoadIconW(hInstance, MAKEINTRESOURCE(IDI_ICON1));
    wc.hCursor = ::LoadCursorW(NULL, IDC_CROSS);    // + Cursor
    wc.hbrBackground = (HBRUSH)::GetStockObject(BLACK_BRUSH);
    wc.lpszMenuName = 0;
    wc.lpszClassName = Gyazo::Windows::sWindowCursorClass;

    if (::RegisterClassW(&wc) == 0)
    {
        Gyazo::ReportError();
    }
}

BOOL InitInstance(HINSTANCE hInst, int nCmdShow)
{
    HWND hWnd;

    screenOffset.cx = ::GetSystemMetrics(SM_XVIRTUALSCREEN);
    screenOffset.cy = ::GetSystemMetrics(SM_YVIRTUALSCREEN);
    screenSize.cx = ::GetSystemMetrics(SM_CXVIRTUALSCREEN);
    screenSize.cy = ::GetSystemMetrics(SM_CYVIRTUALSCREEN);

    hWnd = ::CreateWindowExW(
        WS_EX_TRANSPARENT | WS_EX_TOOLWINDOW | WS_EX_TOPMOST
#if (_WIN32_WINNT >= 0x0500)
        | WS_EX_NOACTIVATE
#endif
        ,
        Gyazo::Windows::sWindowMainClass, NULL, WS_POPUP,
        0, 0, 0, 0,
        NULL, NULL, hInst, NULL);

    // Did not make ... ?
    if (!hWnd)
    {
        return FALSE;
    }

    // I cover the entire screen
    ::MoveWindow(hWnd, screenOffset.cx, screenOffset.cy, screenSize.cx, screenSize.cy, FALSE);

    // (I troubled and is combed SW_MAXIMIZE) ignore the nCmdShow
    ::ShowWindow(hWnd, /*SW_SHOW*/nCmdShow);
    ::UpdateWindow(hWnd);

    // ESC key detection timer
    ::SetTimer(hWnd, 1, 100, NULL);

    // You can layer window
    hClipWnd = ::CreateWindowExW(
        WS_EX_TOOLWINDOW
#if (_WIN32_WINNT >= 0x0500)
        | WS_EX_LAYERED | WS_EX_NOACTIVATE
#endif
        ,
        Gyazo::Windows::sWindowLayerClass, NULL, WS_POPUP,
        0, 0, 0, 0,
        hWnd, NULL, hInst, NULL);

    ::SetLayeredWindowAttributes(hClipWnd, RGB(255, 0, 0), 100, LWA_COLORKEY | LWA_ALPHA);

    hCursorWnd = ::CreateWindowExW(
        WS_EX_TOOLWINDOW
#if (_WIN32_WINNT >= 0x0500)
        | WS_EX_LAYERED | WS_EX_NOACTIVATE
#endif
        ,
        Gyazo::Windows::sWindowCursorClass, NULL, WS_POPUP,
        0, 0, 0, 0,
        hWnd, NULL, hInst, NULL);

    ::SetLayeredWindowAttributes(hCursorWnd, RGB(255, 0, 0), 100, LWA_COLORKEY | LWA_ALPHA);

    return TRUE;
}

// Drawing rubber band
void DrawRubberband(LPRECT newRect)
{
    static bool firstDraw = true;   // First does not perform to erase the previous band

    if (firstDraw)
    {
        // View clip window
        ::ShowWindow(hClipWnd, SW_SHOW);
        ::UpdateWindow(hClipWnd);

        firstDraw = false;
    }

    if (newRect == NULL)
    {
        // Hide clip window
        ::ShowWindow(hClipWnd, SW_HIDE);
        return;
    }

    // Check coordinate
    Gyazo::Rect clipRect(newRect);
    if (clipRect.right < clipRect.left)
    {
        std::swap(clipRect.right, clipRect.left);
    }
    if (clipRect.bottom < clipRect.top)
    {
        std::swap(clipRect.bottom, clipRect.top);
    }

    ::MoveWindow(
        hClipWnd,
        clipRect.left,
        clipRect.top,
        clipRect.right - clipRect.left + 1,
        clipRect.bottom - clipRect.top + 1,
        TRUE);
}

// Drawing rubber band
void DrawCoordinates(LPRECT newRect)
{
    static bool firstDraw = true;   // First does not perform to erase the previous band

    if (firstDraw)
    {
        // View coordinate window
        ::ShowWindow(hCursorWnd, SW_SHOW);
        ::UpdateWindow(hCursorWnd);

        firstDraw = false;
    }

    if (newRect == NULL)
    {
        // Hide the coordinate window
        ::ShowWindow(hCursorWnd, SW_HIDE);
        return;
    }

    Gyazo::Rect coordRect(newRect);
    if (coordRect.left + coordRect.right > screenSize.cx)
    {
        coordRect.left = screenSize.cx - coordRect.right;
    }
    if (coordRect.top + coordRect.bottom > screenSize.cy)
    {
        coordRect.top = screenSize.cy - coordRect.bottom;
    }

    ::MoveWindow(
        hCursorWnd,
        coordRect.left,
        coordRect.top,
        coordRect.right,
        coordRect.bottom,
        TRUE);
}

int DrawLabel(HDC hdc, Gyazo::Size const& textPos, LPCWSTR sText, std::size_t nText)
{
    int result;

    ::SetTextColor(hdc, RGB(0, 0, 0));
    result = ::TextOutW(hdc, textPos.cx + 1, textPos.cy + 1, sText, (int)nText);
    if (result == 0)
    {
        return result;
    }

    ::SetTextColor(hdc, RGB(255, 255, 255));
    result = ::TextOutW(hdc, textPos.cx, textPos.cy, sText, (int)nText);
    if (result == 0)
    {
        return result;
    }

    return result;
}

void EraseBackgroundLayer(HWND const& hWnd)
{
    Gyazo::Rect winRect;
    ::GetWindowRect(hWnd, winRect);
    Gyazo::Size clipSize = winRect;

    HDC hdc = ::GetDC(hWnd);
    HBRUSH hBrush = ::CreateSolidBrush(RGB(100, 100, 100));
    ::SelectObject(hdc, hBrush);
    HPEN hPen = ::CreatePen(PS_DASH, 1, RGB(255, 255, 255));
    ::SelectObject(hdc, hPen);
    ::Rectangle(hdc, 0, 0, clipSize.cx, clipSize.cy);

    // The output size of the rectangle
    int fontHeight = ::MulDiv(8, ::GetDeviceCaps(hdc, LOGPIXELSY), 72);
    ::SelectObject(hdc, Gyazo::Font::GetFont(fontHeight));

    ::SetBkMode(hdc, TRANSPARENT);

    Gyazo::Size textSize, textPos;
    wchar_t sText[100];
    std::size_t nText;

    // Draw top left coordinates (top left corner)
    swprintf_s(sText, Gyazo::Ui::POINT_FORMAT, winRect.left, winRect.top);
    nText = wcslen(sText);
    textPos = clipWinTextBorder;

    DrawLabel(hdc, textPos, sText, nText);

    // Draw bottom right coordinates (bottom right corner)
    swprintf_s(sText, Gyazo::Ui::POINT_FORMAT, winRect.right, winRect.bottom);
    nText = wcslen(sText);
    ::GetTextExtentPointW(hdc, sText, (int)nText, textSize);
    textPos = clipSize - textSize - clipWinTextBorder;

    DrawLabel(hdc, textPos, sText, nText);

    // Draw crop size (center)
    swprintf_s(sText, Gyazo::Ui::POINT_FORMAT, clipSize.cx, clipSize.cy);
    nText = wcslen(sText);
    ::GetTextExtentPointW(hdc, sText, (int)nText, textSize);
    textPos.cx = (clipSize.cx - textSize.cx) / 2;
    textPos.cy = (clipSize.cy - textSize.cy) / 2;

    DrawLabel(hdc, textPos, sText, nText);

    // Release resources
    ::DeleteObject(hPen);
    ::DeleteObject(hBrush);
    Gyazo::Font::Release();
    ::ReleaseDC(hWnd, hdc);
}

// Layer window procedure
LRESULT CALLBACK WndProcClip(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam)
{
    switch (message)
    {
    case WM_ERASEBKGND:
        EraseBackgroundLayer(hWnd);
        return TRUE;

    default:
        return ::DefWindowProcW(hWnd, message, wParam, lParam);
    }
}

void EraseBackgroundCursor(HWND const& hWnd)
{
    HDC hdc = ::GetDC(hWnd);
    HBRUSH hBrush = ::CreateSolidBrush(RGB(100, 100, 100));
    ::SelectObject(hdc, hBrush);
    HPEN hPen = ::CreatePen(PS_DASH, 1, RGB(255, 255, 255));
    ::SelectObject(hdc, hPen);
    ::Rectangle(hdc, 0, 0, cursorTextSize.cx + CURSOR_OFFSET, cursorTextSize.cy);

    // The output size of the rectangle
    int fontHeight = MulDiv(8, GetDeviceCaps(hdc, LOGPIXELSY), 72);
    ::SelectObject(hdc, Gyazo::Font::GetFont(fontHeight));

    ::SetBkMode(hdc, TRANSPARENT);

    Gyazo::Size textPos;
    wchar_t sText[100];

    // Draw mouse coordinates
    swprintf(sText, sizeof(sText) / sizeof(*sText), Gyazo::Ui::POINT_FORMAT, cursorPos.cx, cursorPos.cy);
    std::size_t nText = wcslen(sText);
    ::GetTextExtentPointW(hdc, sText, (int)nText, cursorTextSize);
    textPos.cx = 2;
    textPos.cy = 0;

    DrawLabel(hdc, textPos, sText, nText);

    // Release resources
    ::DeleteObject(hPen);
    ::DeleteObject(hBrush);
    Gyazo::Font::Release();
    ::ReleaseDC(hWnd, hdc);
}

// Cursor window procedure
LRESULT CALLBACK WndProcCursor(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam)
{
    switch (message)
    {
    case WM_ERASEBKGND:
        EraseBackgroundCursor(hWnd);
        return TRUE;

    default:
        return DefWindowProcW(hWnd, message, wParam, lParam);
    }
}

void SaveAndUpload(HWND const& hWnd, Gyazo::Rect& clipRect)
{
    HDC hdc = ::GetDC(NULL);

    // I turn off the line
    DrawRubberband(NULL);

    // Check coordinate
    if (clipRect.right < clipRect.left)
    {
        std::swap(clipRect.right, clipRect.left);
    }
    if (clipRect.bottom < clipRect.top)
    {
        std::swap(clipRect.bottom, clipRect.top);
    }

    // Image capture
    Gyazo::Size imageSize(
        clipRect.right - clipRect.left + 1,
        clipRect.bottom - clipRect.top + 1);

    if (imageSize.cx == 0 || imageSize.cy == 0)
    {
        // It is not already in the image , it is not nothing
        ::ReleaseDC(NULL, hdc);
        ::DestroyWindow(hWnd);
        return;
    }

    // Create a bitmap buffer
    HBITMAP newBMP = ::CreateCompatibleBitmap(hdc, imageSize.cx, imageSize.cy);
    HDC newDC = ::CreateCompatibleDC(hdc);

    // Associated
    ::SelectObject(newDC, newBMP);

    // Portraitをobtain
    ::BitBlt(newDC, 0, 0, imageSize.cx, imageSize.cy,
        hdc, clipRect.left, clipRect.top, SRCCOPY);

    // Hide the window
    ::ShowWindow(hWnd, SW_HIDE);

    // The determination of the temporary file name
    wchar_t tmpDir[MAX_PATH], tmpFile[MAX_PATH];
    ::GetTempPathW(MAX_PATH, tmpDir);
    ::GetTempFileNameW(tmpDir, Gyazo::Common::PREFIX, 0, tmpFile);

    if (Gyazo::BitmapToPng(newBMP, tmpFile))
    {
        Gyazo::UploadFile(tmpFile);
    }
    else
    {
        // PNG save failed
        Gyazo::ErrorMessage(Gyazo::Windows::ERROR_CONVERT_IMAGE);
    }

    // Clean up
    ::DeleteFileW(tmpFile);

    ::DeleteDC(newDC);
    ::DeleteObject(newBMP);

    ::ReleaseDC(NULL, hdc);
}

// Window procedure
LRESULT CALLBACK WndProcMain(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam)
{
    static bool isClip = false;
    static Gyazo::Rect clipRect;

    switch (message)
    {
    case WM_RBUTTONDOWN:
        // Cancellation
        ::DestroyWindow(hWnd);
        return ::DefWindowProcW(hWnd, message, wParam, lParam);

    case WM_TIMER:
        // Detection of the ESC key is pressed
        if (::GetKeyState(VK_ESCAPE) & 0x8000)
        {
            ::DestroyWindow(hWnd);
            return ::DefWindowProcW(hWnd, message, wParam, lParam);
        }
        break;

    case WM_MOUSEMOVE:
        cursorPos.cx = LOWORD(lParam) + screenOffset.cx;
        cursorPos.cy = HIWORD(lParam) + screenOffset.cy;

        if (isClip)
        {
            // Set the new coordinate
            clipRect.right = cursorPos.cx;
            clipRect.bottom = cursorPos.cy;

            DrawRubberband(clipRect);
        }
        else
        {
            cursorWinRect.left = cursorPos.cx + cursorWinOffset.cx;
            cursorWinRect.top = cursorPos.cy + cursorWinOffset.cy;
            cursorWinRect.right = cursorTextSize.cx + CURSOR_OFFSET;
            cursorWinRect.bottom = cursorTextSize.cy;

            DrawCoordinates(cursorWinRect);
            ::SendMessageW(hCursorWnd, WM_ERASEBKGND, NULL, NULL);
        }

        ::SetCapture(hWnd);
        break;


    case WM_LBUTTONDOWN:
        // Clip start
        isClip = true;

        ::ReleaseCapture();

        // hide windows with mouse coordinates
        DrawCoordinates(NULL);

        // Set the initial position
        clipRect.left = LOWORD(lParam) + screenOffset.cx;
        clipRect.top = HIWORD(lParam) + screenOffset.cy;

        // Capture the mouse
        ::SetCapture(hWnd);
        break;

    case WM_LBUTTONUP:
        // Clip end
        isClip = false;

        // And release the mouse capture
        ::ReleaseCapture();

        // Set the new coordinate
        clipRect.right = LOWORD(lParam) + screenOffset.cx;
        clipRect.bottom = HIWORD(lParam) + screenOffset.cy;

        // Screen ni direct drawing,っte -shaped
        SaveAndUpload(hWnd, clipRect);

        ::DestroyWindow(hWnd);
        ::PostQuitMessage(0);
        break;

    case WM_DESTROY:
        ::PostQuitMessage(0);
        break;

    default:
        return ::DefWindowProcW(hWnd, message, wParam, lParam);
    }

    return 0;
}

} // namespace Windows

int WINAPI wWinMain(HINSTANCE hInstance,
    HINSTANCE,
    LPWSTR,
    int nShowCmd)
{
    return Windows::GyazoMain(hInstance, nShowCmd);
}
