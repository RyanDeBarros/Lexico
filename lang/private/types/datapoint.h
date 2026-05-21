#pragma once

#include "declarations.h"
#include "evalcontext.h"
#include "primitives/include.h"

namespace lx
{
	class DataPoint
	{
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
			if constexpr (is_in_variant_v<T, TypeVariant>)
			{
				if (const T* ptr = std::get_if<T>(&_storage))
					return *ptr;
			}
			else if constexpr (is_in_variant_v<CowPtr<T>, TypeVariant>)
			{
				if (const CowPtr<T>* cow = std::get_if<CowPtr<T>>(&_storage))
					return **cow;
			}
			
			throw std::bad_variant_access();
		}

		template<Type T>
		T& get()
		{
			if constexpr (is_in_variant_v<T, TypeVariant>)
			{
				if (T* ptr = std::get_if<T>(&_storage))
					return *ptr;
			}
			else if constexpr (is_in_variant_v<CowPtr<T>, TypeVariant>)
			{
				if (CowPtr<T>* cow = std::get_if<CowPtr<T>>(&_storage))
					return **cow;
			}

			throw std::bad_variant_access();
		}

		template<Type T>
		T move_as(VarContext&& ctx) &&
		{
			return std::move(*this).cast_move(std::move(ctx), remove_cow_t<T>::data_type()).get<T>();
		}

		template<Type T>
		const T* as() const
		{
			if (data_type() == remove_cow_t<T>::data_type())
				return &get<T>();
			else
				return nullptr;
		}

		template<Type T>
		T* as()
		{
			if (data_type() == remove_cow_t<T>::data_type())
				return &get<T>();
			else
				return nullptr;
		}

		DataPoint cast_copy(const VarContext& ctx, const DataType& type) const;
		DataPoint cast_move(VarContext&& ctx, const DataType& type) &&;

		void assign(const EvalContext& env, Variable other);
		bool equals(const EvalContext& env, Variable other) const;

		bool can_cast_implicit(const DataType& to) const;
		bool can_cast_explicit(const DataType& to) const;

		DataType data_type() const;

		void print(const EvalContext& env, std::stringstream& ss) const;

		size_t iterlen(const EvalContext& env) const;
		DataPoint iterget(const EvalContext& env, size_t i) const;
		std::string page_content(const EvalContext& env) const;

		Variable data_member(VarContext& ctx, const std::string_view member);
		Variable invoke_method(VarContext& ctx, const std::string_view method, std::vector<Variable>&& args);
	};

	template<typename T>
	T Variable::consume_as(const EvalContext& env) &&
	{
		VarContext ctx(env, std::move(*this));
		return std::move(ctx.self).consume().move_as<T>(std::move(ctx));
	}
}
