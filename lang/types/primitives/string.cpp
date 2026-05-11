#include "string.h"

#include "include.h"
#include "runtime.h"
#include "constants.h"

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
	
	// TODO add StringView to cast_copy, but not cast_move?

	TypeVariant String::cast_copy(const EvalContext& env, const DataType& type) const
	{
		switch (type.simple())
		{
		case SimpleType::Int:
			return Int::make_from_literal(env, _value);
		case SimpleType::Float:
			return Float::make_from_literal(env, _value);
		case SimpleType::Bool:
			return Bool::make_from_literal(env, _value);
		case SimpleType::String:
			return *this;
		case SimpleType::Pattern:
			return Pattern::make_from_subpattern<SubpatternString>(_value);
		case SimpleType::Void:
			return Void();
		default:
			env.throw_bad_cast(data_type(), type);
		}
	}

	TypeVariant String::cast_move(const EvalContext& env, const DataType& type)
	{
		if (type.simple() == SimpleType::String)
			return std::move(*this);
		else if (type.simple() == SimpleType::Pattern)
			return Pattern::make_from_subpattern<SubpatternString>(std::move(_value));
		else
			return cast_copy(env, type);
	}

	void String::print(const EvalContext& env, std::stringstream& ss) const
	{
		ss << _value;
	}

	Variable String::data_member(VarContext& ctx, const std::string_view member) const
	{
		if (member == "len")
			return ctx.variable(Int(_value.size()));

		ctx.throw_no_data_member(member);
	}

	Variable String::invoke_method(VarContext& ctx, const std::string_view method, std::vector<Variable>&& args) const
	{
		if (method == constants::SUBSCRIPT_OP)
		{
			if (args.size() == 1)
			{
				// TODO if exactly Int -> use Int path. else if exactly IRange -> use IRange path. else if implicitly castable to either, use respective path.

				if (args[0].ref().data_type() == DataType::Int())
				{
					StringView sv(ctx.env, ctx.self, std::move(args[0]).consume_as<Int>(ctx.env));
					sv.assert_valid(ctx.env);
					return ctx.variable(std::move(sv));
				}
				else if (args[0].ref().data_type() == DataType::IRange())
				{
					StringView sv(ctx.env, ctx.self, std::move(args[0]).consume_as<IRange>(ctx.env));
					sv.assert_valid(ctx.env);
					return ctx.variable(std::move(sv));
				}
			}
		}

		ctx.throw_no_method(method, args);
	}

	void String::assign(const EvalContext& env, String&& o)
	{
		// TODO
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
}
