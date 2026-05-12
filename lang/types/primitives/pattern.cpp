#include "pattern.h"

#include "include.h"
#include "runtime.h"

namespace lx
{
	SearchState::SearchState(size_t start)
		: start(start), pos(start)
	{
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
				return { in }; // TODO lots of copies of SearchStates being made throughout. Use CowPtr<std::vector<CaptureFrame>> for caps?
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
		// TODO use greedy bit?
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
		return frontier;
	}

	SubpatternNode& SubpatternDisjunction::clone(NodeConvertMap& conv, std::vector<std::unique_ptr<SubpatternNode>>& arena) const
	{
		auto& node = clone_base<SubpatternDisjunction>(this, conv, arena);
		clone_array(node, conv, arena);
		return node;
	}

	std::vector<SearchState> SubpatternDisjunction::branches(const SearchContext& context, const SearchState& in) const
	{
		// TODO use greedy bit?
		std::vector<SearchState> outcomes;
		for (const SubpatternNode* choice : _array)
		{
			std::vector<SearchState> result = choice->branches(context, in);
			outcomes.insert(outcomes.end(), std::make_move_iterator(result.begin()), std::make_move_iterator(result.end()));
		}
		return outcomes;
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
		// TODO
		return { in };
	}

	SubpatternRepetition::SubpatternRepetition(SubpatternNode& subject, const IRange& range)
		: _subject(&subject), _range(range)
	{
	}

	std::vector<SearchState> SubpatternRepetition::branches(const SearchContext& context, const SearchState& in) const
	{
		// TODO
		return { in };
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
		// TODO
		return { in };
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
		// TODO
		return { in };
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
		// TODO
		return { in };
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

	Pattern::Pattern(const Pattern& other)
	{
		if (other._root)
		{
			NodeConvertMap conv;
			_root = &other._root->clone(conv, _subnodes);
		}
	}

	Pattern::Pattern(Pattern&& other) noexcept
		: _subnodes(std::move(other._subnodes)), _root(other._root)
	{
		other._root = nullptr;
	}

	Pattern& Pattern::operator=(const Pattern& other)
	{
		if (this != &other)
		{
			_subnodes.clear();
			NodeConvertMap conv;
			if (other._root)
				_root = &other._root->clone(conv, _subnodes);
			else
				_root = nullptr;
		}
		return *this;
	}

	Pattern& Pattern::operator=(Pattern&& other) noexcept
	{
		if (this != &other)
		{
			_subnodes = std::move(other._subnodes);
			_root = other._root;
			other._root = nullptr;
		}
		return *this;
	}

	DataType Pattern::data_type()
	{
		return DataType::Pattern();
	}

	TypeVariant Pattern::cast_copy(const VarContext& ctx, const DataType& type) const
	{
		if (type.simple() == SimpleType::Pattern)
			return Pattern(*this);
		else if (type.simple() == SimpleType::Void)
			return Void();
		else
			ctx.env.throw_bad_cast(DataType::Pattern(), type);
	}

	TypeVariant Pattern::cast_move(VarContext&& ctx, const DataType& type) &&
	{
		(void*)this; // ignore const warning
		if (type.simple() == SimpleType::Pattern)
			return Pattern(std::move(*this));
		else
			return cast_copy(ctx, type);
	}

	void Pattern::print(const EvalContext& env, std::stringstream& ss) const
	{
		// TODO v0.2 string representation of pattern
		ss << DataType::Pattern();
	}

	Variable Pattern::data_member(VarContext& ctx, const std::string_view member) const
	{
		ctx.throw_no_data_member(member);
	}

	Variable Pattern::invoke_method(VarContext& ctx, const std::string_view method, std::vector<Variable>&& args) const
	{
		ctx.throw_no_method(method, args);
	}

	void Pattern::assign(const EvalContext& env, Pattern&& o)
	{
		*this = std::move(o);
	}

	bool Pattern::equals(const EvalContext& env, const Pattern& o) const
	{
		if (_root)
			return o._root && _root->equals(o._root);
		else
			return !o._root;
	}

	Pattern Pattern::make_from_symbol(BuiltinSymbol symbol)
	{
		Pattern ptn;
		switch (symbol)
		{
		case BuiltinSymbol::Alphanumeric:
		{
			auto& sub = ptn.make_root<SubpatternDisjunction>();
			for (int i = 'a'; i <= 'z'; ++i)
				sub.append(ptn.make_node<SubpatternChar>(i));
			for (int i = 'A'; i <= 'Z'; ++i)
				sub.append(ptn.make_node<SubpatternChar>(i));
			for (int i = '0'; i <= '9'; ++i)
				sub.append(ptn.make_node<SubpatternChar>(i));
			break;
		}
		case BuiltinSymbol::Digit:
		{
			auto& sub = ptn.make_root<SubpatternDisjunction>();
			for (int i = '0'; i <= '9'; ++i)
				sub.append(ptn.make_node<SubpatternChar>(i));
			break;
		}
		case BuiltinSymbol::Letter:
		{
			auto& sub = ptn.make_root<SubpatternDisjunction>();
			for (int i = 'a'; i <= 'z'; ++i)
				sub.append(ptn.make_node<SubpatternChar>(i));
			for (int i = 'A'; i <= 'Z'; ++i)
				sub.append(ptn.make_node<SubpatternChar>(i));
			break;
		}
		case BuiltinSymbol::Lowercase:
		{
			auto& sub = ptn.make_root<SubpatternDisjunction>();
			for (int i = 'a'; i <= 'z'; ++i)
				sub.append(ptn.make_node<SubpatternChar>(i));
			break;
		}
		case BuiltinSymbol::Newline:
		{
			auto& sub = ptn.make_root<SubpatternDisjunction>();
			sub.append(ptn.make_node<SubpatternString>("\r\n"));
			sub.append(ptn.make_node<SubpatternChar>('\n'));
			sub.append(ptn.make_node<SubpatternChar>('\r'));
			break;
		}
		case BuiltinSymbol::Space:
		{
			auto& sub = ptn.make_root<SubpatternDisjunction>();
			sub.append(ptn.make_node<SubpatternChar>(' '));
			sub.append(ptn.make_node<SubpatternChar>('\t'));
			break;
		}
		case BuiltinSymbol::Uppercase:
		{
			auto& sub = ptn.make_root<SubpatternDisjunction>();
			for (int i = 'A'; i <= 'Z'; ++i)
				sub.append(ptn.make_node<SubpatternChar>(i));
			break;
		}
		case BuiltinSymbol::Varname:
		{
			auto& sub = ptn.make_root<SubpatternDisjunction>();
			for (int i = 'a'; i <= 'z'; ++i)
				sub.append(ptn.make_node<SubpatternChar>(i));
			for (int i = 'A'; i <= 'Z'; ++i)
				sub.append(ptn.make_node<SubpatternChar>(i));
			for (int i = '0'; i <= '9'; ++i)
				sub.append(ptn.make_node<SubpatternChar>(i));
			sub.append(ptn.make_node<SubpatternChar>('_'));
			break;
		}
		case BuiltinSymbol::Whitespace:
		{
			auto& sub = ptn.make_root<SubpatternDisjunction>();
			sub.append(ptn.make_node<SubpatternChar>(' '));
			sub.append(ptn.make_node<SubpatternChar>('\t'));
			sub.append(ptn.make_node<SubpatternString>("\r\n"));
			sub.append(ptn.make_node<SubpatternChar>('\n'));
			sub.append(ptn.make_node<SubpatternChar>('\r'));
			break;
		}
		case BuiltinSymbol::Any:
			ptn.make_root<SubpatternMarker>(PatternMark::Any);
			break;
		case BuiltinSymbol::End:
			ptn.make_root<SubpatternMarker>(PatternMark::End);
			break;
		case BuiltinSymbol::Start:
			ptn.make_root<SubpatternMarker>(PatternMark::Start);
			break;
		default:
		{
			std::stringstream ss;
			ss << __FUNCTION__ << ": unrecognized pattern symbol " << static_cast<int>(symbol);
			throw LxError(ErrorType::Internal, ss.str());
		}
		}
		return ptn;
	}

	Pattern Pattern::make_repeat(Pattern&& pattern, const IRange& range)
	{
		Pattern ptn;
		SubpatternNode& subptn = ptn.take(std::move(pattern));
		ptn.make_root<SubpatternRepetition>(subptn, range);
		return ptn;
	}

	Pattern Pattern::make_backref(const CapId& capid)
	{
		Pattern ptn;
		ptn.make_root<SubpatternBackRef>(capid);
		return ptn;
	}

	Pattern Pattern::make_lazy(Pattern&& pattern)
	{
		Pattern ptn;
		SubpatternNode& subptn = ptn.take(std::move(pattern));
		ptn.make_root<SubpatternLazy>(subptn);
		return ptn;
	}

	Pattern Pattern::make_greedy(Pattern&& pattern)
	{
		Pattern ptn;
		SubpatternNode& subptn = ptn.take(std::move(pattern));
		ptn.make_root<SubpatternGreedy>(subptn);
		return ptn;
	}

	Pattern Pattern::make_capture(Pattern&& pattern, const CapId& capid)
	{
		Pattern ptn;
		SubpatternNode& subptn = ptn.take(std::move(pattern));
		ptn.make_root<SubpatternCapture>(capid, subptn);
		return ptn;
	}

	void Pattern::impl_own(std::unique_ptr<SubpatternNode>&& node)
	{
		_subnodes.push_back(std::move(node));
	}

	SubpatternNode& Pattern::take(Pattern&& pattern)
	{
		_subnodes.insert(_subnodes.end(), std::make_move_iterator(pattern._subnodes.begin()), std::make_move_iterator(pattern._subnodes.end()));
		pattern._subnodes.clear();
		SubpatternNode* root = pattern._root;
		pattern._root = nullptr;
		if (root)
			return *root;
		else
			throw LxError(ErrorType::Internal, "cannot take over pattern with null root");
	}

	void Pattern::set_root(std::unique_ptr<SubpatternNode>&& node)
	{
		SubpatternNode& root = own(std::move(node));
		if (_root)
			throw LxError(ErrorType::Internal, "pattern root already set");
		else
			_root = &root;
	}

	void Pattern::append(Pattern&& pattern)
	{
		if (!_root)
		{
			_subnodes = std::move(pattern._subnodes);
			_root = pattern._root;
			pattern._root = nullptr;
		}
		else if (pattern._root)
		{
			SubpatternNode* lhs_root = _root;
			auto& cat = make_node<SubpatternCatenation>();
			cat.append(*lhs_root);
			cat.append(take(std::move(pattern)));
			_root = &cat;
		}
	}

	Matches Pattern::search(const EvalContext& env, const Snippet& snippet) const
	{
		Matches matches;
		if (_root)
		{
			SearchContext context{ .env = env, .text = snippet.page_content() };
			for (size_t i = 0; i <= context.text.size(); ++i)
			{
				std::vector<SearchState> states = _root->branches(context, SearchState(i));
				// TODO optimize by exiting early in branches() if searching -> put in SearchContext?
				matches.push_back(env, env.runtime.unbound_variable(materialize(env, snippet, std::move(states[0]))));
			}
		}
		return matches;
	}

	Matches Pattern::find_all(const EvalContext& env, const Snippet& snippet) const
	{
		Matches matches;
		if (_root)
		{
			SearchContext context{ .env = env, .text = snippet.page_content() };
			for (size_t i = 0; i <= context.text.size(); ++i)
			{
				std::vector<SearchState> states = _root->branches(context, SearchState(i));
				// TODO remove duplicates (should there even be any?)
				for (SearchState& state : states)
					matches.push_back(env, env.runtime.unbound_variable(materialize(env, snippet, std::move(state))));
			}
		}
		return matches;
	}

	Match Pattern::materialize(const EvalContext& env, const Snippet& snippet, SearchState&& state)
	{
		Match match(snippet, state.start, state.pos - state.start, true);
		for (auto& capture : state.caps)
		{
			if (capture.exists)
			{
				// TODO use materialize(env, snippet, capture.substate) to create submatch object if capture.substate exists
				Cap cap(env, snippet, capture.start, capture.length, capture.exists, std::nullopt);
				match.add_capture(env, std::move(capture.capid), std::move(cap));
			}
		}
		return match;
	}
}
