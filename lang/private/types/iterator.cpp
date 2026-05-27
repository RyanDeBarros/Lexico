#include "iterator.h"

#include "datapoint.h"
#include "evalcontext.h"

namespace lx
{
	Iterator::Iterator(Variable iterable)
		: _iterable(iterable)
	{
	}

	bool Iterator::done(const EvalContext& env) const
	{
		return _pos >= _iterable.ref().iterlen(env);
	}
	
	void Iterator::next()
	{
		++_pos;
	}

	Variable Iterator::get(const EvalContext& env) const
	{
		VarContext ctx(env, _iterable);
		return _iterable.ref().iterget(ctx, _pos);
	}
}
