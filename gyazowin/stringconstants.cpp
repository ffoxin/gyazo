#include "stringconstants.h"

const char_type* szTitle			    = L"Gyazo";		// Text in the title bar
const char_type* szWindowMainClass		= L"GYAZOWINM";	// Main window class name
const char_type* szWindowLayerClass		= L"GYAZOWINL";	// Layer window class name
const char_type* szWindowCursorClass	= L"GYAZOWINC";	// Cursor window class name

const char_type* GYAZO_PREFIX			= L"gya";
const char_type* GYAZO_FONT_NAME		= L"Tahoma";
const char_type* GYAZO_POINT_FORMAT		= L"%d:%d";
const char_type* GYAZO_READ_BINARY		= L"rb";
const char_type* GYAZO_URL_OPEN			= L"open";
const char_type* GYAZO_UPLOAD_SERVER	= L"gyazo.com";
const char_type* GYAZO_UPLOAD_PATH		= L"/upload.cgi";
const char_type* GYAZO_SHARE_PATH		= L"http://ffoxin.github.io/gyazo.htm?id=";
const char_type* GYAZO_USER_AGENT		= L"User-Agent: Gyazowin/1.0\r\n";
const char_type* GYAZO_ID				= L"X-Gyazo-Id";
const char_type* GYAZO_HEADER			= L"Content-type: multipart/form-data; boundary=----BOUNDARYBOUNDARY----";
const char_type* GYAZO_ID_FILENAME		= L"id.txt";
const char_type* GYAZO_DIRNAME			= L"\\Gyazo";
const char_type* GYAZO_ID_FILEPATH		= L"\\id.txt";
const char_type* GYAZO_POST				= L"POST";
const char_type* GYAZO_URL_DIVIDER		= L"/";


const char_type* ERROR_CONVERT_IMAGE	= L"Cannot convert this image";
const char_type* ERROR_OPEN_PNG			= L"PNG open failed";
const char_type* ERROR_WININET_CONFIGURE= L"Cannot configure wininet";
const char_type* ERROR_CONNECTION_INIT	= L"Cannot initiate connection";
const char_type* ERROR_COMPOSE_POST		= L"Cannot compose post request";
const char_type* ERROR_SET_USER_AGENT	= L"Cannot set user agent";
const char_type* ERROR_UPLOAD_IMAGE		= L"Failed to upload (unexpected result code, under maintainance?)";
const char_type* ERROR_UPLOAD_FAILED		= L"Failed to upload";

const char* sBoundary				    = "----BOUNDARYBOUNDARY----";		// boundary
const char* sDivider					= "--";
const char* sContentId				    = "content-disposition: form-data; name=\"id\"";
const char* sContentData				= "content-disposition: form-data; name=\"imagedata\"; filename=\"gyazo.com\"";

const char sCrLf[]				= { 0xd, 0xa, 0x0 };				// Diverted (CR + LF)
