#ifndef UTIL_H
#define UTIL_H

#include <windows.h>
#include <tchar.h>

#include <string>
#include <sstream>

namespace Gyazo
{
#ifdef UNICODE
	typedef std::wstring string;
	typedef std::wstringstream stringstream;
#else
	typedef std::string string;
	typedef std::stringstream stringstream;
#endif

}

#endif // UTIL_H
