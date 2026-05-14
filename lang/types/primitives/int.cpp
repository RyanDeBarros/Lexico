#include "int.h"

#include "include.h"
#include "runtime.h"
#include "find.h"

#include <charconv>

namespace lx
{
	Int::Int(int value)
		: _value(value)
	{
	}

	Int Int::make_from_literal(const EvalContext& env, std::string_view resolved)
	{
		int value;
		auto result = std::from_chars(resolved.data(), resolved.data() + resolved.size(), value);
		if (result.ec == std::errc())
			return Int(value);
		else
		{
			std::stringstream ss;
			ss << "could not convert \"" << resolved << "\" to " << DataType::Int();
			throw env.runtime_error(ss.str());
		}
	}
	
	DataType Int::data_type()
	{
		return DataType::Int();
	}

	TypeVariant Int::cast_copy(const VarContext& ctx, const DataType& type) const
	{
		switch (type.simple())
		{
		case SimpleType::Int:
			return *this;
		case SimpleType::Float:
			return Float(static_cast<float>(_value));
		case SimpleType::Bool:
			return Bool(static_cast<bool>(_value));
		case SimpleType::String:
			return String(std::to_string(_value));
		case SimpleType::IRange:
			return IRange(_value, _value);
		case SimpleType::Pattern:
			return Pattern::make_from_subpattern<SubpatternString>(std::to_string(_value));
		case SimpleType::Void:
			return Void();
		default:
			ctx.env.throw_bad_cast(data_type(), type);
		}
	}
	
	TypeVariant Int::cast_move(VarContext&& ctx, const DataType& type) &&
	{
		(void*)this; // ignore const warning
		return cast_copy(ctx, type);
	}

	void Int::print(const EvalContext& env, std::stringstream& ss) const
	{
		ss << _value;
	}

	Variable Int::data_member(VarContext& ctx, const std::string_view member) const
	{
		ctx.throw_no_data_member(member);
	}

	Variable Int::invoke_method(VarContext& ctx, const std::string_view method, std::vector<Variable>&& args)
	{
		ctx.throw_no_method(method, args);
	}

	void Int::assign(const EvalContext& env, Int&& o)
	{
		_value = o._value;
	}

	bool Int::equals(const EvalContext& env, const Int& o) const
	{
		return _value == o._value;
	}

	int Int::value() const
	{
		return _value;
	}
}
