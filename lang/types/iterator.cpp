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

	DataPoint Iterator::get(const EvalContext& env) const
	{
		return _iterable.ref().iterget(env, _pos);
	}
}
