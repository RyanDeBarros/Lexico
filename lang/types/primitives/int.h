#pragma once

#include "types/declarations.h"

namespace lx
{
	class Int
	{
		int _value;

	public:
		explicit Int(int value);

		static Int make_from_literal(const EvalContext& env, std::string_view resolved);

		static DataType data_type();
		TypeVariant cast_copy(const EvalContext& env, const DataType& type) const;
		TypeVariant cast_move(const EvalContext& env, const DataType& type);
		void print(const EvalContext& env, std::stringstream& ss) const;

		Variable data_member(VarContext& ctx, const std::string_view member) const;
		Variable invoke_method(VarContext& ctx, const std::string_view method, std::vector<Variable>&& args) const;
		void assign(const EvalContext& env, Int&& o);
		bool equals(const EvalContext& env, const Int& o) const;

		int value() const;
	};
}
