#include "stringconstants.h"

namespace Gyazo
{

LPCTSTR szTitle					= _T("Gyazo");		// Text in the title bar
LPCTSTR szWindowMainClass		= _T("GYAZOWINM");	// Main window class name
LPCTSTR szWindowLayerClass		= _T("GYAZOWINL");	// Layer window class name
LPCTSTR szWindowCursorClass		= _T("GYAZOWINC");	// Cursor window class name

LPCTSTR GYAZO_PREFIX			= _T("gya");
LPCTSTR GYAZO_FONT_NAME			= _T("Tahoma");
LPCTSTR GYAZO_POINT_FORMAT		= _T("%d:%d");
LPCTSTR GYAZO_READ_BINARY		= _T("rb");
LPCTSTR GYAZO_URL_OPEN			= _T("open");
LPCTSTR GYAZO_UPLOAD_SERVER		= _T("gyazo.com");
LPCTSTR GYAZO_UPLOAD_PATH		= _T("/upload.cgi");
LPCTSTR GYAZO_SHARE_PATH		= _T("http://ffoxin.github.io/gyazo.htm?id=");
LPCTSTR GYAZO_USER_AGENT		= _T("User-Agent: Gyazowin/1.0\r\n");
LPCTSTR GYAZO_ID				= _T("X-Gyazo-Id");
LPCTSTR GYAZO_HEADER			= _T("Content-type: multipart/form-data; boundary=----BOUNDARYBOUNDARY----");
LPCTSTR GYAZO_ID_FILENAME		= _T("id.txt");
LPCTSTR GYAZO_DIRNAME			= _T("\\Gyazo");
LPCTSTR GYAZO_ID_FILEPATH		= _T("\\id.txt");
LPCTSTR GYAZO_POST				= _T("POST");
LPCTSTR GYAZO_URL_DIVIDER		= _T("/");


LPCTSTR ERROR_CONVERT_IMAGE		= _T("Cannot convert this image");
LPCTSTR ERROR_OPEN_PNG			= _T("PNG open failed");
LPCTSTR ERROR_WININET_CONFIGURE	= _T("Cannot configure wininet");
LPCTSTR ERROR_CONNECTION_INIT	= _T("Cannot initiate connection");
LPCTSTR ERROR_COMPOSE_POST		= _T("Cannot compose post request");
LPCTSTR ERROR_SET_USER_AGENT	= _T("Cannot set user agent");
LPCTSTR ERROR_UPLOAD_IMAGE		= _T("Failed to upload (unexpected result code, under maintainance?)");
LPCTSTR ERROR_UPLOAD_FAILED		= _T("Failed to upload");

LPCSTR sBoundary				= "----BOUNDARYBOUNDARY----";		// boundary
LPCSTR sDivider					= "--";
LPCSTR sContentId				= "content-disposition: form-data; name=\"id\"";
LPCSTR sContentData				= "content-disposition: form-data; name=\"imagedata\"; filename=\"gyazo.com\"";

const char sCrLf[]				= { 0xd, 0xa, 0x0 };				// Diverted (CR + LF)

} // namespace Gyazo
