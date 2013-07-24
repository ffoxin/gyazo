#include "util.h"

//////////////////////////////////////////////////////////////////////////

BaseSize::BaseSize()
    : cx(size.cx)
    , cy(size.cy)
{
    // empty
}

//////////////////////////////////////////////////////////////////////////

GyazoSize::GyazoSize()
{
    Init(0, 0);
}

GyazoSize::GyazoSize(uint32_t x, uint32_t y)
{
    Init(x, y);
}

GyazoSize::GyazoSize(const GyazoSize& size)
{
    Copy(size);
}

GyazoSize::~GyazoSize()
{
    // empty
}

GyazoSize& GyazoSize::operator=(const GyazoSize& size)
{
    if (this != &size)
    {
        Copy(size);
    }

    return *this;
}

void GyazoSize::Init(uint32_t x, uint32_t y)
{
    cx = x;
    cy = y;
}

void GyazoSize::Copy(const GyazoSize& size)
{
    memcpy(&this->size, &size.size, sizeof(size.size));
}

//////////////////////////////////////////////////////////////////////////

BaseRect::BaseRect()
    : left(rect.left)
    , top(rect.top)
    , right(rect.right)
    , bottom(rect.bottom)
{
    // empty
}

//////////////////////////////////////////////////////////////////////////

GyazoRect::GyazoRect()
{
    Init(0, 0, 0, 0);
}

GyazoRect::GyazoRect(uint32_t left, uint32_t top, uint32_t right, uint32_t bottom)
{
    Init(left, top, right, bottom);
}

GyazoRect::GyazoRect(const GyazoRect& rect)
{
    Copy(rect);
}

GyazoRect::~GyazoRect()
{
    // empty
}

GyazoRect& GyazoRect::operator=(const GyazoRect& rect)
{
    if (this != &rect)
    {
        Copy(rect);
    }

    return *this;
}

void GyazoRect::Init(uint32_t left, uint32_t top, uint32_t right, uint32_t bottom)
{
    this->left = left;
    this->top = top;
    this->right = right;
    this->bottom = bottom;
}

void GyazoRect::Copy(const GyazoRect& rect)
{
    memcpy(&this->rect, &rect.rect, sizeof(rect.rect));
}

//////////////////////////////////////////////////////////////////////////
