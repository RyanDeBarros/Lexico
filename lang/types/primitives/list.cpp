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

	// TODO members to push, pop, insert, remove
	// TODO '+' operator to combine lists
	// TODO v0.2 compound assignment operators

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
				{.return_type = underlying, .arg_types = { DataType::Int() }},
			}) },
		};
	}

	Variable List::data_member(VarContext& ctx, const std::string_view member) const
	{
		if (member == constants::MEMBER_LEN)
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

	size_t List::size() const
	{
		return _elements.size();
	}

	const Variable& List::operator[](size_t i) const
	{
		return _elements[i];
	}
}
