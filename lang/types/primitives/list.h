#pragma once

#include "types/declarations.h"

namespace lx
{
	class List
	{
		DataType _underlying;
		std::vector<Variable> _elements;

	public:
		List(const EvalContext& env, const DataType& underlying);
		List(const EvalContext& env, DataType&& underlying);
		List(const EvalContext& env, std::vector<Variable>&& elements);

		DataType data_type() const;
		TypeVariant cast_copy(const VarContext& ctx, const DataType& type) const;
		TypeVariant cast_move(VarContext&& ctx, const DataType& type) &&;
		void print(const EvalContext& env, std::stringstream& ss) const;

		Variable data_member(VarContext& ctx, const std::string_view member) const;
		Variable invoke_method(VarContext& ctx, const std::string_view method, std::vector<Variable>&& args) const;
		void assign(const EvalContext& env, List&& o);
		bool equals(const EvalContext& env, const List& o) const;

		size_t iterlen(const EvalContext& env) const;
		DataPoint iterget(const EvalContext& env, size_t i) const;

		bool push(Variable element);
		size_t size() const;
	};
}
