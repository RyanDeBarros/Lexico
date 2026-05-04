#include "pattern.h"

#include "basic.h"

#include <sstream>

namespace lx
{
	static SubpatternNode& refer_node(const SubpatternNode& from, NodeConvertMap& conv, std::vector<std::unique_ptr<SubpatternNode>>& arena)
	{
		auto it = conv.find(&from);
		if (it != conv.end())
			return *it->second;
		else
			return from.clone(conv, arena);
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
			array.append(refer_node(*el, conv, arena));
		return array;
	}

	const std::vector<SubpatternNode*>& SubpatternArray::array() const
	{
		return _array;
	}

	SubpatternNode& SubpatternRoot::clone(NodeConvertMap& conv, std::vector<std::unique_ptr<SubpatternNode>>& arena) const
	{
		std::stringstream ss;
		ss << __FUNCTION__ << ": cannot clone root node directly";
		throw LxError(ErrorType::Internal, ss.str());
	}

	std::unique_ptr<SubpatternRoot> SubpatternRoot::clone_root(std::vector<std::unique_ptr<SubpatternNode>>& arena) const
	{
		NodeConvertMap conv;
		auto cloned = std::make_unique<SubpatternRoot>();
		conv[this] = cloned.get();
		for (const SubpatternNode* el : array())
			cloned->append(refer_node(*el, conv, arena));
		return cloned;
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
		return clone_base<SubpatternException>(this, conv, arena, refer_node(*_exception, conv, arena));
	}

	SubpatternRepetition::SubpatternRepetition(const IRange& range)
		: _range(range)
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

	SubpatternRepetition::SubpatternRepetition(PatternSimpleRepeatOperator op)
		: _range(simple_repeat_range(op))
	{
	}

	SubpatternNode& SubpatternRepetition::clone(NodeConvertMap& conv, std::vector<std::unique_ptr<SubpatternNode>>& arena) const
	{
		return clone_base<SubpatternRepetition>(this, conv, arena, _range);
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
		return clone_base<SubpatternOptional>(this, conv, arena, refer_node(*_optional, conv, arena));
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
		return clone_base<SubpatternCapture>(this, conv, arena, _capid, refer_node(*_captured, conv, arena));
	}

	SubpatternLazy::SubpatternLazy(SubpatternNode& lazy)
		: _lazy(&lazy)
	{
	}

	SubpatternNode& SubpatternLazy::clone(NodeConvertMap& conv, std::vector<std::unique_ptr<SubpatternNode>>& arena) const
	{
		return clone_base<SubpatternLazy>(this, conv, arena, refer_node(*_lazy, conv, arena));
	}

	Pattern::Pattern()
		: _root(std::make_unique<SubpatternRoot>())
	{
	}

	Pattern::Pattern(const Pattern& other)
	{
		_root = other._root->clone_root(_subnodes);
	}

	Pattern& Pattern::operator=(const Pattern& other)
	{
		if (this != &other)
			*this = Pattern(other);
		return *this;
	}

	Pattern Pattern::make_from(const Int& v)
	{
		return Pattern::make_from(String::make_from(v));
	}

	Pattern Pattern::make_from(const Float & v)
	{
		return Pattern::make_from(String::make_from(v));
	}

	Pattern Pattern::make_from(const Bool& v)
	{
		return Pattern::make_from(String::make_from(v));
	}

	Pattern Pattern::make_from(const String& v)
	{
		Pattern ptn;
		ptn.root().append(ptn.add(std::make_unique<SubpatternString>(std::string(v.value()))));
		return ptn;
	}
	
	Pattern Pattern::make_from(String&& v)
	{
		Pattern ptn;
		ptn.root().append(ptn.add(std::make_unique<SubpatternString>(v.move_string())));
		return ptn;
	}

	Pattern Pattern::make_from(const SRange& v)
	{
		Pattern ptn;
		auto& sub = ptn.add(std::make_unique<SubpatternDisjunction>());
		ptn.root().append(sub);
		for (char c : v.string())
			sub.append(ptn.add(std::make_unique<SubpatternString>(std::string{c})));
		return ptn;
	}

	const SubpatternRoot& Pattern::root() const
	{
		return *_root;
	}

	SubpatternRoot& Pattern::root()
	{
		return *_root;
	}

	void Pattern::impl_add(std::unique_ptr<SubpatternNode>&& node)
	{
		_subnodes.push_back(std::move(node));
	}

	void Pattern::append(Pattern&& pattern)
	{
		_root->append(add(std::move(pattern._root)));
		for (auto& subnode : pattern._subnodes)
			add(std::move(subnode));
	}
}
