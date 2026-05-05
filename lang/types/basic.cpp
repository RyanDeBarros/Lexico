#include "basic.h"

#include "unresolved.h"
#include "pattern.h"
#include "errors.h"
#include "heap.h"

#include <charconv>
#include <sstream>

namespace lx
{
	static constexpr int LOWER_A = 'a';
	static constexpr int LOWER_Z = 'z';
	static constexpr int UPPER_A = 'A';
	static constexpr int UPPER_Z = 'Z';

	Int::Int(int value)
		: _value(value)
	{
	}

	Int Int::make_from_literal(std::string_view resolved)
	{
		int value;
		auto result = std::from_chars(resolved.data(), resolved.data() + resolved.size(), value);
		if (result.ec == std::errc())
			return Int(value);
		else
		{
			std::stringstream ss;
			ss << "could not convert \"" << resolved << "\" to " << DataType::Int;
			throw LxError(ErrorType::Runtime, ss.str());
		}
	}

	TypeVariant Int::cast_copy(DataType type) const
	{
		switch (type)
		{
		case DataType::Int:
			return *this;
		case DataType::Float:
			return Float(static_cast<float>(_value));
		case DataType::Bool:
			return Bool(static_cast<bool>(_value));
		case DataType::String:
			return String(std::to_string(_value));
		case DataType::IRange:
			return IRange(_value, _value);
		case DataType::Pattern:
		{
			Pattern ptn;
			ptn.root().append(ptn.add(std::make_unique<SubpatternString>(std::to_string(_value))));
			return ptn;
		}
		case DataType::Void:
			return Void();
		default:
			throw_bad_cast(DataType::Int, type);
		}
	}
	
	TypeVariant Int::cast_move(DataType type)
	{
		(void*)this; // ignore const warning
		return cast_copy(type);
	}
	
	int Int::value() const
	{
		return _value;
	}

	Float::Float(float value)
		: _value(value)
	{
	}

	Float Float::make_from_literal(std::string_view resolved)
	{
		float value;
		auto result = std::from_chars(resolved.data(), resolved.data() + resolved.size(), value);
		if (result.ec == std::errc())
			return Float(value);
		else
		{
			std::stringstream ss;
			ss << "could not convert \"" << resolved << "\" to " << DataType::Float;
			throw LxError(ErrorType::Runtime, ss.str());
		}
	}

	TypeVariant Float::cast_copy(DataType type) const
	{
		switch (type)
		{
		case DataType::Int:
			return Int(static_cast<int>(_value));
		case DataType::Float:
			return *this;
		case DataType::Bool:
			return Bool(static_cast<bool>(_value));
		case DataType::String:
			return String(std::to_string(_value));
		case DataType::Pattern:
		{
			Pattern ptn;
			ptn.root().append(ptn.add(std::make_unique<SubpatternString>(std::to_string(_value))));
			return ptn;
		}
		case DataType::Void:
			return Void();
		default:
			throw_bad_cast(DataType::Float, type);
		}
	}

	TypeVariant Float::cast_move(DataType type)
	{
		(void*)this; // ignore const warning
		return cast_copy(type);
	}

	float Float::value() const
	{
		return _value;
	}

	Bool::Bool(bool value)
		: _value(value)
	{
	}

	Bool Bool::make_from_literal(std::string_view resolved)
	{
		if (resolved == "true")
			return Bool(true);
		else if (resolved == "false")
			return Bool(false);
		else
		{
			std::stringstream ss;
			ss << "could not convert \"" << resolved << "\" to " << DataType::Bool;
			throw LxError(ErrorType::Runtime, ss.str());
		}
	}

	TypeVariant Bool::cast_copy(DataType type) const
	{
		switch (type)
		{
		case DataType::Int:
			return Int(static_cast<int>(_value));
		case DataType::Float:
			return Float(static_cast<float>(_value));
		case DataType::Bool:
			return *this;
		case DataType::String:
			return String(_value ? "true" : "false");
		case DataType::Void:
			return Void();
		default:
			throw_bad_cast(DataType::Bool, type);
		}
	}

	TypeVariant Bool::cast_move(DataType type)
	{
		(void*)this; // ignore const warning
		return cast_copy(type);
	}

	bool Bool::value() const
	{
		return _value;
	}

	String::String(const std::string& value)
		: _value(value)
	{
	}

	String::String(std::string&& value)
		: _value(std::move(value))
	{
	}

	String String::make_from_literal(std::string_view resolved)
	{
		return String(std::string(resolved));
	}

	TypeVariant String::cast_copy(DataType type) const
	{
		switch (type)
		{
		case DataType::Int:
			return Int::make_from_literal(_value);
		case DataType::Float:
			return Float::make_from_literal(_value);
		case DataType::Bool:
			return Bool::make_from_literal(_value);
		case DataType::String:
			return *this;
		case DataType::Pattern:
		{
			Pattern ptn;
			ptn.root().append(ptn.add(std::make_unique<SubpatternString>(_value)));
			return ptn;
		}
		case DataType::Void:
			return Void();
		default:
			throw_bad_cast(DataType::String, type);
		}
	}

	TypeVariant String::cast_move(DataType type)
	{
		if (type == DataType::String)
			return std::move(*this);
		else if (type == DataType::Pattern)
		{
			Pattern ptn;
			ptn.root().append(ptn.add(std::make_unique<SubpatternString>(std::move(_value))));
			return ptn;
		}
		else
			return cast_copy(type);
	}

	std::string_view String::value() const
	{
		return _value;
	}

	TypeVariant Void::cast_copy(DataType type) const
	{
		if (type == DataType::Void)
			return Void();
		else
			throw_bad_cast(DataType::Match, type);
	}

	TypeVariant Void::cast_move(DataType type)
	{
		(void*)this; // ignore const warning
		return cast_copy(type);
	}

	TypeVariant Match::cast_copy(DataType type) const
	{
		if (type == DataType::Match)
			return *this;
		else if (type == DataType::Void)
			return Void();
		else
			throw_bad_cast(DataType::Match, type);
	}

	TypeVariant Match::cast_move(DataType type)
	{
		if (type == DataType::Match)
			return std::move(*this);
		else
			return cast_copy(type);
	}

	TypeVariant Matches::cast_copy(DataType type) const
	{
		if (type == DataType::Matches)
			return *this;
		else if (type == DataType::Void)
			return Void();
		else
			throw_bad_cast(DataType::Matches, type);
	}

	TypeVariant Matches::cast_move(DataType type)
	{
		if (type == DataType::Matches)
			return std::move(*this);
		else
			return cast_copy(type);
	}

	CapId::CapId(unsigned int uid)
		: _uid(uid)
	{
	}

	TypeVariant CapId::cast_copy(DataType type) const
	{
		if (type == DataType::CapId)
			return *this;
		else if (type == DataType::Void)
			return Void();
		else
			throw_bad_cast(DataType::CapId, type);
	}

	TypeVariant CapId::cast_move(DataType type)
	{
		(void*)this; // ignore const warning
		return cast_copy(type);
	}

	TypeVariant Cap::cast_copy(DataType type) const
	{
		if (type == DataType::Cap)
			return *this;
		else if (type == DataType::Void)
			return Void();
		else
			throw_bad_cast(DataType::Cap, type);
	}

	TypeVariant Cap::cast_move(DataType type)
	{
		if (type == DataType::Cap)
			return std::move(*this);
		else
		{
			(void*)this; // ignore const warning
			return cast_copy(type);
		}
	}

	IRange::IRange(std::optional<int> min, std::optional<int> max)
		: _min(min), _max(max)
	{
	}

	TypeVariant IRange::cast_copy(DataType type) const
	{
		if (type == DataType::IRange)
			return *this;
		else if (type == DataType::Void)
			return Void();
		else
			throw_bad_cast(DataType::IRange, type);
	}

	TypeVariant IRange::cast_move(DataType type)
	{
		(void*)this; // ignore const warning
		return cast_copy(type);
	}

	std::optional<int> IRange::min() const
	{
		return _min;
	}

	std::optional<int> IRange::max() const
	{
		return _max;
	}

	SRange::SRange(std::optional<char> min, std::optional<char> max)
		: _min(min), _max(max)
	{
	}

	static void assert_valid_srange_char(const std::string& m)
	{
		if (m.size() != 1)
		{
			std::stringstream ss;
			ss << "\"" << m << "\" should only consist of one character";
			throw LxError(ErrorType::Runtime, ss.str());
		}
		else if (m[0] < LOWER_A || (m[0] > LOWER_Z && m[0] < UPPER_A) || m[0] > UPPER_Z)
		{
			std::stringstream ss;
			ss << "\"" << m[0] << "\" out of range \"a-z\" and \"A-Z\"";
			throw LxError(ErrorType::Runtime, ss.str());
		}
	}
	
	SRange::SRange(std::optional<std::string> min, std::optional<std::string> max)
		: _min(min && !min->empty() ? std::make_optional((*min)[0]) : std::nullopt), _max(max && !max->empty() ? std::make_optional((*max)[0]) : std::nullopt)
	{
		std::vector<LxError> errors;

		if (min)
		{
			try
			{
				assert_valid_srange_char(*min);
			}
			catch (LxError& e)
			{
				errors.push_back(std::move(e));
			}
		}

		if (max)
		{
			try
			{
				assert_valid_srange_char(*max);
			}
			catch (LxError& e)
			{
				errors.push_back(std::move(e));
			}
		}

		if (!errors.empty())
			throw LxErrorList(errors);
	}

	TypeVariant SRange::cast_copy(DataType type) const
	{
		switch (type)
		{
		case DataType::String:
			return String(string());
		case DataType::SRange:
			return *this;
		case DataType::Pattern:
		{
			Pattern ptn;
			auto& sub = ptn.add(std::make_unique<SubpatternDisjunction>());
			ptn.root().append(sub);
			for (char c : string())
				sub.append(ptn.add(std::make_unique<SubpatternChar>(c)));
			return ptn;
		}
		case DataType::Void:
			return Void();
		default:
			throw_bad_cast(DataType::SRange, type);
		}
	}

	TypeVariant SRange::cast_move(DataType type)
	{
		(void*)this; // ignore const warning
		return cast_copy(type);
	}

	std::optional<char> SRange::min() const
	{
		return _min;
	}

	std::optional<char> SRange::max() const
	{
		return _max;
	}

	static constexpr bool is_lower(char c)
	{
		return c <= LOWER_Z;
	}

	static std::stringstream& append_range(std::stringstream& ss, int min, int max)
	{
		for (int i = min; i <= max; ++i)
			ss << i;
		return ss;
	}

	std::string SRange::string() const
	{
		if (!_min && !_max)
			return "";

		std::stringstream ss;

		if (_min && !_max)
			return append_range(ss, *_min, is_lower(*_min) ? 'z' : 'Z').str();

		if (_max && !_min)
			return append_range(ss, is_lower(*_max) ? 'a' : 'A', *_max).str();

		if (*_min <= *_max)
			return append_range(ss, *_min, *_max).str();

		if (is_lower(*_min))
			return append_range(append_range(ss, *_min, 'z'), 'A', *_max).str();
		else
			return append_range(append_range(ss, *_min, 'Z'), 'a', *_max).str();
	}

	TypeVariant List::cast_copy(DataType type) const
	{
		if (type == DataType::List)
			return *this;
		else if (type == DataType::Void)
			return Void();
		else
			throw_bad_cast(DataType::List, type);
	}

	TypeVariant List::cast_move(DataType type)
	{
		if (type == DataType::List)
			return std::move(*this);
		else
			return cast_copy(type);
	}

	void List::push(const Variable& element)
	{
		_elements.push_back(element);
	}

	void List::push(Variable&& element)
	{
		_elements.push_back(std::move(element));
	}

	MarkerIdentifier marker(BuiltinSymbol symbol)
	{
		switch (symbol)
		{
		case BuiltinSymbol::Any:
			return MarkerIdentifier::Any;
		case BuiltinSymbol::Cap:
			return MarkerIdentifier::Cap;
		case BuiltinSymbol::End:
			return MarkerIdentifier::End;
		case BuiltinSymbol::Start:
			return MarkerIdentifier::Start;
		default:
		{
			std::stringstream ss;
			ss << __FUNCTION__ << ": unrecognized marker " << static_cast<int>(symbol);
			throw LxError(ErrorType::Internal, ss.str());
		}
		}
	}

	Marker::Marker(MarkerIdentifier identifier)
		: _identifier(identifier)
	{
	}

	TypeVariant Marker::cast_copy(DataType type) const
	{
		if (type == DataType::_Marker)
			return *this;
		else if (type == DataType::Void)
			return Void();
		else
			throw_bad_cast(DataType::_Marker, type);
	}

	TypeVariant Marker::cast_move(DataType type)
	{
		(void*)this; // ignore const warning
		return cast_copy(type);
	}

	Scope::Scope(std::optional<unsigned int> lines)
		: _lines(lines)
	{
	}

	TypeVariant Scope::cast_copy(DataType type) const
	{
		if (type == DataType::_Scope)
			return *this;
		else if (type == DataType::Void)
			return Void();
		else
			throw_bad_cast(DataType::_Scope, type);
	}

	TypeVariant Scope::cast_move(DataType type)
	{
		(void*)this; // ignore const warning
		return cast_copy(type);
	}

	static HighlightColor convert_color(BuiltinSymbol symbol)
	{
		switch (symbol)
		{
			case BuiltinSymbol::Yellow:
				return HighlightColor::Yellow;
			case BuiltinSymbol::Red:
				return HighlightColor::Red;
			case BuiltinSymbol::Green:
				return HighlightColor::Green;
			case BuiltinSymbol::Blue:
				return HighlightColor::Blue;
			case BuiltinSymbol::Grey:
				return HighlightColor::Grey;
			case BuiltinSymbol::Purple:
				return HighlightColor::Purple;
			case BuiltinSymbol::Orange:
				return HighlightColor::Orange;
			case BuiltinSymbol::Mono:
				return HighlightColor::Mono;
			default:
			{
				std::stringstream ss;
				ss << __FUNCTION__ << ": could not convert symbol " << static_cast<int>(symbol) << " to highlight color";
				throw LxError(ErrorType::Internal, ss.str());
			}
		}
	}

	Color::Color(BuiltinSymbol symbol)
		: _color(convert_color(symbol))
	{
	}

	TypeVariant Color::cast_copy(DataType type) const
	{
		if (type == DataType::_Color)
			return *this;
		else if (type == DataType::Void)
			return Void();
		else
			throw_bad_cast(DataType::_Color, type);
	}

	TypeVariant Color::cast_move(DataType type)
	{
		(void*)this; // ignore const warning
		return cast_copy(type);
	}
}
