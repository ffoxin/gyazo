#include "gyazolib.h"

#include "gdiinit.h"
#include "stringconstants.h"
#include "convertion.h"

#include <ShlObj.h>
#include <Shlwapi.h>
#include <WinInet.h>

#include <fstream>
#include <memory>
#include <sstream>
#include <string>

typedef std::unique_ptr<void, decltype(&InternetCloseHandle)> InternetHandle_t;

namespace Gyazo
{

// Get the CLSID of the Encoder related to the specified format
// MSDN Library: Retrieving the Class Identifier for an Encoder
bool GetEncoderClsid(std::wstring const& format, CLSID& clsid)
{
    UINT num = 0;     // number of image encoders
    UINT size = 0;    // size of the image encoder array in bytes

    if (Gdiplus::GetImageEncodersSize(&num, &size) != Gdiplus::Status::Ok)
    {
        return false;
    }

    if (size != 0)
    {
        std::unique_ptr<char[]> imageCodecInfo(new char[static_cast<std::size_t>(size)]);
        Gdiplus::ImageCodecInfo* pImageCodecInfo = reinterpret_cast<Gdiplus::ImageCodecInfo*>(imageCodecInfo.get());

        Gdiplus::GetImageEncoders(num, size, pImageCodecInfo);

        for (std::size_t i = 0; i < static_cast<std::size_t>(num); ++i)
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
bool IsPngFile(std::wstring const& fileName)
{
    uint8_t const pngHeader[] = { 0x89, 0x50, 0x4E, 0x47, 0x0D, 0x0A, 0x1A, 0x0A };
    std::size_t const pngHeaderSize = sizeof(pngHeader) / sizeof(*pngHeader);

    uint8_t fileHeader[pngHeaderSize];

    std::ifstream ifs(fileName.c_str(), std::ios_base::in | std::ios_base::binary);
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
BOOL ExecUrl(std::wstring const& url)
{
    // Run the open command
    SHELLEXECUTEINFO lsw = {};
    lsw.cbSize = sizeof(SHELLEXECUTEINFO);
    lsw.lpVerb = Web::URL_OPEN;
    lsw.lpFile = url.c_str();

    return ::ShellExecuteExW(&lsw);
}

// Copy text to clipboard
BOOL SetClipboard(std::wstring const& text)
{
    std::size_t slen = text.size() + 1;

    std::unique_ptr<void, decltype(&::GlobalFree)> ptr_hText(
        ::GlobalAlloc(
        GMEM_DDESHARE | GMEM_MOVEABLE,
        slen * sizeof(wchar_t)),
        &::GlobalFree);
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
BOOL ImageToPng(Gdiplus::Image* image, wchar_t const* fileName)
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
BOOL FileToPng(wchar_t const* srcFile, wchar_t const* destFile)
{
    GdiInit const gpi = GdiInit();

    Gdiplus::Image image(srcFile, 0);

    return ImageToPng(&image, destFile);
}

// save BITMAP to PNG file
BOOL BitmapToPng(HBITMAP hBmp, wchar_t const* fileName)
{
    GdiInit const gpi = GdiInit();

    Gdiplus::Bitmap bitmap(hBmp, NULL);

    return ImageToPng(&bitmap, fileName);
}

class InternetHandle
{
public:
    InternetHandle() = delete;
    InternetHandle(InternetHandle const&) = delete;
    InternetHandle& operator=(InternetHandle const&) = delete;

    explicit InternetHandle(const HINTERNET& hInternet) :
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

        std::wstringstream wss;
        wss << "[" << error << "] " << buf;
        ::LocalFree(buf);

        ::MessageBoxW(NULL, wss.str().c_str(), L"Error", MB_OK);
    }
    ExitProcess(error);
}

std::string BuildRequest(wchar_t const* fileName)
{
    std::ostringstream request; // Outgoing messages

    // Configuring Message
    // "id" part
    // ------BOUNDARYBOUNDARY----
    // content-disposition: form-data; name="id"
    //
    //
    //
    request << Web::sDivider;
    request << Web::sBoundary;
    request << Web::sCRLF;
    request << Web::sContentId;
    request << Web::sCRLF;
    request << Web::sCRLF;
    request << GetId();
    request << Web::sCRLF;

    // - " ImageData " part
    request << Web::sDivider;
    request << Web::sBoundary;
    request << Web::sCRLF;
    request << Web::sContentData;
    request << Web::sCRLF;
    request << Web::sCRLF;

    // Read a PNG file
    std::ifstream png(fileName, std::ios::binary);
    if (png.fail())
    {
        ReportError();
    }

    // Read content & append to buffer
    request << png.rdbuf();
    png.close();

    // Stuff
    request << Web::sCRLF;
    request << Web::sDivider;
    request << Web::sBoundary;
    request << Web::sDivider;
    request << Web::sCRLF;

    return request.str();
}

bool UploadFile(wchar_t const* fileName)
{
    std::string const request = BuildRequest(fileName);

    // WinInet preparation
    InternetHandle hSession(InternetOpenW(
        Windows::sTitle,
        INTERNET_OPEN_TYPE_PRECONFIG,
        NULL,
        NULL,
        0));
    if (!hSession)
    {
        ReportError();
    }

    // Access point
    InternetHandle hConnection(InternetConnectW(
        hSession,
        Web::UPLOAD_SERVER,
        INTERNET_DEFAULT_HTTP_PORT,
        NULL,
        NULL,
        INTERNET_SERVICE_HTTP,
        0,
        NULL));
    if (!hConnection)
    {
        ReportError();
    }

    // Full set requirements before
    InternetHandle hRequest(HttpOpenRequestW(
        hConnection,
        Web::POST,
        Web::UPLOAD_PATH,
        NULL,
        NULL,
        NULL,
        INTERNET_FLAG_DONT_CACHE
        | INTERNET_FLAG_RELOAD,
        NULL));
    if (!hRequest)
    {
        ReportError();
    }

    // User Agent
    BOOL bResult = ::HttpAddRequestHeadersW(
        hRequest,
        Web::USER_AGENT,
        (DWORD)wcslen(Web::USER_AGENT),
        HTTP_ADDREQ_FLAG_ADD
        | HTTP_ADDREQ_FLAG_REPLACE);
    if (bResult == FALSE)
    {
        ReportError();
    }

    bResult = ::HttpSendRequestW(
        hRequest,
        Web::HEADER,
        (DWORD)wcslen(Web::HEADER),
        (LPVOID)request.c_str(),
        (DWORD)request.size());
    if (bResult == FALSE)
    {
        ReportError();
    }

    DWORD bufferLength = 100;
    wchar_t buffer[101] = {};

    // state code
    bResult = ::HttpQueryInfoW(
        hRequest,
        HTTP_QUERY_STATUS_CODE,
        buffer,
        &bufferLength,
        0);
    if (bResult == FALSE ||
        WstringToInt(std::wstring(buffer)) != 200)
    {
        ReportError();
    }

    // get new id
    wcscpy_s(buffer, Web::ID);
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
    std::wstring srcUrl;

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
std::string GetId()
{
    // load ID from the file
    std::string id;
    std::ifstream ifs(GetIdFilePath());
    if (!ifs.fail())
    {
        // read the ID
        ifs >> id;
        ifs.close();
    }
    else
    {
        // (Compatibility with older versions) read the ID from the same directory
        std::wstring idFile = Web::ID_FILENAME;

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
bool SaveId(std::wstring const& sId)
{
    // save the ID to file
    ::CreateDirectoryW(GetIdDirPath().c_str(), NULL);

    std::wofstream ofs;
    ofs.open(GetIdFilePath());
    if (!ofs.fail())
    {
        ofs << sId;
        ofs.close();

        // Delete the old configuration file
        if (::PathFileExistsW(Web::ID_FILENAME))
        {
            ::DeleteFileW(Web::ID_FILENAME);
        }
    }
    else
    {
        return false;
    }

    return true;
}

int ErrorMessage(std::wstring const& text)
{
    return ::MessageBoxW(NULL, text.c_str(), Windows::sTitle, MB_ICONERROR | MB_OK);
}

std::wstring GetIdDirPath()
{
    static wchar_t idDir[MAX_PATH] = {};

    if (!wcslen(idDir))
    {
        ::SHGetSpecialFolderPathW(
            NULL,
            idDir,
            CSIDL_APPDATA,
            FALSE);
        wcscat_s(idDir, Web::DIRNAME);
    }

    return idDir;
}

std::wstring GetIdFilePath()
{
    static std::wstring idFile;

    if (!idFile.length())
    {
        idFile = GetIdDirPath() + Web::ID_FILEPATH;
    }

    return idFile;
}

} // namespace Gyazo
