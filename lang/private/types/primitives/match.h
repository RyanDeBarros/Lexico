#pragma once

#include "capid.h"
#include "types/declarations.h"
#include "types/member.h"
#include "page.h"
#include "util.h"

#include <unordered_map>

namespace lx
{
	class Match
	{
		SnippetSection _section;
		std::unordered_map<CapId, Variable> _captures_by_id;
		std::vector<std::pair<CapId, size_t>> _ordering;

	public:
		Match(Snippet snippet, unsigned int start, unsigned int length);

		static DataType data_type();
		TypeVariant cast_copy(const VarContext& ctx, const DataType& type) const;
		TypeVariant cast_move(VarContext&& ctx, const DataType& type) &&;
		void print(const EvalContext& env, std::stringstream& ss) const;

		static StringMap<MemberSignature> members();
		Variable data_member(VarContext& ctx, const std::string_view member);
		Variable invoke_method(VarContext& ctx, const std::string_view method, std::vector<Variable>&& args);
		void assign(const EvalContext& env, Match&& o);
		bool equals(const EvalContext& env, const Match& o) const;

		size_t hash() const;
		bool equals(const Match& o) const;

		size_t iterlen(const EvalContext& env) const;
		DataPoint iterget(const EvalContext& env, size_t i) const;

		void add_capture(const EvalContext& env, CapId&& id, Cap&& cap);
		Highlight highlight_range() const;
		void adjust_indexes(size_t index, size_t from_length, size_t to_length);
	};
}
