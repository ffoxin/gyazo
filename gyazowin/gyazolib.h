#ifndef GYAZOLIB_H
#define GYAZOLIB_H

// System headers
#include <windows.h>
#include <gdiplus.h>
using namespace Gdiplus;

// Project headers
#include "types.h"

bool    BitmapToPng(HBITMAP hBmp, const wstring& fileName);
int     ErrorMessage(const wstring& lpText);
void    ExecUrl(const wstring& url);
bool    FileToPng(const wstring& destFile, const wstring& srcFile);
bool    GetEncoderClsid(const wstring& format, CLSID& clsid);
wstring GetId();
wstring GetIdDirPath();
wstring GetIdFilePath();
bool    ImageToPng(Image* image, const wstring& fileName);
bool    IsPngFile(const wstring& fileName);
bool    SaveId(const wstring& sId);
void    SetClipBoard(const wstring& text);
bool    UploadFile(const wstring& fileName);

#endif // GYAZOLIB_H
