#pragma once

#include "types/declarations.h"

namespace lx
{
	class List
	{
		DataType _underlying;
		std::vector<Variable> _elements;

		explicit List(const DataType& underlying);
		explicit List(DataType&& underlying);
		explicit List(std::vector<Variable>&& elements);

	public:
		List(const DataType& underlying, const ScriptSegment& segment);
		List(DataType&& underlying, const ScriptSegment& segment);
		List(std::vector<Variable>&& elements, const ScriptSegment& segment);

		static List make_nonvoid_list(const DataType& underlying);
		static List make_nonvoid_list(DataType&& underlying);
		static List make_nonvoid_list(std::vector<Variable>&& elements);

		DataType data_type() const;
		TypeVariant cast_copy(const DataType& type) const;
		TypeVariant cast_move(const DataType& type);
		void print(std::stringstream& ss) const;

		Variable data_member(VarContext& ctx, const std::string_view member) const;
		Variable invoke_method(VarContext& ctx, const std::string_view method, std::vector<Variable>&& args) const;
		bool equals(const List& o) const;

		size_t iterlen() const;
		DataPoint iterget(size_t i) const;

		bool push(Variable element);
	};
}
