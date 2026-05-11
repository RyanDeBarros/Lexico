#include "variable.h"

#include "heap.h"

namespace lx
{
	Variable::Variable(VirtualHeap& heap, unsigned int id)
		: _heap(&heap), _id(id)
	{
	}

	Variable::Variable(VirtualHeap& heap, unsigned int id, unsigned int path)
		: _heap(&heap), _id(id), _path(path)
	{
	}

	Variable::Variable(const Variable& other)
		: _heap(other._heap), _id(other._id), _path(other._path)
	{
		increment();
	}

	Variable::Variable(Variable&& other) noexcept
		: _heap(other._heap), _id(other._id), _path(other._path)
	{
		other._heap = nullptr;
		other._path = 0;
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
			_path = other._path;
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
			_path = other._path;
			other._heap = nullptr;
			other._path = 0;
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

	Variable Variable::subpath(DataPath&& path) const
	{
		if (_heap)
		{
			increment();
			if (DataPath* old_path = _heap->get_path(_path))
				path.steps.insert(path.steps.begin(), old_path->steps.begin(), old_path->steps.end());
			return Variable(*_heap, _id, _heap->new_path(std::move(path)));
		}
		else
			throw LxError(ErrorType::Internal, "heap reference is null");
	}
	
	void Variable::increment() const
	{
		if (_heap)
		{
			_heap->increment_var_ref_count(_id);
			_heap->increment_path_ref_count(_path);
		}
	}
	
	void Variable::decrement() const
	{
		if (_heap)
		{
			_heap->decrement_var_ref_count(_id);
			_heap->decrement_path_ref_count(_path);
		}
	}

	const DataPoint& Variable::ref() const
	{
		if (_heap)
			return _heap->get_var(_id);
		else
			throw LxError(ErrorType::Internal, "heap reference is null");
	}

	DataPoint& Variable::ref()
	{
		if (_heap)
			return _heap->get_var(_id);
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

	const DataPath* Variable::path() const
	{
		if (_heap)
			return _heap->get_path(_path);
		else
			throw LxError(ErrorType::Internal, "heap reference is null");
	}

	DataPath* Variable::path()
	{
		if (_heap)
			return _heap->get_path(_path);
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

	Variable Variable::data_member(const EvalContext& env, const std::string_view member) const
	{
		// TODO use path
		VarContext ctx(env, *this);
		return ref().data_member(ctx, member);
	}

	Variable Variable::invoke_method(const EvalContext& env, const std::string_view method, std::vector<Variable>&& args) const
	{
		// TODO use path
		VarContext ctx(env, *this);
		return ref().invoke_method(ctx, method, std::move(args));
	}
}

size_t std::hash<lx::Variable>::operator()(const lx::Variable& var) const
{
	return var.hash();
}
