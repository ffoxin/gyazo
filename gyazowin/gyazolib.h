#ifndef GYAZOLIB_H
#define GYAZOLIB_H

// System headers
#include <windows.h>
#include <gdiplus.h>
using namespace Gdiplus;

// Project headers
#include "util.h"

extern const char_type* szTitle;

int     GetEncoderClsid(const string& format, CLSID* pClsid);
bool    IsPngFiles(const string& fileName);
void    ExecUrl(const string& url);
void    SetClipBoardText(const string& text);
bool	ImageToPng(Image* image, const string& fileName);
bool	ConvertPng(const string& destFile, const string& srcFile);
bool	SavePng(const string& fileName, HBITMAP hBmp);
bool	UploadFile(const string& fileName);
string	GetId();
bool	SaveId(const string& sId);
int		ErrorMessage(const string& lpText);
string	GetIdDirPath();
string	GetIdFilePath();

#endif // GYAZOLIB_H
