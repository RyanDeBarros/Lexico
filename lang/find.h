#pragma once

#include "types/primitives/capid.h"
#include "types/primitives/irange.h"
#include "page.h"

#include <unordered_map>

namespace lx
{
	struct CaptureList;

	struct SearchState
	{
		size_t start = 0;
		size_t pos = 0;

		CowPtr<std::unordered_map<CapId, CaptureList>> caps;

		SearchState(size_t start);

		size_t hash() const;
		bool operator==(const SearchState&) const = default;

		Match materialize(const EvalContext& env, const Snippet& snippet)&&;
	};

	struct CaptureFrame
	{
		size_t start;
		size_t length;
		SearchState substate;

		CaptureFrame(const SearchState& substate);

		size_t hash() const;
		bool operator==(const CaptureFrame&) const = default;
	};

	struct CaptureList
	{
		CapId capid;
		std::vector<CaptureFrame> frames;

		size_t hash() const;
		bool operator==(const CaptureList&) const = default;

		std::vector<Cap> materialize(const EvalContext& env, const Snippet& snippet) &&;
	};

	struct SearchContext
	{
		const EvalContext& env;
		const std::string_view text;
		bool greedy = true;
	};

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

		virtual std::vector<SearchState> branches(const SearchContext& context, const SearchState& in) const = 0;
	};

	class SubpatternArray : public SubpatternNode
	{
	protected:
		std::vector<SubpatternNode*> _array;

	public:
		SubpatternArray() = default;
		SubpatternArray(std::vector<SubpatternNode*>&& array);

		virtual bool equals(const SubpatternNode* o) const override;
		void append(SubpatternNode& node);

	protected:
		void clone_array(SubpatternArray& into, NodeConvertMap& conv, std::vector<std::unique_ptr<SubpatternNode>>& arena) const;
	};

	class SubpatternChar : public SubpatternNode
	{
		char _ch;

	public:
		SubpatternChar(char ch);

		SubpatternNode& clone(NodeConvertMap& conv, std::vector<std::unique_ptr<SubpatternNode>>& arena) const override;
		bool equals(const SubpatternNode* o) const override;
		char chr() const;

		std::vector<SearchState> branches(const SearchContext& context, const SearchState& in) const override;
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

		std::vector<SearchState> branches(const SearchContext& context, const SearchState& in) const override;
	};

	enum class PatternMark
	{
		Any,
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

		std::vector<SearchState> branches(const SearchContext& context, const SearchState& in) const override;
	};

	class SubpatternCatenation : public SubpatternArray
	{
	public:
		SubpatternNode& clone(NodeConvertMap& conv, std::vector<std::unique_ptr<SubpatternNode>>& arena) const override;
		std::vector<SearchState> branches(const SearchContext& context, const SearchState& in) const override;
	};

	class SubpatternDisjunction : public SubpatternArray
	{
	public:
		SubpatternNode& clone(NodeConvertMap& conv, std::vector<std::unique_ptr<SubpatternNode>>& arena) const override;
		std::vector<SearchState> branches(const SearchContext& context, const SearchState& in) const override;
	};

	class SubpatternException : public SubpatternNode
	{
		SubpatternNode* _subject;
		SubpatternNode* _exception;

	public:
		SubpatternException(SubpatternNode& subject, SubpatternNode& exception);

		SubpatternNode& clone(NodeConvertMap& conv, std::vector<std::unique_ptr<SubpatternNode>>& arena) const override;
		bool equals(const SubpatternNode* o) const override;

		std::vector<SearchState> branches(const SearchContext& context, const SearchState& in) const override;
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

		std::vector<SearchState> branches(const SearchContext& context, const SearchState& in) const override;

	private:
		std::vector<SearchState> repeated(const SearchContext& context, const SearchState& in, const int reps) const;
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

		std::vector<SearchState> branches(const SearchContext& context, const SearchState& in) const override;
	};

	class SubpatternOptional : public SubpatternNode
	{
		SubpatternNode* _optional;

	public:
		SubpatternOptional(SubpatternNode& optional);

		SubpatternNode& clone(NodeConvertMap& conv, std::vector<std::unique_ptr<SubpatternNode>>& arena) const override;
		bool equals(const SubpatternNode* o) const override;

		std::vector<SearchState> branches(const SearchContext& context, const SearchState& in) const override;
	};

	class SubpatternCapture : public SubpatternNode
	{
		CapId _capid;
		SubpatternNode* _captured;

	public:
		SubpatternCapture(CapId capid, SubpatternNode& captured);

		SubpatternNode& clone(NodeConvertMap& conv, std::vector<std::unique_ptr<SubpatternNode>>& arena) const override;
		bool equals(const SubpatternNode* o) const override;

		std::vector<SearchState> branches(const SearchContext& context, const SearchState& in) const override;
	};

	class SubpatternBackRef : public SubpatternNode
	{
		CapId _capid;

	public:
		SubpatternBackRef(CapId capid);

		SubpatternNode& clone(NodeConvertMap& conv, std::vector<std::unique_ptr<SubpatternNode>>& arena) const override;
		bool equals(const SubpatternNode* o) const override;

		std::vector<SearchState> branches(const SearchContext& context, const SearchState& in) const override;
	};

	class SubpatternLazy : public SubpatternNode
	{
		SubpatternNode* _lazy;

	public:
		SubpatternLazy(SubpatternNode& lazy);

		SubpatternNode& clone(NodeConvertMap& conv, std::vector<std::unique_ptr<SubpatternNode>>& arena) const override;
		bool equals(const SubpatternNode* o) const override;

		std::vector<SearchState> branches(const SearchContext& context, const SearchState& in) const override;
	};

	class SubpatternGreedy : public SubpatternNode
	{
		SubpatternNode* _greedy;

	public:
		SubpatternGreedy(SubpatternNode& greedy);

		SubpatternNode& clone(NodeConvertMap& conv, std::vector<std::unique_ptr<SubpatternNode>>& arena) const override;
		bool equals(const SubpatternNode* o) const override;

		std::vector<SearchState> branches(const SearchContext& context, const SearchState& in) const override;
	};
}

template<>
struct std::hash<lx::SearchState>
{
	size_t operator()(const lx::SearchState&) const;
};
