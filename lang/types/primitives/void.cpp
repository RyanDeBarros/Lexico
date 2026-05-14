#include "void.h"

#include "include.h"
#include "runtime.h"

namespace lx
{
	DataType Void::data_type()
	{
		return DataType::Void();
	}

	TypeVariant Void::cast_copy(const VarContext& ctx, const DataType& type) const
	{
		if (type.simple() == SimpleType::Void)
			return Void();
		else
			ctx.env.throw_bad_cast(data_type(), type);
	}

	TypeVariant Void::cast_move(VarContext&& ctx, const DataType& type) &&
	{
		(void*)this; // ignore const warning
		return cast_copy(ctx, type);
	}

	void Void::print(const EvalContext& env, std::stringstream& ss) const
	{
		ss << "";
	}

	Variable Void::data_member(VarContext& ctx, const std::string_view member) const
	{
		ctx.throw_no_data_member(member);
	}

	Variable Void::invoke_method(VarContext& ctx, const std::string_view method, std::vector<Variable>&& args)
	{
		ctx.throw_no_method(method, args);
	}

	void Void::assign(const EvalContext& env, Void&& o)
	{
		// NOP
	}

	bool Void::equals(const EvalContext& env, const Void& o) const
	{
		return true;
	}
}
