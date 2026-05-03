#pragma once

#include <string>
#include <string_view>
#include <unordered_map>

namespace lx
{
	struct TransparentHash
	{
		using is_transparent = void;

		size_t operator()(const std::string_view sv) const;
		size_t operator()(const std::string& s) const;
		size_t operator()(const char* s) const;
	};

	struct TransparentEqual
	{
		using is_transparent = void;

		bool operator()(std::string_view a, std::string_view b) const;
	};

	template<typename Value>
	using StringMap = std::unordered_map<std::string, Value, TransparentHash, TransparentEqual>;
}
