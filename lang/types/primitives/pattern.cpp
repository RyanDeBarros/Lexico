#include "pattern.h"

#include "include.h"
#include "runtime.h"

namespace lx
{
	SubpatternNode& SubpatternNode::refer_node(NodeConvertMap& conv, std::vector<std::unique_ptr<SubpatternNode>>& arena) const
	{
		auto it = conv.find(this);
		if (it != conv.end())
			return *it->second;
		else
			return clone(conv, arena);
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

	SubpatternArray::SubpatternArray(std::vector<SubpatternNode*>&& array)
		: _array(std::move(array))
	{
	}

	SubpatternNode& SubpatternArray::clone(NodeConvertMap& conv, std::vector<std::unique_ptr<SubpatternNode>>& arena) const
	{
		auto& array = clone_base<SubpatternArray>(this, conv, arena);
		for (const SubpatternNode* el : _array)
			array.append(el->refer_node(conv, arena));
		return array;
	}

	void SubpatternArray::append(SubpatternNode& node)
	{
		_array.push_back(&node);
	}

	SubpatternChar::SubpatternChar(char ch)
		: _ch(ch)
	{
	}

	SubpatternNode& SubpatternChar::clone(NodeConvertMap& conv, std::vector<std::unique_ptr<SubpatternNode>>& arena) const
	{
		return clone_base<SubpatternChar>(this, conv, arena, _ch);
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

	SubpatternMarker::SubpatternMarker(PatternMark marker)
		: _marker(marker)
	{
	}

	SubpatternNode& SubpatternMarker::clone(NodeConvertMap& conv, std::vector<std::unique_ptr<SubpatternNode>>& arena) const
	{
		return clone_base<SubpatternMarker>(this, conv, arena, _marker);
	}

	SubpatternException::SubpatternException(SubpatternNode& subject, SubpatternNode& exception)
		: _subject(&subject), _exception(&exception)
	{
	}

	SubpatternNode& SubpatternException::clone(NodeConvertMap& conv, std::vector<std::unique_ptr<SubpatternNode>>& arena) const
	{
		return clone_base<SubpatternException>(this, conv, arena, _subject->refer_node(conv, arena), _exception->refer_node(conv, arena));
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

	SubpatternOptional::SubpatternOptional(SubpatternNode& optional)
		: _optional(&optional)
	{
	}

	SubpatternNode& SubpatternOptional::clone(NodeConvertMap& conv, std::vector<std::unique_ptr<SubpatternNode>>& arena) const
	{
		return clone_base<SubpatternOptional>(this, conv, arena, _optional->refer_node(conv, arena));
	}

	SubpatternBackRef::SubpatternBackRef(CapId capid)
		: _capid(capid)
	{
	}

	SubpatternNode& SubpatternBackRef::clone(NodeConvertMap& conv, std::vector<std::unique_ptr<SubpatternNode>>& arena) const
	{
		return clone_base<SubpatternBackRef>(this, conv, arena, _capid);
	}

	SubpatternCapture::SubpatternCapture(CapId capid, SubpatternNode& captured)
		: _capid(capid), _captured(&captured)
	{
	}

	SubpatternNode& SubpatternCapture::clone(NodeConvertMap& conv, std::vector<std::unique_ptr<SubpatternNode>>& arena) const
	{
		return clone_base<SubpatternCapture>(this, conv, arena, _capid, _captured->refer_node(conv, arena));
	}

	SubpatternLazy::SubpatternLazy(SubpatternNode& lazy)
		: _lazy(&lazy)
	{
	}

	SubpatternNode& SubpatternLazy::clone(NodeConvertMap& conv, std::vector<std::unique_ptr<SubpatternNode>>& arena) const
	{
		return clone_base<SubpatternLazy>(this, conv, arena, _lazy->refer_node(conv, arena));
	}

	Pattern::Pattern()
	{
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

	TypeVariant Pattern::cast_copy(const DataType& type) const
	{
		if (type.simple() == SimpleType::Pattern)
			return Pattern(*this);
		else if (type.simple() == SimpleType::Void)
			return Void();
		else
			throw_bad_cast(DataType::Pattern(), type);
	}

	TypeVariant Pattern::cast_move(const DataType& type)
	{
		(void*)this; // ignore const warning
		if (type.simple() == SimpleType::Pattern)
			return Pattern(std::move(*this));
		else
			return cast_copy(type);
	}

	void Pattern::print(std::stringstream& ss) const
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

	bool Pattern::equals(const Pattern& o) const
	{
		// TODO
		return false;
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
		case BuiltinSymbol::Cap:
			ptn.make_root<SubpatternMarker>(PatternMark::Cap);
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
}
