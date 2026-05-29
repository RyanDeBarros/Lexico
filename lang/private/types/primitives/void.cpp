#include "void.h"

#include "include.h"
#include "runtime.h"

namespace lx
{
	DataType Void::data_type()
	{
		return DataType::Void();
	}

	DataPoint Void::cast_copy(const VarContext& ctx, const DataType& type) const
	{
		if (type.simple() == SimpleType::Void)
			return Void();
		else
			ctx.env.throw_bad_cast(data_type(), type);
	}

	DataPoint Void::cast_move(VarContext&& ctx, const DataType& type) &&
	{
		(void*)this; // ignore const warning
		return cast_copy(ctx, type);
	}

	Variable Void::pass_arg(VarContext ctx)
	{
		return ctx.self.heap().add(Void());
	}

	void Void::print(const EvalContext& env, std::ostream& ss) const
	{
		ss << "";
	}

	Variable Void::data_member(VarContext& ctx, const std::string_view member)
	{
		ctx.throw_no_data_member(member);
	}

	Variable Void::invoke_method(VarContext& ctx, const std::string_view method, std::vector<Variable>&& args)
	{
		ctx.throw_no_method(method, args);
	}

	void Void::assign(const EvalContext& env, Variable o)
	{
		// NOP
	}

	bool Void::equals(const EvalContext& env, Variable o) const
	{
		return true;
	}
}
