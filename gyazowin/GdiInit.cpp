#include "gdiinit.h"

namespace Gyazo
{

GdiInit::GdiInit()
{
    Gdiplus::GdiplusStartup(&m_token, &m_startupInput, NULL);
}

GdiInit::~GdiInit()
{
    Gdiplus::GdiplusShutdown(m_token);
}

} // namespace Gyazo
