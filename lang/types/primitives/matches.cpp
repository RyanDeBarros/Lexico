#include "matches.h"

#include "include.h"
#include "runtime.h"

namespace lx
{
	DataType Matches::data_type()
	{
		return DataType::Matches();
	}

	// TODO allow for casting to list?

	TypeVariant Matches::cast_copy(const VarContext& ctx, const DataType& type) const
	{
		if (type.simple() == SimpleType::Matches)
			return *this;
		else if (type.simple() == SimpleType::Void)
			return Void();
		else
			ctx.env.throw_bad_cast(data_type(), type);
	}

	TypeVariant Matches::cast_move(VarContext&& ctx, const DataType& type) &&
	{
		if (type.simple() == SimpleType::Matches)
			return std::move(*this);
		else
			return cast_copy(ctx, type);
	}

	void Matches::print(const EvalContext& env, std::stringstream& ss) const
	{
		// TODO v0.2 string representation of matches
		ss << DataType::Matches();
	}

	Variable Matches::data_member(VarContext& ctx, const std::string_view member) const
	{
		ctx.throw_no_data_member(member);
	}

	Variable Matches::invoke_method(VarContext& ctx, const std::string_view method, std::vector<Variable>&& args) const
	{
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
