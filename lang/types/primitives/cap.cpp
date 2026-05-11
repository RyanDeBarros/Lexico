#include "cap.h"

#include "include.h"
#include "runtime.h"

namespace lx
{
	DataType Cap::data_type()
	{
		return DataType::Cap();
	}

	TypeVariant Cap::cast_copy(const EvalContext& env, const DataType& type) const
	{
		if (type.simple() == SimpleType::Cap)
			return *this;
		else if (type.simple() == SimpleType::Void)
			return Void();
		else
			env.throw_bad_cast(data_type(), type);
	}

	TypeVariant Cap::cast_move(const EvalContext& env, const DataType& type)
	{
		if (type.simple() == SimpleType::Cap)
			return std::move(*this);
		else
		{
			(void*)this; // ignore const warning
			return cast_copy(env, type);
		}
	}

	void Cap::print(const EvalContext& env, std::stringstream& ss) const
	{
		// TODO v0.2 string representation of cap
		ss << DataType::Cap();
	}

	Variable Cap::data_member(VarContext& ctx, const std::string_view member) const
	{
		if (member == "exists")
		{
			// TODO
			return ctx.variable(Bool(true));
		}
		else if (member == "start")
		{
			// TODO
			return ctx.variable(Int(0));
		}
		else if (member == "end")
		{
			// TODO
			return ctx.variable(Int(0));
		}
		else if (member == "len")
		{
			// TODO
			return ctx.variable(Int(0));
		}
		else if (member == "range")
		{
			// TODO
			return ctx.variable(IRange(0, 0));
		}
		else if (member == "str")
		{
			// TODO
			return ctx.variable(String(""));
		}
		else if (member == "sub")
		{
			// TODO
			return ctx.variable(Match());
		}

		ctx.throw_no_data_member(member);
	}

	Variable Cap::invoke_method(VarContext& ctx, const std::string_view method, std::vector<Variable>&& args) const
	{
		ctx.throw_no_method(method, args);
	}

	void Cap::assign(const EvalContext& env, Cap&& o)
	{
		// TODO
	}

	bool Cap::equals(const EvalContext& env, const Cap& o) const
	{
		// TODO
		return false;
	}
}
