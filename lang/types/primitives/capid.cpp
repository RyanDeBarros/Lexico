#include "capid.h"

#include "include.h"
#include "runtime.h"

namespace lx
{
	CapId::CapId(unsigned int uid)
		: _uid(uid)
	{
	}

	CapId CapId::make_from_literal(const EvalContext& env, std::string_view resolved)
	{
		return env.runtime.capture_id(resolved);
	}

	DataType CapId::data_type()
	{
		return DataType::CapId();
	}

	TypeVariant CapId::cast_copy(const VarContext& ctx, const DataType& type) const
	{
		if (type.simple() == SimpleType::CapId)
			return *this;
		else if (type.simple() == SimpleType::Void)
			return Void();
		else
			ctx.env.throw_bad_cast(data_type(), type);
	}

	TypeVariant CapId::cast_move(VarContext&& ctx, const DataType& type) &&
	{
		(void*)this; // ignore const warning
		return cast_copy(ctx, type);
	}

	void CapId::print(const EvalContext& env, std::stringstream& ss) const
	{
		ss << DataType::CapId();
	}

	Variable CapId::data_member(VarContext& ctx, const std::string_view member) const
	{
		ctx.throw_no_data_member(member);
	}

	Variable CapId::invoke_method(VarContext& ctx, const std::string_view method, std::vector<Variable>&& args)
	{
		ctx.throw_no_method(method, args);
	}

	void CapId::assign(const EvalContext& env, CapId&& o)
	{
		_uid = o._uid;
	}

	bool CapId::equals(const EvalContext& env, const CapId& o) const
	{
		return _uid == o._uid;
	}
	
	size_t CapId::hash() const
	{
		return std::hash<unsigned int>{}(_uid);
	}
}

size_t std::hash<lx::CapId>::operator()(const lx::CapId& c) const
{
	return c.hash();
}
