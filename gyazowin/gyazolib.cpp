// System headers
#include <ShlObj.h>
#include <Shlwapi.h>
#include <WinInet.h>

// STL headers
#include <fstream>
#include <memory>
#include <sstream>
#include <string>
using namespace std;

// Project headers
#include "exceptions.h"
#include "gdiinit.h"
#include "stringconstants.h"

#include "gyazolib.h"

// I get the CLSID of the Encoder corresponding to the specified format
// Cited from MSDN Library: Retrieving the Class Identifier for an Encoder
bool GetEncoderClsid(const wstring& format, CLSID& clsid)
{
    size_t num = 0;     // number of image encoders
    size_t size = 0;    // size of the image encoder array in bytes

    if (GetImageEncodersSize(&num, &size) != Status::Ok)
    {
        return false;
    }

    if (size != 0)
    {
        shared_ptr<char> imageCodecInfo(
            new char[size],
            [](char* p)
        {
            delete[] p;
        } // deleter functor
        );
        ImageCodecInfo* pImageCodecInfo = reinterpret_cast<ImageCodecInfo*>(imageCodecInfo.get());

        GetImageEncoders(num, size, pImageCodecInfo);

        for (size_t i = 0; i < num; ++i)
        {
            if (format == pImageCodecInfo[i].MimeType)
            {
                clsid = pImageCodecInfo[i].Clsid;
                return true;
            }
        }
    }

    return false;
}

// Look at the header whether PNG image check (once)
bool IsPngFile(const wstring& fileName)
{
    const uint8_t pngHeader[] = { 0x89, 0x50, 0x4E, 0x47, 0x0D, 0x0A, 0x1A, 0x0A };
    const size_t pngHeaderSize = sizeof(pngHeader) / sizeof(*pngHeader);

    uint8_t fileHeader[pngHeaderSize];

    ifstream ifs(fileName.c_str(), ios_base::in | ios_base::binary);
    if (!ifs.is_open())
    {
        return false;
    }

    ifs.read(reinterpret_cast<char *>(fileHeader), pngHeaderSize);
    if (!ifs.good())
    {
        return false;
    }

    // compare
    if (memcmp(pngHeader, fileHeader, sizeof(pngHeader)) != 0)
    {
        return false;
    }

    return true;
}

// I open a browser (char *) URL that is specified
void ExecUrl(const wstring& url)
{
    // Run the open command
    SHELLEXECUTEINFO lsw = { };
    lsw.cbSize = sizeof(SHELLEXECUTEINFO);
    lsw.lpVerb = GYAZO_URL_OPEN;
    lsw.lpFile = url.c_str();

    ShellExecuteExW(&lsw);
}

// Copy text to clipboard
void SetClipBoard(const wstring& text)
{
    size_t slen = text.size() + 1;

    HGLOBAL hText = GlobalAlloc(
        GMEM_DDESHARE | GMEM_MOVEABLE,
        slen * sizeof(wchar_t)
        );

    if (hText == NULL)
    {
        return;
    }

    LPVOID lockedMem = GlobalLock(hText);
    if (lockedMem == NULL)
    {
        GlobalFree(hText);
        return;
    }
    wchar_t* pText = static_cast<wchar_t *>(lockedMem);
    wcscpy_s(pText, slen, text.c_str());
    GlobalUnlock(hText);

    // I open the clipboard
    OpenClipboard(NULL);
    EmptyClipboard();
    SetClipboardData(CF_UNICODETEXT, hText);
    CloseClipboard();

    // Liberation
    GlobalFree(hText);
}

// Save Image class data to file
bool ImageToPng(Image* image, const wstring& fileName)
{
    if (image->GetLastStatus() == 0)
    {
        CLSID clsidEncoder;
        if (GetEncoderClsid(L"image/png", clsidEncoder))
        {
            if (image->Save(fileName.c_str(), &clsidEncoder, 0) == 0)
            {
                return true;
            }
        }
    }

    return false;
}

// Convert to PNG format
bool FileToPng(const wstring& destFile, const wstring& srcFile)
{
    const GdiInit gpi = GdiInit();

    Image image(srcFile.c_str(), 0);

    return ImageToPng(&image, destFile);
}

// save BITMAP to PNG file
bool BitmapToPng(HBITMAP hBmp, const wstring& fileName)
{
    const GdiInit gpi = GdiInit();

    Bitmap bitmap(hBmp, NULL);

    return ImageToPng(&bitmap, fileName);
}

bool UploadFile(const wstring& fileName)
{
    ostringstream buf; // Outgoing messages

    // Get an ID
    wstring tidStr = GetId();
    string idStr(tidStr.begin(), tidStr.end());

    // Configuring Message
    // -- "id" part
    buf << sDivider;
    buf << sBoundary;
    buf << sCrLf;
    buf << sContentId;
    buf << sCrLf;
    buf << sCrLf;
    buf << idStr;
    buf << sCrLf;

    // - " ImageData " part
    buf << sDivider;
    buf << sBoundary;
    buf << sCrLf;
    buf << sContentData;
    buf << sCrLf;
    buf << sCrLf;

    try
    {
        // Read a PNG file
        ifstream png(fileName, ios::binary);
        if (png.fail())
        {
            throw ExOpenPng();
        }

        // Read content & append to buffer
        buf << png.rdbuf();
        png.close();

        // Stuff
        buf << sCrLf;
        buf << sDivider;
        buf << sBoundary;
        buf << sDivider;
        buf << sCrLf;

        // Message completion
        string msg(buf.str());

        // WinInet preparation (proxy required)
        HINTERNET hSession = InternetOpenW(sTitle, INTERNET_OPEN_TYPE_PRECONFIG, NULL, NULL, 0);
        if (hSession == NULL)
        {
            throw ExWininetConfigure();
        }

        // Access point
        HINTERNET hConnection = InternetConnectW(hSession, GYAZO_UPLOAD_SERVER,
                                                 INTERNET_DEFAULT_HTTP_PORT, NULL, NULL,
                                                 INTERNET_SERVICE_HTTP, 0, NULL);
        if (hConnection == NULL)
        {
            throw ExConnectionInit();
        }

        // Full set requirements before
        HINTERNET hRequest = HttpOpenRequestW(hConnection, GYAZO_POST, GYAZO_UPLOAD_PATH,
                                              NULL, NULL, NULL,
                                              INTERNET_FLAG_DONT_CACHE | INTERNET_FLAG_RELOAD,
                                              NULL);
        if (hRequest == NULL)
        {
            throw ExComposePost();
        }

        // User Agent
        BOOL bResult = HttpAddRequestHeadersW(hRequest, GYAZO_USER_AGENT, wcslen(GYAZO_USER_AGENT),
                                              HTTP_ADDREQ_FLAG_ADD | HTTP_ADDREQ_FLAG_REPLACE);
        if (bResult == FALSE)
        {
            throw ExSetUserAgent();
        }

        bResult = HttpSendRequestW(hRequest, GYAZO_HEADER, wcslen(GYAZO_HEADER),
                                   (LPVOID)msg.c_str(), (DWORD)msg.length());
        if (bResult == FALSE)
        {
            throw ExFailedUpload();
        }

        DWORD nResponse = 8;
        wchar_t response[8] = { };

        // state code
        HttpQueryInfoW(hRequest, HTTP_QUERY_STATUS_CODE, response, &nResponse, 0);
        if (stoi(response) != 200)
        {
            throw ExUploadImage();
        }

        // get new id
        DWORD nId = 100;
        wchar_t id[100];

        wcscpy_s(id, GYAZO_ID);

        HttpQueryInfoW(hRequest, HTTP_QUERY_CUSTOM, id, &nId, 0);
        if (GetLastError() != ERROR_HTTP_HEADER_NOT_FOUND
            && nId != 0)
        {
            // Save new id
            SaveId(id);
        }

        // Read URL results
        DWORD nUrl;
        char url[1024];
        wstring srcUrl;

        // Never so long , but once well
        while (InternetReadFile(hRequest, url, 1024, &nUrl) == TRUE
               && nUrl != 0)
        {
            srcUrl.append(url, url + nUrl);
        }

        srcUrl.insert(srcUrl.find(L"gyazo.com"), L"cache.");
        srcUrl += L".png";

        // Copy the URL to the clipboard
        SetClipBoard(srcUrl);

        // Launch an URL
        ExecUrl(srcUrl);
    }
    catch (IExTemplate& ex)
    {
        ex.report();
        return false;
    }

    return true;
}

// I generate load the ID
wstring GetId()
{
    // load ID from the file
    wifstream ifs;
    wstring sId;

    ifs.open(GetIdFilePath());
    if (!ifs.fail())
    {
        // read the ID
        ifs >> sId;
        ifs.close();
    }
    else
    {
        // (Compatibility with older versions) read the ID from the same directory
        wstring idFile = GYAZO_ID_FILENAME;

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
bool SaveId(const wstring& sId)
{
    // save the ID to file
    CreateDirectoryW(GetIdDirPath().c_str(), NULL);

    wofstream ofs;
    ofs.open(GetIdFilePath());
    if (!ofs.fail())
    {
        ofs << sId;
        ofs.close();

        // Delete the old configuration file
        if (PathFileExistsW(GYAZO_ID_FILENAME))
        {
            DeleteFileW(GYAZO_ID_FILENAME);
        }
    }
    else
    {
        return false;
    }

    return true;
}

int ErrorMessage(const wstring& text)
{
    return MessageBoxW(NULL, text.c_str(), sTitle, MB_ICONERROR | MB_OK);
}

wstring GetIdDirPath()
{
    static wchar_t idDir[MAX_PATH] = { };

    if (!wcslen(idDir))
    {
        SHGetSpecialFolderPathW(
            NULL,
            idDir,
            CSIDL_APPDATA,
            FALSE);
        wcscat_s(idDir, GYAZO_DIRNAME);
    }

    return idDir;
}

wstring GetIdFilePath()
{
    static wstring idFile;

    if (!idFile.length())
    {
        idFile = GetIdDirPath() + GYAZO_ID_FILEPATH;
    }

    return idFile;
}
