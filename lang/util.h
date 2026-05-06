#pragma once

#include "types/datatype.h"

#include <string_view>
#include <unordered_map>
#include <unordered_set>

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
	using StringSet = std::unordered_set<std::string, TransparentHash, TransparentEqual>;

	struct FunctionCallSignature
	{
		std::string identifier;
		std::vector<DataType> arg_types;
	};

	struct FunctionCallHash
	{
		size_t operator()(const FunctionCallSignature& fc) const;
	};

	struct FunctionCallEqual
	{
		bool operator()(const FunctionCallSignature& a, const FunctionCallSignature& b) const;
	};

	using FunctionCallSet = std::unordered_set<FunctionCallSignature, FunctionCallHash, FunctionCallEqual>;
}
