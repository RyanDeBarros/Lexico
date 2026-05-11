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
		TypeVariant cast_copy(const VarContext& ctx, const DataType& type) const;
		TypeVariant cast_move(VarContext&& ctx, const DataType& type) &&;
		void print(const EvalContext& env, std::stringstream& ss) const;

		Variable data_member(VarContext& ctx, const std::string_view member) const;
		Variable invoke_method(VarContext& ctx, const std::string_view method, std::vector<Variable>&& args) const;
		void assign(const EvalContext& env, CapId&& o);
		bool equals(const EvalContext& env, const CapId& o) const;

		bool operator==(const CapId&) const = default;
	};
}
