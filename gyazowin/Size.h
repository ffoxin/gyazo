#pragma once

#include <windows.h>

namespace Gyazo
{

class BaseSize
{
public:
    BaseSize();
    BaseSize(BaseSize const&) = delete;
    BaseSize& operator=(BaseSize const& baseSize);

protected:
    void Init(LONG const& cx_, LONG const& cy_);

public:
    SIZE    size;
    // references to related fields of size
    LONG&   cx;
    LONG&   cy;
};

class Size : public BaseSize
{
public:
    Size();
    Size(LONG const& x_, LONG const& y_);
    explicit Size(SIZE const& other);
    Size(Size const& other);
    ~Size();

    Size& operator=(SIZE const& other);
    Size& operator=(Size const& other);

    Size& operator+=(Size const& other);
    Size& operator-=(Size const& other);

    operator LPSIZE();

    friend Size operator+(Size const& lhr, Size const& rhr);
    friend Size operator-(Size const& lhr, Size const& rhr);
};

Size operator+(Size const& lhr, Size const& rhr);
Size operator-(Size const& lhr, Size const& rhr);

} // namespace Gyazo
