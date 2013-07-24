#include "gdiinit.h"

GdiScopeInit::GdiScopeInit()
{
	GdiplusStartup(&m_token, &m_startupInput, NULL);
}

GdiScopeInit::~GdiScopeInit()
{
	GdiplusShutdown(m_token);
}
