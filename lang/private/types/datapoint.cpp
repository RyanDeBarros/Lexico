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

	Variable DataPoint::pass_arg(VarContext ctx)
	{
		return std::visit([&ctx](auto&& v) { return remove_cow(v).pass_arg(std::move(ctx)); }, _storage);
	}

	void DataPoint::assign(const EvalContext& env, Variable other)
	{
		std::visit([&env, &other](auto& v) { remove_cow(v).assign(env, std::move(other)); }, _storage);
	}

	bool DataPoint::equals(const EvalContext& env, Variable other) const
	{
		return std::visit([&env, &other](const auto& v) { return remove_cow(v).equals(env, std::move(other)); }, _storage);
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

	void DataPoint::print(const EvalContext& env, std::ostream& ss) const
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
			throw env.runtime_error(data_type().repr() + " is not iterable");
	}

	Variable DataPoint::iterget(VarContext& ctx, size_t i) const
	{
		if (data_type().is_iterable())
			return std::visit([&ctx, i](const auto& v) -> Variable {
				if constexpr (requires { remove_cow(v).iterget(ctx, i); })
					return remove_cow(v).iterget(ctx, i);
				else
					throw ctx.env.internal_error(remove_cow(v).data_type().repr() + " should implement 'iterget' but it doesn't");
			}, _storage);
		else
			throw ctx.env.runtime_error(data_type().repr() + " is not iterable");
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
			throw env.runtime_error(data_type().repr() + " is not pageable");
	}

	Variable DataPoint::data_member(VarContext& ctx, const std::string_view member)
	{
		return std::visit([&ctx, member](auto& v) -> Variable { return remove_cow(v).data_member(ctx, member); }, _storage);
	}

	Variable DataPoint::invoke_method(VarContext& ctx, const std::string_view method, std::vector<Variable>&& args)
	{
		return std::visit([&ctx, method, &args](auto& v) -> Variable { return remove_cow(v).invoke_method(ctx, method, std::move(args)); }, _storage);
	}

	const DataPoint& DataPoint::root() const
	{
		return std::visit([this](const auto& v) -> const DataPoint& {
			if constexpr (requires { remove_cow(v).root(); })
				return remove_cow(v).root();
			else
				return *this;
		}, _storage);
	}

	DataPoint& DataPoint::root()
	{
		return std::visit([this](auto& v) -> DataPoint& {
			if constexpr (requires { remove_cow(v).root(); })
				return remove_cow(v).root();
			else
				return *this;
		}, _storage);
	}
}
