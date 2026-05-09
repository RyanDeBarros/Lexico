#pragma once

#include "types/declarations.h"

namespace lx
{
	class String
	{
		std::string _value;

	public:
		explicit String(const std::string& value);
		explicit String(std::string&& value);

		static String make_from_literal(std::string_view resolved);

		static DataType data_type();
		TypeVariant cast_copy(const DataType& type) const;
		TypeVariant cast_move(const DataType& type);
		void print(std::stringstream& ss) const;

		DataPoint resolve_path(VarContext& ctx, const PathStep& step);
		DataPoint consume_path(VarContext& ctx, const PathStep& step) &&;
		void assign_path(VarContext& ctx, const PathStep& step, DataPoint&& to);

		Variable data_member(VarContext& ctx, const std::string_view member) const;
		Variable invoke_method(VarContext& ctx, const std::string_view method, std::vector<Variable>&& args) const;
		bool equals(const String& o) const;

		size_t iterlen() const;
		DataPoint iterget(size_t i) const;
		std::string page_content() const;

		std::string_view value() const;
	};
}
