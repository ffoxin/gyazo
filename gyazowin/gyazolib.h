#ifndef GYAZOLIB_H
#define GYAZOLIB_H

// System headers
#include <windows.h>
#include <gdiplus.h>
using namespace Gdiplus;

// Project headers
#include "types.h"

BOOL    BitmapToPng(HBITMAP hBmp, const wchar_t* fileName);
int     ErrorMessage(const wstring& lpText);
BOOL    ExecUrl(const wstring& url);
BOOL    FileToPng(const wchar_t* srcFile, const wchar_t* destFile);
bool    GetEncoderClsid(const wstring& format, CLSID& clsid);
string  GetId();
wstring GetIdDirPath();
wstring GetIdFilePath();
BOOL    ImageToPng(Image* image, const wchar_t* fileName);
bool    IsPngFile(const wstring& fileName);
void    ReportError();
bool    SaveId(const wstring& sId);
BOOL    SetClipboard(const wstring& text);
bool    UploadFile(const wchar_t* fileName);
#endif // GYAZOLIB_H
