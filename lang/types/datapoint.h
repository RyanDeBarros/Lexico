#pragma once

#include "declarations.h"
#include "evalcontext.h"
#include "primitives/include.h"

namespace lx
{
	class DataPoint
	{
		// TODO variant is 56 bytes and potentially climbing, which is inefficient for smaller types like Int and Bool, especially for being stored in the heap
		TypeVariant _storage;

	public:
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

		template<Type T>
		const T* as() const
		{
			if (data_type() == T::data_type())
				return &get<T>();
			else
				return nullptr;
		}

		template<Type T>
		T* as()
		{
			if (data_type() == T::data_type())
				return &get<T>();
			else
				return nullptr;
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

		Variable data_member(VarContext& ctx, const std::string_view member) const;
		Variable invoke_method(VarContext& ctx, const std::string_view method, std::vector<Variable>&& args) const;
	};

	template<typename T>
	T Variable::consume_as() &&
	{
		return std::move(*this).consume().move_as<T>();
	}
}
