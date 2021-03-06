#pragma once

// System headers
#include <windows.h>

namespace Gyazo
{

class Font
{
public:
    static HFONT GetFont(int fontHeight);
    static void Release();

private:
    explicit Font(int fontHeight);
    ~Font();

private:
    static Font* m_instance;
    HFONT m_font;
    int m_fontHeight;
};

} // namespace Gyazo
