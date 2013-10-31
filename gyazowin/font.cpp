#include "font.h"

// Project headers
#include "stringconstants.h"

GyazoFont* GyazoFont::m_instance = NULL;

HFONT GyazoFont::GetFont(int fontHeight)
{
    if (m_instance != NULL
        && fontHeight != m_instance->m_fontHeight)
    {
        Release();
    }

    if (m_instance == NULL)
    {
        m_instance = new GyazoFont(fontHeight);
    }

    return m_instance->m_font;
}

GyazoFont::GyazoFont(int fontHeight)
{
    m_fontHeight = fontHeight;

    m_font = CreateFontW(
        (-1) * fontHeight,  // Font height
        0,                  // Text parcels
        0,                  // Angle of text
        0,                  // Angle of the x-axis and the baseline
        FW_REGULAR,         // The font weight (thickness)
        FALSE,              // Italic
        FALSE,              // Underline
        FALSE,              // Strike through
        ANSI_CHARSET,       // Character set
        OUT_DEFAULT_PRECIS, // Output Accuracy
        CLIP_DEFAULT_PRECIS,// Clipping accuracy
        PROOF_QUALITY,      // Output Quality
        FIXED_PITCH | FF_MODERN, // Family pitch
        GYAZO_FONT_NAME     // Face name
        );
}

GyazoFont::~GyazoFont()
{
    DeleteObject(m_font);
}

void GyazoFont::Release()
{
    delete m_instance;
    m_instance = NULL;
}
