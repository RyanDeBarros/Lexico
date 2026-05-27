#pragma once

#include <array>
#include <functional>
#include <string_view>
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

		bool contains(const Highlight& other) const;
		bool contains(size_t index) const;
		bool disjoint(const Highlight& other) const;
	};

	class HighlightSet
	{
		std::vector<Highlight> _list;

	public:
		const std::vector<Highlight>& list() const;
		void insert(Highlight range);
		void remove(Highlight range);
	};

	struct HighlightCharView
	{
		char c;
		bool new_match;
		size_t index;
		size_t row;
		size_t col;
		HighlightColor color;
	};

	struct HighlightMap
	{
		std::vector<std::string_view> lines;
		std::array<HighlightSet, static_cast<size_t>(HighlightColor::_Count)> array;

		const HighlightSet& operator[](HighlightColor color) const;
		HighlightSet& operator[](HighlightColor color);

		void visit_chars(std::function<void(const HighlightCharView&)> visitor);
	};
}
