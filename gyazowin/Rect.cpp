#include "rect.h"

namespace Gyazo
{

BaseRect::BaseRect()
    : left(rect.left)
    , top(rect.top)
    , right(rect.right)
    , bottom(rect.bottom)
{
    // empty
}

BaseRect& BaseRect::operator=(BaseRect const& baseRect)
{
    if (this != &baseRect)
    {
        rect = baseRect.rect;
    }

    return *this;
}

void BaseRect::Init(LONG const& left_, LONG const& top_, LONG const& right_, LONG const& bottom_)
{
    left = left_;
    top = top_;
    right = right_;
    bottom = bottom_;
}

//----------------------------------

Rect::Rect()
{
    Init(0, 0, 0, 0);
}

Rect::Rect(LONG const& left_, LONG const& top_, LONG const& right_, LONG const& bottom_)
{
    Init(left_, top_, right_, bottom_);
}

Rect::Rect(RECT const& other)
{
    rect = other;
}

Rect::Rect(Rect const& other)
{
    rect = other.rect;
}

Rect::Rect(LPRECT other)
{
    rect = *other;
}

Rect::~Rect()
{
    // empty
}

Rect& Rect::operator=(RECT const& rect_)
{
    if (&rect != &rect_)
    {
        rect = rect_;
    }

    return *this;
}

Rect& Rect::operator=(Rect const& rect_)
{
    return operator=(rect_.rect);
}

Rect::operator LPRECT()
{
    return &rect;
}

Rect::operator Size() const
{
    return Size(right - left, bottom - top);
}

} // namespace Gyazo
