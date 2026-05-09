#include "void.h"

#include "include.h"
#include "runtime.h"

namespace lx
{
	DataType Void::data_type()
	{
		return DataType::Void();
	}

	TypeVariant Void::cast_copy(const DataType& type) const
	{
		if (type.simple() == SimpleType::Void)
			return Void();
		else
			throw_bad_cast(data_type(), type);
	}

	TypeVariant Void::cast_move(const DataType& type)
	{
		(void*)this; // ignore const warning
		return cast_copy(type);
	}

	void Void::print(std::stringstream& ss) const
	{
		ss << "";
	}

	Variable Void::data_member(VarContext& ctx, const std::string_view member) const
	{
		ctx.throw_no_data_member(member);
	}

	Variable Void::invoke_method(VarContext& ctx, const std::string_view method, std::vector<Variable>&& args) const
	{
		ctx.throw_no_method(method, args);
	}

	bool Void::equals(const Void& o) const
	{
		return true;
	}
}
