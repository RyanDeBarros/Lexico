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
		DataPoint(const DataPoint&);
		DataPoint(const TypeVariant&);
		DataPoint(TypeVariant&&);

		template<Type T>
		DataPoint(T&& var) : _storage(std::forward<T>(var)) {}

		static DataPoint make_from_literal(DataType type, std::string_view resolved);

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
		T copy_as() const
		{
			return std::get<T>(std::visit([](const auto& v) { return v.cast_copy(to_enum<T>); }, _storage));
		}

		template<Type T>
		T move_as()
		{
			return std::get<T>(std::visit([](auto&& v) { return v.cast_move(to_enum<T>); }, std::move(_storage)));
		}

		DataPoint cast_copy(DataType type) const;
		DataPoint cast_move(DataType type);

		template<typename To, typename From> requires Type<std::decay_t<From>>
		static TypeVariant cast_type(From&& obj)
		{
			if constexpr (std::is_rvalue_reference_v<From&&>)
				return std::forward<From>(obj).cast_move(to_enum<To>);
			else
				return obj.cast_copy(to_enum<To>);
		}

		template<typename T> requires Type<std::decay_t<T>>
		void setval(T&& obj)
		{
			std::visit([&](auto& v) {
				using To = std::decay_t<decltype(v)>;
				v = std::get<To>(cast_type<To>(std::forward<T>(obj)));
			}, _storage);
		}

		void set(const DataPoint& other);
		void set(DataPoint&& other);

		bool can_cast_implicit(DataType to) const;
		bool can_cast_explicit(DataType to) const;
	};
}
