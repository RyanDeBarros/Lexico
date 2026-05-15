#pragma once

#include "operations.h"
#include "errors.h"
#include "cow_ptr.h"

#include <sstream>
#include <variant>

namespace lx
{
	class DataPoint;
	class Runtime;
	struct EvalContext;
	struct VarContext;
	class Variable;

	class Int;
	class Float;
	class Bool;
	class String;
	class StringView;
	class Void;
	class Pattern;
	class Match;
	class Matches;
	class CapId;
	class Cap;
	class IRange;
	class SRange;
	class List;

	using TypeVariant = std::variant<
		Int,
		Float,
		Bool,
		String,
		CowPtr<StringView>,
		Void,
		Pattern,
		CowPtr<Match>,
		Matches,
		CapId,
		CowPtr<Cap>,
		IRange,
		SRange,
		CowPtr<List>
	>;

	template<typename T>
	concept Type =
		std::is_same_v<T, Int> ||
		std::is_same_v<T, Float> ||
		std::is_same_v<T, Bool> ||
		std::is_same_v<T, String> ||
		std::is_same_v<T, StringView> ||
		std::is_same_v<T, Void> ||
		std::is_same_v<T, Pattern> ||
		std::is_same_v<T, Match> ||
		std::is_same_v<T, Matches> ||
		std::is_same_v<T, CapId> ||
		std::is_same_v<T, Cap> ||
		std::is_same_v<T, IRange> ||
		std::is_same_v<T, SRange> ||
		std::is_same_v<T, List>;
}
