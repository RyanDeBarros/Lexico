#include "cap.h"

#include "include.h"
#include "runtime.h"

namespace lx
{
	Cap::Cap(const EvalContext& env, Snippet snippet, unsigned int start, unsigned int length, bool exists, std::optional<Variable> submatch)
		: _snippet(std::move(snippet)), _start(start), _length(length), _exists(exists), _submatch(std::move(submatch))
	{
		if (_submatch && _submatch->ref().data_type() != DataType::Match())
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

	Variable Cap::data_member(VarContext& ctx, const std::string_view member) const
	{
		if (member == "exists")
			return ctx.variable(Bool(_exists));
		else if (member == "start")
			return ctx.variable(Int(_snippet.absolute(_start)));
		else if (member == "len")
			return ctx.variable(Int(_length));
		else if (member == "str")
			return ctx.variable(String(std::string(_snippet.page_content().substr(_start, _length))));
		else if (member == "sub")
		{
			if (_submatch)
				return *_submatch;
			else
				return ctx.variable(Match(_snippet, _start, _length, false));
		}

		ctx.throw_no_data_member(member);
	}

	Variable Cap::invoke_method(VarContext& ctx, const std::string_view method, std::vector<Variable>&& args) const
	{
		ctx.throw_no_method(method, args);
	}

	void Cap::assign(const EvalContext& env, Cap&& o)
	{
		// TODO
	}

	bool Cap::equals(const EvalContext& env, const Cap& o) const
	{
		// TODO
		return false;
	}

	bool Cap::exists() const
	{
		return _exists;
	}
}
