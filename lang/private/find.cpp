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
		for (unsigned int i = 0; i < caps->size(); ++i)
		{
			std::optional<CaptureList>& list = (*caps)[i];
			if (list)
				for (Cap& capture : std::move(*list).materialize(env, snippet, context))
					match.add_capture(env, context.from_local(i), std::move(capture));
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

	SearchYield::SearchYield(SearchExit policy)
		: policy(policy)
	{
	}

	SearchExit SearchYield::operator()(SearchState state)
	{
		final_states.push_back(std::move(state));
		return policy;
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

	IRange SubpatternNode::matching_range() const
	{
		if (!_matching_range)
			_matching_range = impl_matching_range();
		return *_matching_range;
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

	SearchExit SubpatternChar::match(const SearchContext& context, const SearchState& in, MatchYield& yield) const
	{
		if (in.pos >= context.text.size())
			return SearchExit::Continue;

		if (context.text[in.pos] != _ch)
			return SearchExit::Continue;

		SearchState out = in;
		++out.pos;
		return yield(std::move(out));
	}

	IRange SubpatternChar::impl_matching_range() const
	{
		return IRange(1, 1);
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

	SearchExit SubpatternString::match(const SearchContext& context, const SearchState& in, MatchYield& yield) const
	{
		if (in.pos >= context.text.size())
			return SearchExit::Continue;

		if (in.pos + _string.size() > context.text.size())
			return SearchExit::Continue;

		if (context.text.substr(in.pos, _string.size()) != _string)
			return SearchExit::Continue;

		SearchState out = in;
		out.pos += _string.size();
		return yield(std::move(out));
	}

	IRange SubpatternString::impl_matching_range() const
	{
		int sz = static_cast<int>(_string.size());
		return IRange(sz, sz);
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

	SearchExit SubpatternMarker::match(const SearchContext& context, const SearchState& in, MatchYield& yield) const
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
		return SearchExit::Continue;
	}

	IRange SubpatternMarker::impl_matching_range() const
	{
		switch (_marker)
		{
		case PatternMark::Any:
			return IRange(1, 1);
		case PatternMark::Start:
			return IRange(0, 0);
		case PatternMark::End:
			return IRange(0, 0);
		default:
			return IRange(0, 0);
		}
	}

	SubpatternNode& SubpatternCatenation::clone(NodeConvertMap& conv, std::vector<std::unique_ptr<SubpatternNode>>& arena) const
	{
		auto& node = clone_base<SubpatternCatenation>(this, conv, arena);
		clone_array(node, conv, arena);
		return node;
	}
	
	SearchExit SubpatternCatenation::match(const SearchContext& context, const SearchState& in, MatchYield& yield) const
	{
		return match_from(0, context, in, yield);
	}

	IRange SubpatternCatenation::impl_matching_range() const
	{
		int total_min = 0;
		std::optional<int> total_max = 0;

		for (const SubpatternNode* subnode : _array)
		{
			IRange range = subnode->matching_range();
			if (range.min())
				total_min += *range.min();
			if (range.max())
			{
				if (total_max)
					*total_max += *range.max();
			}
			else
				total_max = std::nullopt;
		}
		return IRange(total_min, total_max);
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

		SearchExit operator()(SearchState state) override
		{
			return node.match_from(index + 1, context, state, downstream);
		}
	};

	SearchExit SubpatternCatenation::match_from(size_t index, const SearchContext& context, const SearchState& in, MatchYield& yield) const
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

	SearchExit SubpatternDisjunction::match(const SearchContext& context, const SearchState& in, MatchYield& yield) const
	{
		if (context.greedy)
		{
			for (auto it = _array.begin(); it != _array.end(); ++it)
				if ((*it)->match(context, in, yield) == SearchExit::Terminate)
					return SearchExit::Terminate;
		}
		else
		{
			for (auto it = _array.rbegin(); it != _array.rend(); ++it)
				if ((*it)->match(context, in, yield) == SearchExit::Terminate)
					return SearchExit::Terminate;
		}
		return SearchExit::Continue;
	}

	IRange SubpatternDisjunction::impl_matching_range() const
	{
		std::optional<int> min = std::nullopt;
		std::optional<int> max = 0;

		for (const SubpatternNode* subnode : _array)
		{
			IRange range = subnode->matching_range();
			if (range.min())
			{
				if (min)
					*min = std::min(*min, *range.min());
				else
					min = *range.min();
			}
			if (range.max())
			{
				if (max)
					*max = std::max(*max, *range.max());
			}
			else
				max = std::nullopt;
		}
		return IRange(min ? *min : 0, max);
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
		SearchExit operator()(SearchState) override
		{
			return SearchExit::Terminate;
		}
	};

	SearchExit SubpatternException::match(const SearchContext& context, const SearchState& in, MatchYield& yield) const
	{
		ProbeYield probe;
		if (_exception->match(context, in, probe) == SearchExit::Continue)
			return _subject->match(context, in, yield);
		else
			return SearchExit::Continue;
	}

	IRange SubpatternException::impl_matching_range() const
	{
		return _subject->matching_range();
	}

	SubpatternRepetition::SubpatternRepetition(SubpatternNode& subject, const IRange& range)
		: _subject(&subject), _range(range)
	{
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

	SearchExit SubpatternRepetition::match(const SearchContext& context, const SearchState& in, MatchYield& yield) const
	{
		const int left = context.text.size() - in.pos;
		const IRange bounds = matching_range();
		if (bounds.min() && *bounds.min() > left)
			return SearchExit::Continue;

		const IRange subbounds = _subject->matching_range();
		const int submin = subbounds.min() ? *subbounds.min() : 0;
		const int fullmax = bounds.max() ? std::min(*bounds.max(), left) : left;
		const int absmax = submin > 0 ? (fullmax + submin - 1) / submin : left;
		
		const int min = _range.min() ? *_range.min() : 0;
		const int max = std::min(absmax, _range.max() ? *_range.max() : left);

		if (min > max)
			return SearchExit::Continue;

		if (context.greedy)
		{
			for (int reps = max; reps >= min; --reps)
				if (match_next(context, in, yield, reps) == SearchExit::Terminate)
					return SearchExit::Terminate;
		}
		else
		{
			for (int reps = min; reps <= max; ++reps)
				if (match_next(context, in, yield, reps) == SearchExit::Terminate)
					return SearchExit::Terminate;
		}
		return SearchExit::Continue;
	}

	IRange SubpatternRepetition::impl_matching_range() const
	{
		IRange range = _subject->matching_range();
		if (range.min())
		{
			if (_range.min())
				*range.min() *= *_range.min();
			else
				*range.min() = 0;
		}
		if (range.max())
		{
			if (_range.max())
				*range.max() *= *_range.max();
			else
				range.max() = std::nullopt;
		}
		return range;
	}

	struct RepeatYield : public MatchYield
	{
		const SubpatternRepetition& node;
		const SearchContext& context;
		int reps_left;
		size_t previous_pos;
		MatchYield& downstream;

		RepeatYield(const SubpatternRepetition& node, const SearchContext& context, size_t previous_pos, int reps_left, MatchYield& downstream)
			: node(node), context(context), reps_left(reps_left), previous_pos(previous_pos), downstream(downstream)
		{
		}

		SearchExit operator()(SearchState state) override
		{
			if (state.pos > previous_pos)
				return node.match_next(context, state, downstream, reps_left - 1);
			else
				return SearchExit::Continue;
		}
	};

	SearchExit SubpatternRepetition::match_next(const SearchContext& context, const SearchState& in, MatchYield& yield, const int reps_left) const
	{
		if (reps_left <= 0)
			return yield(in);

		RepeatYield repeat(*this, context, in.pos, reps_left, yield);
		return _subject->match(context, in, repeat);
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

		SearchExit operator()(SearchState state) override
		{
			if (state.pos < end)
				return SearchExit::Continue;

			exact = state.pos == end;
			return SearchExit::Terminate;
		}
	};

	SearchExit SubpatternLookaround::match(const SearchContext& context, const SearchState& in, MatchYield& yield) const
	{
		switch (_mode)
		{
		case LookaroundMode::Ahead:
		{
			ProbeYield probe;
			if (_subject->match(context, in, probe) == SearchExit::Terminate)
				return yield(in);
			break;
		}
		case LookaroundMode::NotAhead:
		{
			ProbeYield probe;
			if (_subject->match(context, in, probe) == SearchExit::Continue)
				return yield(in);
			break;
		}
		case LookaroundMode::Behind:
		{
			SearchContext subcontext = context.up_to(in.pos);
			const IRange bounds = _subject->matching_range();
			for (int i = in.pos - (bounds.min() ? *bounds.min() : 0); i >= in.pos - (bounds.max() ? *bounds.max() : in.pos); --i)
			{
				BackSearchYield back_search(i);
				_subject->match(subcontext, SearchState(i), back_search);
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
			SearchContext subcontext = context.up_to(in.pos);
			const IRange bounds = _subject->matching_range();
			for (int i = in.pos - (bounds.min() ? *bounds.min() : 0); i >= in.pos - (bounds.max() ? *bounds.max() : in.pos); --i)
			{
				BackSearchYield back_search(i);
				_subject->match(subcontext, SearchState(i), back_search);
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
		return SearchExit::Continue;
	}

	IRange SubpatternLookaround::impl_matching_range() const
	{
		if (_mode == LookaroundMode::Ahead)
			return _subject->matching_range();
		else
			return IRange(0, 0);
	}

	SubpatternOptional::SubpatternOptional(SubpatternNode& optional)
		: _optional(&optional)
	{
	}

	SearchExit SubpatternOptional::match(const SearchContext& context, const SearchState& in, MatchYield& yield) const
	{
		if (context.greedy)
			return (_optional->match(context, in, yield) == SearchExit::Terminate || yield(in) == SearchExit::Terminate)
				? SearchExit::Terminate : SearchExit::Continue;
		else
			return (yield(in) == SearchExit::Terminate || _optional->match(context, in, yield) == SearchExit::Terminate)
				? SearchExit::Terminate : SearchExit::Continue;
	}

	IRange SubpatternOptional::impl_matching_range() const
	{
		return IRange(0, _optional->matching_range().max());
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

		SearchExit operator()(SearchState state) override
		{
			SearchState substate = state;
			substate.start = start;

			if (capid >= state.caps->size())
				state.caps->resize(static_cast<size_t>(capid + 1));

			std::optional<CaptureList>& list = (*state.caps)[capid];

			if (list)
				list->frames.push_back(CaptureFrame(substate));
			else
				list = CaptureList{ .frames = { CaptureFrame(substate) } };

			return downstream(std::move(state));
		}
	};

	SearchExit SubpatternCapture::match(const SearchContext& context, const SearchState& in, MatchYield& yield) const
	{
		CaptureYield upstream(context, _capid, in.pos, yield);
		return _captured->match(context, in, upstream);
	}

	IRange SubpatternCapture::impl_matching_range() const
	{
		return _captured->matching_range();
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

	SearchExit SubpatternBackRef::match(const SearchContext& context, const SearchState& in, MatchYield& yield) const
	{
		if (!context.capid_exists(_capid))
			return SearchExit::Continue;

		unsigned int capid = context.local_capid(_capid);
		if (capid >= in.caps->size() || !(*in.caps)[capid].has_value())
			return SearchExit::Continue;

		SearchState out = in;
		CaptureList& caplist = (*out.caps)[capid].value();
		const CaptureFrame& last_frame = caplist.frames.back();
		std::string_view sv = context.text.substr(last_frame.start, last_frame.length);
		if (in.pos + sv.size() > context.text.size())
			return SearchExit::Continue;

		if (context.text.substr(in.pos, sv.size()) != sv)
			return SearchExit::Continue;

		SearchState substate = last_frame.substate;
		substate.start = in.pos;
		substate.pos = in.pos + sv.size();
		caplist.frames.push_back(CaptureFrame(substate));
		return yield(std::move(out));
	}

	IRange SubpatternBackRef::impl_matching_range() const
	{
		return IRange(0, std::nullopt);
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

	SearchExit SubpatternLazy::match(const SearchContext& context, const SearchState& in, MatchYield& yield) const
	{
		SearchContext ctx = context;
		ctx.greedy = false;
		return _lazy->match(ctx, in, yield);
	}

	IRange SubpatternLazy::impl_matching_range() const
	{
		return IRange(0, 0);
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

	SearchExit SubpatternGreedy::match(const SearchContext& context, const SearchState& in, MatchYield& yield) const
	{
		SearchContext ctx = context;
		ctx.greedy = true;
		return _greedy->match(ctx, in, yield);
	}

	IRange SubpatternGreedy::impl_matching_range() const
	{
		return IRange(0, 0);
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

	SearchExit SubpatternSRange::match(const SearchContext& context, const SearchState& in, MatchYield& yield) const
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
		return SearchExit::Continue;
	}

	IRange SubpatternSRange::impl_matching_range() const
	{
		if (_range.empty())
			return IRange(0, 0);
		else
			return IRange(1, 1);
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

	SearchExit SubpatternBuiltin::match(const SearchContext& context, const SearchState& in, MatchYield& yield) const
	{
		if (in.pos >= context.text.size())
			return SearchExit::Continue;
		
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
			return SearchExit::Continue;
	}

	IRange SubpatternBuiltin::impl_matching_range() const
	{
		switch (_type)
		{
		case BuiltinSubpattern::Newline:
			return IRange(1, 2);
		case BuiltinSubpattern::Space:
			return IRange(1, 1);
		case BuiltinSubpattern::Varname:
			return IRange(1, 1);
		case BuiltinSubpattern::Whitespace:
			return IRange(1, 2);
		default:
			return IRange(0, 0);
		}
	}
}
