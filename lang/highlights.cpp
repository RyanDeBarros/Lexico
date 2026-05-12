#include "highlights.h"

namespace lx
{
	size_t Highlight::end() const
	{
		return start + length;
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
}
