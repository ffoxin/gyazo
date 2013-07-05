#ifndef UTIL_H
#define UTIL_H

// Project headers
#include "types.h"

namespace Gyazo
{

	template<typename T>
	inline void swap(T& lr, T& rr)
	{
		T temp = lr;
		lr = rr;
		rr = temp;
	}

	class BaseSize
	{
	public:
		BaseSize();

	public:
		Size_t size;
		// references to related fields of size
		uint32_t&	cx;
		uint32_t&	cy;
	};

	class Size : public BaseSize
	{
	public:
		Size();
		Size(uint32_t x, uint32_t y);
		Size(const Size& size);
		~Size();

		Size& operator=(const Size& size);

	private:
		void Init(uint32_t x, uint32_t y);
		void Copy(const Size& size);
	};

	class BaseRect
	{
	public:
		BaseRect();
	public:
		Rect_t		rect;
		// references to related fields of rect
		uint32_t&	left;
		uint32_t&	top;
		uint32_t&	right;
		uint32_t&	bottom;
	};

	class Rect : public BaseRect
	{
	public:
		Rect();
		Rect(uint32_t left, uint32_t top, uint32_t right, uint32_t bottom);
		Rect(const Rect& rect);
		~Rect();

		Rect& operator=(const Rect& rect);

	private:
		void Init(uint32_t left, uint32_t top, uint32_t right, uint32_t bottom);
		void Copy(const Rect& rect);
	};

} // namespace Gyazo

#endif // UTIL_H
