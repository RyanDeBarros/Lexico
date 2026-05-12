#pragma once

#include "types/declarations.h"
#include "types/member.h"
#include "util.h"

namespace lx
{
	class Matches
	{
		std::vector<Variable> _matches;

	public:
		static DataType data_type();
		TypeVariant cast_copy(const VarContext& ctx, const DataType& type) const;
		TypeVariant cast_move(VarContext&& ctx, const DataType& type) &&;
		void print(const EvalContext& env, std::stringstream& ss) const;

		static StringMap<MemberSignature> members();
		Variable data_member(VarContext& ctx, const std::string_view member) const;
		Variable invoke_method(VarContext& ctx, const std::string_view method, std::vector<Variable>&& args) const;
		void assign(const EvalContext& env, Matches&& o);
		bool equals(const EvalContext& env, const Matches& o) const;

		size_t iterlen(const EvalContext& env) const;
		DataPoint iterget(const EvalContext& env, size_t i) const;

		void append(Matches&& matches);
		void push_back(const EvalContext& env, Variable match);
		const Match& match(size_t i) const;
		size_t size() const;
	};
}
