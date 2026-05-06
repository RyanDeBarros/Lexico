#pragma once

#include "operations.h"
#include "errors.h"

#include <sstream>

namespace lx
{
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
	M(List) Sep \
	M(Unresolved)

#define LX_EXPAND_BY_PUBLIC_TYPE(M, Sep) \
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

#define LX_EXPAND_BY_INTERNAL_TYPE(M, Sep) \
	M(Unresolved)

#define LX_FORWARD_DECLARE(U) class U;
	LX_EXPAND_BY_TYPE(LX_FORWARD_DECLARE,)
#undef LX_FORWARD_DECLARE

#define LX_IDENTITY(U) U
#define LX_COMMA ,

	using TypeVariant = std::variant<
		LX_EXPAND_BY_TYPE(LX_IDENTITY, LX_COMMA)
	>;

	using PublicTypeVariant = std::variant<
		LX_EXPAND_BY_PUBLIC_TYPE(LX_IDENTITY, LX_COMMA)
	>;

	extern PublicTypeVariant to_public(const TypeVariant& v);
	extern PublicTypeVariant to_public(TypeVariant&& v);

#undef LX_IDENTITY
#undef LX_COMMA

#define LX_IS_SAME_V(U) std::is_same_v<T, U>
#define LX_OR ||

	template<typename T>
	concept Type = LX_EXPAND_BY_TYPE(LX_IS_SAME_V, LX_OR);

	template<typename T>
	concept PublicType = LX_EXPAND_BY_PUBLIC_TYPE(LX_IS_SAME_V, LX_OR);

#undef LX_IS_SAME_V
#undef LX_OR

	template<typename>
	constexpr bool deferred_false_v = false;

	template<typename T>
	struct ToEnum
	{
		static_assert(deferred_false_v<T>);
	};

#define LX_PUBLIC_TYPE_TO_ENUM_CLASS(U) \
	template<> \
	struct ToEnum<U> \
	{ \
		constexpr static DataType value = DataType::##U; \
	};

#define LX_INTERNAL_TYPE_TO_ENUM_CLASS(U) \
	template<> \
	struct ToEnum<U> \
	{ \
		constexpr static DataType value = DataType::_##U; \
	};

	LX_EXPAND_BY_PUBLIC_TYPE(LX_PUBLIC_TYPE_TO_ENUM_CLASS,);
	LX_EXPAND_BY_INTERNAL_TYPE(LX_INTERNAL_TYPE_TO_ENUM_CLASS,);

#undef LX_PUBLIC_TYPE_TO_ENUM_CLASS
#undef LX_INTERNAL_TYPE_TO_ENUM_CLASS

	template<Type T>
	constexpr DataType to_enum = ToEnum<T>::value;

	template<DataType T>
	struct ToType
	{
		static_assert(deferred_false_v<T>);
	};

#define LX_PUBLIC_TYPE_TO_ENUM_CLASS(U) \
	template<> \
	struct ToType<DataType::##U> \
	{ \
		using type = U; \
	};

#define LX_INTERNAL_TYPE_TO_ENUM_CLASS(U) \
	template<> \
	struct ToType<DataType::_##U> \
	{ \
		using type = U; \
	};

	LX_EXPAND_BY_PUBLIC_TYPE(LX_PUBLIC_TYPE_TO_ENUM_CLASS,);
	LX_EXPAND_BY_INTERNAL_TYPE(LX_INTERNAL_TYPE_TO_ENUM_CLASS,);

#undef LX_ENUM_CLASS_TO_PUBLIC_TYPE
#undef LX_ENUM_CLASS_TO_INTERNAL_TYPE

	template<DataType T>
	using to_type = typename ToType<T>::type;

	[[noreturn]] extern void throw_bad_cast(DataType from, DataType to);
}

#undef LX_EXPAND_BY_TYPE
#undef LX_EXPAND_BY_PUBLIC_TYPE
#undef LX_EXPAND_BY_INTERNAL_TYPE
