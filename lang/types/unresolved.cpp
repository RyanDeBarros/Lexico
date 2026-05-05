#include "unresolved.h"

namespace lx
{
	Unresolved::Unresolved(const PublicTypeVariant& v)
		: _v(v)
	{
	}

	Unresolved::Unresolved(PublicTypeVariant&& v)
		: _v(std::move(v))
	{
	}

	TypeVariant Unresolved::cast_copy(DataType type) const
	{
		if (type == DataType::_Unresolved)
			return *this;
		else if (type == DataType::Void)
			return Void();
		else
			return std::visit([type](const auto& v) -> TypeVariant { return v.cast_copy(type); }, _v);
	}

	TypeVariant Unresolved::cast_move(DataType type)
	{
		if (type == DataType::_Unresolved)
			return std::move(*this);
		else if (type == DataType::Void)
			return Void();
		else
			return std::visit([type](auto&& v) -> TypeVariant { return v.cast_move(type); }, std::move(_v));
	}

	void Unresolved::print(std::stringstream& ss) const
	{
		std::visit([&ss](const auto& v) { v.print(ss); }, _v);
	}
}
