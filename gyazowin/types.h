#ifndef TYPES_H
#define TYPES_H

// STL headers
#include <string>

// Project headers
#include "rect.h"
#include "size.h"

typedef signed char         int8_t;
typedef signed short        int16_t;
typedef signed int          int32_t;
typedef signed __int64      int64_t;
typedef unsigned char       uint8_t;
typedef unsigned short      uint16_t;
typedef unsigned int        uint32_t;
typedef unsigned __int64    uint64_t;

typedef std::string         cstring;
typedef std::wstring        wstring;

typedef wstring             string;
typedef string::value_type  char_type;

#define Text(x) (L ## x)

#endif // TYPES_H
