#ifndef UTIL_H
#define UTIL_H

// System headers
#include <windows.h>
#include <tchar.h>

// STL headers
#include <string>
#include <sstream>
#include <fstream>

namespace Gyazo
{

#ifdef UNICODE
typedef std::wstring		tstring;

typedef std::wstringstream	tstringstream;
typedef std::wostringstream	tostringstream;
typedef std::wistringstream	tistringstream;

typedef std::wofstream		tofstream;
typedef std::wifstream		tifstream;
#else
typedef std::string			tstring;

typedef std::stringstream	tstringstream;
typedef std::ostringstream	tostringstream;
typedef std::istringstream	tistringstream;

typedef std::ofstream		tofstream;
typedef std::ifstream		tifstream;
#endif

template<typename T>
void swap(T& lr, T& rr)
{
	lr ^= rr ^= lr ^= rr;
}

class Size
{
public:
	inline Size() :
		cx(size.cx), 
		cy(size.cy)
	{
		Init(0, 0);
	}

	inline Size(const LONG& x, const LONG& y) :
		cx(size.cx), 
		cy(size.cy)
	{
		Init(x, y);
	}

	inline Size(const Size& sz) :
		cx(size.cx), 
		cy(size.cy)
	{
		Init(sz.cx, sz.cy);
	}

	inline Size(const SIZE& sz) :
		cx(size.cx), 
		cy(size.cy)
	{
		Init(sz.cx, sz.cy);
	}

	inline void Set(const LONG& x, const LONG& y)
	{
		Init(x, y);
	}

	inline Size& operator=(const Size& sz)
	{
		if (this != &sz)
		{
			Init(sz.cx, sz.cy);
		}

		return *this;
	}

	inline Size& operator=(const SIZE& sz)
	{
		Init(sz.cx, sz.cy);

		return *this;
	}

	inline operator SIZE&()
	{
		return size;
	}

	inline operator SIZE*()
	{
		return &size;
	}

public:
	SIZE	size;
	LONG&	cx;
	LONG&	cy;

private:
	inline void Init(const LONG& x, const LONG& y)
	{
		size.cx = x;
		size.cy = y;
	}
};

class Rect
{
public:
	inline Rect() :
		left(rect.left), 
		top(rect.top), 
		right(rect.right), 
		bottom(rect.bottom)
	{
		Init(0, 0, 0, 0);
	}

	inline Rect(
		const LONG& newLeft, 
		const LONG& newTop, 
		const LONG& newRight, 
		const LONG& newBottom
		) :
		left(rect.left), 
		top(rect.top), 
		right(rect.right), 
		bottom(rect.bottom)
	{
		Init(newLeft, newTop, newRight, newBottom);
	}

	inline Rect(
		const Rect& rc
		) :
		left(rect.left), 
		top(rect.top), 
		right(rect.right), 
		bottom(rect.bottom)
	{
		Init(rc.left, rc.top, rc.right, rc.bottom);
	}

	inline Rect(
		const RECT& rc
		) :
		left(rect.left), 
		top(rect.top), 
		right(rect.right), 
		bottom(rect.bottom)
	{
		Init(rc.left, rc.top, rc.right, rc.bottom);
	}

	inline void Set(
		const LONG& newLeft, 
		const LONG& newTop, 
		const LONG& newRight, 
		const LONG& newBottom
		)
	{
		Init(newLeft, newTop, newRight, newBottom);
	}

	inline Rect& operator=(
		const Rect& rc
		)
	{
		if (this != &rc)
		{
			Init(rc.left, rc.top, rc.right, rc.bottom);
		}

		return *this;
	}

	inline Rect& operator=(
		const RECT& rc
		)
	{
		Init(rc.left, rc.top, rc.right, rc.bottom);

		return *this;
	}

	inline operator RECT&()
	{
		return rect;
	}

	inline operator RECT*()
	{
		return &rect;
	}

public:
	RECT	rect;
	LONG&	left;
	LONG&	top;
	LONG&	right;
	LONG&	bottom;

private:
	inline void Init(
		const LONG& newLeft, 
		const LONG& newTop, 
		const LONG& newRight, 
		const LONG& newBottom
		)
	{
		rect.left = newLeft;
		rect.top = newTop;
		rect.right = newRight;
		rect.bottom = newBottom;
	}
};

}

#endif // UTIL_H
