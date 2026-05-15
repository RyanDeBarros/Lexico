#pragma once

#include <array>
#include <vector>

namespace lx
{
	enum class HighlightColor
	{
		Yellow,
		Red,
		Green,
		Blue,
		Grey,
		Purple,
		Orange,
		Mono,
		_Count
	};

	struct Highlight
	{
		size_t start;
		size_t length;

		size_t end() const;
	};

	class HighlightSet
	{
		std::vector<Highlight> _list;

	public:
		const std::vector<Highlight>& list() const;
		void insert(Highlight range);
		void remove(Highlight range);
	};

	struct HighlightMap
	{
		std::array<HighlightSet, static_cast<size_t>(HighlightColor::_Count)> array;

		const HighlightSet& operator[](HighlightColor color) const;
		HighlightSet& operator[](HighlightColor color);
	};

}
