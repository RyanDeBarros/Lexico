#include "find.h"

#include "runtime.h"
#include "types/evalcontext.h"
#include "types/primitives/include.h"

namespace lx
{
	SearchState::SearchState(size_t start)
		: start(start), pos(start)
	{
	}

	Match SearchState::materialize(const EvalContext& env, const Snippet& snippet, const SearchContext& context) &&
	{
		Match match(snippet, start, pos - start);
		for (auto& list : *caps)
		{
			if (list)
				for (Cap& capture : std::move(*list).materialize(env, snippet, context))
					match.add_capture(env, context.from_local(list->capid), std::move(capture));
		}
		return match;
	}

	CaptureFrame::CaptureFrame(const SearchState& substate)
		: start(substate.start), length(substate.pos - substate.start), substate(substate)
	{
	}

	std::vector<Cap> CaptureList::materialize(const EvalContext& env, const Snippet& snippet, const SearchContext& context) &&
	{
		std::vector<Cap> caps;
		for (CaptureFrame& cap : frames)
			caps.emplace_back(env, snippet, static_cast<unsigned int>(cap.start), static_cast<unsigned int>(cap.length),
				env.runtime.unbound_variable(std::move(cap.substate).materialize(env, snippet, context)));
		return caps;
	}

	SearchContext::SearchContext(const EvalContext& env, const std::string_view text)
		: env(env), text(text), greedy(true), local_capids(std::make_shared<std::unordered_map<CapId, unsigned int>>()),
		reverse_lut(std::make_shared<std::unordered_map<unsigned int, CapId>>())
	{
	}

	SearchContext SearchContext::up_to(size_t pos) const
	{
		SearchContext ctx(env, text.substr(0, pos));
		ctx.greedy = greedy;
		ctx.local_capids = local_capids;
		ctx.reverse_lut = reverse_lut;
		return ctx;
	}

	bool SearchContext::capid_exists(const CapId& capid) const
	{
		return local_capids->contains(capid);
	}

	unsigned int SearchContext::local_capid(const CapId& capid) const
	{
		auto it = local_capids->insert(std::make_pair(capid, static_cast<unsigned int>(local_capids->size())));
		if (it.second)
			reverse_lut->insert(std::make_pair(it.first->second, it.first->first));
		return it.first->second;
	}

	CapId SearchContext::from_local(unsigned int capid) const
	{
		auto it = reverse_lut->find(capid);
		if (it != reverse_lut->end())
			return it->second;
		else
		{
			std::stringstream ss;
			ss << __FUNCTION__ << ": local capid not found";
			throw LxError(ErrorType::Internal, ss.str());
		}
	}

	bool SearchYield::operator()(SearchState state)
	{
		final_states.push_back(std::move(state));
		return find_first;
	}

	template<std::derived_from<SubpatternNode> T, typename... Args>
	static T& clone_base(const T* from, NodeConvertMap& conv, std::vector<std::unique_ptr<SubpatternNode>>& arena, Args&&... args)
	{
		auto cloned = std::make_unique<T>(std::forward<Args>(args)...);
		T& base = *cloned;
		arena.push_back(std::move(cloned));
		conv[from] = &base;
		return base;
	}

	SubpatternNode& SubpatternNode::refer_node(NodeConvertMap& conv, std::vector<std::unique_ptr<SubpatternNode>>& arena) const
	{
		auto it = conv.find(this);
		if (it != conv.end())
			return *it->second;
		else
			return clone(conv, arena);
	}

	SubpatternArray::SubpatternArray(std::vector<SubpatternNode*>&& array)
		: _array(std::move(array))
	{
	}

	bool SubpatternArray::equals(const SubpatternNode* o) const
	{
		if (auto ptr = dynamic_cast<const SubpatternArray*>(o))
		{
			if (_array.size() != ptr->_array.size())
				return false;

			for (size_t i = 0; i < _array.size(); ++i)
				if (!_array[i]->equals(ptr->_array[i]))
					return false;

			return true;
		}
		else if (_array.size() == 1)
			return _array[0]->equals(o);
		else
			return false;
	}

	void SubpatternArray::append(SubpatternNode& node)
	{
		_array.push_back(&node);
	}

	void SubpatternArray::clone_array(SubpatternArray& into, NodeConvertMap& conv, std::vector<std::unique_ptr<SubpatternNode>>& arena) const
	{
		for (const SubpatternNode* el : _array)
			into.append(el->refer_node(conv, arena));
	}

	SubpatternChar::SubpatternChar(char ch)
		: _ch(ch)
	{
	}

	SubpatternNode& SubpatternChar::clone(NodeConvertMap& conv, std::vector<std::unique_ptr<SubpatternNode>>& arena) const
	{
		return clone_base<SubpatternChar>(this, conv, arena, _ch);
	}

	bool SubpatternChar::equals(const SubpatternNode* o) const
	{
		if (auto ptr = dynamic_cast<const SubpatternChar*>(o))
			return _ch == ptr->_ch;
		else if (auto ptr = dynamic_cast<const SubpatternString*>(o))
			return ptr->string().size() == 1 && ptr->string()[0] == _ch;
		else
			return false;
	}

	char SubpatternChar::chr() const
	{
		return _ch;
	}

	bool SubpatternChar::match(const SearchContext& context, const SearchState& in, MatchYield& yield) const
	{
		if (in.pos >= context.text.size())
			return false;

		if (context.text[in.pos] != _ch)
			return false;

		SearchState out = in;
		++out.pos;
		return yield(std::move(out));
	}

	SubpatternString::SubpatternString(const std::string& string)
		: _string(string)
	{
	}

	SubpatternString::SubpatternString(std::string&& string)
		: _string(std::move(string))
	{
	}

	SubpatternNode& SubpatternString::clone(NodeConvertMap& conv, std::vector<std::unique_ptr<SubpatternNode>>& arena) const
	{
		return clone_base<SubpatternString>(this, conv, arena, _string);
	}

	bool SubpatternString::equals(const SubpatternNode* o) const
	{
		if (auto ptr = dynamic_cast<const SubpatternString*>(o))
			return _string == ptr->_string;
		else if (auto ptr = dynamic_cast<const SubpatternChar*>(o))
			return _string.size() == 1 && _string[0] == ptr->chr();
		else
			return false;
	}

	std::string_view SubpatternString::string() const
	{
		return _string;
	}

	bool SubpatternString::match(const SearchContext& context, const SearchState& in, MatchYield& yield) const
	{
		if (in.pos >= context.text.size())
			return false;

		if (in.pos + _string.size() > context.text.size())
			return false;

		if (context.text.substr(in.pos, _string.size()) != _string)
			return false;

		SearchState out = in;
		out.pos += _string.size();
		return yield(std::move(out));
	}

	SubpatternMarker::SubpatternMarker(PatternMark marker)
		: _marker(marker)
	{
	}

	SubpatternNode& SubpatternMarker::clone(NodeConvertMap& conv, std::vector<std::unique_ptr<SubpatternNode>>& arena) const
	{
		return clone_base<SubpatternMarker>(this, conv, arena, _marker);
	}

	bool SubpatternMarker::equals(const SubpatternNode* o) const
	{
		if (auto ptr = dynamic_cast<const SubpatternMarker*>(o))
			return _marker == ptr->_marker;
		else
			return false;
	}

	bool SubpatternMarker::match(const SearchContext& context, const SearchState& in, MatchYield& yield) const
	{
		switch (_marker)
		{
		case PatternMark::Any:
		{
			if (in.pos < context.text.size())
			{
				SearchState out = in;
				++out.pos;
				return yield(std::move(out));
			}
			break;
		}
		case PatternMark::Start:
		{
			if (in.pos == 0)
				return yield(in);
			break;
		}
		case PatternMark::End:
		{
			if (in.pos == context.text.size())
				return yield(in);
			break;
		}
		}
		return false;
	}

	SubpatternNode& SubpatternCatenation::clone(NodeConvertMap& conv, std::vector<std::unique_ptr<SubpatternNode>>& arena) const
	{
		auto& node = clone_base<SubpatternCatenation>(this, conv, arena);
		clone_array(node, conv, arena);
		return node;
	}
	
	bool SubpatternCatenation::match(const SearchContext& context, const SearchState& in, MatchYield& yield) const
	{
		return match_from(0, context, in, yield);
	}

	struct SubsequentYield : public MatchYield
	{
		const SubpatternCatenation& node;
		const SearchContext& context;
		size_t index;
		MatchYield& downstream;

		SubsequentYield(const SubpatternCatenation& node, const SearchContext& context, size_t index, MatchYield& downstream)
			: node(node), context(context), index(index), downstream(downstream)
		{
		}

		bool operator()(SearchState state) override
		{
			return node.match_from(index + 1, context, state, downstream);
		}
	};

	bool SubpatternCatenation::match_from(size_t index, const SearchContext& context, const SearchState& in, MatchYield& yield) const
	{
		if (index >= _array.size())
			return yield(in);

		SubsequentYield subseq(*this, context, index, yield);
		return _array[index]->match(context, in, subseq);
	}

	SubpatternNode& SubpatternDisjunction::clone(NodeConvertMap& conv, std::vector<std::unique_ptr<SubpatternNode>>& arena) const
	{
		auto& node = clone_base<SubpatternDisjunction>(this, conv, arena);
		clone_array(node, conv, arena);
		return node;
	}

	bool SubpatternDisjunction::match(const SearchContext& context, const SearchState& in, MatchYield& yield) const
	{
		if (context.greedy)
		{
			for (auto it = _array.begin(); it != _array.end(); ++it)
				if ((*it)->match(context, in, yield))
					return true;
		}
		else
		{
			for (auto it = _array.rbegin(); it != _array.rend(); ++it)
				if ((*it)->match(context, in, yield))
					return true;
		}
		return false;
	}

	SubpatternException::SubpatternException(SubpatternNode& subject, SubpatternNode& exception)
		: _subject(&subject), _exception(&exception)
	{
	}

	SubpatternNode& SubpatternException::clone(NodeConvertMap& conv, std::vector<std::unique_ptr<SubpatternNode>>& arena) const
	{
		return clone_base<SubpatternException>(this, conv, arena, _subject->refer_node(conv, arena), _exception->refer_node(conv, arena));
	}

	bool SubpatternException::equals(const SubpatternNode* o) const
	{
		if (auto ptr = dynamic_cast<const SubpatternException*>(o))
			return _subject->equals(ptr->_subject) && _exception->equals(ptr->_exception);
		else
			return false;
	}

	struct ProbeYield : public MatchYield
	{
		bool operator()(SearchState) override
		{
			return true;
		}
	};

	bool SubpatternException::match(const SearchContext& context, const SearchState& in, MatchYield& yield) const
	{
		ProbeYield probe;
		if (!_exception->match(context, in, probe))
			return _subject->match(context, in, yield);
		else
			return false;
	}

	SubpatternRepetition::SubpatternRepetition(SubpatternNode& subject, const IRange& range)
		: _subject(&subject), _range(range)
	{
	}

	bool SubpatternRepetition::match(const SearchContext& context, const SearchState& in, MatchYield& yield) const
	{
		const int min = _range.min() ? *_range.min() : 0;
		const int max = _range.max() ? *_range.max() : context.text.size() - in.pos;
		if (min > max)
			return false;

		if (context.greedy)
		{
			for (int reps = max; reps >= min; --reps)
				if (match_next(context, in, yield, reps))
					return true;
		}
		else
		{
			for (int reps = min; reps <= max; ++reps)
				if (match_next(context, in, yield, reps))
					return true;
		}
		return false;
	}

	struct RepeatYield : public MatchYield
	{
		const SubpatternRepetition& node;
		const SearchContext& context;
		int reps_left;
		MatchYield& downstream;

		RepeatYield(const SubpatternRepetition& node, const SearchContext& context, int reps_left, MatchYield& downstream)
			: node(node), context(context), reps_left(reps_left), downstream(downstream)
		{
		}

		bool operator()(SearchState state) override
		{
			return node.match_next(context, state, downstream, reps_left - 1);
		}
	};

	bool SubpatternRepetition::match_next(const SearchContext& context, const SearchState& in, MatchYield& yield, const int reps_left) const
	{
		if (reps_left <= 0)
			return yield(in);

		RepeatYield repeat(*this, context, reps_left, yield);
		return _subject->match(context, in, repeat);
	}

	static IRange simple_repeat_range(PatternSimpleRepeatOperator op)
	{
		switch (op)
		{
		case PatternSimpleRepeatOperator::Asterisk:
			return IRange(0, std::nullopt);
		case PatternSimpleRepeatOperator::Plus:
			return IRange(1, std::nullopt);
		default:
		{
			std::stringstream ss;
			ss << "unrecognized simple repeat operator \"" << static_cast<int>(op) << "\"";
			throw LxError(ErrorType::Internal, ss.str());
		}
		}
	}

	SubpatternRepetition::SubpatternRepetition(SubpatternNode& subject, PatternSimpleRepeatOperator op)
		: _subject(&subject), _range(simple_repeat_range(op))
	{
	}

	SubpatternNode& SubpatternRepetition::clone(NodeConvertMap& conv, std::vector<std::unique_ptr<SubpatternNode>>& arena) const
	{
		return clone_base<SubpatternRepetition>(this, conv, arena, _subject->refer_node(conv, arena), _range);
	}

	bool SubpatternRepetition::equals(const SubpatternNode* o) const
	{
		if (auto ptr = dynamic_cast<const SubpatternRepetition*>(o))
			return _range == ptr->_range && _subject->equals(ptr->_subject);
		else
			return false;
	}

	LookaroundMode lookaround_mode(PrefixOperator op)
	{
		switch (op)
		{
		case PrefixOperator::Ahead:
			return LookaroundMode::Ahead;
		case PrefixOperator::NotAhead:
			return LookaroundMode::NotAhead;
		case PrefixOperator::Behind:
			return LookaroundMode::Behind;
		case PrefixOperator::NotBehind:
			return LookaroundMode::NotBehind;
		default:
		{
			std::stringstream ss;
			ss << "could not convert prefix operator \"" << static_cast<int>(op) << "\" to lookaround mode";
			throw LxError(ErrorType::Internal, ss.str());
		}
		}
	}

	SubpatternLookaround::SubpatternLookaround(LookaroundMode mode, SubpatternNode& subject)
		: _mode(mode), _subject(&subject)
	{
	}

	SubpatternNode& SubpatternLookaround::clone(NodeConvertMap& conv, std::vector<std::unique_ptr<SubpatternNode>>& arena) const
	{
		return clone_base<SubpatternLookaround>(this, conv, arena, _mode, _subject->refer_node(conv, arena));
	}

	bool SubpatternLookaround::equals(const SubpatternNode* o) const
	{
		if (auto ptr = dynamic_cast<const SubpatternLookaround*>(o))
			return _mode == ptr->_mode && _subject->equals(ptr->_subject);
		else
			return false;
	}
	
	struct BackSearchYield : public MatchYield
	{
		size_t end;
		bool exact = false;

		BackSearchYield(size_t end) : end(end) {}

		bool operator()(SearchState state) override
		{
			if (state.pos < end)
				return false;

			exact = state.pos == end;
			return true;
		}
	};

	bool SubpatternLookaround::match(const SearchContext& context, const SearchState& in, MatchYield& yield) const
	{
		switch (_mode)
		{
		case LookaroundMode::Ahead:
		{
			ProbeYield probe;
			if (_subject->match(context, in, probe))
				return yield(in);
			break;
		}
		case LookaroundMode::NotAhead:
		{
			ProbeYield probe;
			if (!_subject->match(context, in, probe))
				return yield(in);
			break;
		}
		case LookaroundMode::Behind:
		{
			// TODO virtual max_range() function that calculate the lower bound of where iteration index should end, instead of always going to 0
			for (int i = in.pos; i >= 0; --i)
			{
				BackSearchYield back_search(i);
				_subject->match(context.up_to(i), SearchState(i), back_search);
				if (back_search.exact)
				{
					yield(in);
					break;
				}
			}
			break;
		}
		case LookaroundMode::NotBehind:
		{
			bool pass = true;
			for (int i = in.pos; i >= 0; --i)
			{
				BackSearchYield back_search(i);
				_subject->match(context.up_to(i), SearchState(i), back_search);
				if (back_search.exact)
				{
					pass = false;
					break;
				}
			}
			if (pass)
				yield(in);
			break;
		}
		}
		return false;
	}

	SubpatternOptional::SubpatternOptional(SubpatternNode& optional)
		: _optional(&optional)
	{
	}

	bool SubpatternOptional::match(const SearchContext& context, const SearchState& in, MatchYield& yield) const
	{
		if (context.greedy)
			return _optional->match(context, in, yield) || yield(in);
		else
			return yield(in) || _optional->match(context, in, yield);
	}

	SubpatternNode& SubpatternOptional::clone(NodeConvertMap& conv, std::vector<std::unique_ptr<SubpatternNode>>& arena) const
	{
		return clone_base<SubpatternOptional>(this, conv, arena, _optional->refer_node(conv, arena));
	}

	bool SubpatternOptional::equals(const SubpatternNode* o) const
	{
		if (auto ptr = dynamic_cast<const SubpatternOptional*>(o))
			return _optional->equals(ptr->_optional);
		else
			return false;
	}

	SubpatternCapture::SubpatternCapture(CapId capid, SubpatternNode& captured)
		: _capid(capid), _captured(&captured)
	{
	}

	SubpatternNode& SubpatternCapture::clone(NodeConvertMap& conv, std::vector<std::unique_ptr<SubpatternNode>>& arena) const
	{
		return clone_base<SubpatternCapture>(this, conv, arena, _capid, _captured->refer_node(conv, arena));
	}

	bool SubpatternCapture::equals(const SubpatternNode* o) const
	{
		if (auto ptr = dynamic_cast<const SubpatternCapture*>(o))
			return _capid == ptr->_capid && _captured->equals(ptr->_captured);
		else
			return false;
	}

	struct CaptureYield : public MatchYield
	{
		unsigned int capid;
		size_t start;
		MatchYield& downstream;

		CaptureYield(const SearchContext& context, CapId capid, size_t start, MatchYield& downstream)
			: capid(context.local_capid(capid)), start(start), downstream(downstream)
		{
		}

		bool operator()(SearchState state) override
		{
			if (capid >= state.caps->size() || !(*state.caps)[capid].has_value())
			{
				SearchState substate = state;
				substate.start = start;

				if (capid >= state.caps->size())
					state.caps->resize(static_cast<size_t>(capid + 1));

				(*state.caps)[capid] = CaptureList{ .capid = capid, .frames = { CaptureFrame(substate) } };
				return downstream(std::move(state));
			}
			else
				return false;
		}
	};

	bool SubpatternCapture::match(const SearchContext& context, const SearchState& in, MatchYield& yield) const
	{
		CaptureYield upstream(context, _capid, in.pos, yield);
		return _captured->match(context, in, upstream);
	}

	SubpatternBackRef::SubpatternBackRef(CapId capid)
		: _capid(capid)
	{
	}

	SubpatternNode& SubpatternBackRef::clone(NodeConvertMap& conv, std::vector<std::unique_ptr<SubpatternNode>>& arena) const
	{
		return clone_base<SubpatternBackRef>(this, conv, arena, _capid);
	}

	bool SubpatternBackRef::equals(const SubpatternNode* o) const
	{
		if (auto ptr = dynamic_cast<const SubpatternBackRef*>(o))
			return _capid == ptr->_capid;
		else
			return false;
	}

	bool SubpatternBackRef::match(const SearchContext& context, const SearchState& in, MatchYield& yield) const
	{
		if (!context.capid_exists(_capid))
			return false;

		unsigned int capid = context.local_capid(_capid);
		if (capid >= in.caps->size() || !(*in.caps)[capid].has_value())
			return false;

		SearchState out = in;
		CaptureList& caplist = (*out.caps)[capid].value();
		// TODO v0.2 if caplist is recursive -> recompute instead of matching initial frame exactly
		const CaptureFrame& initial_frame = caplist.frames[0];
		std::string_view sv = context.text.substr(initial_frame.start, initial_frame.length);
		if (in.pos + sv.size() > context.text.size())
			return false;

		if (context.text.substr(in.pos, sv.size()) != sv)
			return false;

		SearchState substate = initial_frame.substate;
		substate.start = in.pos;
		substate.pos = in.pos + sv.size();
		caplist.frames.push_back(CaptureFrame(substate));
		return yield(std::move(out));
	}

	SubpatternLazy::SubpatternLazy(SubpatternNode& lazy)
		: _lazy(&lazy)
	{
	}

	SubpatternNode& SubpatternLazy::clone(NodeConvertMap& conv, std::vector<std::unique_ptr<SubpatternNode>>& arena) const
	{
		return clone_base<SubpatternLazy>(this, conv, arena, _lazy->refer_node(conv, arena));
	}

	bool SubpatternLazy::equals(const SubpatternNode* o) const
	{
		if (auto ptr = dynamic_cast<const SubpatternLazy*>(o))
			return _lazy->equals(ptr->_lazy);
		else
			return false;
	}

	bool SubpatternLazy::match(const SearchContext& context, const SearchState& in, MatchYield& yield) const
	{
		SearchContext ctx = context;
		ctx.greedy = false;
		return _lazy->match(ctx, in, yield);
	}

	SubpatternGreedy::SubpatternGreedy(SubpatternNode& greedy)
		: _greedy(&greedy)
	{
	}

	SubpatternNode& SubpatternGreedy::clone(NodeConvertMap& conv, std::vector<std::unique_ptr<SubpatternNode>>& arena) const
	{
		return clone_base<SubpatternGreedy>(this, conv, arena, _greedy->refer_node(conv, arena));
	}

	bool SubpatternGreedy::equals(const SubpatternNode* o) const
	{
		if (auto ptr = dynamic_cast<const SubpatternGreedy*>(o))
			return _greedy->equals(ptr->_greedy);
		else
			return false;
	}

	bool SubpatternGreedy::match(const SearchContext& context, const SearchState& in, MatchYield& yield) const
	{
		SearchContext ctx = context;
		ctx.greedy = true;
		return _greedy->match(ctx, in, yield);
	}

	SubpatternSRange::SubpatternSRange(SRange range)
		: _range(std::move(range))
	{
	}

	SubpatternNode& SubpatternSRange::clone(NodeConvertMap& conv, std::vector<std::unique_ptr<SubpatternNode>>& arena) const
	{
		return clone_base<SubpatternSRange>(this, conv, arena, _range);
	}

	bool SubpatternSRange::equals(const SubpatternNode* o) const
	{
		if (auto ptr = dynamic_cast<const SubpatternSRange*>(o))
			return _range == ptr->_range;
		else
			return false;
	}

	bool SubpatternSRange::match(const SearchContext& context, const SearchState& in, MatchYield& yield) const
	{
		if (in.pos >= context.text.size())
		{
			if (_range.empty())
				return yield(in);
		}
		else if (_range.contains(context.text[in.pos]))
		{
			SearchState out = in;
			++out.pos;
			return yield(std::move(out));
		}
		return false;
	}

	SubpatternBuiltin::SubpatternBuiltin(BuiltinSubpattern type)
		: _type(type)
	{
	}

	SubpatternNode& SubpatternBuiltin::clone(NodeConvertMap& conv, std::vector<std::unique_ptr<SubpatternNode>>& arena) const
	{
		return clone_base<SubpatternBuiltin>(this, conv, arena, _type);
	}

	bool SubpatternBuiltin::equals(const SubpatternNode* o) const
	{
		if (auto ptr = dynamic_cast<const SubpatternBuiltin*>(o))
			return _type == ptr->_type;
		else
			return false;
	}

	bool SubpatternBuiltin::match(const SearchContext& context, const SearchState& in, MatchYield& yield) const
	{
		if (in.pos >= context.text.size())
			return false;
		
		const char c = context.text[in.pos];
		bool matches = false;

		switch (_type)
		{
		case BuiltinSubpattern::Newline:
			if (c == '\n')
				matches = true;
			else if (c == '\r')
			{
				if (in.pos + 1 < context.text.size() && context.text[in.pos + 1] == '\n')
				{
					SearchState out = in;
					out.pos += 2;
					return yield(std::move(out));
				}
				else
					matches = true;
			}
			break;
		case BuiltinSubpattern::Space:
			if (c == ' ' || c == '\t')
				matches = true;
			break;
		case BuiltinSubpattern::Varname:
			if (c == '_' || (c >= '0' && c <= '9') || (c >= 'a' && c <= 'z') || (c >= 'A' && c <= 'Z'))
				matches = true;
			break;
		case BuiltinSubpattern::Whitespace:
			if (c == ' ' || c == '\t' || c == '\n')
				matches = true;
			else if (c == '\r')
			{
				if (in.pos + 1 < context.text.size() && context.text[in.pos + 1] == '\n')
				{
					SearchState out = in;
					out.pos += 2;
					return yield(std::move(out));
				}
				else
					matches = true;
			}
			break;
		}

		if (matches)
		{
			SearchState out = in;
			++out.pos;
			return yield(std::move(out));
		}
		else
			return false;
	}
}
