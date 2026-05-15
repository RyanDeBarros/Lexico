#include "util.h"

namespace lx
{
	size_t TransparentHash::operator()(const std::string_view sv) const
	{
		return std::hash<std::string_view>{}(sv);
	}

	size_t TransparentHash::operator()(const std::string& s) const
	{
		return std::hash<std::string_view>{}(s);
	}

	size_t TransparentHash::operator()(const char* s) const
	{
		return std::hash<std::string_view>{}(s);
	}

	bool TransparentEqual::operator()(std::string_view a, std::string_view b) const
	{
		return a == b;
	}

	size_t hash_combine(size_t hash, size_t value)
	{
		return hash ^ (value + 0x9e3779b9 + (hash << 6) + (hash >> 2));
	}

	void adjust_range_resize(size_t& start, size_t& length, size_t index, size_t from_length, size_t to_length)
	{
		if (start + length <= index)
			return;

		if (start >= index + from_length)
		{
			if (to_length > from_length)
				start += to_length - from_length;
			else
				start -= from_length - to_length;
		}
		else if (start >= index)
		{
			if (start + length <= index + from_length)
				length = 0;
			else
				length -= index + from_length - start;

			start = index + to_length;
		}
		else if (start + length > index + from_length)
		{
			if (to_length > from_length)
				length += to_length - from_length;
			else
				length -= from_length - to_length;
		}
		else
			length -= start + length - index;
	}
}
