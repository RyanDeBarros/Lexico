#pragma once

#include "types/primitives/capid.h"
#include "types/primitives/irange.h"
#include "types/primitives/srange.h"
#include "page.h"

#include <unordered_map>

namespace lx
{
	struct CaptureList;
	struct SearchContext;

	struct SearchState
	{
		size_t start = 0;
		size_t pos = 0;

		CowPtr<std::vector<std::optional<CaptureList>>> caps;

		SearchState(size_t start);

		Match materialize(const EvalContext& env, const Snippet& snippet, const SearchContext& context) &&;
	};

	struct CaptureFrame
	{
		size_t start;
		size_t length;
		SearchState substate;

		CaptureFrame(const SearchState& substate);
	};

	struct CaptureList
	{
		unsigned int capid;
		std::vector<CaptureFrame> frames;

		std::vector<Cap> materialize(const EvalContext& env, const Snippet& snippet, const SearchContext& context) &&;
	};

	struct SearchContext
	{
		const EvalContext& env;
		const std::string_view text;
		bool greedy = true;
		std::shared_ptr<std::unordered_map<CapId, unsigned int>> local_capids;
		std::shared_ptr<std::unordered_map<unsigned int, CapId>> reverse_lut;

		SearchContext(const EvalContext& env, const std::string_view text);

		SearchContext up_to(size_t pos) const;

		bool capid_exists(const CapId& capid) const;
		unsigned int local_capid(const CapId& capid) const;
		CapId from_local(unsigned int capid) const;
	};

	struct MatchYield
	{
		virtual ~MatchYield() = default;
		virtual bool operator()(SearchState state) = 0;
	};

	struct SearchYield : public MatchYield
	{
		std::vector<SearchState> final_states;
		bool find_first;

		SearchYield(bool find_first) : find_first(find_first) {}

		bool operator()(SearchState state) override;
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

		virtual bool match(const SearchContext& context, const SearchState& in, MatchYield& yield) const = 0;
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

		bool match(const SearchContext& context, const SearchState& in, MatchYield& yield) const override;
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

		bool match(const SearchContext& context, const SearchState& in, MatchYield& yield) const override;
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

		bool match(const SearchContext& context, const SearchState& in, MatchYield& yield) const override;
	};

	class SubpatternCatenation : public SubpatternArray
	{
	public:
		SubpatternNode& clone(NodeConvertMap& conv, std::vector<std::unique_ptr<SubpatternNode>>& arena) const override;
		bool match(const SearchContext& context, const SearchState& in, MatchYield& yield) const override;

		bool match_from(size_t index, const SearchContext& context, const SearchState& in, MatchYield& yield) const;
	};

	class SubpatternDisjunction : public SubpatternArray
	{
	public:
		SubpatternNode& clone(NodeConvertMap& conv, std::vector<std::unique_ptr<SubpatternNode>>& arena) const override;
		bool match(const SearchContext& context, const SearchState& in, MatchYield& yield) const override;
	};

	class SubpatternException : public SubpatternNode
	{
		SubpatternNode* _subject;
		SubpatternNode* _exception;

	public:
		SubpatternException(SubpatternNode& subject, SubpatternNode& exception);

		SubpatternNode& clone(NodeConvertMap& conv, std::vector<std::unique_ptr<SubpatternNode>>& arena) const override;
		bool equals(const SubpatternNode* o) const override;

		bool match(const SearchContext& context, const SearchState& in, MatchYield& yield) const override;
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

		bool match(const SearchContext& context, const SearchState& in, MatchYield& yield) const override;

		bool match_next(const SearchContext& context, const SearchState& in, MatchYield& yield, const int reps_left) const;
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

		bool match(const SearchContext& context, const SearchState& in, MatchYield& yield) const override;
	};

	class SubpatternOptional : public SubpatternNode
	{
		SubpatternNode* _optional;

	public:
		SubpatternOptional(SubpatternNode& optional);

		SubpatternNode& clone(NodeConvertMap& conv, std::vector<std::unique_ptr<SubpatternNode>>& arena) const override;
		bool equals(const SubpatternNode* o) const override;

		bool match(const SearchContext& context, const SearchState& in, MatchYield& yield) const override;
	};

	class SubpatternCapture : public SubpatternNode
	{
		CapId _capid;
		SubpatternNode* _captured;

	public:
		SubpatternCapture(CapId capid, SubpatternNode& captured);

		SubpatternNode& clone(NodeConvertMap& conv, std::vector<std::unique_ptr<SubpatternNode>>& arena) const override;
		bool equals(const SubpatternNode* o) const override;

		bool match(const SearchContext& context, const SearchState& in, MatchYield& yield) const override;
	};

	class SubpatternBackRef : public SubpatternNode
	{
		CapId _capid;

	public:
		SubpatternBackRef(CapId capid);

		SubpatternNode& clone(NodeConvertMap& conv, std::vector<std::unique_ptr<SubpatternNode>>& arena) const override;
		bool equals(const SubpatternNode* o) const override;

		bool match(const SearchContext& context, const SearchState& in, MatchYield& yield) const override;
	};

	class SubpatternLazy : public SubpatternNode
	{
		SubpatternNode* _lazy;

	public:
		SubpatternLazy(SubpatternNode& lazy);

		SubpatternNode& clone(NodeConvertMap& conv, std::vector<std::unique_ptr<SubpatternNode>>& arena) const override;
		bool equals(const SubpatternNode* o) const override;

		bool match(const SearchContext& context, const SearchState& in, MatchYield& yield) const override;
	};

	class SubpatternGreedy : public SubpatternNode
	{
		SubpatternNode* _greedy;

	public:
		SubpatternGreedy(SubpatternNode& greedy);

		SubpatternNode& clone(NodeConvertMap& conv, std::vector<std::unique_ptr<SubpatternNode>>& arena) const override;
		bool equals(const SubpatternNode* o) const override;

		bool match(const SearchContext& context, const SearchState& in, MatchYield& yield) const override;
	};

	class SubpatternSRange : public SubpatternNode
	{
		SRange _range;

	public:
		SubpatternSRange(SRange range);

		SubpatternNode& clone(NodeConvertMap& conv, std::vector<std::unique_ptr<SubpatternNode>>& arena) const override;
		bool equals(const SubpatternNode* o) const override;

		bool match(const SearchContext& context, const SearchState& in, MatchYield& yield) const override;
	};

	enum class BuiltinSubpattern
	{
		Newline,
		Space,
		Varname,
		Whitespace,
	};

	class SubpatternBuiltin : public SubpatternNode
	{
		BuiltinSubpattern _type;

	public:
		SubpatternBuiltin(BuiltinSubpattern type);

		SubpatternNode& clone(NodeConvertMap& conv, std::vector<std::unique_ptr<SubpatternNode>>& arena) const override;
		bool equals(const SubpatternNode* o) const override;

		bool match(const SearchContext& context, const SearchState& in, MatchYield& yield) const override;
	};
}
