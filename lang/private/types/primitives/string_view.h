#pragma once

#include "types/declarations.h"
#include "types/variable.h"
#include "types/member.h"
#include "int.h"
#include "irange.h"
#include "util.h"

#include <functional>

namespace lx
{
	class StringView
	{
		Variable _ref;
		String* _string;

	public:
		using Indexer = std::variant<Int, IRange>;

	private:
		Indexer _indexer;

	public:
		StringView(const EvalContext& env, Variable string, const Indexer& indexer);
		StringView(const StringView&) noexcept;
		StringView(StringView&&) noexcept;
		StringView& operator=(const StringView&);
		StringView& operator=(StringView&&) noexcept;

		static DataType data_type();
		TypeVariant cast_copy(const VarContext& ctx, const DataType& type) const;
		TypeVariant cast_move(VarContext&& ctx, const DataType& type) &&;
		void print(const EvalContext& env, std::stringstream& ss) const;

		static StringMap<MemberSignature> members();
		Variable data_member(VarContext& ctx, const std::string_view member);
		Variable invoke_method(VarContext& ctx, const std::string_view method, std::vector<Variable>&& args);
		void assign(const EvalContext& env, StringView&& o);
		bool equals(const EvalContext& env, const StringView& o) const;

		size_t iterlen(const EvalContext& env) const;
		DataPoint iterget(const EvalContext& env, size_t i) const;
		std::string page_content(const EvalContext& env) const;

		std::string copy_value(const EvalContext& env) const;
		std::string consume_value(const EvalContext& env) &&;
		StringView substring(const EvalContext& env, const Int& index) const;
		StringView substring(const EvalContext& env, const IRange& range) const;
		StringView substring(const EvalContext& env, const Indexer& indexer) const;

		void assert_valid(const EvalContext& env) const;

	private:
		static void assert_in_range(const EvalContext& env, const Indexer& indexer, const int min, const int max);
		static void throw_out_of_range(const EvalContext& env, const Indexer& indexer, const int len);
		int min_index() const;
		int max_index() const;
		std::string_view string() const;
		char chr(int i, int min, int max) const;
		int idx(int i, int min, int max) const;

	public:
		void insert(const EvalContext& env, Int&& index, String&& s);
		void insert(const EvalContext& env, Int&& index, StringView&& s);

		size_t size() const;
		void visit(const EvalContext& env, std::function<void(char)> visitor) const;
		Variable string_variable() const;
	};
}
