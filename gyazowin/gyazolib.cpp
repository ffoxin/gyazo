// System headers
#include <ShlObj.h>
#include <Shlwapi.h>
#include <WinInet.h>
//#include <winhttp.h>

// STL headers
#include <fstream>
#include <memory>
#include <sstream>
#include <string>
using namespace std;

// Project headers
#include "gdiinit.h"
#include "stringconstants.h"

#include "gyazolib.h"

// Get the CLSID of the Encoder related to the specified format
// MSDN Library: Retrieving the Class Identifier for an Encoder
bool GetEncoderClsid(const wstring& format, CLSID& clsid)
{
    UINT num = 0;     // number of image encoders
    UINT size = 0;    // size of the image encoder array in bytes

    if (::GetImageEncodersSize(&num, &size) != Status::Ok)
    {
        return false;
    }

    if (size != 0)
    {
        unique_ptr<char[]> imageCodecInfo(new char[size]);
        ImageCodecInfo* pImageCodecInfo = reinterpret_cast<ImageCodecInfo*>(imageCodecInfo.get());

        ::GetImageEncoders(num, size, pImageCodecInfo);

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

    if (memcmp(pngHeader, fileHeader, sizeof(pngHeader)) != 0)
    {
        return false;
    }

    return true;
}

// Open a browser (char *) URL that is specified
BOOL ExecUrl(const wstring& url)
{
    // Run the open command
    SHELLEXECUTEINFO lsw = { };
    lsw.cbSize = sizeof(SHELLEXECUTEINFO);
    lsw.lpVerb = GYAZO_URL_OPEN;
    lsw.lpFile = url.c_str();

    return ::ShellExecuteExW(&lsw);
}

// Copy text to clipboard
BOOL SetClipboard(const wstring& text)
{
    size_t slen = text.size() + 1;

    unique_ptr<void, decltype(&::GlobalFree)> ptr_hText(
        ::GlobalAlloc(
        GMEM_DDESHARE | GMEM_MOVEABLE,
        slen * sizeof(wchar_t)
        ),
        &::GlobalFree
        );
    HGLOBAL hText = ptr_hText.get();

    if (hText == NULL)
    {
        return FALSE;
    }

    LPVOID lockedMem = ::GlobalLock(hText);
    if (lockedMem == NULL)
    {
        return FALSE;
    }

    wchar_t* pText = static_cast<wchar_t *>(lockedMem);
    wcscpy_s(pText, slen, text.c_str());
    if (::GlobalUnlock(hText) == 0)
    {
        return FALSE;
    }

    if (::OpenClipboard(NULL) == 0)
    {
        return FALSE;
    }
    if (::EmptyClipboard() == 0)
    {
        return FALSE;
    }
    if (::SetClipboardData(CF_UNICODETEXT, hText) == NULL)
    {
        return FALSE;
    }
    if (::CloseClipboard() == 0)
    {
        return FALSE;
    }

    return TRUE;
}

// Save Image class data to file
BOOL ImageToPng(Image* image, const wchar_t* fileName)
{
    if (image->GetLastStatus() != 0)
    {
        return FALSE;
    }

    CLSID clsidEncoder;
    if (!GetEncoderClsid(L"image/png", clsidEncoder))
    {
        return FALSE;
    }

    if (image->Save(fileName, &clsidEncoder, 0) != 0)
    {
        return FALSE;
    }

    return TRUE;
}

// Convert to PNG format
BOOL FileToPng(const wchar_t* srcFile, const wchar_t* destFile)
{
    const GdiInit gpi = GdiInit();

    Image image(srcFile, 0);

    return ImageToPng(&image, destFile);
}

// save BITMAP to PNG file
BOOL BitmapToPng(HBITMAP hBmp, const wchar_t* fileName)
{
    const GdiInit gpi = GdiInit();

    Bitmap bitmap(hBmp, NULL);

    return ImageToPng(&bitmap, fileName);
}

typedef unique_ptr<void, decltype(&InternetCloseHandle)> InternetHandle_t;
class InternetHandle
{
public:
    InternetHandle() = delete;
    InternetHandle(const InternetHandle&) = delete;
    InternetHandle& operator=(const InternetHandle&) = delete;

    InternetHandle(HINTERNET hInternet) :
        m_hInternet(hInternet, &InternetCloseHandle)
    {
    }
    ~InternetHandle()
    {
    }
    operator HINTERNET()
    {
        return m_hInternet.get();
    }


private:
    InternetHandle_t m_hInternet;

};

void ReportError()
{
    int error = GetLastError();
    if (error)
    {
        wchar_t* buf;
        ::FormatMessageW(FORMAT_MESSAGE_ALLOCATE_BUFFER
                       | FORMAT_MESSAGE_FROM_SYSTEM
                       | FORMAT_MESSAGE_FROM_HMODULE,
                       GetModuleHandleW(L"Winhttp.dll"),
                       error,
                       MAKELANGID(LANG_NEUTRAL, SUBLANG_DEFAULT),
                       (LPTSTR)&buf,
                       0,
                       NULL);

        wstringstream wss;
        wss << "[" << error << "] " << buf;
        ::LocalFree(buf);

        ::MessageBoxW(NULL, wss.str().c_str(), L"Error", MB_OK);
    }
    ExitProcess(error);
}

string BuildRequest(const wchar_t* fileName)
{
    ostringstream request; // Outgoing messages

    // Configuring Message
    // "id" part
    // ------BOUNDARYBOUNDARY----
    // content-disposition: form-data; name="id"
    //
    //
    //
    request << sDivider;
    request << sBoundary;
    request << sCRLF;
    request << sContentId;
    request << sCRLF;
    request << sCRLF;
    request << GetId();
    request << sCRLF;

    // - " ImageData " part
    request << sDivider;
    request << sBoundary;
    request << sCRLF;
    request << sContentData;
    request << sCRLF;
    request << sCRLF;

    // Read a PNG file
    ifstream png(fileName, ios::binary);
    if (png.fail())
    {
        ReportError();
    }

    // Read content & append to buffer
    request << png.rdbuf();
    png.close();

    // Stuff
    request << sCRLF;
    request << sDivider;
    request << sBoundary;
    request << sDivider;
    request << sCRLF;

    return request.str();
}

bool UploadFile(const wchar_t* fileName)
{
    string request = BuildRequest(fileName);

    // WinInet preparation
    InternetHandle hSession = ::InternetOpenW(
        sTitle,
        INTERNET_OPEN_TYPE_PRECONFIG,
        NULL,
        NULL,
        0);
    if (!hSession)
    {
        ReportError();
    }

    // Access point
    InternetHandle hConnection = ::InternetConnectW(
        hSession,
        GYAZO_UPLOAD_SERVER,
        INTERNET_DEFAULT_HTTP_PORT,
        NULL,
        NULL,
        INTERNET_SERVICE_HTTP,
        0,
        NULL);
    if (!hConnection)
    {
        ReportError();
    }

    // Full set requirements before
    InternetHandle hRequest = ::HttpOpenRequestW(
        hConnection,
        GYAZO_POST,
        GYAZO_UPLOAD_PATH,
        NULL,
        NULL,
        NULL,
        INTERNET_FLAG_DONT_CACHE
        | INTERNET_FLAG_RELOAD,
        NULL);
    if (!hRequest)
    {
        ReportError();
    }

    // User Agent
    BOOL bResult = ::HttpAddRequestHeadersW(
        hRequest,
        GYAZO_USER_AGENT,
        (DWORD)wcslen(GYAZO_USER_AGENT),
        HTTP_ADDREQ_FLAG_ADD
        | HTTP_ADDREQ_FLAG_REPLACE);
    if (bResult == FALSE)
    {
        ReportError();
    }

    bResult = ::HttpSendRequestW(
        hRequest,
        GYAZO_HEADER,
        (DWORD)wcslen(GYAZO_HEADER),
        (LPVOID)request.c_str(),
        (DWORD)request.size());
    if (bResult == FALSE)
    {
        ReportError();
    }

    DWORD bufferLength = 100;
    wchar_t buffer[101] = { };

    // state code
    bResult = ::HttpQueryInfoW(
        hRequest,
        HTTP_QUERY_STATUS_CODE,
        buffer,
        &bufferLength,
        0);
    if (bResult == FALSE ||
        stoi(buffer) != 200)
    {
        ReportError();
    }

    // get new id
    wcscpy_s(buffer, GYAZO_ID);
    bResult = ::HttpQueryInfoW(
        hRequest,
        HTTP_QUERY_CUSTOM,
        buffer,
        &bufferLength,
        0);
    if (bResult != FALSE
        && GetLastError() != ERROR_HTTP_HEADER_NOT_FOUND
        && bufferLength != 0)
    {
        // Save new id
        SaveId(buffer);
    }

    // Read URL results
    DWORD urlLength;
    char url[1024];
    wstring srcUrl;

    // Never so long , but once well
    while (::InternetReadFile(hRequest, url, 10, &urlLength) != FALSE
           && urlLength != 0)
    {
        srcUrl.append(url, url + urlLength);
    }

    // Prepare url
    srcUrl.insert(srcUrl.find(L"gyazo.com"), L"cache.");
    srcUrl.append(L".png");

    // Copy URL to clipboard and open in browser
    SetClipboard(srcUrl);
    ExecUrl(srcUrl);

    return true;
}

// I generate load the ID
string GetId()
{
    // load ID from the file
    string id;
    ifstream ifs(GetIdFilePath());
    if (!ifs.fail())
    {
        // read the ID
        ifs >> id;
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
            ifs >> id;
            ifs.close();
        }
    }

    return id;
}

// Save ID
bool SaveId(const wstring& sId)
{
    // save the ID to file
    ::CreateDirectoryW(GetIdDirPath().c_str(), NULL);

    wofstream ofs;
    ofs.open(GetIdFilePath());
    if (!ofs.fail())
    {
        ofs << sId;
        ofs.close();

        // Delete the old configuration file
        if (::PathFileExistsW(GYAZO_ID_FILENAME))
        {
            ::DeleteFileW(GYAZO_ID_FILENAME);
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
    return ::MessageBoxW(NULL, text.c_str(), sTitle, MB_ICONERROR | MB_OK);
}

wstring GetIdDirPath()
{
    static wchar_t idDir[MAX_PATH] = { };

    if (!wcslen(idDir))
    {
        ::SHGetSpecialFolderPathW(
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
