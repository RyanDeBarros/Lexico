#pragma once

#include "types/declarations.h"

namespace lx
{
	// TODO make IRange right-open while keeping SRange inclusive. This allows for pure insertion into strings at indexes.

	class IRange
	{
		std::optional<int> _min;
		std::optional<int> _max;

	public:
		IRange(std::optional<int> min, std::optional<int> max);

		static DataType data_type();
		TypeVariant cast_copy(const VarContext& ctx, const DataType& type) const;
		TypeVariant cast_move(VarContext&& ctx, const DataType& type) &&;
		void print(const EvalContext& env, std::stringstream& ss) const;

		Variable data_member(VarContext& ctx, const std::string_view member) const;
		Variable invoke_method(VarContext& ctx, const std::string_view method, std::vector<Variable>&& args) const;
		void assign(const EvalContext& env, IRange&& o);
		bool equals(const EvalContext& env, const IRange& o) const;

		size_t iterlen(const EvalContext& env) const;
		DataPoint iterget(const EvalContext& env, size_t i) const;

		std::optional<int> min() const;
		std::optional<int> max() const;

		bool operator==(const IRange&) const = default;
	};
}
