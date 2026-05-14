#include "match.h"

#include "include.h"
#include "runtime.h"
#include "constants.h"

namespace lx
{
	Match::Match(Snippet snippet, unsigned int start, unsigned int length)
		: _snippet(std::move(snippet)), _start(start), _length(length)
	{
	}

	DataType Match::data_type()
	{
		return DataType::Match();
	}

	TypeVariant Match::cast_copy(const VarContext& ctx, const DataType& type) const
	{
		if (type.simple() == SimpleType::Match)
			return *this;
		else if (type.simple() == SimpleType::Void)
			return Void();
		else
			ctx.env.throw_bad_cast(data_type(), type);
	}

	TypeVariant Match::cast_move(VarContext&& ctx, const DataType& type) &&
	{
		if (type.simple() == SimpleType::Match)
			return std::move(*this);
		else
			return cast_copy(ctx, type);
	}

	void Match::print(const EvalContext& env, std::stringstream& ss) const
	{
		// TODO v0.2 string representation of match
		ss << DataType::Match();
	}

	StringMap<MemberSignature> Match::members()
	{
		return {
			{ constants::MEMBER_START, MemberSignature::make_data(constants::MEMBER_START, DataType::Int()) },
			{ constants::MEMBER_LEN, MemberSignature::make_data(constants::MEMBER_LEN, DataType::Int()) },
			{ constants::MEMBER_STR, MemberSignature::make_data(constants::MEMBER_STR, DataType::String()) },
			{ constants::SUBSCRIPT_OP, MemberSignature::make_method(constants::SUBSCRIPT_OP, {
				{ .return_type = DataType::Cap(), .arg_types = { DataType::CapId() } },
			}) },
			{ constants::MEMBER_STR, MemberSignature::make_method(constants::MEMBER_STR, {
				{ .return_type = DataType::String(), .arg_types = { DataType::CapId() } },
			}) },
		};
	}

	Variable Match::data_member(VarContext& ctx, const std::string_view member) const
	{
		if (member == constants::MEMBER_START)
			return ctx.variable(Int(_snippet.absolute(_start)));
		else if (member == constants::MEMBER_LEN)
			return ctx.variable(Int(_length));
		else if (member == constants::MEMBER_STR)
			return ctx.variable(String(std::string(_snippet.page_content().substr(_start, _length))));

		ctx.throw_no_data_member(member);
	}

	Variable Match::invoke_method(VarContext& ctx, const std::string_view method, std::vector<Variable>&& args) const
	{
		if (method == constants::SUBSCRIPT_OP)
		{
			if (args.size() == 1)
			{
				if (args[0].ref().data_type() == DataType::CapId())
				{
					auto it = _captures_by_id.find(args[0].ref().get<CapId>());
					if (it != _captures_by_id.end())
						return it->second;
					else
						return ctx.variable(List(ctx.env, DataType::Cap()));
				}
			}
		}
		else if (method == constants::MEMBER_STR)
		{
			if (args.size() == 1)
			{
				if (args[0].ref().data_type() == DataType::CapId())
				{
					auto it = _captures_by_id.find(args[0].ref().get<CapId>());
					if (it != _captures_by_id.end())
						return ctx.variable(it->second.ref().get<Cap>().str());
					else
						return ctx.variable(String(""));
				}
			}
		}

		ctx.throw_no_method(method, args);
	}

	void Match::assign(const EvalContext& env, Match&& o)
	{
		*this = std::move(o);
	}

	bool Match::equals(const EvalContext& env, const Match& o) const
	{
		return equals(o);
	}

	size_t Match::hash() const
	{
		size_t h = 0;
		h = hash_combine(h, std::hash<unsigned int>{}(_snippet.absolute(_start)));
		h = hash_combine(h, std::hash<unsigned int>{}(_length));
		for (const auto& [capid, index] : _ordering)
		{
			auto it = _captures_by_id.find(capid);
			if (it != _captures_by_id.end())
			{
				const Cap& cap = it->second.ref().get<List>()[index].ref().get<Cap>();
				h = hash_combine(h, std::hash<unsigned int>{}(cap.start()));
				h = hash_combine(h, std::hash<unsigned int>{}(cap.length()));
			}
		}
		return h;
	}

	bool Match::equals(const Match& o) const
	{
		return _snippet.placement_equals(o._snippet, _start, _length, o._start, o._length) && _ordering == o._ordering && _captures_by_id == o._captures_by_id;
	}

	size_t Match::iterlen(const EvalContext& env) const
	{
		return _ordering.size();
	}

	DataPoint Match::iterget(const EvalContext& env, size_t i) const
	{
		auto it = _captures_by_id.find(_ordering[i].first);
		if (it != _captures_by_id.end())
			return it->second.ref().iterget(env, _ordering[i].second);
		else
		{
			std::stringstream ss;
			ss << __FUNCTION__ << ": can't find capture by id";
			throw env.internal_error(ss.str());
		}
	}

	void Match::add_capture(const EvalContext& env, CapId&& id, Cap&& cap)
	{
		auto it = _captures_by_id.find(id);
		if (it == _captures_by_id.end())
			it = _captures_by_id.try_emplace(id, env.runtime.unbound_variable(List(env, DataType::Cap()))).first;

		List& list = it->second.ref().get<List>();
		size_t idx = list.size();
		list.push(env.runtime.unbound_variable(std::move(cap)));
		_ordering.push_back(std::make_pair(std::move(id), idx));
	}

	Highlight Match::highlight_range() const
	{
		return { .start = _snippet.absolute(_start), .length = _length };
	}
}
