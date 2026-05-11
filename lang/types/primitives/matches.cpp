#include "matches.h"

#include "include.h"
#include "runtime.h"

namespace lx
{
	DataType Matches::data_type()
	{
		return DataType::Matches();
	}

	TypeVariant Matches::cast_copy(const EvalContext& env, const DataType& type) const
	{
		if (type.simple() == SimpleType::Matches)
			return *this;
		else if (type.simple() == SimpleType::Void)
			return Void();
		else
			env.throw_bad_cast(data_type(), type);
	}

	TypeVariant Matches::cast_move(const EvalContext& env, const DataType& type)
	{
		if (type.simple() == SimpleType::Matches)
			return std::move(*this);
		else
			return cast_copy(env, type);
	}

	void Matches::print(const EvalContext& env, std::stringstream& ss) const
	{
		// TODO v0.2 string representation of matches
		ss << DataType::Matches();
	}

	Variable Matches::data_member(VarContext& ctx, const std::string_view member) const
	{
		ctx.throw_no_data_member(member);
	}

	Variable Matches::invoke_method(VarContext& ctx, const std::string_view method, std::vector<Variable>&& args) const
	{
		ctx.throw_no_method(method, args);
	}

	void Matches::assign(const EvalContext& env, Matches&& o)
	{
		// TODO
	}

	bool Matches::equals(const EvalContext& env, const Matches& o) const
	{
		// TODO
		return false;
	}

	size_t Matches::iterlen(const EvalContext& env) const
	{
		// TODO
		return 0;
	}

	DataPoint Matches::iterget(const EvalContext& env, size_t i) const
	{
		// TODO
		return Match();
	}
}
