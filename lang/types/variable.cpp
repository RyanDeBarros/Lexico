#include "variable.h"

#include "heap.h"

namespace lx
{
	Variable::Variable(VirtualHeap& heap, unsigned int id)
		: _heap(&heap), _id(id)
	{
	}

	Variable::Variable(const Variable& other)
		: _heap(other._heap), _id(other._id)
	{
		if (_heap)
			_heap->increment_ref_count(_id);
	}

	Variable::Variable(Variable&& other) noexcept
		: _heap(other._heap), _id(other._id)
	{
		other._heap = nullptr;
	}

	Variable::~Variable()
	{
		if (_heap)
			_heap->decrement_ref_count(_id);
	}

	Variable& Variable::operator=(const Variable& other)
	{
		if (this != &other)
		{
			if (_heap)
				_heap->decrement_ref_count(_id);
			_heap = other._heap;
			_id = other._id;
			if (_heap)
				_heap->increment_ref_count(_id);
		}
		return *this;
	}

	Variable& Variable::operator=(Variable&& other) noexcept
	{
		if (this != &other)
		{
			if (_heap)
				_heap->decrement_ref_count(_id);
			_heap = other._heap;
			_id = other._id;
			other._heap = nullptr;
		}
		return *this;
	}

	const DataPoint& Variable::ref() const
	{
		if (_heap)
			return _heap->get(_id);
		else
			throw LxError(ErrorType::Internal, "heap reference is null");
	}

	DataPoint& Variable::ref()
	{
		if (_heap)
			return _heap->get(_id);
		else
			throw LxError(ErrorType::Internal, "heap reference is null");
	}

	DataPoint Variable::dp()
	{
		if (_heap)
		{
			DataPoint dp = _heap->dp(_id);
			_heap = nullptr;
			return dp;
		}
		else
			throw LxError(ErrorType::Internal, "heap reference is null");
	}

	bool Variable::unbound() const
	{
		if (_heap)
			return _heap->unbound(_id);
		else
			throw LxError(ErrorType::Internal, "heap reference is null");
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
