#include "size.h"

//----------------------------------

BaseSize::BaseSize()
    : cx(size.cx)
    , cy(size.cy) {
    // empty
}

void BaseSize::Init(const LONG& cx_, const LONG& cy_) {
    cx = cx_;
    cy = cy_;
}

//----------------------------------

GyazoSize::GyazoSize() {
    Init(0, 0);
}

GyazoSize::GyazoSize(const LONG& x_, const LONG& y_) {
    Init(x_, y_);
}

GyazoSize::GyazoSize(const SIZE& size_) {
    size = size_;
}

GyazoSize::GyazoSize(const GyazoSize& size_) {
    size = size_.size;
}

GyazoSize::~GyazoSize() {
    // empty
}

GyazoSize& GyazoSize::operator=(const SIZE& size_) {
    if (&size != &size_) {
        size = size_;
    }

    return *this;
}

GyazoSize& GyazoSize::operator=(const GyazoSize& size_) {
    return operator=(size_.size);
}

GyazoSize& GyazoSize::operator+=(const GyazoSize& size_) {
    cx += size_.cx;
    cy += size_.cy;

    return *this;
}

GyazoSize& GyazoSize::operator-=(const GyazoSize& size_) {
    cx -= size_.cx;
    cy -= size_.cy;

    return *this;
}

GyazoSize::operator LPSIZE() {
    return &size;
}

GyazoSize operator+(const GyazoSize& lhr, const GyazoSize& rhr) {
    GyazoSize temp(lhr);
    temp += rhr;
    return temp;
}

GyazoSize operator-(const GyazoSize& lhr, const GyazoSize& rhr) {
    GyazoSize temp(lhr);
    temp -= rhr;
    return temp;
}
