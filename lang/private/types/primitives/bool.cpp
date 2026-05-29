#include "bool.h"

#include "include.h"
#include "runtime.h"

namespace lx
{
	Bool::Bool(bool value)
		: _value(value)
	{
	}

	Bool Bool::make_from_literal(const EvalContext& env, std::string_view resolved)
	{
		if (resolved == "true")
			return Bool(true);
		else if (resolved == "false")
			return Bool(false);
		else
		{
			std::stringstream ss;
			ss << "could not convert \"" << resolved << "\" to " << DataType::Bool();
			throw env.runtime_error(ss.str());
		}
	}

	DataType Bool::data_type()
	{
		return DataType::Bool();
	}

	DataPoint Bool::cast_copy(const VarContext& ctx, const DataType& type) const
	{
		switch (type.simple())
		{
		case SimpleType::Int:
			return Int(static_cast<int>(_value));
		case SimpleType::Float:
			return Float(static_cast<float>(_value));
		case SimpleType::Bool:
			return Bool(*this);
		case SimpleType::String:
			return String(_value ? "true" : "false");
		case SimpleType::Void:
			return Void();
		default:
			ctx.env.throw_bad_cast(data_type(), type);
		}
	}

	DataPoint Bool::cast_move(VarContext&& ctx, const DataType& type) &&
	{
		(void*)this; // ignore const warning
		return cast_copy(ctx, type);
	}

	Variable Bool::pass_arg(VarContext ctx)
	{
		return ctx.self.heap().add(Bool(_value));
	}

	void Bool::print(const EvalContext& env, std::ostream& ss) const
	{
		ss << (_value ? "true" : "false");
	}

	Variable Bool::data_member(VarContext& ctx, const std::string_view member)
	{
		ctx.throw_no_data_member(member);
	}

	Variable Bool::invoke_method(VarContext& ctx, const std::string_view method, std::vector<Variable>&& args)
	{
		ctx.throw_no_method(method, args);
	}

	void Bool::assign(const EvalContext& env, Variable o)
	{
		_value = std::move(o).consume_as<Bool>(env)._value;
	}

	bool Bool::equals(const EvalContext& env, Variable o) const
	{
		return _value == std::move(o).cast(env, data_type()).ref().get<Bool>()._value;
	}

	bool Bool::value() const
	{
		return _value;
	}
}
