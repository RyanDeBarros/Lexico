#include "api/highlights.h"

namespace lx
{
	size_t Highlight::end() const
	{
		return start + length;
	}

	bool Highlight::contains(const Highlight& other) const
	{
		return other.start >= start && other.end() <= end();
	}

	bool Highlight::contains(size_t index) const
	{
		return index >= start && index < end();
	}

	bool Highlight::disjoint(const Highlight& other) const
	{
		return end() <= other.start || other.end() <= start;
	}

	const std::vector<Highlight>& HighlightSet::list() const
	{
		return _list;
	}
	
	void HighlightSet::insert(Highlight range)
	{
		if (range.length == 0)
			return;

		size_t new_start = range.start;
		size_t new_end = range.end();

		// skip leftward ranges
		auto it = _list.begin();
		while (it != _list.end() && it->end() < new_start)
			++it;

		// merge overlapping/adjacent ranges
		while (it != _list.end() && it->start <= new_end)
		{
			new_start = std::min(new_start, it->start);
			new_end = std::max(new_end, it->end());
			it = _list.erase(it);
		}

		_list.insert(it, Highlight{ .start = new_start, .length = new_end - new_start });
	}
	
	void HighlightSet::remove(Highlight range)
	{
		if (range.length == 0)
			return;

		const size_t remove_start = range.start;
		const size_t remove_end = range.end();

		auto it = _list.begin();
		while (it != _list.end())
		{
			const size_t cur_start = it->start;
			const size_t cur_end = it->end();

			// no overlap
			if (cur_end <= remove_start)
			{
				++it;
				continue;
			}
			else if (cur_start >= remove_end)
				return;

			// remove entire range
			if (remove_start <= cur_start && remove_end >= cur_end)
			{
				it = _list.erase(it);
				continue;
			}

			// split into two ranges
			if (remove_start > cur_start && remove_end < cur_end)
			{
				Highlight right{ .start = remove_end, .length = cur_end - remove_end };
				it->length = remove_start - cur_start;
				_list.insert(std::next(it), right);
				return;
			}

			// trim left side
			if (remove_start <= cur_start)
			{
				it->start = remove_end;
				it->length = cur_end - remove_end;
				++it;
				continue;
			}

			// trim right side
			it->length = remove_start - cur_start;
			++it;
		}
	}

	const HighlightSet& HighlightMap::operator[](HighlightColor color) const
	{
		return array[static_cast<size_t>(color)];
	}
	
	HighlightSet& HighlightMap::operator[](HighlightColor color)
	{
		return array[static_cast<size_t>(color)];
	}

	void HighlightMap::calc_line_offsets()
	{
		line_offsets.clear();
		size_t sum = 0;
		for (const auto& line : lines)
		{
			line_offsets.push_back(sum);
			sum += line.size() + 1; // +1 for '\n'
		}
	}

	void HighlightMap::visit(HighlightColor color, std::function<void(const HighlightVisit&)> visitor) const
	{
		const HighlightSet& highlight_set = array[color_idx(color)];
		size_t line_idx = 0;
		for (const Highlight& highlight : highlight_set.list())
		{
			while (highlight.start > line_offsets[line_idx] + lines[line_idx].size())
				++line_idx;

			size_t col = highlight.start - line_offsets[line_idx];
			HighlightVisit visit{
				.color = color,
				.highlight = highlight,
				.text = lines[line_idx].substr(col, highlight.length),
				.line = line_idx,
				.col = col,
			};
			visitor(visit);
		}
	}
}
