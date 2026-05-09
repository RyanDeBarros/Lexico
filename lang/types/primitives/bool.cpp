#include "bool.h"

#include "include.h"
#include "runtime.h"

namespace lx
{
	Bool::Bool(bool value)
		: _value(value)
	{
	}

	Bool Bool::make_from_literal(std::string_view resolved)
	{
		if (resolved == "true")
			return Bool(true);
		else if (resolved == "false")
			return Bool(false);
		else
		{
			std::stringstream ss;
			ss << "could not convert \"" << resolved << "\" to " << DataType::Bool();
			throw LxError(ErrorType::Runtime, ss.str());
		}
	}

	DataType Bool::data_type()
	{
		return DataType::Bool();
	}

	TypeVariant Bool::cast_copy(const DataType& type) const
	{
		switch (type.simple())
		{
		case SimpleType::Int:
			return Int(static_cast<int>(_value));
		case SimpleType::Float:
			return Float(static_cast<float>(_value));
		case SimpleType::Bool:
			return *this;
		case SimpleType::String:
			return String(_value ? "true" : "false");
		case SimpleType::Void:
			return Void();
		default:
			throw_bad_cast(data_type(), type);
		}
	}

	TypeVariant Bool::cast_move(const DataType& type)
	{
		(void*)this; // ignore const warning
		return cast_copy(type);
	}

	void Bool::print(std::stringstream& ss) const
	{
		ss << (_value ? "true" : "false");
	}

	Variable Bool::data_member(VarContext& ctx, const std::string_view member) const
	{
		ctx.throw_no_data_member(member);
	}

	Variable Bool::invoke_method(VarContext& ctx, const std::string_view method, std::vector<Variable>&& args) const
	{
		ctx.throw_no_method(method, args);
	}

	bool Bool::equals(const Bool& o) const
	{
		return _value == o._value;
	}

	bool Bool::value() const
	{
		return _value;
	}
}
