#include "iterator.h"

#include "datapoint.h"

namespace lx
{
	Iterator::Iterator(const Variable& iterable)
		: _iterable(iterable)
	{
	}

	bool Iterator::done() const
	{
		return _pos >= _iterable.ref().iterlen();
	}
	
	void Iterator::next()
	{
		++_pos;
	}

	DataPoint Iterator::get() const
	{
		return _iterable.ref().iterget(_pos);
	}
}
