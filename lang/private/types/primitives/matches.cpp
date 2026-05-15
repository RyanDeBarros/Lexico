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

	Variable Matches::data_member(VarContext& ctx, const std::string_view member)
	{
		if (member == constants::MEMBER_LEN)
			return ctx.variable(Int(_matches.size()));

		ctx.throw_no_data_member(member);
	}

	Variable Matches::invoke_method(VarContext& ctx, const std::string_view method, std::vector<Variable>&& args)
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

	const Match& Matches::match(size_t i) const
	{
		return _matches[i].ref().get<Match>();
	}

	size_t Matches::size() const
	{
		return _matches.size();
	}

	struct MatchPtrHash
	{
		size_t operator()(const Match* m) const
		{
			return m->hash();
		}
	};

	struct MatchPtrEqual
	{
		bool operator()(const Match* a, const Match* b) const
		{
			return a->equals(*b);
		}
	};

	void Matches::remove_duplicates()
	{
		std::vector<Variable> unique_matches;
		std::unordered_set<const Match*, MatchPtrHash, MatchPtrEqual> seen;

		for (Variable& v : _matches)
		{
			const Match& m = v.ref().get<Match>();
			if (!seen.contains(&m))
			{
				seen.insert(&m);
				unique_matches.push_back(std::move(v));
			}
		}

		_matches = std::move(unique_matches);
	}

	void Matches::adjust_indexes(size_t index, size_t from_length, size_t to_length)
	{
		for (size_t i = 0; i < _matches.size(); ++i)
			_matches[i].ref().get<Match>().adjust_indexes(index, from_length, to_length);
	}
}
