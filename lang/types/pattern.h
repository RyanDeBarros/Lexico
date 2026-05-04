#pragma once

#include "declarations.h"
#include "basic.h"

#include <memory>

namespace lx
{
	class SubpatternNode;

	using NodeConvertMap = std::unordered_map<const SubpatternNode*, SubpatternNode*>;

	class SubpatternNode
	{
	public:
		virtual ~SubpatternNode() = default;

		virtual SubpatternNode& clone(NodeConvertMap& conv, std::vector<std::unique_ptr<SubpatternNode>>& arena) const = 0;
	};

	class SubpatternArray : public SubpatternNode
	{
		std::vector<SubpatternNode*> _array;

	public:
		void append(SubpatternNode& node);

		virtual SubpatternNode& clone(NodeConvertMap& conv, std::vector<std::unique_ptr<SubpatternNode>>& arena) const override;

	protected:
		const std::vector<SubpatternNode*>& array() const;
	};

	class SubpatternRoot : public SubpatternArray
	{
	public:
		SubpatternNode& clone(NodeConvertMap& conv, std::vector<std::unique_ptr<SubpatternNode>>& arena) const override;
		std::unique_ptr<SubpatternRoot> clone_root(std::vector<std::unique_ptr<SubpatternNode>>& arena) const;
	};

	class SubpatternString : public SubpatternNode
	{
		std::string _string;

	public:
		SubpatternString(const std::string& string);
		SubpatternString(std::string&& string);

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

		SubpatternNode& clone(NodeConvertMap& conv, std::vector<std::unique_ptr<SubpatternNode>>& arena) const override;
	};

	class SubpatternRepetition : public SubpatternNode
	{
		IRange _range;

	public:
		SubpatternRepetition(const IRange& range);
		SubpatternRepetition(PatternSimpleRepeatOperator op);

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

		SubpatternNode& clone(NodeConvertMap& conv, std::vector<std::unique_ptr<SubpatternNode>>& arena) const override;
	};

	class SubpatternOptional : public SubpatternNode
	{
		SubpatternNode* _optional;

	public:
		SubpatternOptional(SubpatternNode& optional);

		SubpatternNode& clone(NodeConvertMap& conv, std::vector<std::unique_ptr<SubpatternNode>>& arena) const override;
	};

	class SubpatternBackRef : public SubpatternNode
	{
		CapId _capid;

	public:
		SubpatternBackRef(CapId capid);

		SubpatternNode& clone(NodeConvertMap& conv, std::vector<std::unique_ptr<SubpatternNode>>& arena) const override;
	};

	class SubpatternCapture : public SubpatternNode
	{
		CapId _capid;
		SubpatternNode* _captured;

	public:
		SubpatternCapture(CapId capid, SubpatternNode& captured);

		SubpatternNode& clone(NodeConvertMap& conv, std::vector<std::unique_ptr<SubpatternNode>>& arena) const override;
	};

	class SubpatternLazy : public SubpatternNode
	{
		SubpatternNode* _lazy;

	public:
		SubpatternLazy(SubpatternNode& lazy);

		SubpatternNode& clone(NodeConvertMap& conv, std::vector<std::unique_ptr<SubpatternNode>>& arena) const override;
	};

	class Pattern
	{
		std::vector<std::unique_ptr<SubpatternNode>> _subnodes;
		std::unique_ptr<SubpatternRoot> _root;

	public:
		Pattern();
		Pattern(const Pattern& other);
		Pattern(Pattern&& other) noexcept = default;
		Pattern& operator=(const Pattern& other);
		Pattern& operator=(Pattern&& other) noexcept = default;

		const SubpatternRoot& root() const;
		SubpatternRoot& root();

		TypeVariant cast_copy(DataType type) const;
		TypeVariant cast_move(DataType type);

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

		void append(Pattern&& pattern);
	};
}
