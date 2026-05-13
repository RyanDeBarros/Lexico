#include "cap.h"

#include "include.h"
#include "runtime.h"
#include "constants.h"

namespace lx
{
	Cap::Cap(const EvalContext& env, Snippet snippet, unsigned int start, unsigned int length, Variable submatch)
		: _snippet(std::move(snippet)), _start(start), _length(length), _submatch(std::move(submatch))
	{
		if (_submatch.ref().data_type() != DataType::Match())
			throw env.internal_error("Cap() submatch does not have type " + DataType::Match().repr());
	}

	DataType Cap::data_type()
	{
		return DataType::Cap();
	}

	TypeVariant Cap::cast_copy(const VarContext& ctx, const DataType& type) const
	{
		if (type.simple() == SimpleType::Cap)
			return *this;
		else if (type.simple() == SimpleType::Void)
			return Void();
		else
			ctx.env.throw_bad_cast(data_type(), type);
	}

	TypeVariant Cap::cast_move(VarContext&& ctx, const DataType& type) &&
	{
		if (type.simple() == SimpleType::Cap)
			return std::move(*this);
		else
		{
			(void*)this; // ignore const warning
			return cast_copy(ctx, type);
		}
	}

	void Cap::print(const EvalContext& env, std::stringstream& ss) const
	{
		// TODO v0.2 string representation of cap
		ss << DataType::Cap();
	}

	StringMap<MemberSignature> Cap::members()
	{
		return {
			{ constants::MEMBER_START, MemberSignature::make_data(constants::MEMBER_START, DataType::Int()) },
			{ constants::MEMBER_LEN, MemberSignature::make_data(constants::MEMBER_LEN, DataType::Int()) },
			{ constants::MEMBER_STR, MemberSignature::make_data(constants::MEMBER_STR, DataType::String()) },
			{ constants::MEMBER_SUB, MemberSignature::make_data(constants::MEMBER_SUB, DataType::Match()) },
		};
	}

	Variable Cap::data_member(VarContext& ctx, const std::string_view member) const
	{
		if (member == constants::MEMBER_START)
			return ctx.variable(Int(_snippet.absolute(_start)));
		else if (member == constants::MEMBER_LEN)
			return ctx.variable(Int(_length));
		else if (member == constants::MEMBER_STR)
			return ctx.variable(str());
		else if (member == constants::MEMBER_SUB)
			return _submatch;

		ctx.throw_no_data_member(member);
	}

	Variable Cap::invoke_method(VarContext& ctx, const std::string_view method, std::vector<Variable>&& args) const
	{
		ctx.throw_no_method(method, args);
	}

	void Cap::assign(const EvalContext& env, Cap&& o)
	{
		*this = std::move(o);
	}

	bool Cap::equals(const EvalContext& env, const Cap& o) const
	{
		return _snippet.placement_equals(o._snippet, _start, _length, o._start, o._length) && _submatch.ref().get<Match>().equals(env, o._submatch.ref().get<Match>());
	}

	String Cap::str() const
	{
		return String(std::string(_snippet.page_content().substr(_start, _length)));
	}
}
