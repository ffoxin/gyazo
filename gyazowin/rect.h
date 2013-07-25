#ifndef RECT_H
#define RECT_H

// System headers
#include <windows.h>

// Project headers
#include "size.h"

class BaseRect {
public:
    BaseRect();

protected:
    void Init(const LONG& left_, const LONG& top_, const LONG& right_, const LONG& bottom_);

public:
    RECT    rect;
    // references to related fields of rect
    LONG&   left;
    LONG&   top;
    LONG&   right;
    LONG&   bottom;
};

class GyazoRect : public BaseRect {
public:
    GyazoRect();
    GyazoRect(const LONG& left_, const LONG& top_, const LONG& right_, const LONG& bottom_);
    GyazoRect(const RECT& rect_);
    GyazoRect(const GyazoRect& rect_);
    ~GyazoRect();

    GyazoRect& operator=(const RECT& rect_);
    GyazoRect& operator=(const GyazoRect& rect_);

    operator LPRECT();
    operator GyazoSize() const;
};

#endif // RECT_H
