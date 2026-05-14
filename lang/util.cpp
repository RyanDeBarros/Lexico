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
}
