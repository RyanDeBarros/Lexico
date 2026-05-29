#pragma once

#include "types/declarations.h"
#include "types/member.h"
#include "types/variable.h"
#include "util.h"

namespace lx
{
	class Ref
	{
		Variable _subobject;

	public:
		Ref(Variable subobject);

		DataType data_type() const;
		DataPoint cast_copy(const VarContext& ctx, const DataType& type) const;
		DataPoint cast_move(VarContext&& ctx, const DataType& type)&&;
		Variable pass_arg(VarContext ctx);
		void print(const EvalContext& env, std::ostream& ss) const;

		Variable data_member(VarContext& ctx, const std::string_view member);
		Variable invoke_method(VarContext& ctx, const std::string_view method, std::vector<Variable>&& args);
		void assign(const EvalContext& env, Variable o);
		bool equals(const EvalContext& env, Variable o) const;

		size_t iterlen(const EvalContext& env) const;
		Variable iterget(VarContext& ctx, size_t i) const;

		const DataPoint& val() const;
		DataPoint& val();
		const DataPoint& root() const;
		DataPoint& root();
		Variable root_var() const;
		Variable dereference() const;
	};
}
