#ifndef FONT_H
#define FONT_H

// System headers
#include <windows.h>

class GyazoFont {
public:
    static HFONT GetFont(int fontHeight);
    static void Release();

private:
    GyazoFont(int fontHeight);
    ~GyazoFont();

private:
    static GyazoFont* m_instance;
    HFONT m_font;
    int m_fontHeight;
};

#endif // FONT_H
