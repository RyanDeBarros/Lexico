#include "list.h"

#include "include.h"
#include "runtime.h"
#include "constants.h"

namespace lx
{
	// TODO ListView - maybe even modify StringView to become a generic View. At the very least, define generic View for List and Matches
	// TODO v0.2 negative indexes - even in StringView

	static DataType underlying_of(const EvalContext& env, const std::vector<Variable>& elements)
	{
		if (elements.empty())
			throw env.runtime_error("unresolved list cannot initialize with no elements");

		std::vector<LxError> errors;
		DataType underlying = elements[0].ref().data_type();
		for (size_t i = 1; i < elements.size(); ++i)
		{
			if (elements[i].ref().data_type() != underlying)
			{
				std::stringstream ss;
				ss << "element [" << i << "] has type " << elements[i].ref().data_type() << ", which doesn't match type of first element: " << underlying;
				errors.push_back(env.runtime_error(ss.str()));
			}
		}

		if (errors.empty())
			return underlying;
		else
			throw LxErrorList(errors);
	}

	List::List(const EvalContext& env, const DataType& underlying)
		: _underlying(underlying)
	{
		if (_underlying.simple() == SimpleType::Void)
			throw env.runtime_error("list cannot have void underlying type");
	}

	List::List(const EvalContext& env, DataType&& underlying)
		: _underlying(std::move(underlying))
	{
		if (_underlying.simple() == SimpleType::Void)
			throw env.runtime_error("list cannot have void underlying type");
	}

	List::List(const EvalContext& env, std::vector<Variable>&& elements)
		: _underlying(underlying_of(env, elements))
	{
		_elements = std::move(elements);
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

	StringMap<MemberSignature> List::members()
	{
		return {
			{ constants::MEMBER_LEN, MemberSignature::make_data(constants::MEMBER_LEN, DataType::Int()) },
		};
	}

	StringMap<MemberSignature> List::members(const DataType& underlying)
	{
		return {
			{ constants::SUBSCRIPT_OP, MemberSignature::make_method(constants::SUBSCRIPT_OP, {
				{ .return_type = underlying, .arg_types = { DataType::Int() } },
			}) },
			{ constants::MEMBER_PUSH, MemberSignature::make_method(constants::MEMBER_PUSH, {
				{ .return_type = DataType::Void(), .arg_types = { underlying } },
				{ .return_type = DataType::Void(), .arg_types = { DataType::Int(), underlying } },
			}) },
			{ constants::MEMBER_POP, MemberSignature::make_method(constants::MEMBER_POP, {
				{ .return_type = underlying, .arg_types = {} },
				{ .return_type = underlying, .arg_types = { DataType::Int() } },
			}) },
		};
	}

	Variable List::data_member(VarContext& ctx, const std::string_view member) const
	{
		if (member == constants::MEMBER_LEN)
			return ctx.variable(Int(size()));

		ctx.throw_no_data_member(member);
	}

	Variable List::invoke_method(VarContext& ctx, const std::string_view method, std::vector<Variable>&& args)
	{
		if (method == constants::SUBSCRIPT_OP)
		{
			if (args.size() == 1)
			{
				if (args[0].ref().data_type().simple() == SimpleType::Int)
					return _elements[std::move(args[0]).consume_as<Int>(ctx.env).value()];
			}
		}
		else if (method == constants::MEMBER_PUSH)
		{
			if (args.size() == 1)
			{
				push(ctx.env, std::move(args[0]));
				return ctx.variable(Void());
			}
			else if (args.size() == 2)
			{
				if (args[0].ref().data_type().simple() == SimpleType::Int)
				{
					int index = std::move(args[0]).consume_as<Int>(ctx.env).value();
					insert(ctx.env, index, std::move(args[1]));
					return ctx.variable(Void());
				}
			}
		}
		else if (method == constants::MEMBER_POP)
		{
			if (args.size() == 0)
				return pop(ctx.env);
			else if (args.size() == 1)
			{
				if (args[0].ref().data_type().simple() == SimpleType::Int)
					return remove(ctx.env, std::move(args[0]).consume_as<Int>(ctx.env).value());
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

	void List::push(const EvalContext& env, Variable element)
	{
		if (element.ref().data_type() == _underlying)
			_elements.push_back(std::move(element));
		else
		{
			std::stringstream ss;
			ss << "cannot push: list underlying type is " << _underlying << ", but element resolved to " << element.ref().data_type();
			throw env.runtime_error(ss.str());
		}
	}

	void List::insert(const EvalContext& env, size_t i, Variable element)
	{
		if (element.ref().data_type() == _underlying)
		{
			if (i <= _elements.size())
				_elements.insert(_elements.begin() + i, std::move(element));
			else
			{
				std::stringstream ss;
				ss << "cannot insert: index " << i << " out of range for list of size " << _elements.size();
				throw env.runtime_error(ss.str());
			}
		}
		else
		{
			std::stringstream ss;
			ss << "cannot insert: list underlying type is " << _underlying << ", but element resolved to " << element.ref().data_type();
			throw env.runtime_error(ss.str());
		}
	}

	Variable List::pop(const EvalContext& env)
	{
		if (!_elements.empty())
		{
			Variable back = std::move(_elements.back());
			_elements.pop_back();
			return back;
		}
		else
			throw env.runtime_error("cannot pop: list is empty");
	}

	Variable List::remove(const EvalContext& env, size_t i)
	{
		if (i < _elements.size())
		{
			Variable removed = std::move(_elements[i]);
			_elements.erase(_elements.begin() + i);
			return removed;
		}
		else
		{
			std::stringstream ss;
			ss << "cannot remove: index " << i << " out of range for list of size " << _elements.size();
			throw env.runtime_error(ss.str());
		}
	}

	size_t List::size() const
	{
		return _elements.size();
	}

	const Variable& List::operator[](size_t i) const
	{
		return _elements[i];
	}

	Variable& List::operator[](size_t i)
	{
		return _elements[i];
	}
}
