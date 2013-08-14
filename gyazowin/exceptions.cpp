#include "stringconstants.h"

#include "exceptions.h"

//========================================================================

ExRegisterClass::ExRegisterClass(const wstring& className) :
    m_className(className)
{
    // empty
}

std::wstring ExRegisterClass::report()
{
    return ERROR_REGISTER_CLASS + m_className;
}

//========================================================================

std::wstring ExFailedUpload::report()
{
    return ERROR_UPLOAD_FAILED;
}

//========================================================================

std::wstring ExOpenPng::report()
{
    return ERROR_OPEN_PNG;
}

//========================================================================

std::wstring ExWininetConfigure::report()
{
    return ERROR_WININET_CONFIGURE;
}

//========================================================================

std::wstring ExConnectionInit::report()
{
    return ERROR_CONNECTION_INIT;
}

//========================================================================

std::wstring ExComposePost::report()
{
    return ERROR_COMPOSE_POST;
}

//========================================================================

std::wstring ExSetUserAgent::report()
{
    return ERROR_SET_USER_AGENT;
}

//========================================================================

std::wstring ExUploadImage::report()
{
    return ERROR_UPLOAD_IMAGE;
}
