#pragma once

#include "declarations.h"

namespace lx
{
	class Int
	{
		int _value;

	public:
		Int(int value);

		static Int make_from_literal(std::string_view resolved);

		static Int make_from(const Int& v);
		static Int make_from(const Float& v);
		static Int make_from(const Bool& v);
		static Int make_from(const String& v);

		int value() const;
	};

	class Float
	{
		float _value;

	public:
		Float(float value);

		static Float make_from_literal(std::string_view resolved);

		static Float make_from(const Int& v);
		static Float make_from(const Float& v);
		static Float make_from(const Bool& v);
		static Float make_from(const String& v);

		float value() const;
	};

	class Bool
	{
		bool _value;

	public:
		Bool(bool value);

		static Bool make_from_literal(std::string_view resolved);

		static Bool make_from(const Int& v);
		static Bool make_from(const Float& v);
		static Bool make_from(const Bool& v);
		static Bool make_from(const String& v);

		bool value() const;
	};

	class String
	{
		std::string _value;

	public:
		String(const std::string& value);
		String(std::string&& value);

		static String make_from_literal(std::string_view resolved);

		static String make_from(const Int& v);
		static String make_from(const Float& v);
		static String make_from(const Bool& v);
		static String make_from(const String& v);
		static String make_from(String&& v);
		static String make_from(const SRange& v);

		std::string_view value() const;
		std::string&& move_string();
	};

	class Void
	{
	public:
		template<typename T>
		static Void make_from(T&&)
		{
			return {};
		}
	};

	class Match
	{
	};

	class Matches
	{
	};

	class CapId
	{
		unsigned int _uid;

	public:
		CapId(unsigned int uid);
	};

	class Cap
	{
	};

	class IRange
	{
		std::optional<int> _min;
		std::optional<int> _max;

	public:
		IRange(std::optional<int> min, std::optional<int> max);

		static IRange make_from(const Int& v);
		static IRange make_from(const IRange& v);

		std::optional<int> min() const;
		std::optional<int> max() const;
	};

	class SRange
	{
		std::optional<char> _min;
		std::optional<char> _max;

	public:
		SRange(std::optional<char> min, std::optional<char> max);
		SRange(std::optional<std::string> min, std::optional<std::string> max);

		static SRange make_from(const SRange& v);

		std::optional<char> min() const;
		std::optional<char> max() const;
		std::string string() const;
	};

	class List
	{
	};

	class Unresolved
	{
	};

	class Marker
	{
	};

	class Scope
	{
	};

	class Color
	{
	};
}
