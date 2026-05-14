#include "variable.h"

#include "heap.h"

namespace lx
{
	Variable::Variable(VirtualHeap& heap, unsigned int id)
		: _heap(&heap), _id(id)
	{
	}
	\
	Variable::Variable(const Variable& other)
		: _heap(other._heap), _id(other._id)
	{
		increment();
	}

	Variable::Variable(Variable&& other) noexcept
		: _heap(other._heap), _id(other._id)
	{
		other._heap = nullptr;
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
			increment();
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
			other._heap = nullptr;
		}
		return *this;
	}

	Variable Variable::root() const
	{
		if (_heap)
		{
			increment();
			return Variable(*_heap, _id);
		}
		else
			throw LxError(ErrorType::Internal, "heap reference is null");
	}
	
	void Variable::increment() const
	{
		if (_heap)
			_heap->increment_ref_count(_id);
	}
	
	void Variable::decrement() const
	{
		if (_heap)
			_heap->decrement_ref_count(_id);
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

	bool Variable::temporary() const
	{
		if (_heap)
		{
			return _heap->unbound(_id) && !_heap->get(_id).data_type().is_view();
		}
		else
			throw LxError(ErrorType::Internal, "heap reference is null");
	}

	DataPoint Variable::cast(const EvalContext& env, const DataType& to) &&
	{
		VarContext ctx(env, std::move(*this));
		DataPoint& me = ctx.self.ref();

		if (ctx.self.unbound())
			return std::move(me).cast_move(std::move(ctx), to);
		else
			return me.cast_copy(ctx, to);
	}

	size_t Variable::hash() const
	{
		return std::hash<unsigned int>{}(_id);
	}

	Variable Variable::data_member(const EvalContext& env, const std::string_view member)
	{
		VarContext ctx(env, *this);
		return ref().data_member(ctx, member);
	}

	Variable Variable::invoke_method(const EvalContext& env, const std::string_view method, std::vector<Variable>&& args)
	{
		VarContext ctx(env, *this);
		return ref().invoke_method(ctx, method, std::move(args));
	}
}

size_t std::hash<lx::Variable>::operator()(const lx::Variable& var) const
{
	return var.hash();
}
