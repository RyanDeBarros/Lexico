#pragma once

#include "types/declarations.h"

namespace lx
{
	class SubpatternNode;
	class Snippet;

	class Pattern
	{
		std::vector<std::unique_ptr<SubpatternNode>> _subnodes;
		SubpatternNode* _root = nullptr;

	public:
		Pattern();
		Pattern(const Pattern& other);
		Pattern(Pattern&& other) noexcept;
		~Pattern();
		Pattern& operator=(const Pattern& other);
		Pattern& operator=(Pattern&& other) noexcept;

		static DataType data_type();
		TypeVariant cast_copy(const VarContext& ctx, const DataType& type) const;
		TypeVariant cast_move(VarContext&& ctx, const DataType& type) &&;
		void print(const EvalContext& env, std::stringstream& ss) const;

		Variable data_member(VarContext& ctx, const std::string_view member) const;
		Variable invoke_method(VarContext& ctx, const std::string_view method, std::vector<Variable>&& args) const;
		void assign(const EvalContext& env, Pattern&& o);
		bool equals(const EvalContext& env, const Pattern& o) const;

		static Pattern make_from_symbol(BuiltinSymbol symbol);
		static Pattern make_repeat(Pattern&& pattern, const IRange& range);
		static Pattern make_backref(const CapId& capid);
		static Pattern make_lazy(Pattern&& pattern);
		static Pattern make_greedy(Pattern&& pattern);
		static Pattern make_capture(Pattern&& pattern, const CapId& capid);
		
	private:
		void impl_own(std::unique_ptr<SubpatternNode>&& node);

	public:
		template<std::derived_from<SubpatternNode> T>
		T& own(std::unique_ptr<T>&& node)
		{
			T* ptr = node.get();
			impl_own(std::move(node));
			return *ptr;
		}

		SubpatternNode& take(Pattern&& pattern);
		
	private:
		void set_root(std::unique_ptr<SubpatternNode>&& node);

	public:
		template<std::derived_from<SubpatternNode> T, typename... Args>
		T& make_root(Args&&... args)
		{
			auto sub = std::make_unique<T>(std::forward<Args>(args)...);
			T* n = sub.get();
			set_root(std::move(sub));
			return *n;
		}

		template<std::derived_from<SubpatternNode> T, typename... Args>
		T& make_node(Args&&... args)
		{
			return own(std::make_unique<T>(std::forward<Args>(args)...));
		}

		template<std::derived_from<SubpatternNode> T, typename... Args>
		static Pattern make_from_subpattern(Args&&... args)
		{
			Pattern ptn;
			ptn.make_root<T>(std::forward<Args>(args)...);
			return ptn;
		}

		void append(Pattern&& pattern);

		Matches search(const EvalContext& env, const Snippet& snippet) const;
		Matches find_all(const EvalContext& env, const Snippet& snippet) const;
	};
}
