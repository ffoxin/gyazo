#ifndef GYAZOLIB_H
#define GYAZOLIB_H

// System headers
#include <windows.h>
#include <gdiplus.h>
using namespace Gdiplus;

// Project headers
#include "types.h"

bool    GetEncoderClsid(const string& format, CLSID& clsid);
bool    IsPngFiles(const string& fileName);
void    ExecUrl(const string& url);
void    SetClipBoardText(const string& text);
bool    ImageToPng(Image* image, const string& fileName);
bool    FileToPng(const string& destFile, const string& srcFile);
bool    BitmapToPng(HBITMAP hBmp, const string& fileName);
bool    UploadFile(const string& fileName);
string  GetId();
bool    SaveId(const string& sId);
int     ErrorMessage(const string& lpText);
string  GetIdDirPath();
string  GetIdFilePath();

#endif // GYAZOLIB_H
