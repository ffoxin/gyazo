#include "stringconstants.h"

const char_type* const sTitle                   = Text("Gyazo");        // Text in the title bar
const char_type* const sWindowMainClass         = Text("GYAZOWINM");    // Main window class name
const char_type* const sWindowLayerClass        = Text("GYAZOWINL");    // Layer window class name
const char_type* const sWindowCursorClass       = Text("GYAZOWINC");    // Cursor window class name

const char_type* const GYAZO_PREFIX             = Text("gya");
const char_type* const GYAZO_FONT_NAME          = Text("Tahoma");
const char_type* const GYAZO_POINT_FORMAT       = Text("%d:%d");
const char_type* const GYAZO_READ_BINARY        = Text("rb");
const char_type* const GYAZO_URL_OPEN           = Text("open");
const char_type* const GYAZO_UPLOAD_SERVER      = Text("gyazo.com");
const char_type* const GYAZO_UPLOAD_PATH        = Text("/upload.cgi");
const char_type* const GYAZO_SHARE_PATH         = Text("http://ffoxin.github.io/gyazo.htm?id=");
const char_type* const GYAZO_USER_AGENT         = Text("User-Agent: Gyazowin/1.0\r\n");
const char_type* const GYAZO_ID                 = Text("X-Gyazo-Id");
const char_type* const GYAZO_HEADER             = Text("Content-type: multipart/form-data; boundary=----BOUNDARYBOUNDARY----");
const char_type* const GYAZO_ID_FILENAME        = Text("id.txt");
const char_type* const GYAZO_DIRNAME            = Text("\\Gyazo");
const char_type* const GYAZO_ID_FILEPATH        = Text("\\id.txt");
const char_type* const GYAZO_POST               = Text("POST");
const char_type* const GYAZO_URL_DIVIDER        = Text("/");

const char_type* const ERROR_CONVERT_IMAGE      = Text("Cannot convert this image");
const char_type* const ERROR_OPEN_PNG           = Text("PNG open failed");
const char_type* const ERROR_WININET_CONFIGURE  = Text("Cannot configure wininet");
const char_type* const ERROR_CONNECTION_INIT    = Text("Cannot initiate connection");
const char_type* const ERROR_COMPOSE_POST       = Text("Cannot compose post request");
const char_type* const ERROR_SET_USER_AGENT     = Text("Cannot set user agent");
const char_type* const ERROR_UPLOAD_IMAGE       = Text("Failed to upload (unexpected result code, under maintainance?)");
const char_type* const ERROR_UPLOAD_FAILED      = Text("Failed to upload");

const char* const sBoundary                     = "----BOUNDARYBOUNDARY----";       // boundary
const char* const sDivider                      = "--";
const char* const sContentId                    = "content-disposition: form-data; name=\"id\"";
const char* const sContentData                  = "content-disposition: form-data; name=\"imagedata\"); "
                                                  "filename=\"gyazo.com\"";

const char sCrLf[]                              = { 0xd, 0xa, 0x0 };                // Diverted (CR + LF)
