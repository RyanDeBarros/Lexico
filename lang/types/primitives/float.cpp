#include "float.h"

#include "include.h"
#include "runtime.h"

#include <charconv>

namespace lx
{
	Float::Float(float value)
		: _value(value)
	{
	}

	Float Float::make_from_literal(std::string_view resolved)
	{
		float value;
		auto result = std::from_chars(resolved.data(), resolved.data() + resolved.size(), value);
		if (result.ec == std::errc())
			return Float(value);
		else
		{
			std::stringstream ss;
			ss << "could not convert \"" << resolved << "\" to " << DataType::Float();
			throw LxError(ErrorType::Runtime, ss.str());
		}
	}

	DataType Float::data_type()
	{
		return DataType::Float();
	}

	TypeVariant Float::cast_copy(const DataType& type) const
	{
		switch (type.simple())
		{
		case SimpleType::Int:
			return Int(static_cast<int>(_value));
		case SimpleType::Float:
			return *this;
		case SimpleType::Bool:
			return Bool(static_cast<bool>(_value));
		case SimpleType::String:
			return String(std::to_string(_value));
		case SimpleType::Pattern:
			Pattern::make_from_subpattern<SubpatternString>(std::to_string(_value));
		case SimpleType::Void:
			return Void();
		default:
			throw_bad_cast(data_type(), type);
		}
	}

	TypeVariant Float::cast_move(const DataType& type)
	{
		(void*)this; // ignore const warning
		return cast_copy(type);
	}

	void Float::print(std::stringstream& ss) const
	{
		ss << _value;
	}

	Variable Float::data_member(VarContext& ctx, const std::string_view member) const
	{
		ctx.throw_no_data_member(member);
	}

	Variable Float::invoke_method(VarContext& ctx, const std::string_view method, std::vector<Variable>&& args) const
	{
		ctx.throw_no_method(method, args);
	}

	bool Float::equals(const Float& o) const
	{
		return _value == o._value;
	}

	float Float::value() const
	{
		return _value;
	}
}
