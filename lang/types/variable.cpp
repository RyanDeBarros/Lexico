#include "variable.h"

#include "heap.h"

namespace lx
{
	Variable::Variable(DataHeap& heap, unsigned int id, bool temporary)
		: _heap(&heap), _id(id), _temporary(temporary), _ref_count(std::make_shared<unsigned int>(1))
	{
	}

	Variable::Variable(const Variable& other)
		: _heap(other._heap), _id(other._id), _temporary(other._temporary), _ref_count(other._ref_count)
	{
		if (_ref_count)
			++*_ref_count;
	}

	Variable::Variable(Variable&& other) noexcept
		: _heap(other._heap), _id(other._id), _temporary(other._temporary), _ref_count(std::move(other._ref_count))
	{
	}

	Variable::~Variable()
	{
		decrement();
	}

	Variable& Variable::operator=(const Variable& other)
	{
		if (this != &other)
		{
			decrement();
			_heap = other._heap;
			_id = other._id;
			_temporary = other._temporary;
			_ref_count = other._ref_count;
			if (_ref_count)
				++*_ref_count;
		}
		return *this;
	}

	Variable& Variable::operator=(Variable&& other) noexcept
	{
		if (this != &other)
		{
			decrement();
			_heap = other._heap;
			_id = other._id;
			_temporary = other._temporary;
			_ref_count = std::move(other._ref_count);
		}
		return *this;
	}

	void Variable::decrement()
	{
		if (_ref_count)
		{
			--*_ref_count;
			if (*_ref_count == 0)
			{
				_heap->remove(_id);
				_ref_count.reset();
			}
		}
	}

	const DataPoint& Variable::ref() const
	{
		if (_ref_count)
			return _heap->ref(_id);
		else
			throw LxError(ErrorType::Internal, "variable reference count is null");
	}

	DataPoint& Variable::ref()
	{
		if (_ref_count)
			return _heap->ref(_id);
		else
			throw LxError(ErrorType::Internal, "variable reference count is null");
	}

	DataPoint Variable::dp()
	{
		if (_ref_count)
		{
			if (_temporary)
			{
				if (*_ref_count != 1)
					throw LxError(ErrorType::Internal, "variable temporary reference count is not 1");

				DataPoint dp = std::move(_heap->ref(_id));
				decrement();
				return dp;
			}
			else
				return _heap->ref(_id);
		}
		else
			throw LxError(ErrorType::Internal, "variable reference count is null");
	}

	bool Variable::temporary() const
	{
		return _temporary;
	}

	size_t Variable::hash() const
	{
		return std::hash<unsigned int>{}(_id);
	}
}

size_t std::hash<lx::Variable>::operator()(const lx::Variable& var) const
{
	return var.hash();
}
