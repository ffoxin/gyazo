#ifndef GDIINIT_H
#define GDIINIT_H

// System headers
#include <windows.h>
#include <gdiplus.h>
using namespace Gdiplus;

class GdiInit {
public:
    GdiInit();
    ~GdiInit();

private:
    GdiplusStartupInput     m_startupInput;
    ULONG_PTR               m_token;
};

#endif // GDIINIT_H
