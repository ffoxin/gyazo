#include "util.h"

namespace Gyazo
{

	//////////////////////////////////////////////////////////////////////////

	BaseSize::BaseSize()
		: cx(size.cx)
		, cy(size.cy)
	{
		// empty
	}

	//////////////////////////////////////////////////////////////////////////

	Size::Size()
	{
		Init(0, 0);
	}

	Size::Size(uint32_t x, uint32_t y)
	{
		Init(x, y);
	}

	Size::Size(const Size& size)
	{
		Copy(size);
	}

	Size::~Size()
	{
		// empty
	}

	Size& Size::operator=(const Size& size)
	{
		if (this != &size)
		{
			Copy(size);
		}

		return *this;
	}

	void Size::Init(uint32_t x, uint32_t y)
	{
		cx = x;
		cy = y;
	}

	void Size::Copy(const Size& size)
	{
		memcpy(&this->size, &size.size, sizeof(size.size));
	}

	//////////////////////////////////////////////////////////////////////////

	BaseRect::BaseRect()
		: left(rect.left)
		, top(rect.top)
		, right(rect.right)
		, bottom(rect.bottom)
	{
		// empty
	}

	//////////////////////////////////////////////////////////////////////////

	Rect::Rect()
	{
		Init(0, 0, 0, 0);
	}

	Rect::Rect(uint32_t left, uint32_t top, uint32_t right, uint32_t bottom)
	{
		Init(left, top, right, bottom);
	}

	Rect::Rect(const Rect& rect)
	{
		Copy(rect);
	}

	Rect::~Rect()
	{
		// empty
	}

	Rect& Rect::operator=(const Rect& rect)
	{
		Copy(rect);
	}

	void Rect::Init(uint32_t left, uint32_t top, uint32_t right, uint32_t bottom)
	{
		this->left = left;
		this->top = top;
		this->right = right;
		this->bottom = bottom;
	}

	void Rect::Copy(const Rect& rect)
	{
		memcpy(&this->rect, &rect.rect, sizeof(rect.rect));
	}

	//////////////////////////////////////////////////////////////////////////

} // namespace Gyazo

