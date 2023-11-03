#pragma once


namespace hf
{
	template<typename _Ty>
	class Rect
	{
	public:

		Rect() : x(0), y(0), w(0), h(0) { }
		Rect(_Ty _x, _Ty _y, _Ty _w, _Ty _h) : x(_x), y(_y), w(_w), h(_h) {  }

		bool Contains(_Ty _x, _Ty _y)
		{
			if (_x >= x && _y >= y && _x < x + w && _y < y + h)
				return true;

			return false;
		}

		_Ty x, y, w, h;
	};

	using IntRect = Rect<int>;
}