#ifndef GDIINIT_H
#define GDIINIT_H

// System headers
#include <gdiplus.h>
using namespace Gdiplus;

class GdiScopeInit
{
public:
	GdiScopeInit()
	{
		GdiplusStartup(&m_gdiplusToken, &m_gdiplusStartupInput, NULL);
	}
	~GdiScopeInit()
	{
		GdiplusShutdown(m_gdiplusToken);
	}

private:
	GdiplusStartupInput	m_gdiplusStartupInput;
	ULONG_PTR			m_gdiplusToken;
};

#endif // GDIINIT_H
