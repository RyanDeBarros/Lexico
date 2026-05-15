#pragma once

#include "types/declarations.h"

namespace lx
{
	class Float
	{
		float _value;

	public:
		explicit Float(float value);

		static Float make_from_literal(const EvalContext& env, std::string_view resolved);

		static DataType data_type();
		TypeVariant cast_copy(const VarContext& ctx, const DataType& type) const;
		TypeVariant cast_move(VarContext&& ctx, const DataType& type) &&;
		void print(const EvalContext& env, std::stringstream& ss) const;

		Variable data_member(VarContext& ctx, const std::string_view member);
		Variable invoke_method(VarContext& ctx, const std::string_view method, std::vector<Variable>&& args);
		void assign(const EvalContext& env, Float&& o);
		bool equals(const EvalContext& env, const Float& o) const;

		float value() const;
	};
}
