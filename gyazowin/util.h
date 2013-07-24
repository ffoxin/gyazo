#ifndef UTIL_H
#define UTIL_H

// Project headers
#include "types.h"

class BaseSize
{
public:
    BaseSize();

public:
    Size_t size;
    // references to related fields of size
    uint32_t&	cx;
    uint32_t&	cy;
};

class GyazoSize : public BaseSize
{
public:
    GyazoSize();
    GyazoSize(uint32_t x, uint32_t y);
    GyazoSize(const GyazoSize& size);
    ~GyazoSize();

    GyazoSize& operator=(const GyazoSize& size);

private:
    void Init(uint32_t x, uint32_t y);
    void Copy(const GyazoSize& size);
};

class BaseRect
{
public:
    BaseRect();
public:
    RECT		rect;
    // references to related fields of rect
    uint32_t&	left;
    uint32_t&	top;
    uint32_t&	right;
    uint32_t&	bottom;
};

class GyazoRect : public BaseRect
{
public:
    GyazoRect();
    GyazoRect(uint32_t left, uint32_t top, uint32_t right, uint32_t bottom);
    GyazoRect(const GyazoRect& rect);
    ~GyazoRect();

    GyazoRect& operator=(const GyazoRect& rect);

private:
    void Init(uint32_t left, uint32_t top, uint32_t right, uint32_t bottom);
    void Copy(const GyazoRect& rect);
};

#endif // UTIL_H
