#pragma once

#include "size.h"

#include <windows.h>

namespace Gyazo
{

class BaseRect
{
public:
    BaseRect();
    BaseRect(BaseRect const&) = delete;
    BaseRect& operator=(BaseRect const& baseRect);

protected:
    void Init(LONG const& left_, LONG const& top_, LONG const& right_, LONG const& bottom_);

public:
    RECT    rect;
    // references to related fields of rect
    LONG&   left;
    LONG&   top;
    LONG&   right;
    LONG&   bottom;
};

class Rect : public BaseRect
{
public:
    Rect();
    Rect(LONG const& left_, LONG const& top_, LONG const& right_, LONG const& bottom_);
    explicit Rect(RECT const& other);
    explicit Rect(LPRECT other);
    Rect(Rect const& other);
    ~Rect();

    Rect& operator=(RECT const& other);
    Rect& operator=(Rect const& other);

    operator LPRECT();
    operator Size() const;
};

} // namespace Gyazo
