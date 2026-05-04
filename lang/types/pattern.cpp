#include "pattern.h"

#include "basic.h"

#include <sstream>

namespace lx
{
	void SubpatternArray::append(SubpatternNode& node)
	{
		_array.push_back(&node);
	}

	SubpatternString::SubpatternString(const std::string& string)
		: _string(string)
	{
	}

	SubpatternString::SubpatternString(std::string&& string)
		: _string(std::move(string))
	{
	}

	SubpatternException::SubpatternException(SubpatternNode& exception)
		: _exception(&exception)
	{
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

	SubpatternOptional::SubpatternOptional(SubpatternNode& optional)
		: _optional(&optional)
	{
	}

	SubpatternBackRef::SubpatternBackRef(CapId capid)
		: _capid(capid)
	{
	}

	SubpatternCapture::SubpatternCapture(CapId capid, SubpatternNode& captured)
		: _capid(capid), _captured(&captured)
	{
	}

	SubpatternLazy::SubpatternLazy(SubpatternNode& lazy)
		: _lazy(&lazy)
	{
	}

	Pattern::Pattern()
		: _root(std::make_unique<SubpatternRoot>())
	{
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
