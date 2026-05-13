#include "pattern.h"

#include "include.h"
#include "runtime.h"
#include "find.h"

namespace lx
{
	Pattern::Pattern() = default;

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

	Pattern::~Pattern() = default;

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
			// TODO Don't use massive disjunctions like this internally. Define SubpatternIRange, SubpatternSRange, and SubpatternBuiltin.
			ptn.make_root<SubpatternSRange>(SRange(std::nullopt, std::nullopt));
			break;
		}
		case BuiltinSymbol::Digit:
		{
			ptn.make_root<SubpatternSRange>(SRange('0', '9'));
			break;
		}
		case BuiltinSymbol::Letter:
		{
			ptn.make_root<SubpatternSRange>(SRange('a', 'Z'));
			break;
		}
		case BuiltinSymbol::Lowercase:
		{
			ptn.make_root<SubpatternSRange>(SRange('a', 'z'));
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
			ptn.make_root<SubpatternSRange>(SRange('A', 'Z'));
			break;
		}
		case BuiltinSymbol::Varname:
		{
			auto& sub = ptn.make_root<SubpatternDisjunction>();
			sub.append(ptn.make_node<SubpatternSRange>(SRange(std::nullopt, std::nullopt)));
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
				std::vector<SearchState> states = _root->match(context, SearchState(i));
				// TODO optimize by exiting early in branches() if searching -> put in SearchContext?
				matches.push_back(env, env.runtime.unbound_variable(std::move(states[0]).materialize(env, snippet)));
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
				std::vector<SearchState> states = _root->match(context, SearchState(i));
				for (SearchState& state : states)
					matches.push_back(env, env.runtime.unbound_variable(std::move(state).materialize(env, snippet)));
			}
		}
		return matches;
	}
}
