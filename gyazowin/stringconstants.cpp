#include "stringconstants.h"

const wchar_t* const sTitle = L"Gyazo";                  // Text in the title bar
const wchar_t* const sWindowMainClass = L"GYAZOWINM";    // Main window class name
const wchar_t* const sWindowLayerClass = L"GYAZOWINL";   // Layer window class name
const wchar_t* const sWindowCursorClass = L"GYAZOWINC";  // Cursor window class name

const wchar_t* const GYAZO_PREFIX = L"gya";
const wchar_t* const GYAZO_FONT_NAME = L"Tahoma";
const wchar_t* const GYAZO_POINT_FORMAT = L"%d:%d";
const wchar_t* const GYAZO_READ_BINARY = L"rb";
const wchar_t* const GYAZO_URL_OPEN = L"open";
const wchar_t* const GYAZO_UPLOAD_SERVER = L"gyazo.com";
const wchar_t* const GYAZO_UPLOAD_PATH = L"/upload.cgi";
const wchar_t* const GYAZO_USER_AGENT = L"User-Agent: Gyazowin/1.0\r\n";
const wchar_t* const GYAZO_ID = L"X-Gyazo-Id";
const wchar_t* const GYAZO_HEADER = L"Content-type: multipart/form-data; boundary=----BOUNDARYBOUNDARY----";
const wchar_t* const GYAZO_ID_FILENAME = L"id.txt";
const wchar_t* const GYAZO_DIRNAME = L"\\Gyazo";
const wchar_t* const GYAZO_ID_FILEPATH = L"\\id.txt";
const wchar_t* const GYAZO_POST = L"POST";
const wchar_t* const GYAZO_URL_DIVIDER = L"/";

const wchar_t* const ERROR_CONVERT_IMAGE = L"Cannot convert this image";

const char* const sBoundary = "----BOUNDARYBOUNDARY----";  // boundary
const char* const sDivider = "--";
const char* const sContentId = "content-disposition: form-data; name=\"id\"";
const char* const sContentData = "content-disposition: form-data; name=\"imagedata\"); "
"filename=\"gyazo.com\"";

const char sCRLF[] = { 0xd, 0xa, 0x0 };                  // Diverted (CR + LF)
