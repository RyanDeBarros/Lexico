#pragma once

#include "operations.h"
#include "errors.h"

#include <sstream>
#include <variant>

namespace lx
{
	class Runtime;
	class DataPoint;

#define LX_EXPAND_BY_TYPE(M, Sep) \
	M(Int) Sep \
	M(Float) Sep \
	M(Bool) Sep \
	M(String) Sep \
	M(Void) Sep \
	M(Pattern) Sep \
	M(Match) Sep \
	M(Matches) Sep \
	M(CapId) Sep \
	M(Cap) Sep \
	M(IRange) Sep \
	M(SRange) Sep \
	M(List)

#define LX_FORWARD_DECLARE(U) class U;
	LX_EXPAND_BY_TYPE(LX_FORWARD_DECLARE,)
#undef LX_FORWARD_DECLARE

#define LX_IDENTITY(U) U
#define LX_COMMA ,

	using TypeVariant = std::variant<
		LX_EXPAND_BY_TYPE(LX_IDENTITY, LX_COMMA)
	>;

#undef LX_IDENTITY
#undef LX_COMMA

#define LX_IS_SAME_V(U) std::is_same_v<T, U>
#define LX_OR ||

	template<typename T>
	concept Type = LX_EXPAND_BY_TYPE(LX_IS_SAME_V, LX_OR);

#undef LX_IS_SAME_V
#undef LX_OR

	[[noreturn]] extern void throw_bad_cast(const DataType& from, const DataType& to);
}

#undef LX_EXPAND_BY_TYPE
