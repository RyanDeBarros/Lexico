#include "basic.h"

#include "errors.h"

#include <charconv>
#include <sstream>

namespace lx
{
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
		return String(std::move(v._value));
	}

	std::string_view String::value() const
	{
		return _value;
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
}
