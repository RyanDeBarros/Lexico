#include "find.h"

#include "util.h"
#include "runtime.h"
#include "types/evalcontext.h"
#include "types/primitives/include.h"

namespace lx
{
	SearchState::SearchState(size_t start)
		: start(start), pos(start)
	{
	}

	size_t SearchState::hash() const
	{
		size_t h = 0;
		h = hash_combine(h, std::hash<size_t>{}(start));
		h = hash_combine(h, std::hash<size_t>{}(pos));
		for (const auto& [capid, list] : *caps)
		{
			h = hash_combine(h, capid.hash());
			h = hash_combine(h, list.hash());
		}
		return h;
	}

	Match SearchState::materialize(const EvalContext& env, const Snippet& snippet) &&
	{
		Match match(snippet, start, pos - start);
		for (auto& [capid, list] : *caps)
			for (auto& capture : std::move(list).materialize(env, snippet))
				match.add_capture(env, CapId(capid), std::move(capture));
		return match;
	}

	CaptureFrame::CaptureFrame(const SearchState& substate)
		: start(substate.start), length(substate.pos - substate.start), substate(substate)
	{
	}

	size_t CaptureFrame::hash() const
	{
		size_t h = 0;
		h = hash_combine(h, std::hash<size_t>{}(start));
		h = hash_combine(h, std::hash<size_t>{}(length));
		h = hash_combine(h, substate.hash());
		return h;
	}

	size_t CaptureList::hash() const
	{
		size_t h = 0;
		h = hash_combine(h, capid.hash());
		for (const auto& cap : frames)
			h = hash_combine(h, cap.hash());
		return h;
	}

	std::vector<Cap> CaptureList::materialize(const EvalContext& env, const Snippet& snippet) &&
	{
		std::vector<Cap> caps;
		for (auto& cap : frames)
			caps.emplace_back(env, snippet, static_cast<unsigned int>(cap.start), static_cast<unsigned int>(cap.length),
				env.runtime.unbound_variable(std::move(cap.substate).materialize(env, snippet)));
		return caps;
	}

	static std::vector<SearchState> remove_duplicates(std::vector<SearchState>&& outcomes)
	{
		std::unordered_set<SearchState> seen;
		std::vector<SearchState> unique;
		for (SearchState& state : outcomes)
		{
			if (!seen.contains(state))
			{
				seen.insert(state);
				unique.push_back(std::move(state));
			}
		}
		return unique;
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

	std::vector<SearchState> SubpatternChar::branches(const SearchContext& context, const SearchState& in) const
	{
		if (in.pos >= context.text.size())
			return {};

		if (context.text[in.pos] != _ch)
			return {};

		SearchState out = in;
		++out.pos;
		return { std::move(out) };
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

	std::vector<SearchState> SubpatternString::branches(const SearchContext& context, const SearchState& in) const
	{
		if (in.pos >= context.text.size())
			return {};

		if (in.pos + _string.size() > context.text.size())
			return {};

		if (context.text.substr(in.pos, _string.size()) != _string)
			return {};

		SearchState out = in;
		out.pos += _string.size();
		return { std::move(out) };
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

	std::vector<SearchState> SubpatternMarker::branches(const SearchContext& context, const SearchState& in) const
	{
		switch (_marker)
		{
		case PatternMark::Any:
		{
			if (in.pos >= context.text.size())
				return {};

			SearchState out = in;
			++out.pos;
			return { std::move(out) };
		}
		case PatternMark::Start:
		{
			if (in.pos != 0)
				return {};
			else
				return { in };
		}
		case PatternMark::End:
		{
			if (in.pos != context.text.size())
				return {};
			else
				return { in };
		}
		default:
			return {};
		}
	}

	SubpatternNode& SubpatternCatenation::clone(NodeConvertMap& conv, std::vector<std::unique_ptr<SubpatternNode>>& arena) const
	{
		auto& node = clone_base<SubpatternCatenation>(this, conv, arena);
		clone_array(node, conv, arena);
		return node;
	}

	std::vector<SearchState> SubpatternCatenation::branches(const SearchContext& context, const SearchState& in) const
	{
		std::vector<SearchState> frontier;
		frontier.push_back(in);
		for (const SubpatternNode* element : _array)
		{
			std::vector<SearchState> new_frontier;
			for (const SearchState& state : frontier)
			{
				std::vector<SearchState> intermediate = element->branches(context, state);
				new_frontier.insert(new_frontier.end(), std::make_move_iterator(intermediate.begin()), std::make_move_iterator(intermediate.end()));
			}
			frontier = std::move(new_frontier);
		}

		return remove_duplicates(std::move(frontier));
	}

	SubpatternNode& SubpatternDisjunction::clone(NodeConvertMap& conv, std::vector<std::unique_ptr<SubpatternNode>>& arena) const
	{
		auto& node = clone_base<SubpatternDisjunction>(this, conv, arena);
		clone_array(node, conv, arena);
		return node;
	}

	std::vector<SearchState> SubpatternDisjunction::branches(const SearchContext& context, const SearchState& in) const
	{
		std::vector<SearchState> outcomes;

		if (context.greedy)
		{
			for (auto it = _array.begin(); it != _array.end(); ++it)
			{
				std::vector<SearchState> result = (*it)->branches(context, in);
				outcomes.insert(outcomes.end(), std::make_move_iterator(result.begin()), std::make_move_iterator(result.end()));
			}
		}
		else
		{
			for (auto it = _array.rbegin(); it != _array.rend(); ++it)
			{
				std::vector<SearchState> result = (*it)->branches(context, in);
				outcomes.insert(outcomes.end(), std::make_move_iterator(result.begin()), std::make_move_iterator(result.end()));
			}
		}

		return remove_duplicates(std::move(outcomes));
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

	std::vector<SearchState> SubpatternException::branches(const SearchContext& context, const SearchState& in) const
	{
		std::vector<SearchState> outcomes = _subject->branches(context, in);
		std::vector<SearchState> without = _exception->branches(context, in);
		std::unordered_set<SearchState> forbidden(std::make_move_iterator(without.begin()), std::make_move_iterator(without.end()));

		std::vector<SearchState> keep;
		for (SearchState& outcome : outcomes)
			if (!forbidden.contains(outcome))
				keep.push_back(std::move(outcome));
		return keep;
	}

	SubpatternRepetition::SubpatternRepetition(SubpatternNode& subject, const IRange& range)
		: _subject(&subject), _range(range)
	{
	}

	std::vector<SearchState> SubpatternRepetition::branches(const SearchContext& context, const SearchState& in) const
	{
		const int min = _range.min() ? *_range.min() : 0;
		const int max = _range.max() ? *_range.max() : context.text.size() - in.pos;
		if (min > max)
			return {};

		std::vector<SearchState> outcomes;
		if (context.greedy)
		{
			for (int reps = max; reps >= min; --reps)
			{
				std::vector<SearchState> result = repeated(context, in, reps);
				outcomes.insert(outcomes.end(), std::make_move_iterator(result.begin()), std::make_move_iterator(result.end()));
			}
		}
		else
		{
			for (int reps = min; reps <= max; ++reps)
			{
				std::vector<SearchState> result = repeated(context, in, reps);
				outcomes.insert(outcomes.end(), std::make_move_iterator(result.begin()), std::make_move_iterator(result.end()));
			}
		}

		return remove_duplicates(std::move(outcomes));
	}

	std::vector<SearchState> SubpatternRepetition::repeated(const SearchContext& context, const SearchState& in, const int reps) const
	{
		std::vector<SearchState> frontier;
		frontier.push_back(in);
		for (size_t i = 0; i < reps; ++i)
		{
			std::vector<SearchState> new_frontier;
			for (const SearchState& state : frontier)
			{
				std::vector<SearchState> intermediate = _subject->branches(context, state);
				new_frontier.insert(new_frontier.end(), std::make_move_iterator(intermediate.begin()), std::make_move_iterator(intermediate.end()));
			}
			frontier = std::move(new_frontier);
		}
		return frontier;
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

	std::vector<SearchState> SubpatternLookaround::branches(const SearchContext& context, const SearchState& in) const
	{
		// TODO
		return { in };
	}

	SubpatternOptional::SubpatternOptional(SubpatternNode& optional)
		: _optional(&optional)
	{
	}

	std::vector<SearchState> SubpatternOptional::branches(const SearchContext& context, const SearchState& in) const
	{
		std::vector<SearchState> outcomes = _optional->branches(context, in);
		if (context.greedy)
			outcomes.push_back(in);
		else
			outcomes.insert(outcomes.begin(), in);
		return outcomes;
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

	std::vector<SearchState> SubpatternCapture::branches(const SearchContext& context, const SearchState& in) const
	{
		std::vector<SearchState> inner = _captured->branches(context, in);
		std::vector<SearchState> result;

		for (SearchState& state : inner)
		{
			if (!state.caps->contains(_capid))
			{
				SearchState substate = state;
				substate.start = in.pos;
				state.caps->insert(std::make_pair(_capid, CaptureList{ .capid = _capid, .frames = { CaptureFrame(substate) } }));
				result.push_back(std::move(state));
			}
		}

		return result;
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

	std::vector<SearchState> SubpatternBackRef::branches(const SearchContext& context, const SearchState& in) const
	{
		SearchState out = in;
		auto it = out.caps->find(_capid);
		if (it != out.caps->end())
		{
			CaptureList& caplist = it->second;
			const CaptureFrame& initial_frame = caplist.frames[0];
			std::string_view sv = context.text.substr(initial_frame.start, initial_frame.length);
			if (in.pos + sv.size() <= context.text.size())
			{
				if (context.text.substr(in.pos, sv.size()) == sv)
				{
					SearchState substate = initial_frame.substate;
					substate.start = in.pos;
					substate.pos = in.pos + sv.size();
					caplist.frames.push_back(CaptureFrame(substate));
					return { std::move(out) };
				}
			}
		}

		return {};
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

	std::vector<SearchState> SubpatternLazy::branches(const SearchContext& context, const SearchState& in) const
	{
		SearchContext ctx = context;
		ctx.greedy = false;
		return _lazy->branches(ctx, in);
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

	std::vector<SearchState> SubpatternGreedy::branches(const SearchContext& context, const SearchState& in) const
	{
		SearchContext ctx = context;
		ctx.greedy = true;
		return _greedy->branches(ctx, in);
	}


}
