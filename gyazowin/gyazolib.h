#ifndef GYAZOLIB_H
#define GYAZOLIB_H

// System headers
#include <windows.h>
#include <gdiplus.h>
using namespace Gdiplus;
#include <tchar.h>

// Project headers
#include "util.h"

namespace Gyazo
{
	extern LPCTSTR		szTitle;

	int					GetEncoderClsid(LPCWSTR format, CLSID* pClsid);
	bool				IsPng(LPCTSTR fileName);
	void				ExecUrl(LPCTSTR url);
	void				SetClipBoardText(LPCTSTR str);
	bool				ImageToPng(Image* image, LPCTSTR fileName);
	bool				ConvertPng(LPCTSTR destFile, LPCTSTR srcFile);
	bool				SavePng(LPCTSTR fileName, HBITMAP hBmp);
	bool				UploadFile(LPCTSTR fileName);
	tstring				GetId();
	bool				SaveId(LPCTSTR sId);
	int					ErrorMessage(LPCTSTR lpText);
	LPCTSTR				GetIdDirPath();
	LPCTSTR				GetIdFilePath();
}

#endif // GYAZOLIB_H
