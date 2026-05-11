#pragma once

#include "types/declarations.h"
#include "types/variable.h"
#include "int.h"
#include "irange.h"

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
		TypeVariant cast_copy(const EvalContext& env, const DataType& type) const;
		TypeVariant cast_move(const EvalContext& env, const DataType& type);
		void print(const EvalContext& env, std::stringstream& ss) const;

		Variable data_member(VarContext& ctx, const std::string_view member) const;
		Variable invoke_method(VarContext& ctx, const std::string_view method, std::vector<Variable>&& args) const;
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
		int min_index() const;
		int max_index() const;
		std::string_view string() const;
		char chr(int i, int min, int max) const;
		int idx(int i, int min, int max) const;
	};
}
