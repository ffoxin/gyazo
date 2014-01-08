#include "rect.h"

BaseRect::BaseRect()
: left(rect.left)
, top(rect.top)
, right(rect.right)
, bottom(rect.bottom)
{
    // empty
}

BaseRect& BaseRect::operator=(const BaseRect& baseRect)
{
    rect = baseRect.rect;
}

void BaseRect::Init(const LONG& left_, const LONG& top_, const LONG& right_, const LONG& bottom_)
{
    left = left_;
    top = top_;
    right = right_;
    bottom = bottom_;
}

//----------------------------------

GyazoRect::GyazoRect()
{
    Init(0, 0, 0, 0);
}

GyazoRect::GyazoRect(const LONG& left_, const LONG& top_, const LONG& right_, const LONG& bottom_)
{
    Init(left_, top_, right_, bottom_);
}

GyazoRect::GyazoRect(const RECT& rect_)
{
    rect = rect_;
}

GyazoRect::GyazoRect(const GyazoRect& rect_)
{
    rect = rect_.rect;
}

GyazoRect::~GyazoRect()
{
    // empty
}

GyazoRect& GyazoRect::operator=(const RECT& rect_)
{
    if (&rect != &rect_)
    {
        rect = rect_;
    }

    return *this;
}

GyazoRect& GyazoRect::operator=(const GyazoRect& rect_)
{
    return operator=(rect_.rect);
}

GyazoRect::operator LPRECT()
{
    return &rect;
}

GyazoRect::operator GyazoSize() const
{
    return GyazoSize(right - left, bottom - top);
}
