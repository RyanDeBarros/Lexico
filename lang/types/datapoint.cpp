#include "datapoint.h"

namespace lx
{
	DataPoint::DataPoint(const TypeVariant& v)
		: _storage(v)
	{
	}

	DataPoint::DataPoint(TypeVariant&& v)
		: _storage(std::move(v))
	{
	}

	DataPoint DataPoint::make_from_literal(const EvalContext& env, DataType type, std::string_view resolved)
	{
		switch (type.simple())
		{
		case SimpleType::Int:
			return Int::make_from_literal(env, resolved);
		case SimpleType::Float:
			return Float::make_from_literal(env, resolved);
		case SimpleType::Bool:
			return Bool::make_from_literal(env, resolved);
		case SimpleType::String:
			return String::make_from_literal(env, resolved);
		case SimpleType::CapId:
			return CapId::make_from_literal(env, resolved);
		default:
		{
			std::stringstream ss;
			ss << __FUNCTION__ << ": " << type << " does not support literal construction";
			throw env.internal_error(ss.str());
		}
		}
	}

	const TypeVariant& DataPoint::variant() const
	{
		return _storage;
	}

	TypeVariant& DataPoint::variant()
	{
		return _storage;
	}

	DataPoint DataPoint::cast_copy(const VarContext& ctx, const DataType& type) const
	{
		return std::visit([&ctx, &type](const auto& v) { return DataPoint(remove_cow(v).cast_copy(ctx, type)); }, _storage);
	}

	DataPoint DataPoint::cast_move(VarContext&& ctx, const DataType& type) &&
	{
		return std::visit([&ctx, &type](auto&& v) { return DataPoint(remove_cow(std::move(v)).cast_move(std::move(ctx), type)); }, std::move(_storage));
	}

	void DataPoint::assign(const EvalContext& env, Variable other)
	{
		DataPoint casted = std::move(other).cast(env, data_type());
		std::visit([&env, &casted](auto& v) { remove_cow(v).assign(env, std::move(casted.get<remove_cow_t<decltype(v)>>())); }, _storage);
	}

	bool DataPoint::equals(const EvalContext& env, Variable other) const
	{
		DataPoint casted = std::move(other).cast(env, data_type());
		if (other.ref().can_cast_implicit(data_type()))
			return std::visit([&env, &casted](const auto& v) { return remove_cow(v).equals(env, casted.get<remove_cow_t<decltype(v)>>()); }, _storage);
		else
			return false;
	}

	bool DataPoint::can_cast_implicit(const DataType& to) const
	{
		return data_type().can_cast_implicit(to);
	}

	bool DataPoint::can_cast_explicit(const DataType& to) const
	{
		return data_type().can_cast_explicit(to);
	}

	DataType DataPoint::data_type() const
	{
		return std::visit([](const auto& v) -> DataType { return remove_cow(v).data_type(); }, _storage);
	}

	void DataPoint::print(const EvalContext& env, std::stringstream& ss) const
	{
		std::visit([&env, &ss](const auto& v) { remove_cow(v).print(env, ss); }, _storage);
	}

	size_t DataPoint::iterlen(const EvalContext& env) const
	{
		if (data_type().is_iterable())
			return std::visit([&env](const auto& v) -> size_t {
				if constexpr (requires { remove_cow(v).iterlen(env); })
					return remove_cow(v).iterlen(env);
				else
					throw env.internal_error(remove_cow(v).data_type().repr() + " should implement 'iterlen' but it doesn't");
			}, _storage);
		else
			throw env.internal_error("iterlen(): " + data_type().repr() + " is not iterable");
	}

	DataPoint DataPoint::iterget(const EvalContext& env, size_t i) const
	{
		if (data_type().is_iterable())
			return std::visit([&env, i](const auto& v) -> DataPoint {
				if constexpr (requires { remove_cow(v).iterget(env, i); })
					return remove_cow(v).iterget(env, i);
				else
					throw env.internal_error(remove_cow(v).data_type().repr() + " should implement 'iterget' but it doesn't");
			}, _storage);
		else
			throw env.internal_error("iterget(): " + data_type().repr() + " is not iterable");
	}

	std::string DataPoint::page_content(const EvalContext& env) const
	{
		if (data_type().is_pageable())
			return std::visit([&env](const auto& v) -> std::string {
			if constexpr (requires { remove_cow(v).page_content(env); })
				return remove_cow(v).page_content(env);
			else
				throw env.internal_error(remove_cow(v).data_type().repr() + " should implement 'page_content' but it doesn't");
			}, _storage);
		else
			throw env.internal_error("page_content(): " + data_type().repr() + " is not pageable");
	}

	Variable DataPoint::data_member(VarContext& ctx, const std::string_view member)
	{
		return std::visit([&ctx, member](auto& v) -> Variable { return remove_cow(v).data_member(ctx, member); }, _storage);
	}

	Variable DataPoint::invoke_method(VarContext& ctx, const std::string_view method, std::vector<Variable>&& args)
	{
		return std::visit([&ctx, method, &args](auto& v) -> Variable { return remove_cow(v).invoke_method(ctx, method, std::move(args)); }, _storage);
	}
}
