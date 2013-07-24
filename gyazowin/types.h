#ifndef TYPES_H
#define TYPES_H

// STL headers
#include <string>
#include <sstream>
#include <fstream>

#ifndef _HAS_CPP0X
#  define DELETE_METHOD ;
#else // _HAS_CPP0X
#  define DELETE_METHOD = delete
#endif // _HAS_CPP0X

namespace Gyazo
{

#if defined (_MSC_VER )
    typedef signed char          int8_t;
    typedef signed short         int16_t;
    typedef signed int           int32_t;
    typedef signed __int64       int64_t;
    typedef unsigned char        uint8_t;
    typedef unsigned short       uint16_t;
    typedef unsigned int         uint32_t;
    typedef unsigned __int64     uint64_t;
#elif defined (LINUX) || defined(LINUX_ARM)
    typedef signed char          int8_t;
    typedef signed short         int16_t;
    typedef signed int           int32_t;
    typedef signed long long     int64_t;
    typedef unsigned char        uint8_t;
    typedef unsigned short       uint16_t;
    typedef unsigned int         uint32_t;
    typedef unsigned long long   uint64_t;
#else
#include <stdint.h>
#endif

#ifndef UNICODE
    typedef char				Char_t, *PChar_t;
    typedef std::string			String;

    typedef std::stringstream	stringstream;
    typedef std::ostringstream	ostringstream;
    typedef std::istringstream	istringstream;

    typedef std::ofstream		ofstream;
    typedef std::ifstream		ifstream;
    typedef std::fstream		fstream;
#else // UNICODE
    typedef wchar_t			    Char_t, *PChar_t;
    typedef std::wstring	    byte_string;

    typedef std::wstringstream	stringstream;
    typedef std::wostringstream	ostringstream;
    typedef std::wistringstream	istringstream;

    typedef std::wofstream		ofstream;
    typedef std::wifstream		ifstream;
    typedef std::wfstream		fstream;
#endif // UNICODE

    // byte string
    typedef std::string		byte_string;
    // Unicode string
    typedef std::wstring	wide_string;

    // Unicode streams
    typedef std::wofstream	wide_ofstream;
    typedef std::wifstream	wide_ifstream;
    typedef std::wfstream	wide_fstream;

    // byte streams
    typedef std::ofstream	byte_ofstream;
    typedef std::ifstream	byte_ifstream;
    typedef std::fstream	byte_fstream;

    typedef struct size_s
    {
        uint32_t cx;
        uint32_t cy;
    } Size_t, *PSize_t;

    typedef struct rect_s
    {
        uint32_t left;
        uint32_t top;
        uint32_t right;
        uint32_t bottom;
    } Rect_t, *PRect_t;

} // namespace Gyazo

#endif // TYPES_H
