#pragma once

#include "types/declarations.h"
#include "types/member.h"
#include "page.h"
#include "capid.h"
#include "util.h"

#include <unordered_map>

namespace lx
{
	class Match
	{
		Snippet _snippet;
		unsigned int _start = 0;
		unsigned int _length = 0;
		bool _exists;

		std::unordered_map<CapId, Variable> _captures_by_id;
		std::vector<std::pair<CapId, size_t>> _ordering;

	public:
		Match(Snippet snippet, unsigned int start, unsigned int length, bool exists);

		static DataType data_type();
		TypeVariant cast_copy(const VarContext& ctx, const DataType& type) const;
		TypeVariant cast_move(VarContext&& ctx, const DataType& type) &&;
		void print(const EvalContext& env, std::stringstream& ss) const;

		static StringMap<MemberSignature> members();
		Variable data_member(VarContext& ctx, const std::string_view member) const;
		Variable invoke_method(VarContext& ctx, const std::string_view method, std::vector<Variable>&& args) const;
		void assign(const EvalContext& env, Match&& o);
		bool equals(const EvalContext& env, const Match& o) const;

		size_t iterlen(const EvalContext& env) const;
		DataPoint iterget(const EvalContext& env, size_t i) const;

		void add_capture(const EvalContext& env, CapId&& id, Cap&& cap);
		bool exists() const;
		void assert_exists(const EvalContext& env) const;
		Highlight highlight_range() const;
	};
}
