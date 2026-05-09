#pragma once

#include "types/declarations.h"

namespace lx
{
	class CapId
	{
		unsigned int _uid;

	public:
		explicit CapId(unsigned int uid);

		static DataType data_type();
		TypeVariant cast_copy(const DataType& type) const;
		TypeVariant cast_move(const DataType& type);
		void print(std::stringstream& ss) const;

		Variable data_member(VarContext& ctx, const std::string_view member) const;
		Variable invoke_method(VarContext& ctx, const std::string_view method, std::vector<Variable>&& args) const;
		bool equals(const CapId& o) const;
	};
}
