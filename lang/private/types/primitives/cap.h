#pragma once

#include "types/declarations.h"
#include "types/member.h"
#include "match.h"
#include "util.h"

namespace lx
{
	class Cap
	{
		SnippetSection _section;
		Variable _submatch;
		
	public:
		Cap(const EvalContext& env, Snippet snippet, unsigned int start, unsigned int length, Variable submatch);

		static DataType data_type();
		TypeVariant cast_copy(const VarContext& ctx, const DataType& type) const;
		TypeVariant cast_move(VarContext&& ctx, const DataType& type) &&;
		void print(const EvalContext& env, std::stringstream& ss) const;

		static StringMap<MemberSignature> members();
		Variable data_member(VarContext& ctx, const std::string_view member);
		Variable invoke_method(VarContext& ctx, const std::string_view method, std::vector<Variable>&& args);
		void assign(const EvalContext& env, Cap&& o);
		bool equals(const EvalContext& env, const Cap& o) const;
		String str() const;

		unsigned int start() const;
		unsigned int length() const;

		void adjust_indexes(size_t index, size_t from_length, size_t to_length);
	};
}
