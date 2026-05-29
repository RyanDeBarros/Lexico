#pragma once

#include "types/declarations.h"

namespace lx
{
	class IRange
	{
		std::optional<int> _min;
		std::optional<int> _max;

	public:
		IRange(std::optional<int> min, std::optional<int> max);

		static DataType data_type();
		TypeVariant cast_copy(const VarContext& ctx, const DataType& type) const;
		TypeVariant cast_move(VarContext&& ctx, const DataType& type) &&;
		Variable pass_arg(VarContext ctx);
		void print(const EvalContext& env, std::ostream& ss) const;

		Variable data_member(VarContext& ctx, const std::string_view member);
		Variable invoke_method(VarContext& ctx, const std::string_view method, std::vector<Variable>&& args);
		void assign(const EvalContext& env, Variable o);
		bool equals(const EvalContext& env, Variable o) const;

		size_t iterlen(const EvalContext& env) const;
		Variable iterget(VarContext& ctx, size_t i) const;

		std::optional<int> min() const;
		std::optional<int>& min();
		std::optional<int> max() const;
		std::optional<int>& max();

		bool operator==(const IRange&) const = default;
	};
}
