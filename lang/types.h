#pragma once

#include <string>
#include <variant>
#include <vector>
#include <optional>

#include "pattern.h"
#include "match.h"

namespace lx
{
	struct VoidType
	{
		std::monostate value;
	};

	struct IntType
	{
		int value;
	};

	struct FloatType
	{
		float value;
	};

	struct BoolType
	{
		bool value;
	};

	struct StringType
	{
		std::string value;
	};

	struct PatternType
	{
		Pattern value;
	};

	struct MatchType
	{
		Match value;
	};

	struct MatchesType
	{
		Matches value;
	};

	struct CapIdType
	{
		std::string value;
	};

	struct CapType
	{
		Cap value;
	};

	struct IRangeType
	{
		std::optional<IntType> min;
		std::optional<IntType> max;
	};

	struct SRangeType
	{
		std::optional<StringType> min;
		std::optional<StringType> max;
	};

	struct ListType;

	using DataType = std::variant<
		VoidType,
		IntType,
		FloatType,
		BoolType,
		StringType,
		PatternType,
		MatchType,
		MatchesType,
		CapIdType,
		CapType,
		IRangeType,
		SRangeType,
		ListType
	>;

	struct ListType
	{
		std::vector<DataType> value;
	};
}
