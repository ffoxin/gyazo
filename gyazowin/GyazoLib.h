#pragma once

#include "Types.h"

#include <windows.h>
#include <gdiplus.h>

namespace Gyazo
{

BOOL    BitmapToPng(HBITMAP hBmp, wchar_t const* fileName);
int     ErrorMessage(std::wstring const& lpText);
BOOL    ExecUrl(std::wstring const& url);
BOOL    FileToPng(wchar_t const* srcFile, wchar_t const* destFile);
bool    GetEncoderClsid(std::wstring const& format, CLSID& clsid);
std::string  GetId();
std::wstring GetIdFilePath();
BOOL    ImageToPng(Gdiplus::Image* image, wchar_t const* fileName);
bool    IsPngFile(std::wstring const& fileName);
void    ReportError();
bool    SaveId(std::wstring const& sId);
BOOL    SetClipboard(std::wstring const& text);
bool    UploadFile(wchar_t const* fileName);

} // namespace Gyazo
