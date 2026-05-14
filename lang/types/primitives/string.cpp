#include "string.h"

#include "include.h"
#include "runtime.h"
#include "constants.h"
#include "find.h"

namespace lx
{
	String::String(const std::string& value)
		: _value(value)
	{
	}

	String::String(std::string&& value)
		: _value(std::move(value))
	{
	}

	String String::make_from_literal(const EvalContext& env, std::string_view resolved)
	{
		return String(std::string(resolved));
	}

	DataType String::data_type()
	{
		return DataType::String();
	}
	
	TypeVariant String::cast_copy(const VarContext& ctx, const DataType& type) const
	{
		switch (type.simple())
		{
		case SimpleType::Int:
			return Int::make_from_literal(ctx.env, _value);
		case SimpleType::Float:
			return Float::make_from_literal(ctx.env, _value);
		case SimpleType::Bool:
			return Bool::make_from_literal(ctx.env, _value);
		case SimpleType::String:
			return *this;
		case SimpleType::StringView:
			return StringView(ctx.env, ctx.self, IRange(std::nullopt, std::nullopt));
		case SimpleType::Pattern:
			return Pattern::make_from_subpattern<SubpatternString>(_value);
		case SimpleType::Void:
			return Void();
		default:
			ctx.env.throw_bad_cast(data_type(), type);
		}
	}

	TypeVariant String::cast_move(VarContext&& ctx, const DataType& type) &&
	{
		if (type.simple() == SimpleType::String)
			return std::move(*this);
		else if (type.simple() == SimpleType::StringView)
			return StringView(ctx.env, std::move(ctx.self), IRange(std::nullopt, std::nullopt));
		else if (type.simple() == SimpleType::Pattern)
			return Pattern::make_from_subpattern<SubpatternString>(std::move(_value));
		else
			return cast_copy(ctx, type);
	}

	void String::print(const EvalContext& env, std::stringstream& ss) const
	{
		ss << _value;
	}

	static Variable substring_by_index(VarContext& ctx, Variable&& arg)
	{
		StringView sv(ctx.env, ctx.self, std::move(arg).consume_as<Int>(ctx.env));
		sv.assert_valid(ctx.env);
		return ctx.variable(std::move(sv));
	}

	static Variable substring_by_range(VarContext& ctx, Variable&& arg)
	{
		StringView sv(ctx.env, ctx.self, std::move(arg).consume_as<IRange>(ctx.env));
		sv.assert_valid(ctx.env);
		return ctx.variable(std::move(sv));
	}

	StringMap<MemberSignature> String::members()
	{
		return {
			{ constants::MEMBER_LEN, MemberSignature::make_data(constants::MEMBER_LEN, DataType::Int()) },
			{ constants::SUBSCRIPT_OP, MemberSignature::make_method(constants::SUBSCRIPT_OP, {
				{.return_type = DataType::String(), .arg_types = { DataType::Int() } },
				{.return_type = DataType::String(), .arg_types = { DataType::IRange() } },
			}) },
		};
	}

	Variable String::data_member(VarContext& ctx, const std::string_view member) const
	{
		if (member == constants::MEMBER_LEN)
			return ctx.variable(Int(_value.size()));

		ctx.throw_no_data_member(member);
	}

	Variable String::invoke_method(VarContext& ctx, const std::string_view method, std::vector<Variable>&& args) const
	{
		if (method == constants::SUBSCRIPT_OP)
		{
			if (args.size() == 1)
			{
				if (args[0].ref().data_type() == DataType::Int())
					return substring_by_index(ctx, std::move(args[0]));
				else if (args[0].ref().data_type() == DataType::IRange())
					return substring_by_range(ctx, std::move(args[0]));
				else if (args[0].ref().can_cast_implicit(DataType::Int()))
					return substring_by_index(ctx, std::move(args[0]));
				else if (args[0].ref().can_cast_implicit(DataType::IRange()))
					return substring_by_range(ctx, std::move(args[0]));
			}
		}

		ctx.throw_no_method(method, args);
	}

	void String::assign(const EvalContext& env, String&& o)
	{
		_value = std::move(o._value);
	}

	bool String::equals(const EvalContext& env, const String& o) const
	{
		return _value == o._value;
	}

	size_t String::iterlen(const EvalContext& env) const
	{
		return _value.size();
	}

	DataPoint String::iterget(const EvalContext& env, size_t i) const
	{
		return String({ _value[i] });
	}

	std::string String::page_content(const EvalContext& env) const
	{
		return _value;
	}

	std::string_view String::value() const
	{
		return _value;
	}

	std::string String::steal() &&
	{
		return std::move(_value);
	}
}
