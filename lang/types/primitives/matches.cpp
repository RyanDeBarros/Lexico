#include "matches.h"

#include "include.h"
#include "runtime.h"
#include "constants.h"

namespace lx
{
	DataType Matches::data_type()
	{
		return DataType::Matches();
	}

	TypeVariant Matches::cast_copy(const VarContext& ctx, const DataType& type) const
	{
		if (type.simple() == SimpleType::Matches)
			return *this;
		else if (type.simple() == SimpleType::Void)
			return Void();
		else if (type == DataType::List(DataType::Match()))
			return List(ctx.env, std::vector(_matches.begin(), _matches.end()));
		else
			ctx.env.throw_bad_cast(data_type(), type);
	}

	TypeVariant Matches::cast_move(VarContext&& ctx, const DataType& type) &&
	{
		if (type.simple() == SimpleType::Matches)
			return std::move(*this);
		else if (type.simple() == SimpleType::Void)
			return Void();
		else if (type == DataType::List(DataType::Match()))
			return List(ctx.env, std::vector(std::make_move_iterator(_matches.begin()), std::make_move_iterator(_matches.end())));
		else
			ctx.env.throw_bad_cast(data_type(), type);
	}

	void Matches::print(const EvalContext& env, std::stringstream& ss) const
	{
		// TODO v0.2 string representation of matches
		ss << DataType::Matches();
	}

	StringMap<MemberSignature> Matches::members()
	{
		return {
			{ constants::MEMBER_LEN, MemberSignature::make_data(constants::MEMBER_LEN, DataType::Int()) },
			{ constants::SUBSCRIPT_OP, MemberSignature::make_method(constants::SUBSCRIPT_OP, {
				{ .return_type = DataType::Match(), .arg_types = { DataType::Int() }},
			}) },
		};
	}

	Variable Matches::data_member(VarContext& ctx, const std::string_view member) const
	{
		if (member == constants::MEMBER_LEN)
			return ctx.variable(Int(_matches.size()));

		ctx.throw_no_data_member(member);
	}

	Variable Matches::invoke_method(VarContext& ctx, const std::string_view method, std::vector<Variable>&& args) const
	{
		if (method == constants::SUBSCRIPT_OP)
		{
			if (args.size() == 1)
			{
				if (args[0].ref().data_type().simple() == SimpleType::Int)
					return _matches[std::move(args[0]).consume_as<Int>(ctx.env).value()];
			}
		}

		ctx.throw_no_method(method, args);
	}

	void Matches::assign(const EvalContext& env, Matches&& o)
	{
		_matches = std::move(o._matches);
	}

	bool Matches::equals(const EvalContext& env, const Matches& o) const
	{
		return _matches == o._matches;
	}

	size_t Matches::iterlen(const EvalContext& env) const
	{
		return _matches.size();
	}

	DataPoint Matches::iterget(const EvalContext& env, size_t i) const
	{
		return _matches[i].ref();
	}

	void Matches::append(Matches&& matches)
	{
		if (_matches.empty())
			_matches = std::move(matches._matches);
		else
			_matches.insert(_matches.end(), std::make_move_iterator(matches._matches.begin()), std::make_move_iterator(matches._matches.end()));
	}

	void Matches::push_back(const EvalContext& env, Variable match)
	{
		if (match.ref().data_type() == DataType::Match())
			_matches.push_back(std::move(match));
		else
		{
			std::stringstream ss;
			ss << "cannot add item of type " << match.ref().data_type() << " to " << DataType::Matches() << " object";
			throw env.runtime_error(ss.str());
		}
	}
}
