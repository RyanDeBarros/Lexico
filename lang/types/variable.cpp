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

	DataPoint Variable::consume() &&
	{
		if (_heap)
		{
			DataPoint dp = _heap->detach(_id);
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

	Variable Variable::data_member(Runtime& env, const ScriptSegment& segment, const std::string_view member) const
	{
		return ref().data_member(*this, env, segment, member);
	}

	Variable Variable::invoke_method(Runtime& env, const ScriptSegment& segment, const std::string_view method, std::vector<Variable>&& args) const
	{
		return ref().invoke_method(*this, env, segment, method, std::move(args));
	}
}

size_t std::hash<lx::Variable>::operator()(const lx::Variable& var) const
{
	return var.hash();
}
