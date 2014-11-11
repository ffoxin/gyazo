#pragma once

// System headers
#include <windows.h>
#include <gdiplus.h>

namespace Gyazo
{

class GdiInit
{
public:
    GdiInit();
    ~GdiInit();

private:
    Gdiplus::GdiplusStartupInput     m_startupInput;
    ULONG_PTR               m_token;
};

} // namespace Gyazo
