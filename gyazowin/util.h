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
	typedef std::wstring		string;

	typedef std::wstringstream	stringstream;
	typedef std::wostringstream	ostringstream;
	typedef std::wistringstream	istringstream;

	typedef std::wofstream		ofstream;
	typedef std::wifstream		ifstream;
#else
	typedef std::string			string;

	typedef std::stringstream	stringstream;
	typedef std::ostringstream	ostringstream;
	typedef std::istringstream	istringstream;

	typedef std::ofstream		ofstream;
	typedef std::ifstream		ifstream;
#endif

	template<typename T>
	void swap(T& lr, T& rr)
	{
		lr ^= rr ^= lr ^= rr;
	}
}

#endif // UTIL_H
