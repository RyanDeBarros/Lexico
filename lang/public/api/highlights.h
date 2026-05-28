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

	constexpr size_t color_count()
	{
		return static_cast<size_t>(HighlightColor::_Count);
	}

	constexpr size_t color_idx(HighlightColor color)
	{
		return static_cast<size_t>(color);
	}

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

	struct HighlightVisit
	{
		HighlightColor color;
		Highlight highlight;
		std::string_view text;
		size_t line;
		size_t col;
	};

	struct HighlightMap
	{
		std::vector<std::string_view> lines;
		std::vector<size_t> line_offsets;
		std::array<HighlightSet, color_count()> array;

		const HighlightSet& operator[](HighlightColor color) const;
		HighlightSet& operator[](HighlightColor color);

		void calc_line_offsets();
		void visit(HighlightColor color, std::function<void(const HighlightVisit&)> visitor) const;
	};
}
