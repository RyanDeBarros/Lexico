#include "match.h"

#include "include.h"
#include "runtime.h"
#include "constants.h"

namespace lx
{
	Match::Match(Snippet snippet, unsigned int start, unsigned int length)
		: _section(std::move(snippet), start, length)
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
			{ MemberSignature::make_data_pair(constants::MEMBER_START, DataType::Int()) },
			{ MemberSignature::make_data_pair(constants::MEMBER_LEN, DataType::Int()) },
			{ MemberSignature::make_data_pair(constants::MEMBER_STR, DataType::String()) },
			{ MemberSignature::make_method_pair(constants::SUBSCRIPT_OP, {
				{ .return_type = DataType::List(DataType::Cap()), .arg_types = { DataType::CapId() } },
			}) },
			{ MemberSignature::make_method_pair(constants::MEMBER_TEXT, {
				{ .return_type = DataType::String(), .arg_types = { DataType::CapId() } },
				{ .return_type = DataType::String(), .arg_types = { DataType::CapId(), DataType::Int() } },
			}) },
		};
	}

	Variable Match::data_member(VarContext& ctx, const std::string_view member)
	{
		if (member == constants::MEMBER_START)
			return ctx.variable(Int(_section.absolute_start()));
		else if (member == constants::MEMBER_LEN)
			return ctx.variable(Int(_section.length));
		else if (member == constants::MEMBER_STR)
			return ctx.variable(_section.str());

		ctx.throw_no_data_member(member);
	}

	Variable Match::invoke_method(VarContext& ctx, const std::string_view method, std::vector<Variable>&& args)
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
		else if (method == constants::MEMBER_TEXT)
		{
			if (args.size() == 1 || args.size() == 2)
			{
				std::optional<int> idx = 0;
				if (args.size() == 2)
				{
					if (args[1].ref().data_type() == DataType::Int())
						idx = args[1].ref().get<Int>().value();
					else
						idx = std::nullopt;
				}

				if (args[0].ref().data_type() == DataType::CapId() && idx.has_value())
				{
					auto it = _captures_by_id.find(args[0].ref().get<CapId>());
					if (it != _captures_by_id.end())
						return ctx.variable(it->second.ref().get<List>().at(ctx.env, *idx).ref().get<Cap>().str());
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
		size_t h = _section.hash();
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
		return _section == o._section && _ordering == o._ordering && _captures_by_id == o._captures_by_id;
	}

	size_t Match::iterlen(const EvalContext& env) const
	{
		return _ordering.size();
	}

	Variable Match::iterget(VarContext& ctx, size_t i) const
	{
		auto it = _captures_by_id.find(_ordering[i].first);
		if (it != _captures_by_id.end())
			return it->second.ref().iterget(ctx, _ordering[i].second);
		else
		{
			std::stringstream ss;
			ss << __FUNCTION__ << ": can't find capture by id";
			throw ctx.env.internal_error(ss.str());
		}
	}

	void Match::add_capture(const EvalContext& env, CapId&& id, Cap&& cap)
	{
		auto it = _captures_by_id.find(id);
		if (it == _captures_by_id.end())
			it = _captures_by_id.try_emplace(id, env.runtime.unbound_variable(List(env, DataType::Cap()))).first;

		List& list = it->second.ref().get<List>();
		size_t idx = list.size();
		list.push(env, env.runtime.unbound_variable(std::move(cap)));
		_ordering.push_back(std::make_pair(std::move(id), idx));
	}

	Highlight Match::highlight_range() const
	{
		return { .start = _section.absolute_start(), .length = _section.length };
	}

	void Match::adjust_indexes(size_t index, size_t from_length, size_t to_length)
	{
		_section.adjust_indexes(index, from_length, to_length);
		for (auto& [_, caplist] : _captures_by_id)
		{
			auto& list = caplist.ref().get<List>();
			for (size_t i = 0; i < list.size(); ++i)
				list[i].ref().get<Cap>().adjust_indexes(index, from_length, to_length);
		}
	}
}
