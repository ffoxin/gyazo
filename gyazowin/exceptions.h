#ifndef EXCEPTIONS_H
#define EXCEPTIONS_H

#include <exception>

#include "types.h"

class IExTemplate
{
public:
    virtual wstring report() = 0;
};

class ExRegisterClass : public IExTemplate
{
public:
    ExRegisterClass(const wstring& className);

    wstring report();

private:
    const wstring& m_className;
};

class ExFailedUpload : public IExTemplate
{
public:
    wstring report();
};

class ExOpenPng : public IExTemplate
{
public:
    wstring report();
};

class ExWininetConfigure : public IExTemplate
{
public:
    wstring report();
};

class ExConnectionInit : public IExTemplate
{
public:
    wstring report();
};

class ExComposePost : public IExTemplate
{
public:
    wstring report();
};

class ExSetUserAgent : public IExTemplate
{
public:
    wstring report();
};

class ExUploadImage : public IExTemplate
{
public:
    wstring report();
};

#endif // EXCEPTIONS_H
