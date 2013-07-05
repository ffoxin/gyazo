#ifndef FONT_H
#define FONT_H

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
		Font(int fontHeight);
		~Font();

	private:
		static Font* m_instance;
		HFONT m_font;
		int m_fontHeight;
	};

}

#endif // FONT_H
