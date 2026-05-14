#pragma once

#include "types/declarations.h"
#include "types/member.h"
#include "util.h"

namespace lx
{
	class String
	{
		friend class StringView;
		std::string _value;

	public:
		explicit String(const std::string& value);
		explicit String(std::string&& value);

		static String make_from_literal(const EvalContext& env, std::string_view resolved);

		static DataType data_type();
		TypeVariant cast_copy(const VarContext& ctx, const DataType& type) const;
		TypeVariant cast_move(VarContext&& ctx, const DataType& type) &&;
		void print(const EvalContext& env, std::stringstream& ss) const;

		static StringMap<MemberSignature> members();
		Variable data_member(VarContext& ctx, const std::string_view member);
		Variable invoke_method(VarContext& ctx, const std::string_view method, std::vector<Variable>&& args);
		void assign(const EvalContext& env, String&& o);
		bool equals(const EvalContext& env, const String& o) const;

		size_t iterlen(const EvalContext& env) const;
		DataPoint iterget(const EvalContext& env, size_t i) const;
		std::string page_content(const EvalContext& env) const;

		std::string_view value() const;
		std::string steal() &&;

		void insert(const EvalContext& env, Int&& index, String&& s);
		void insert(const EvalContext& env, Int&& index, StringView&& s);
	};
}
