#ifndef SIZE_H
#define SIZE_H

#include <windows.h>

class BaseSize
{
public:
    BaseSize();
    BaseSize& operator=(const BaseSize& baseSize);

protected:
    void Init(const LONG& cx_, const LONG& cy_);

public:
    SIZE    size;
    // references to related fields of size
    LONG&   cx;
    LONG&   cy;
};

class GyazoSize : public BaseSize
{
public:
    GyazoSize();
    GyazoSize(const LONG& x_, const LONG& y_);
    GyazoSize(const SIZE& size_);
    GyazoSize(const GyazoSize& size_);
    ~GyazoSize();

    GyazoSize& operator=(const SIZE& size_);
    GyazoSize& operator=(const GyazoSize& size_);

    GyazoSize& operator+=(const GyazoSize& size_);
    GyazoSize& operator-=(const GyazoSize& size_);

    operator LPSIZE();

    friend GyazoSize operator+(const GyazoSize& lhr, const GyazoSize& rhr);
    friend GyazoSize operator-(const GyazoSize& lhr, const GyazoSize& rhr);
};

GyazoSize operator+(const GyazoSize& lhr, const GyazoSize& rhr);
GyazoSize operator-(const GyazoSize& lhr, const GyazoSize& rhr);

#endif // SIZE_H
