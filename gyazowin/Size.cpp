#include "Size.h"

#include <utility>

namespace Gyazo
{

BaseSize::BaseSize()
    : cx(size.cx)
    , cy(size.cy)
{
    // empty
}

BaseSize& BaseSize::operator=(BaseSize const& baseSize)
{
    if (this != &baseSize)
    {
        size = baseSize.size;
    }

    return *this;
}

void BaseSize::Init(LONG const& cx_, LONG const& cy_)
{
    cx = cx_;
    cy = cy_;
}

//----------------------------------

Size::Size()
{
    Init(0, 0);
}

Size::Size(LONG const& x_, LONG const& y_)
{
    Init(x_, y_);
}

Size::Size(SIZE const& size_)
{
    size = size_;
}

Size::Size(Size const& size_)
{
    size = size_.size;
}

Size::~Size()
{
    // empty
}

Size& Size::operator=(SIZE const& size_)
{
    if (&size != &size_)
    {
        size = size_;
    }

    return *this;
}

Size& Size::operator=(Size const& size_)
{
    return operator=(size_.size);
}

Size& Size::operator+=(Size const& size_)
{
    cx += size_.cx;
    cy += size_.cy;

    return *this;
}

Size& Size::operator-=(Size const& size_)
{
    cx -= size_.cx;
    cy -= size_.cy;

    return *this;
}

Size::operator LPSIZE()
{
    return &size;
}

Size operator+(Size const& lhr, Size const& rhr)
{
    Size temp(lhr);
    temp += rhr;
    return temp;
}

Size operator-(Size const& lhr, Size const& rhr)
{
    Size temp(lhr);
    temp -= rhr;
    return temp;
}

} // namespace Gyazo
