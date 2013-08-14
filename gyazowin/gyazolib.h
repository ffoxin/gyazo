#ifndef GYAZOLIB_H
#define GYAZOLIB_H

// System headers
#include <windows.h>
#include <gdiplus.h>
using namespace Gdiplus;

// Project headers
#include "types.h"

bool    GetEncoderClsid(const wstring& format, CLSID& clsid);
bool    IsPngFile(const wstring& fileName);
void    ExecUrl(const wstring& url);
void    SetClipBoard(const wstring& text);
bool    ImageToPng(Image* image, const wstring& fileName);
bool    FileToPng(const wstring& destFile, const wstring& srcFile);
bool    BitmapToPng(HBITMAP hBmp, const wstring& fileName);
bool    UploadFile(const wstring& fileName);
wstring  GetId();
bool    SaveId(const wstring& sId);
int     ErrorMessage(const wstring& lpText);
wstring  GetIdDirPath();
wstring  GetIdFilePath();

#endif // GYAZOLIB_H
