#include "gyazolib.h"

// System headers
#include <ShlObj.h>
#include <Shlwapi.h>
#include <WinInet.h>

// Project headers
#include "gdiinit.h"

namespace Gyazo
{

// Constants
LPCTSTR szTitle = _T("Gyazo");		// Text in the title bar

// I get the CLSID of the Encoder corresponding to the specified format
// Cited from MSDN Library: Retrieving the Class Identifier for an Encoder
int GetEncoderClsid(LPCWSTR format, CLSID* pClsid)
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

// Look at the header whether PNG image check (once)
bool IsPng(LPCTSTR fileName)
{
	unsigned char pngHead[] = { 0x89, 0x50, 0x4E, 0x47, 0x0D, 0x0A, 0x1A, 0x0A };
	unsigned char readHead[8];

	FILE *fp = NULL;

	if (_tfopen_s(&fp, fileName, _T("rb")) != 0
		|| fread(readHead, 1, 8, fp) != 8)
	{
		// Can not read the file
		return false;
	}
	fclose(fp);

	// compare
	if (memcmp(pngHead, readHead, sizeof(pngHead)) != 0)
	{
		return false;
	}

	return true;
}

// I open a browser (char *) URL that is specified
void ExecUrl(LPCTSTR url)
{
	// Run the open command
	SHELLEXECUTEINFO lsw = {};
	lsw.cbSize = sizeof(SHELLEXECUTEINFO);
	lsw.lpVerb = _T("open");
	lsw.lpFile = url;

	ShellExecuteEx(&lsw);
}

// Copy text to clipboard
void SetClipBoardText(LPCTSTR str)
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

// Save Image class data to file
bool ImageToPng(Image* image, LPCTSTR fileName)
{
	bool result = false;

	if (image->GetLastStatus() == 0)
	{
		CLSID clsidEncoder;
		if (GetEncoderClsid(L"image/png", &clsidEncoder) != -1)
		{
			if (image->Save(fileName, &clsidEncoder, 0) == 0)
			{
				result = true;
			}
		}
	}

	return result;
}

// Convert to PNG format
bool ConvertPng(LPCTSTR destFile, LPCTSTR srcFile)
{
	// Initialization GDI+
	const GdiScopeInit& gpi = GdiScopeInit();

	Image* image = new Image(srcFile, 0);

	bool result = ImageToPng(image, destFile);

	// Clean up
	delete image;

	return result;
}

// PNG formatでsave (GDI + Use)
bool SavePng(LPCTSTR fileName, HBITMAP hBmp)
{
	// Initialization GDI+
	const GdiScopeInit& gpi = GdiScopeInit();

	Bitmap* bitmap = new Bitmap(hBmp, NULL);

	bool result = ImageToPng(bitmap, fileName);

	// Clean up
	delete bitmap;

	return result;
}

// I want to upload a PNG file .
bool UploadFile(LPCTSTR fileName)
{
	LPCTSTR UPLOAD_SERVER	= _T("gyazo.com");
	LPCTSTR UPLOAD_PATH		= _T("/upload.cgi");
	LPCTSTR	SHARE_PATH		= _T("http://ffoxin.github.io/gyazo.htm?id=");

	const char*	sBoundary = "----BOUNDARYBOUNDARY----";		// boundary
	const char	sCrLf[] = { 0xd, 0xa, 0x0 };				// Diverted (CR + LF)
	LPCTSTR szHeader = 
		_T("Content-type: multipart/form-data; boundary=----BOUNDARYBOUNDARY----");

	std::ostringstream buf;	// Outgoing messages

	// Get an ID
	tstring tidStr = GetId();
	std::string idStr(tidStr.begin(), tidStr.end());

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

	// Read a PNG file
	std::ifstream png;
	png.open(fileName, std::ios::binary);
	if (png.fail())
	{
		png.close();
		ErrorMessage(_T("PNG open failed"));
		return false;
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
	std::string msg(buf.str());

	// WinInet preparation (proxy required Full setをはuse)
	HINTERNET hSession = InternetOpen(
		szTitle, 
		INTERNET_OPEN_TYPE_PRECONFIG, 
		NULL, 
		NULL, 
		0);
	if (hSession == NULL)
	{
		ErrorMessage(_T("Cannot configure wininet"));
		return false;
	}

	// Access point
	HINTERNET hConnection = InternetConnect(hSession, 
		UPLOAD_SERVER, INTERNET_DEFAULT_HTTP_PORT,
		NULL, NULL, INTERNET_SERVICE_HTTP, 0, NULL);
	if (hSession == NULL)
	{
		ErrorMessage(_T("Cannot initiate connection"));
		return false;
	}

	// Full set requirements before
	HINTERNET hRequest = HttpOpenRequest(hConnection,
		_T("POST"), UPLOAD_PATH, NULL,
		NULL, NULL, INTERNET_FLAG_DONT_CACHE | INTERNET_FLAG_RELOAD, NULL);
	if (hRequest == NULL)
	{
		ErrorMessage(_T("Cannot compose post request"));
		return false;
	}

	// User Agentを 指定
	LPCTSTR userAgent = _T("User-Agent: Gyazowin/1.0\r\n");
	BOOL bResult = HttpAddRequestHeaders(
		hRequest, userAgent, _tcslen(userAgent), 
		HTTP_ADDREQ_FLAG_ADD | HTTP_ADDREQ_FLAG_REPLACE);
	if (bResult == FALSE)
	{
		ErrorMessage(_T("Cannot set user agent"));
		return false;
	}

	// Requirementsをmessenger
	bResult = HttpSendRequest(hRequest,
		szHeader,
		lstrlen(szHeader),
		(LPVOID)msg.c_str(),
		(DWORD) msg.length());
	if (bResult == TRUE)
	{
		// Success requiresは

		DWORD nResponse = 8;
		TCHAR response[8];

		// state codeを 取得
		HttpQueryInfo(hRequest, HTTP_QUERY_STATUS_CODE, response, &nResponse, 0);
		if (_ttoi(response) != 200)
		{
			// upload 失敗 (status error)
			ErrorMessage(_T("Failed to upload (unexpected result code, under maintainance?)"));
		}
		else
		{
			// Upload succeeded

			// Get new id
			DWORD nId = 100;
			TCHAR id[100];

			_tcscpy_s(id, _T("X-Gyazo-Id"));

			HttpQueryInfo(hRequest, HTTP_QUERY_CUSTOM, id, &nId, 0);
			if (GetLastError() != ERROR_HTTP_HEADER_NOT_FOUND 
				&& nId != 0)
			{
				// Save new id
				SaveId(id);
			}

			// Read URL results
			DWORD nUrl;
			char url[1024];
			tstring srcUrl;
			tstring shareUrl = SHARE_PATH;

			// Never so long , but once well
			while (InternetReadFile(hRequest, (LPVOID) url, 1024, &nUrl) == TRUE
				&& nUrl != 0)
			{
				srcUrl.append(url, url + nUrl);
			}

			shareUrl += srcUrl.substr(srcUrl.find_last_of(_T("/")) + 1);

			// Copy the URL to the clipboard
			SetClipBoardText(shareUrl.c_str());

			// Launch an URL
			ExecUrl(shareUrl.c_str());

			return true;
		}
	}
	else
	{
		// Upload failed
		ErrorMessage(_T("Failed to upload"));
		return false;
	}

	return false;
}

// I generate load the ID
tstring GetId()
{
	// load ID from the file
	tifstream ifs;
	tstring sId;

	ifs.open(GetIdFilePath());
	if (!ifs.fail())
	{
		// read the ID
		ifs >> sId;
		ifs.close();
	}
	else
	{
		// (Compatibility with older versions) to read the ID from the same directory
		LPCTSTR idFile = _T("id.txt");

		ifs.close();
		ifs.open(idFile);
		if (!ifs.fail())
		{
			ifs >> sId;
			ifs.close();
		}
	}

	return sId;
}

// Save ID
bool SaveId(LPCTSTR sId)
{
	// save the ID to file
	CreateDirectory(GetIdDirPath(), NULL);

	tofstream ofs;
	ofs.open(GetIdFilePath());
	if (!ofs.fail())
	{
		ofs << sId;
		ofs.close();

		// Delete the old configuration file
		LPCTSTR idFile = _T("id.txt");

		if (PathFileExists(idFile))
		{
			DeleteFile(idFile);
		}
	}
	else
	{
		return false;
	}

	return true;
}

int ErrorMessage(LPCTSTR lpText)
{
	return MessageBox(NULL, lpText,	szTitle, MB_ICONERROR | MB_OK);
}

LPCTSTR GetIdDirPath()
{
	static TCHAR idDir[MAX_PATH] = {};

	if (*idDir == TCHAR(0))
	{
		SHGetSpecialFolderPath(NULL, idDir, CSIDL_APPDATA, FALSE);
		_tcscat_s(idDir, _T("\\Gyazo"));
	}

	return idDir;
}

LPCTSTR GetIdFilePath()
{
	static TCHAR idFile[MAX_PATH] = {};

	if (*idFile == TCHAR(0))
	{
		_tcscpy_s(idFile, GetIdDirPath());
		_tcscat_s(idFile, _T("\\id.txt"));
	}

	return idFile;
}

} // namespace Gyazo
