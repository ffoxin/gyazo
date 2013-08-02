// System headers
#include <ShlObj.h>
#include <Shlwapi.h>
#include <WinInet.h>

// STL headers
#include <fstream>
#include <memory>
#include <sstream>
#include <string>

// Project headers
#include "gdiinit.h"
#include "stringconstants.h"

#include "gyazolib.h"

// I get the CLSID of the Encoder corresponding to the specified format
// Cited from MSDN Library: Retrieving the Class Identifier for an Encoder
bool GetEncoderClsid(const string& format, CLSID& clsid) {
    size_t num = 0;     // number of image encoders
    size_t size = 0;    // size of the image encoder array in bytes

    GetImageEncodersSize(&num, &size);
    if (size != 0) {
        std::shared_ptr<char> imageCodecInfo(
            new char[size], 
            [](char* p){ delete[] p; } // deleter functor
        );
        ImageCodecInfo* pImageCodecInfo = reinterpret_cast<ImageCodecInfo*>(imageCodecInfo.get());

        GetImageEncoders(num, size, pImageCodecInfo);

        for (size_t i = 0; i < num; ++i) {
            if (format == pImageCodecInfo[i].MimeType) {
                clsid = pImageCodecInfo[i].Clsid;
                return true;
            }
        }
    }

    return false;
}

// Look at the header whether PNG image check (once)
bool IsPngFile(const string& fileName) {
    const uint8_t pngHeader[] = { 0x89, 0x50, 0x4E, 0x47, 0x0D, 0x0A, 0x1A, 0x0A };
    const size_t pngHeaderSize = sizeof(pngHeader) / sizeof(*pngHeader);

    uint8_t fileHeader[pngHeaderSize];

    std::ifstream ifs(fileName.c_str(), std::ios_base::in | std::ios_base::binary);
    if (!ifs.is_open()) {
        return false;
    }

    ifs.read(reinterpret_cast<char *>(fileHeader), pngHeaderSize);
    if (!ifs.good()) {
        return false;
    }

    // compare
    if (memcmp(pngHeader, fileHeader, sizeof(pngHeader)) != 0) {
        return false;
    }

    return true;
}

// I open a browser (char *) URL that is specified
void ExecUrl(const string& url) {
    // Run the open command
    SHELLEXECUTEINFO lsw = {};
    lsw.cbSize = sizeof(SHELLEXECUTEINFO);
    lsw.lpVerb = GYAZO_URL_OPEN;
    lsw.lpFile = url.c_str();

    ShellExecuteExW(&lsw);
}

// Copy text to clipboard
void SetClipBoardText(const string& text) {
    size_t slen = text.size() + 1;

    HGLOBAL hText = GlobalAlloc(
        GMEM_DDESHARE | GMEM_MOVEABLE, 
        slen * sizeof(char_type)
        );

    char_type* pText = static_cast<char_type *>(GlobalLock(hText));
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
bool ImageToPng(Image* image, const string& fileName) {
    if (image->GetLastStatus() == 0) {
        CLSID clsidEncoder;
        if (GetEncoderClsid(L"image/png", clsidEncoder)) {
            if (image->Save(fileName.c_str(), &clsidEncoder, 0) == 0) {
                return true;
            }
        }
    }

    return false;
}

// Convert to PNG format
bool FileToPng(const string& destFile, const string& srcFile) {
    const GdiInit gpi = GdiInit();

    Image image(srcFile.c_str(), 0);

    return ImageToPng(&image, destFile);
}

// save BITMAP to PNG file
bool BitmapToPng(HBITMAP hBmp, const string& fileName) {
    const GdiInit gpi = GdiInit();

    Bitmap bitmap(hBmp, NULL);

    return ImageToPng(&bitmap, fileName);
}

bool UploadFile(const string& fileName) {
    std::ostringstream buf; // Outgoing messages

    // Get an ID
    string tidStr = GetId();
    std::string idStr(tidStr.begin(), tidStr.end());

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

    // Read a PNG file
    std::ifstream png;
    png.open(fileName, std::ios::binary);
    if (png.fail()) {
        png.close();
        ErrorMessage(ERROR_OPEN_PNG);
        return false;
    }
    buf << png.rdbuf();     // read all & append to buffer
    png.close();

    // Last
    buf << sCrLf;
    buf << sDivider;
    buf << sBoundary;
    buf << sDivider;
    buf << sCrLf;

    // Message completion
    std::string msg(buf.str());

    // WinInet preparation (proxy required Full setをはuse)
    HINTERNET hSession = InternetOpenW(
        sTitle, 
        INTERNET_OPEN_TYPE_PRECONFIG, 
        NULL, 
        NULL, 
        0);
    if (hSession == NULL) {
        ErrorMessage(ERROR_WININET_CONFIGURE);
        return false;
    }

    // Access point
    HINTERNET hConnection = InternetConnectW(
        hSession, 
        GYAZO_UPLOAD_SERVER, 
        INTERNET_DEFAULT_HTTP_PORT,
        NULL, 
        NULL, 
        INTERNET_SERVICE_HTTP, 
        0, 
        NULL);
    if (hSession == NULL) {
        ErrorMessage(ERROR_CONNECTION_INIT);
        return false;
    }

    // Full set requirements before
    HINTERNET hRequest = HttpOpenRequestW(
        hConnection,
        GYAZO_POST, 
        GYAZO_UPLOAD_PATH, 
        NULL, 
        NULL, 
        NULL, 
        INTERNET_FLAG_DONT_CACHE | INTERNET_FLAG_RELOAD, 
        NULL);
    if (hRequest == NULL) {
        ErrorMessage(ERROR_COMPOSE_POST);
        return false;
    }

    // User Agentを 指定
    BOOL bResult = HttpAddRequestHeadersW(
        hRequest, 
        GYAZO_USER_AGENT, 
        wcslen(GYAZO_USER_AGENT), 
        HTTP_ADDREQ_FLAG_ADD | HTTP_ADDREQ_FLAG_REPLACE);
    if (bResult == FALSE) {
        ErrorMessage(ERROR_SET_USER_AGENT);
        return false;
    }

    bResult = HttpSendRequestW(hRequest,
        GYAZO_HEADER,
        wcslen(GYAZO_HEADER),
        (LPVOID)msg.c_str(),
        (DWORD) msg.length());
    if (bResult == TRUE) {
        DWORD nResponse = 8;
        wchar_t response[8];

        // state code
        HttpQueryInfoW(
            hRequest, 
            HTTP_QUERY_STATUS_CODE, 
            response, 
            &nResponse, 
            0);
        if (std::stoi(response) != 200) {
            // upload error
            ErrorMessage(ERROR_UPLOAD_IMAGE);
        }
        else {
            // upload succeeded

            // get new id
            DWORD nId = 100;
            wchar_t id[100];

            wcscpy_s(id, GYAZO_ID);

            HttpQueryInfoW(
                hRequest, 
                HTTP_QUERY_CUSTOM, 
                id, 
                &nId, 
                0);
            if (GetLastError() != ERROR_HTTP_HEADER_NOT_FOUND 
                && nId != 0) {
                // Save new id
                SaveId(id);
            }

            // Read URL results
            DWORD nUrl;
            char url[1024];
            string srcUrl;

            // Never so long , but once well
            while (InternetReadFile(hRequest, url, 1024, &nUrl) == TRUE
                && nUrl != 0) {
                srcUrl.append(url, url + nUrl);
            }

            srcUrl.insert(srcUrl.find(Text("gyazo.com")), Text("cache."));
            srcUrl += Text(".png");

            // Copy the URL to the clipboard
            SetClipBoardText(srcUrl);

            // Launch an URL
            ExecUrl(srcUrl);

            return true;
        }
    }
    else {
        // Upload failed
        ErrorMessage(ERROR_UPLOAD_FAILED);
        return false;
    }

    return false;
}

// I generate load the ID
string GetId() {
    // load ID from the file
    std::wifstream ifs;
    string sId;

    ifs.open(GetIdFilePath());
    if (!ifs.fail()) {
        // read the ID
        ifs >> sId;
        ifs.close();
    }
    else {
        // (Compatibility with older versions) read the ID from the same directory
        string idFile = GYAZO_ID_FILENAME;

        ifs.close();
        ifs.open(idFile);
        if (!ifs.fail()) {
            ifs >> sId;
            ifs.close();
        }
    }

    return sId;
}

// Save ID
bool SaveId(const string& sId) {
    // save the ID to file
    CreateDirectoryW(GetIdDirPath().c_str(), NULL);

    std::wofstream ofs;
    ofs.open(GetIdFilePath());
    if (!ofs.fail()) {
        ofs << sId;
        ofs.close();

        // Delete the old configuration file

        if (PathFileExistsW(GYAZO_ID_FILENAME)) {
            DeleteFileW(GYAZO_ID_FILENAME);
        }
    }
    else {
        return false;
    }

    return true;
}

int ErrorMessage(const string& text) {
    return MessageBoxW(NULL, text.c_str(), sTitle, MB_ICONERROR | MB_OK);
}

string GetIdDirPath() {
    static wchar_t idDir[MAX_PATH] = {};

    if (*idDir == wchar_t(0)) {
        SHGetSpecialFolderPathW(
            NULL, 
            idDir, 
            CSIDL_APPDATA, 
            FALSE);
        wcscat_s(idDir, GYAZO_DIRNAME);
    }

    return idDir;
}

string GetIdFilePath() {
    static string idFile;

    if (!idFile.length()) {
        idFile = GetIdDirPath() + GYAZO_ID_FILEPATH;
    }

    return idFile;
}
