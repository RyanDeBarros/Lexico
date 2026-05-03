#pragma once

#include "declarations.h"
#include "pattern.h"
#include "basic.h"

namespace lx
{
	class DataPoint
	{
		TypeVariant _storage;

	public:
		template<Type T>
		DataPoint(T&& var) : _storage(std::forward<T>(var)) {}

		template<Type T>
		static DataPoint make_from_literal(std::string_view resolved)
		{
			if constexpr (requires { T::make_from_literal(resolved); })
			{
				return DataPoint(T::make_from_literal(resolved));
			}
			else
			{
				std::stringstream ss;
				ss << "cannot make " << to_enum<T> << " from literal string_view";
				throw LxError(ErrorType::Internal, ss.str());
			}
		}

		template<Type T>
		const T& get() const
		{
			return std::get<T>(_storage);
		}

		template<Type T>
		T& get()
		{
			return std::get<T>(_storage);
		}

		template<Type T>
		void set(T&& obj)
		{
			std::visit([&](auto& v) {
				using To = std::decay_t<decltype(v)>;
				v = cast<To>(std::forward<T>(obj));
			}, _storage);
		}

		bool can_cast_implicit(DataType to) const;
		bool can_cast_explicit(DataType to) const;
	};
}
