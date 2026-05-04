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

		template<typename T> requires Type<std::decay_t<T>>
		void setval(T&& obj)
		{
			std::visit([&](auto& v) {
				using To = std::decay_t<decltype(v)>;
				v = cast<To>(std::forward<T>(obj));
			}, _storage);
		}

		void set(const DataPoint& other);
		void set(DataPoint&& other);

		bool can_cast_implicit(DataType to) const;
		bool can_cast_explicit(DataType to) const;
	};
}
