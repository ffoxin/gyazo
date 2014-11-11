#include "stringconstants.h"

namespace Gyazo
{

namespace Common
{

wchar_t const* const PREFIX = L"gya";
wchar_t const* const READ_BINARY = L"rb";

} // namespace Common

namespace Ui
{

wchar_t const* const FONT_NAME = L"Tahoma";
wchar_t const* const POINT_FORMAT = L"%d:%d";

} // namespace Ui

namespace Windows
{

wchar_t const* const sTitle = L"Gyazo";  // Text in the title bar
wchar_t const* const sWindowMainClass = L"GYAZO_WIN_MAIN";  // Main window class name
wchar_t const* const sWindowLayerClass = L"GYAZO_WIN_LAYER";  // Layer window class name
wchar_t const* const sWindowCursorClass = L"GYAZO_WIN_CURSOR";  // Cursor window class name
wchar_t const* const ERROR_CONVERT_IMAGE = L"Cannot convert this image";

} // namespace Windows

namespace Web
{

wchar_t const* const URL_OPEN = L"open";
wchar_t const* const UPLOAD_SERVER = L"gyazo.com";
wchar_t const* const UPLOAD_PATH = L"/upload.cgi";
wchar_t const* const USER_AGENT = L"User-Agent: Gyazowin/1.0\r\n";
wchar_t const* const ID = L"X-Gyazo-Id";
wchar_t const* const HEADER = L"Content-type: multipart/form-data; boundary=----BOUNDARYBOUNDARY----";
wchar_t const* const ID_FILENAME = L"id.txt";
wchar_t const* const DIRNAME = L"\\Gyazo";
wchar_t const* const ID_FILEPATH = L"\\id.txt";
wchar_t const* const POST = L"POST";
wchar_t const* const URL_DIVIDER = L"/";

char const* const sBoundary = "----BOUNDARYBOUNDARY----";  // boundary
char const* const sDivider = "--";
char const* const sContentId = "content-disposition: form-data; name=\"id\"";
char const* const sContentData = "content-disposition: form-data; name=\"imagedata\"); filename=\"gyazo.com\"";

char const sCRLF[] = { 0xd, 0xa, 0x0 };  // Diverted (CR + LF)

} // namespace Web

} // namespace Gyazo
