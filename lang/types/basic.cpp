#include "basic.h"

#include "errors.h"

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

	Int Int::make_from(const Int& v)
	{
		return Int(v.value());
	}
	
	Int Int::make_from(const Float& v)
	{
		return Int(static_cast<int>(v.value()));
	}
	
	Int Int::make_from(const Bool& v)
	{
		return Int(static_cast<int>(v.value()));
	}
	
	Int Int::make_from(const String& v)
	{
		return make_from_literal(v.value());
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

	Float Float::make_from(const Int& v)
	{
		return Float(static_cast<float>(v.value()));
	}

	Float Float::make_from(const Float& v)
	{
		return Float(v.value());
	}

	Float Float::make_from(const Bool& v)
	{
		return Float(static_cast<float>(v.value()));
	}

	Float Float::make_from(const String& v)
	{
		return make_from_literal(v.value());
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

	Bool Bool::make_from(const Int& v)
	{
		return Bool(static_cast<bool>(v.value()));
	}

	Bool Bool::make_from(const Float& v)
	{
		return Bool(static_cast<bool>(v.value()));
	}

	Bool Bool::make_from(const Bool& v)
	{
		return Bool(v.value());
	}

	Bool Bool::make_from(const String& v)
	{
		return make_from_literal(v.value());
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

	String String::make_from(const Int& v)
	{
		return String(std::to_string(v.value()));
	}

	String String::make_from(const Float& v)
	{
		return String(std::to_string(v.value()));
	}

	String String::make_from(const Bool& v)
	{
		return String(v.value() ? "true" : "false");
	}

	String String::make_from(const String& v)
	{
		return String(std::string(v.value()));
	}

	String String::make_from(String&& v)
	{
		return String(v.move_string());
	}

	String String::make_from(const SRange& v)
	{
		return String(v.string());
	}

	std::string_view String::value() const
	{
		return _value;
	}

	std::string&& String::move_string()
	{
		return std::move(_value);
	}

	CapId::CapId(unsigned int uid)
		: _uid(uid)
	{
	}

	IRange::IRange(std::optional<int> min, std::optional<int> max)
		: _min(min), _max(max)
	{
	}

	IRange IRange::make_from(const Int& v)
	{
		return IRange(v.value(), v.value());
	}

	IRange IRange::make_from(const IRange& v)
	{
		return IRange(v.min(), v.max());
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

	SRange SRange::make_from(const SRange& v)
	{
		return SRange(v.min(), v.max());
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
}
