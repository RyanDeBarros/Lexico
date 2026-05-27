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

	void HighlightMap::visit_chars(std::function<void(const HighlightCharView&)> visitor)
	{
		std::array<size_t, static_cast<size_t>(HighlightColor::_Count)> highlight_indexes{};
		std::fill(highlight_indexes.begin(), highlight_indexes.end(), 0);

		size_t index = 0;
		for (size_t row = 0; row < lines.size(); ++row)
		{
			for (size_t col = 0; col < lines[row].size(); ++col)
			{
				for (size_t i = 0; i < array.size(); ++i)
				{
					const HighlightSet& highlight_set = array[i];
					size_t& highlight_idx = highlight_indexes[i];
					while (highlight_idx < highlight_set.list().size() && index > highlight_set.list()[highlight_idx].end())
						++highlight_idx;

					if (highlight_idx < highlight_set.list().size() && highlight_set.list()[highlight_idx].contains(index))
					{
						HighlightCharView view{
							.c = lines[row][col],
							.new_match = index == highlight_set.list()[highlight_idx].start,
							.index = index,
							.row = row,
							.col = col,
							.color = static_cast<HighlightColor>(i)
						};

						visitor(view);
					}
				}

				++index;
			}
		}
	}
}
