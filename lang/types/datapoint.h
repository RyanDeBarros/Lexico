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

		static DataPoint make_from_literal(const EvalContext& env, DataType type, std::string_view resolved);

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
		T move_as(const EvalContext& env)
		{
			return std::get<T>(std::visit([&env](auto&& v) { return v.cast_move(env, T::data_type()); }, std::move(_storage)));
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

		DataPoint cast_copy(const EvalContext& env, const DataType& type) const;
		DataPoint cast_move(const EvalContext& env, const DataType& type);

		void assign(const EvalContext& env, const DataPoint& other);
		void assign(const EvalContext& env, DataPoint&& other);

		bool equals(const EvalContext& env, const DataPoint& other) const;
		bool equals(const EvalContext& env, DataPoint&& other) const;

		bool can_cast_implicit(const DataType& to) const;
		bool can_cast_explicit(const DataType& to) const;

		DataType data_type() const;

		void print(const EvalContext& env, std::stringstream& ss) const;

		size_t iterlen(const EvalContext& env) const;
		DataPoint iterget(const EvalContext& env, size_t i) const;
		std::string page_content(const EvalContext& env) const;

		Variable data_member(VarContext& ctx, const std::string_view member) const;
		Variable invoke_method(VarContext& ctx, const std::string_view method, std::vector<Variable>&& args) const;
	};

	template<typename T>
	T Variable::consume_as(const EvalContext& env) &&
	{
		return std::move(*this).consume().move_as<T>(env);
	}
}
