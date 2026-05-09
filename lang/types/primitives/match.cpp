#include "match.h"

#include "include.h"
#include "runtime.h"
#include "constants.h"

namespace lx
{
	DataType Match::data_type()
	{
		return DataType::Match();
	}

	TypeVariant Match::cast_copy(const DataType& type) const
	{
		if (type.simple() == SimpleType::Match)
			return *this;
		else if (type.simple() == SimpleType::Void)
			return Void();
		else
			throw_bad_cast(data_type(), type);
	}

	TypeVariant Match::cast_move(const DataType& type)
	{
		if (type.simple() == SimpleType::Match)
			return std::move(*this);
		else
			return cast_copy(type);
	}

	void Match::print(std::stringstream& ss) const
	{
		// TODO v0.2 string representation of match
		ss << DataType::Match();
	}

	Variable Match::data_member(VarContext& ctx, const std::string_view member) const
	{
		// TODO use constants for these member names
		if (member == "caps")
		{
			// TODO
			return ctx.variable(List::make_nonvoid_list(DataType::Cap()));
		}
		else if (member == "start")
		{
			// TODO
			return ctx.variable(Int(0));
		}
		else if (member == "end")
		{
			// TODO
			return ctx.variable(Int(0));
		}
		else if (member == "len")
		{
			// TODO
			return ctx.variable(Int(0));
		}
		else if (member == "range")
		{
			// TODO
			return ctx.variable(IRange(0, 0));
		}
		else if (member == "str")
		{
			// TODO
			return ctx.variable(String(""));
		}

		ctx.throw_no_data_member(member);
	}

	Variable Match::invoke_method(VarContext& ctx, const std::string_view method, std::vector<Variable>&& args) const
	{
		if (method == constants::SUBSCRIPT_OP)
		{
			if (args.size() == 1)
			{
				if (args[0].ref().data_type().simple() == SimpleType::CapId)
				{
					// TODO
					return ctx.variable(Cap());
				}
				else if (args[0].ref().data_type().simple() == SimpleType::Int)
				{
					// TODO
					return ctx.variable(Cap());
				}
			}
		}

		ctx.throw_no_method(method, args);
	}

	bool Match::equals(const Match& o) const
	{
		// TODO
		return false;
	}

	size_t Match::iterlen() const
	{
		// TODO
		return 0;
	}

	DataPoint Match::iterget(size_t i) const
	{
		// TODO
		return Cap();
	}
}
