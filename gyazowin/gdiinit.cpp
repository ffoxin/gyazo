#include "gdiinit.h"

GdiInit::GdiInit() {
    GdiplusStartup(&m_token, &m_startupInput, NULL);
}

GdiInit::~GdiInit() {
    GdiplusShutdown(m_token);
}
