#include "pattern.h"

#include "basic.h"
#include "runtime.h"

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

	SubpatternRoot::SubpatternRoot(SubpatternNode& proxy)
		: _proxy(&proxy)
	{
	}

	SubpatternRoot& SubpatternRoot::clone(NodeConvertMap& conv, std::vector<std::unique_ptr<SubpatternNode>>& arena) const
	{
		if (_proxy)
			return clone_base<SubpatternRoot>(this, conv, arena, _proxy->refer_node(conv, arena));
		else
			return clone_base<SubpatternRoot>(this, conv, arena);
	}

	const SubpatternNode& SubpatternRoot::proxy() const
	{
		if (_proxy)
			return *_proxy;
		else
			throw LxError(ErrorType::Internal, "pattern root proxy is null");
	}

	SubpatternNode& SubpatternRoot::proxy()
	{
		if (_proxy)
			return *_proxy;
		else
			throw LxError(ErrorType::Internal, "pattern root proxy is null");
	}

	void SubpatternRoot::set_proxy(SubpatternNode& proxy)
	{
		if (_proxy)
			throw LxError(ErrorType::Internal, "cannot replace pattern root");
		else
			_proxy = &proxy;
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
		_root = &own(std::make_unique<SubpatternRoot>());
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

	Variable Pattern::data_member(Runtime& env, const ScriptSegment& segment, const std::string_view member) const
	{
		std::stringstream ss;
		ss << data_type() << " does not have a data member '" << member << "'";
		throw LxError::segment_error(segment, ErrorType::Runtime, ss.str());
	}

	Variable Pattern::invoke_method(Runtime& env, const ScriptSegment& segment, const std::string_view method, std::vector<Variable>&& args) const
	{
		std::stringstream ss;
		ss << data_type() << " does not have a method '" << method << "' that matches the argument list ";
		print_list(ss, args, [](const Variable& v) { return v.ref().data_type(); });
		throw LxError::segment_error(segment, ErrorType::Runtime, ss.str());
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
			auto sub = std::make_unique<SubpatternDisjunction>();
			for (int i = 'a'; i <= 'z'; ++i)
				sub->append(ptn.own(std::make_unique<SubpatternChar>(i)));
			for (int i = 'A'; i <= 'Z'; ++i)
				sub->append(ptn.own(std::make_unique<SubpatternChar>(i)));
			for (int i = '0'; i <= '9'; ++i)
				sub->append(ptn.own(std::make_unique<SubpatternChar>(i)));
			ptn.set_proxy_root(std::move(sub));
			break;
		}
		case BuiltinSymbol::Digit:
		{
			auto sub = std::make_unique<SubpatternDisjunction>();
			for (int i = '0'; i <= '9'; ++i)
				sub->append(ptn.own(std::make_unique<SubpatternChar>(i)));
			ptn.set_proxy_root(std::move(sub));
			break;
		}
		case BuiltinSymbol::Letter:
		{
			auto sub = std::make_unique<SubpatternDisjunction>();
			for (int i = 'a'; i <= 'z'; ++i)
				sub->append(ptn.own(std::make_unique<SubpatternChar>(i)));
			for (int i = 'A'; i <= 'Z'; ++i)
				sub->append(ptn.own(std::make_unique<SubpatternChar>(i)));
			ptn.set_proxy_root(std::move(sub));
			break;
		}
		case BuiltinSymbol::Lowercase:
		{
			auto sub = std::make_unique<SubpatternDisjunction>();
			for (int i = 'a'; i <= 'z'; ++i)
				sub->append(ptn.own(std::make_unique<SubpatternChar>(i)));
			ptn.set_proxy_root(std::move(sub));
			break;
		}
		case BuiltinSymbol::Newline:
		{
			auto sub = std::make_unique<SubpatternDisjunction>();
			sub->append(ptn.own(std::make_unique<SubpatternString>("\r\n")));
			sub->append(ptn.own(std::make_unique<SubpatternChar>('\n')));
			sub->append(ptn.own(std::make_unique<SubpatternChar>('\r')));
			ptn.set_proxy_root(std::move(sub));
			break;
		}
		case BuiltinSymbol::Space:
		{
			auto sub = std::make_unique<SubpatternDisjunction>();
			sub->append(ptn.own(std::make_unique<SubpatternChar>(' ')));
			sub->append(ptn.own(std::make_unique<SubpatternChar>('\t')));
			ptn.set_proxy_root(std::move(sub));
			break;
		}
		case BuiltinSymbol::Uppercase:
		{
			auto sub = std::make_unique<SubpatternDisjunction>();
			for (int i = 'A'; i <= 'Z'; ++i)
				sub->append(ptn.own(std::make_unique<SubpatternChar>(i)));
			ptn.set_proxy_root(std::move(sub));
			break;
		}
		case BuiltinSymbol::Varname:
		{
			auto sub = std::make_unique<SubpatternDisjunction>();
			for (int i = 'a'; i <= 'z'; ++i)
				sub->append(ptn.own(std::make_unique<SubpatternChar>(i)));
			for (int i = 'A'; i <= 'Z'; ++i)
				sub->append(ptn.own(std::make_unique<SubpatternChar>(i)));
			for (int i = '0'; i <= '9'; ++i)
				sub->append(ptn.own(std::make_unique<SubpatternChar>(i)));
			sub->append(ptn.own(std::make_unique<SubpatternChar>('_')));
			ptn.set_proxy_root(std::move(sub));
			break;
		}
		case BuiltinSymbol::Whitespace:
		{
			auto sub = std::make_unique<SubpatternDisjunction>();
			sub->append(ptn.own(std::make_unique<SubpatternChar>(' ')));
			sub->append(ptn.own(std::make_unique<SubpatternChar>('\t')));
			sub->append(ptn.own(std::make_unique<SubpatternString>("\r\n")));
			sub->append(ptn.own(std::make_unique<SubpatternChar>('\n')));
			sub->append(ptn.own(std::make_unique<SubpatternChar>('\r')));
			ptn.set_proxy_root(std::move(sub));
			break;
		}
		case BuiltinSymbol::Any:
			ptn.set_proxy_root(std::make_unique<SubpatternMarker>(PatternMark::Any));
			break;
		case BuiltinSymbol::Cap:
			ptn.set_proxy_root(std::make_unique<SubpatternMarker>(PatternMark::Cap));
			break;
		case BuiltinSymbol::End:
			ptn.set_proxy_root(std::make_unique<SubpatternMarker>(PatternMark::End));
			break;
		case BuiltinSymbol::Start:
			ptn.set_proxy_root(std::make_unique<SubpatternMarker>(PatternMark::Start));
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
		ptn.set_proxy_root(std::make_unique<SubpatternRepetition>(subptn, range));
		return ptn;
	}

	Pattern Pattern::make_backref(const CapId& capid)
	{
		Pattern ptn;
		ptn.set_proxy_root(std::make_unique<SubpatternBackRef>(capid));
		return ptn;
	}

	Pattern Pattern::make_lazy(Pattern&& pattern)
	{
		Pattern ptn;
		SubpatternNode& subptn = ptn.take(std::move(pattern));
		ptn.set_proxy_root(std::make_unique<SubpatternLazy>(subptn));
		return ptn;
	}

	Pattern Pattern::make_capture(Pattern&& pattern, const CapId& capid)
	{
		Pattern ptn;
		SubpatternNode& subptn = ptn.take(std::move(pattern));
		ptn.set_proxy_root(std::make_unique<SubpatternCapture>(capid, subptn));
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

	const SubpatternRoot& Pattern::root() const
	{
		if (_root)
			return *_root;
		else
			throw LxError(ErrorType::Internal, "pattern root is null");
	}

	SubpatternRoot& Pattern::root()
	{
		if (_root)
			return *_root;
		else
			throw LxError(ErrorType::Internal, "pattern root is null");
	}

	void Pattern::set_proxy_root(std::unique_ptr<SubpatternNode>&& node)
	{
		root().set_proxy(own(std::move(node)));
	}
}
