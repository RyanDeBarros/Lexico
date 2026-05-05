#include "pattern.h"

#include "basic.h"
#include "unresolved.h"

#include <sstream>

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

	void SubpatternArray::append(SubpatternNode& node)
	{
		_array.push_back(&node);
	}

	SubpatternNode& SubpatternArray::clone(NodeConvertMap& conv, std::vector<std::unique_ptr<SubpatternNode>>& arena) const
	{
		auto& array = clone_base<SubpatternArray>(this, conv, arena);
		for (const SubpatternNode* el : _array)
			array.append(el->refer_node(conv, arena));
		return array;
	}

	const std::vector<SubpatternNode*>& SubpatternArray::array() const
	{
		return _array;
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

	SubpatternException::SubpatternException(SubpatternNode& exception)
		: _exception(&exception)
	{
	}

	SubpatternNode& SubpatternException::clone(NodeConvertMap& conv, std::vector<std::unique_ptr<SubpatternNode>>& arena) const
	{
		return clone_base<SubpatternException>(this, conv, arena, _exception->refer_node(conv, arena));
	}

	SubpatternRepetition::SubpatternRepetition(SubpatternNode& repeated, const IRange& range)
		: _repeated(&repeated), _range(range)
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

	SubpatternRepetition::SubpatternRepetition(SubpatternNode& repeated, PatternSimpleRepeatOperator op)
		: _repeated(&repeated), _range(simple_repeat_range(op))
	{
	}

	SubpatternNode& SubpatternRepetition::clone(NodeConvertMap& conv, std::vector<std::unique_ptr<SubpatternNode>>& arena) const
	{
		return clone_base<SubpatternRepetition>(this, conv, arena, _repeated->refer_node(conv, arena), _range);
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

	SubpatternLookaround::SubpatternLookaround(LookaroundMode mode)
		: _mode(mode)
	{
	}

	SubpatternNode& SubpatternLookaround::clone(NodeConvertMap& conv, std::vector<std::unique_ptr<SubpatternNode>>& arena) const
	{
		return clone_base<SubpatternLookaround>(this, conv, arena, _mode);
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
		NodeConvertMap conv;
		conv[&other] = this;
		for (const SubpatternNode* el : array())
			append(el->refer_node(conv, _subnodes));
	}

	Pattern& Pattern::operator=(const Pattern& other)
	{
		if (this != &other)
			*this = Pattern(other);
		return *this;
	}

	TypeVariant Pattern::cast_copy(DataType type) const
	{
		if (type == DataType::Pattern)
			return Pattern(*this);
		else if (type == DataType::Void)
			return Void();
		else
			throw_bad_cast(DataType::Pattern, type);
	}

	TypeVariant Pattern::cast_move(DataType type)
	{
		(void*)this; // ignore const warning
		if (type == DataType::Pattern)
			return Pattern(std::move(*this));
		else
			return cast_copy(type);
	}

	Pattern Pattern::make_from_symbol(BuiltinSymbol symbol)
	{
		Pattern ptn;
		switch (symbol)
		{
		case BuiltinSymbol::Alphanumeric:
		{
			auto sub = std::make_unique<SubpatternDisjunction>();
			for (int i = 'a'; i <= 'z'; ++i)
				sub->append(ptn.add(std::make_unique<SubpatternChar>(i)));
			for (int i = 'A'; i <= 'Z'; ++i)
				sub->append(ptn.add(std::make_unique<SubpatternChar>(i)));
			for (int i = '0'; i <= '9'; ++i)
				sub->append(ptn.add(std::make_unique<SubpatternChar>(i)));
			ptn.append(ptn.add(std::move(sub)));
			break;
		}
		case BuiltinSymbol::Digit:
		{
			auto sub = std::make_unique<SubpatternDisjunction>();
			for (int i = '0'; i <= '9'; ++i)
				sub->append(ptn.add(std::make_unique<SubpatternChar>(i)));
			ptn.append(ptn.add(std::move(sub)));
			break;
		}
		case BuiltinSymbol::Letter:
		{
			auto sub = std::make_unique<SubpatternDisjunction>();
			for (int i = 'a'; i <= 'z'; ++i)
				sub->append(ptn.add(std::make_unique<SubpatternChar>(i)));
			for (int i = 'A'; i <= 'Z'; ++i)
				sub->append(ptn.add(std::make_unique<SubpatternChar>(i)));
			ptn.append(ptn.add(std::move(sub)));
			break;
		}
		case BuiltinSymbol::Lowercase:
		{
			auto sub = std::make_unique<SubpatternDisjunction>();
			for (int i = 'a'; i <= 'z'; ++i)
				sub->append(ptn.add(std::make_unique<SubpatternChar>(i)));
			ptn.append(ptn.add(std::move(sub)));
			break;
		}
		case BuiltinSymbol::Newline:
		{
			auto sub = std::make_unique<SubpatternDisjunction>();
			sub->append(ptn.add(std::make_unique<SubpatternString>("\r\n")));
			sub->append(ptn.add(std::make_unique<SubpatternChar>('\n')));
			sub->append(ptn.add(std::make_unique<SubpatternChar>('\r')));
			ptn.append(ptn.add(std::move(sub)));
			break;
		}
		case BuiltinSymbol::Space:
		{
			auto sub = std::make_unique<SubpatternDisjunction>();
			sub->append(ptn.add(std::make_unique<SubpatternChar>(' ')));
			sub->append(ptn.add(std::make_unique<SubpatternChar>('\t')));
			ptn.append(ptn.add(std::move(sub)));
			break;
		}
		case BuiltinSymbol::Uppercase:
		{
			auto sub = std::make_unique<SubpatternDisjunction>();
			for (int i = 'A'; i <= 'Z'; ++i)
				sub->append(ptn.add(std::make_unique<SubpatternChar>(i)));
			ptn.append(ptn.add(std::move(sub)));
			break;
		}
		case BuiltinSymbol::Varname:
		{
			auto sub = std::make_unique<SubpatternDisjunction>();
			for (int i = 'a'; i <= 'z'; ++i)
				sub->append(ptn.add(std::make_unique<SubpatternChar>(i)));
			for (int i = 'A'; i <= 'Z'; ++i)
				sub->append(ptn.add(std::make_unique<SubpatternChar>(i)));
			for (int i = '0'; i <= '9'; ++i)
				sub->append(ptn.add(std::make_unique<SubpatternChar>(i)));
			sub->append(ptn.add(std::make_unique<SubpatternChar>('_')));
			ptn.append(ptn.add(std::move(sub)));
			break;
		}
		case BuiltinSymbol::Whitespace:
		{
			auto sub = std::make_unique<SubpatternDisjunction>();
			sub->append(ptn.add(std::make_unique<SubpatternChar>(' ')));
			sub->append(ptn.add(std::make_unique<SubpatternChar>('\t')));
			sub->append(ptn.add(std::make_unique<SubpatternString>("\r\n")));
			sub->append(ptn.add(std::make_unique<SubpatternChar>('\n')));
			sub->append(ptn.add(std::make_unique<SubpatternChar>('\r')));
			ptn.append(ptn.add(std::move(sub)));
			break;
		}
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
		Pattern& subptn = ptn.add(std::move(pattern));
		ptn.append(ptn.add(std::make_unique<SubpatternRepetition>(subptn, range)));
		return ptn;
	}

	Pattern Pattern::make_backref(const CapId& capid)
	{
		Pattern ptn;
		ptn.append(ptn.add(std::make_unique<SubpatternBackRef>(capid)));
		return ptn;
	}

	Pattern Pattern::make_lazy(Pattern&& pattern)
	{
		Pattern ptn;
		Pattern& subptn = ptn.add(std::move(pattern));
		ptn.append(ptn.add(std::make_unique<SubpatternLazy>(subptn)));
		return ptn;
	}

	Pattern Pattern::make_capture(Pattern&& pattern, const CapId& capid)
	{
		Pattern ptn;
		Pattern& subptn = ptn.add(std::move(pattern));
		ptn.append(ptn.add(std::make_unique<SubpatternCapture>(capid, subptn)));
		return ptn;
	}

	void Pattern::impl_add(std::unique_ptr<SubpatternNode>&& node)
	{
		_subnodes.push_back(std::move(node));
	}

	Pattern& Pattern::add(Pattern&& pattern)
	{
		auto ptn = std::make_unique<Pattern>(std::move(pattern));
		Pattern& p = *ptn;
		_subnodes.push_back(std::move(ptn));
		return p;
	}
}
