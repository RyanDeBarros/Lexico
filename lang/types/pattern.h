#pragma once

#include "declarations.h"
#include "basic.h"

#include <memory>
#include <unordered_map>

namespace lx
{
	class SubpatternNode;

	using NodeConvertMap = std::unordered_map<const SubpatternNode*, SubpatternNode*>;

	class SubpatternNode
	{
	public:
		virtual ~SubpatternNode() = default;

	protected:
		virtual SubpatternNode& clone(NodeConvertMap& conv, std::vector<std::unique_ptr<SubpatternNode>>& arena) const = 0;

	public:
		SubpatternNode& refer_node(NodeConvertMap& conv, std::vector<std::unique_ptr<SubpatternNode>>& arena) const;
	};

	class SubpatternArray : public SubpatternNode
	{
		std::vector<SubpatternNode*> _array;

	public:
		void append(SubpatternNode& node);

	protected:
		virtual SubpatternNode& clone(NodeConvertMap& conv, std::vector<std::unique_ptr<SubpatternNode>>& arena) const override;
		const std::vector<SubpatternNode*>& array() const;
	};

	class SubpatternChar : public SubpatternNode
	{
		char _ch;

	public:
		SubpatternChar(char ch);

	protected:
		SubpatternNode& clone(NodeConvertMap& conv, std::vector<std::unique_ptr<SubpatternNode>>& arena) const override;
	};

	class SubpatternString : public SubpatternNode
	{
		std::string _string;

	public:
		SubpatternString(const std::string& string);
		SubpatternString(std::string&& string);

	protected:
		SubpatternNode& clone(NodeConvertMap& conv, std::vector<std::unique_ptr<SubpatternNode>>& arena) const override;
	};

	class SubpatternCatenation : public SubpatternArray
	{
	};

	class SubpatternDisjunction : public SubpatternArray
	{
	};

	class SubpatternException : public SubpatternNode
	{
		SubpatternNode* _exception;

	public:
		SubpatternException(SubpatternNode& exception);

	protected:
		SubpatternNode& clone(NodeConvertMap& conv, std::vector<std::unique_ptr<SubpatternNode>>& arena) const override;
	};

	class SubpatternRepetition : public SubpatternNode
	{
		SubpatternNode* _repeated;
		IRange _range;

	public:
		SubpatternRepetition(SubpatternNode& repeated, const IRange& range);
		SubpatternRepetition(SubpatternNode& repeated, PatternSimpleRepeatOperator op);

	protected:
		SubpatternNode& clone(NodeConvertMap& conv, std::vector<std::unique_ptr<SubpatternNode>>& arena) const override;
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
		LookaroundMode _mode;

	public:
		SubpatternLookaround(LookaroundMode mode);

	protected:
		SubpatternNode& clone(NodeConvertMap& conv, std::vector<std::unique_ptr<SubpatternNode>>& arena) const override;
	};

	class SubpatternOptional : public SubpatternNode
	{
		SubpatternNode* _optional;

	public:
		SubpatternOptional(SubpatternNode& optional);

	protected:
		SubpatternNode& clone(NodeConvertMap& conv, std::vector<std::unique_ptr<SubpatternNode>>& arena) const override;
	};

	class SubpatternBackRef : public SubpatternNode
	{
		CapId _capid;

	public:
		SubpatternBackRef(CapId capid);

	protected:
		SubpatternNode& clone(NodeConvertMap& conv, std::vector<std::unique_ptr<SubpatternNode>>& arena) const override;
	};

	class SubpatternCapture : public SubpatternNode
	{
		CapId _capid;
		SubpatternNode* _captured;

	public:
		SubpatternCapture(CapId capid, SubpatternNode& captured);

	protected:
		SubpatternNode& clone(NodeConvertMap& conv, std::vector<std::unique_ptr<SubpatternNode>>& arena) const override;
	};

	class SubpatternLazy : public SubpatternNode
	{
		SubpatternNode* _lazy;

	public:
		SubpatternLazy(SubpatternNode& lazy);

	protected:
		SubpatternNode& clone(NodeConvertMap& conv, std::vector<std::unique_ptr<SubpatternNode>>& arena) const override;
	};

	class Pattern : public SubpatternArray
	{
		std::vector<std::unique_ptr<SubpatternNode>> _subnodes;

	public:
		Pattern();
		Pattern(const Pattern& other);
		Pattern(Pattern&&) noexcept = default;
		Pattern& operator=(const Pattern& other);
		Pattern& operator=(Pattern&&) noexcept = default;

		static DataType data_type();
		TypeVariant cast_copy(const DataType& type) const;
		TypeVariant cast_move(const DataType& type);
		void print(std::stringstream& ss) const;

		static Pattern make_from_symbol(BuiltinSymbol symbol);
		static Pattern make_repeat(Pattern&& pattern, const IRange& range);
		static Pattern make_backref(const CapId& capid);
		static Pattern make_lazy(Pattern&& pattern);
		static Pattern make_capture(Pattern&& pattern, const CapId& capid);

	private:
		void impl_add(std::unique_ptr<SubpatternNode>&& node);

	public:
		template<std::derived_from<SubpatternNode> T>
		T& add(std::unique_ptr<T>&& node)
		{
			T* ptr = node.get();
			impl_add(std::move(node));
			return *ptr;
		}

		void append_pattern(Pattern&& pattern);
		Pattern& add(Pattern&& pattern);
	};
}
