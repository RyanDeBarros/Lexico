#include "list.h"

#include "include.h"
#include "runtime.h"
#include "constants.h"

namespace lx
{
	// TODO ListView

	static DataType underlying_of(const std::vector<Variable>& elements, const ScriptSegment* segment)
	{
		if (elements.empty())
		{
			if (segment)
				throw LxError::segment_error(*segment, ErrorType::Runtime, "unresolved list cannot initialize with no elements");
			else
				return DataType::Void();
		}

		std::vector<LxError> errors;
		DataType underlying = elements[0].ref().data_type();
		for (size_t i = 1; i < elements.size(); ++i)
		{
			if (elements[i].ref().data_type() != underlying)
			{
				std::stringstream ss;
				ss << __FUNCTION__ << ": element [" << i << "] has type " << elements[i].ref().data_type() << ", which doesn't match type of first element: " << underlying;
				if (segment)
					errors.push_back(LxError::segment_error(*segment, ErrorType::Runtime, ss.str()));
				else
					return DataType::Void();
			}
		}

		if (errors.empty())
			return underlying;
		else
			throw LxErrorList(errors);
	}

	List::List(const DataType& underlying)
		: _underlying(underlying)
	{
	}

	List::List(DataType&& underlying)
		: _underlying(std::move(underlying))
	{
	}

	List::List(std::vector<Variable>&& elements)
		: _underlying(underlying_of(elements, nullptr))
	{
		_elements = std::move(elements);
	}

	List::List(const DataType& underlying, const ScriptSegment& segment)
		: _underlying(underlying)
	{
		if (_underlying.simple() == SimpleType::Void)
			throw LxError::segment_error(segment, ErrorType::Runtime, "list cannot have void underlying type");
	}

	List::List(DataType&& underlying, const ScriptSegment& segment)
		: _underlying(std::move(underlying))
	{
		if (_underlying.simple() == SimpleType::Void)
			throw LxError::segment_error(segment, ErrorType::Runtime, "list cannot have void underlying type");
	}

	List::List(std::vector<Variable>&& elements, const ScriptSegment& segment)
		: _underlying(underlying_of(elements, &segment))
	{
		_elements = std::move(elements);
	}

	List List::make_nonvoid_list(const DataType& underlying)
	{
		return List(underlying);
	}

	List List::make_nonvoid_list(DataType&& underlying)
	{
		return List(std::move(underlying));
	}

	List List::make_nonvoid_list(std::vector<Variable>&& elements)
	{
		return List(std::move(elements));
	}

	DataType List::data_type() const
	{
		return DataType::List(_underlying);
	}

	TypeVariant List::cast_copy(const VarContext& ctx, const DataType& type) const
	{
		if (type == DataType::List(_underlying))
			return *this;
		else if (type.simple() == SimpleType::Void)
			return Void();
		else
			ctx.env.throw_bad_cast(data_type(), type);
	}

	TypeVariant List::cast_move(VarContext&& ctx, const DataType& type) &&
	{
		if (type == DataType::List(_underlying))
			return std::move(*this);
		else
			return cast_copy(ctx, type);
	}

	void List::print(const EvalContext& env, std::stringstream& ss) const
	{
		ss << "[";
		for (size_t i = 0; i < _elements.size(); ++i)
		{
			_elements[i].ref().print(env, ss);
			if (i + 1 < _elements.size())
				ss << ", ";
		}
		ss << "]";
	}

	Variable List::data_member(VarContext& ctx, const std::string_view member) const
	{
		if (member == "len")
			return ctx.variable(Int(_elements.size()));

		ctx.throw_no_data_member(member);
	}

	Variable List::invoke_method(VarContext& ctx, const std::string_view method, std::vector<Variable>&& args) const
	{
		if (method == constants::SUBSCRIPT_OP)
		{
			if (args.size() == 1)
			{
				if (args[0].ref().data_type().simple() == SimpleType::Int)
					return _elements[std::move(args[0]).consume_as<Int>(ctx.env).value()];
			}
		}

		ctx.throw_no_method(method, args);
	}

	void List::assign(const EvalContext& env, List&& o)
	{
		_elements = std::move(o._elements);
	}

	bool List::equals(const EvalContext& env, const List& o) const
	{
		if (_elements.size() != o._elements.size())
			return false;
		
		for (size_t i = 0; i < _elements.size(); ++i)
			if (!_elements[i].ref().equals(env, o._elements[i]))
				return false;

		return true;
	}

	size_t List::iterlen(const EvalContext& env) const
	{
		return _elements.size();
	}

	DataPoint List::iterget(const EvalContext& env, size_t i) const
	{
		return _elements[i].ref();
	}

	bool List::push(Variable element)
	{
		if (element.ref().data_type() == _underlying)
		{
			_elements.push_back(std::move(element));
			return true;
		}
		else
			return false;
	}
}
