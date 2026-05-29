#pragma once

#include "types/declarations.h"

namespace lx
{
	class Void
	{
	public:
		static DataType data_type();
		DataPoint cast_copy(const VarContext& ctx, const DataType& type) const;
		DataPoint cast_move(VarContext&& ctx, const DataType& type) &&;
		Variable pass_arg(VarContext ctx);
		void print(const EvalContext& env, std::ostream& ss) const;

		Variable data_member(VarContext& ctx, const std::string_view member);
		Variable invoke_method(VarContext& ctx, const std::string_view method, std::vector<Variable>&& args);
		void assign(const EvalContext& env, Variable o);
		bool equals(const EvalContext& env, Variable o) const;
	};
}
