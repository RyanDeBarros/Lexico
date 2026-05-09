#include "capid.h"

#include "include.h"
#include "runtime.h"

namespace lx
{
	CapId::CapId(unsigned int uid)
		: _uid(uid)
	{
	}

	DataType CapId::data_type()
	{
		return DataType::CapId();
	}

	TypeVariant CapId::cast_copy(const DataType& type) const
	{
		if (type.simple() == SimpleType::CapId)
			return *this;
		else if (type.simple() == SimpleType::Void)
			return Void();
		else
			throw_bad_cast(data_type(), type);
	}

	TypeVariant CapId::cast_move(const DataType& type)
	{
		(void*)this; // ignore const warning
		return cast_copy(type);
	}

	void CapId::print(std::stringstream& ss) const
	{
		ss << DataType::CapId();
	}

	Variable CapId::data_member(VarContext& ctx, const std::string_view member) const
	{
		ctx.throw_no_data_member(member);
	}

	Variable CapId::invoke_method(VarContext& ctx, const std::string_view method, std::vector<Variable>&& args) const
	{
		ctx.throw_no_method(method, args);
	}

	bool CapId::equals(const CapId& o) const
	{
		return _uid == o._uid;
	}
}
