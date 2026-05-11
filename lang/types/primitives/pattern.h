#pragma once

#include "types/declarations.h"
#include "capid.h"
#include "irange.h"

#include <unordered_map>

namespace lx
{
	class SubpatternNode;

	using NodeConvertMap = std::unordered_map<const SubpatternNode*, SubpatternNode*>;

	class SubpatternNode
	{
	public:
		SubpatternNode() = default;
		SubpatternNode(const SubpatternNode&) = delete;
		virtual ~SubpatternNode() = default;

		virtual SubpatternNode& clone(NodeConvertMap& conv, std::vector<std::unique_ptr<SubpatternNode>>& arena) const = 0;
		SubpatternNode& refer_node(NodeConvertMap& conv, std::vector<std::unique_ptr<SubpatternNode>>& arena) const;
		virtual bool equals(const SubpatternNode* o) const = 0;
	};

	class SubpatternArray : public SubpatternNode
	{
	protected:
		std::vector<SubpatternNode*> _array;

	public:
		SubpatternArray() = default;
		SubpatternArray(std::vector<SubpatternNode*>&& array);

		virtual SubpatternNode& clone(NodeConvertMap& conv, std::vector<std::unique_ptr<SubpatternNode>>& arena) const override;
		virtual bool equals(const SubpatternNode* o) const override;
		void append(SubpatternNode& node);
	};

	class SubpatternChar : public SubpatternNode
	{
		char _ch;

	public:
		SubpatternChar(char ch);

		SubpatternNode& clone(NodeConvertMap& conv, std::vector<std::unique_ptr<SubpatternNode>>& arena) const override;
		bool equals(const SubpatternNode* o) const override;
		char chr() const;
	};

	class SubpatternString : public SubpatternNode
	{
		std::string _string;

	public:
		SubpatternString(const std::string& string);
		SubpatternString(std::string&& string);

		SubpatternNode& clone(NodeConvertMap& conv, std::vector<std::unique_ptr<SubpatternNode>>& arena) const override;
		bool equals(const SubpatternNode* o) const override;
		std::string_view string() const;
	};

	enum class PatternMark
	{
		Any,
		Cap,
		End,
		Start
	};

	class SubpatternMarker : public SubpatternNode
	{
		PatternMark _marker;

	public:
		SubpatternMarker(PatternMark marker);

		SubpatternNode& clone(NodeConvertMap& conv, std::vector<std::unique_ptr<SubpatternNode>>& arena) const override;
		bool equals(const SubpatternNode* o) const override;
	};

	class SubpatternCatenation : public SubpatternArray
	{
	};

	class SubpatternDisjunction : public SubpatternArray
	{
	};

	class SubpatternException : public SubpatternNode
	{
		SubpatternNode* _subject;
		SubpatternNode* _exception;

	public:
		SubpatternException(SubpatternNode& subject, SubpatternNode& exception);

		SubpatternNode& clone(NodeConvertMap& conv, std::vector<std::unique_ptr<SubpatternNode>>& arena) const override;
		bool equals(const SubpatternNode* o) const override;
	};

	class SubpatternRepetition : public SubpatternNode
	{
		SubpatternNode* _subject;
		IRange _range;

	public:
		SubpatternRepetition(SubpatternNode& subject, const IRange& range);
		SubpatternRepetition(SubpatternNode& subject, PatternSimpleRepeatOperator op);

		SubpatternNode& clone(NodeConvertMap& conv, std::vector<std::unique_ptr<SubpatternNode>>& arena) const override;
		bool equals(const SubpatternNode* o) const override;
	};

	enum class LookaroundMode
	{
		Ahead,
		Behind,
		NotAhead,
		NotBehind,
	};

	extern LookaroundMode lookaround_mode(PrefixOperator op);

	class SubpatternLookaround : public SubpatternNode
	{
		SubpatternNode* _subject;
		LookaroundMode _mode;

	public:
		SubpatternLookaround(LookaroundMode mode, SubpatternNode& subject);

		SubpatternNode& clone(NodeConvertMap& conv, std::vector<std::unique_ptr<SubpatternNode>>& arena) const override;
		bool equals(const SubpatternNode* o) const override;
	};

	class SubpatternOptional : public SubpatternNode
	{
		SubpatternNode* _optional;

	public:
		SubpatternOptional(SubpatternNode& optional);

		SubpatternNode& clone(NodeConvertMap& conv, std::vector<std::unique_ptr<SubpatternNode>>& arena) const override;
		bool equals(const SubpatternNode* o) const override;
	};

	class SubpatternBackRef : public SubpatternNode
	{
		CapId _capid;

	public:
		SubpatternBackRef(CapId capid);

		SubpatternNode& clone(NodeConvertMap& conv, std::vector<std::unique_ptr<SubpatternNode>>& arena) const override;
		bool equals(const SubpatternNode* o) const override;
	};

	class SubpatternCapture : public SubpatternNode
	{
		CapId _capid;
		SubpatternNode* _captured;

	public:
		SubpatternCapture(CapId capid, SubpatternNode& captured);

		SubpatternNode& clone(NodeConvertMap& conv, std::vector<std::unique_ptr<SubpatternNode>>& arena) const override;
		bool equals(const SubpatternNode* o) const override;
	};

	class SubpatternLazy : public SubpatternNode
	{
		SubpatternNode* _lazy;

	public:
		SubpatternLazy(SubpatternNode& lazy);

		SubpatternNode& clone(NodeConvertMap& conv, std::vector<std::unique_ptr<SubpatternNode>>& arena) const override;
		bool equals(const SubpatternNode* o) const override;
	};

	class Pattern
	{
		std::vector<std::unique_ptr<SubpatternNode>> _subnodes;
		SubpatternNode* _root = nullptr;

	public:
		Pattern();
		Pattern(const Pattern& other);
		Pattern(Pattern&& other) noexcept;
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

		Matches find_all(const std::string_view text) const;
	};
}
