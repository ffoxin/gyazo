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

typedef std::wstring string;
typedef wchar_t char_type;

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

#endif // TYPES_H
