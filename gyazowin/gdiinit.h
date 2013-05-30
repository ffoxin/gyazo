#ifndef GDIINIT_H
#define GDIINIT_H

#include <gdiplus.h>
using namespace Gdiplus;

class GdiPlusInit
{
public:
	GdiPlusInit()
	{
		GdiplusStartup(&m_gdiplusToken, &m_gdiplusStartupInput, NULL);
	}
	~GdiPlusInit()
	{
		GdiplusShutdown(m_gdiplusToken);
	}

private:
	GdiplusStartupInput	m_gdiplusStartupInput;
	ULONG_PTR			m_gdiplusToken;
};

#endif // GDIINIT_H
