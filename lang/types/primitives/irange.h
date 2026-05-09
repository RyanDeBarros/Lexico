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
		TypeVariant cast_copy(const DataType& type) const;
		TypeVariant cast_move(const DataType& type);
		void print(std::stringstream& ss) const;

		Variable data_member(VarContext& ctx, const std::string_view member) const;
		Variable invoke_method(VarContext& ctx, const std::string_view method, std::vector<Variable>&& args) const;
		bool equals(const IRange& o) const;

		size_t iterlen() const;
		DataPoint iterget(size_t i) const;

		std::optional<int> min() const;
		std::optional<int> max() const;
	};
}
