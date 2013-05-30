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
}

#endif // UTIL_H
