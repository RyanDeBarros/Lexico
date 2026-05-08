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

		const TypeVariant& variant() const;
		TypeVariant& variant();

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
		T move_as()
		{
			return std::get<T>(std::visit([](auto&& v) { return v.cast_move(T::data_type()); }, std::move(_storage)));
		}

		DataPoint cast_copy(const DataType& type) const;
		DataPoint cast_move(const DataType& type);

		template<typename From> requires Type<std::decay_t<From>>
		static TypeVariant cast_type(From&& obj, const DataType& to)
		{
			if constexpr (std::is_rvalue_reference_v<From&&>)
				return std::forward<From>(obj).cast_move(to);
			else
				return obj.cast_copy(to);
		}

		template<typename T> requires Type<std::decay_t<T>>
		void setval(T&& obj)
		{
			std::visit([&](auto& v) {
				using To = std::decay_t<decltype(v)>;
				v = std::get<To>(cast_type(std::forward<T>(obj), v.data_type()));
			}, _storage);
		}

		void set(const DataPoint& other);
		void set(DataPoint&& other);
		bool equals(const DataPoint& other) const;
		bool equals(DataPoint&& other) const;

		bool can_cast_implicit(const DataType& to) const;
		bool can_cast_explicit(const DataType& to) const;

		DataType data_type() const;

		void print(std::stringstream& ss) const;

		size_t iterlen() const;
		DataPoint iterget(size_t i) const;
		std::string page_content() const;

		Variable data_member(Runtime& env, const ScriptSegment& segment, const std::string_view member) const;
		Variable invoke_method(Runtime& env, const ScriptSegment& segment, const std::string_view method, std::vector<Variable>&& args) const;
	};
}
