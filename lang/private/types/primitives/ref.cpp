#include "ref.h"

#include "include.h"
#include "runtime.h"

// TODO all primitives should allow for cast_copy/cast_move to ref[primitive]

namespace lx
{
	Ref::Ref(Variable subobject)
		: _subobject(subobject)
	{
	}

	DataType Ref::data_type() const
	{
		return DataType::Ref(_subobject.ref().data_type());
	}

	// TODO cast to underlying to copy by value
	
	TypeVariant Ref::cast_copy(const VarContext& ctx, const DataType& type) const
	{
		if (type == data_type())
			return Ref(*this);
		else
			ctx.env.throw_bad_cast(data_type(), type);
	}
	
	TypeVariant Ref::cast_move(VarContext&& ctx, const DataType& type) &&
	{
		(void*)this; // ignore const warning
		return cast_copy(ctx, type);
	}
	
	Variable Ref::pass_arg(VarContext ctx)
	{
		return ctx.self;
	}
	
	void Ref::print(const EvalContext& env, std::ostream& ss) const
	{
		ss << '@';
		_subobject.ref().print(env, ss);
	}
	
	Variable Ref::data_member(VarContext& ctx, const std::string_view member)
	{
		return root().data_member(ctx, member);
	}
	
	Variable Ref::invoke_method(VarContext& ctx, const std::string_view method, std::vector<Variable>&& args)
	{
		return root().invoke_method(ctx, method, std::move(args));
	}
	
	void Ref::assign(const EvalContext& env, Variable o)
	{
		_subobject.ref().assign(env, std::move(o));
	}
	
	bool Ref::equals(const EvalContext& env, Variable o) const
	{
		return _subobject.ref().equals(env, o);
	}

	size_t Ref::iterlen(const EvalContext& env) const
	{
		return _subobject.ref().iterlen(env);
	}

	Variable Ref::iterget(VarContext& ctx, size_t i) const
	{
		return _subobject.ref().iterget(ctx, i);
	}

	const DataPoint& Ref::val() const
	{
		return _subobject.ref();
	}

	DataPoint& Ref::val()
	{
		return _subobject.ref();
	}

	const DataPoint& Ref::root() const
	{
		const DataPoint* sub = &val();
		while (sub->data_type().simple() == SimpleType::Ref)
			sub = &sub->get<Ref>().val();
		return *sub;
	}

	DataPoint& Ref::root()
	{
		DataPoint* sub = &val();
		while (sub->data_type().simple() == SimpleType::Ref)
			sub = &sub->get<Ref>().val();
		return *sub;
	}
}
