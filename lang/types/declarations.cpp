#include "declarations.h"

#include "basic.h"
#include "pattern.h"
#include "unresolved.h"

namespace lx
{
	PublicTypeVariant to_public(const TypeVariant& v)
	{
		return std::visit([](const auto& v) -> PublicTypeVariant {
			using T = std::decay_t<decltype(v)>;
			if constexpr (PublicType<T>)
				return v;
			else
			{
				std::stringstream ss;
				ss << __FUNCTION__ << ": " << to_enum<T> << " is not a public type";
				throw LxError(ErrorType::Internal, ss.str());
			}
		}, v);
	}

	PublicTypeVariant to_public(TypeVariant&& v)
	{
		return std::visit([](auto&& v) -> PublicTypeVariant {
			using T = std::decay_t<decltype(v)>;
			if constexpr (PublicType<T>)
				return std::forward<decltype(v)>(v);
			else
			{
				std::stringstream ss;
				ss << __FUNCTION__ << ": " << to_enum<T> << " is not a public type";
				throw LxError(ErrorType::Internal, ss.str());
			}
		}, std::move(v));
	}

	void throw_bad_cast(DataType from, DataType to)
	{
		std::stringstream ss;
		ss << "bad cast from " << from << " to " << to;
		throw LxError(ErrorType::Internal, ss.str());
	}
}
