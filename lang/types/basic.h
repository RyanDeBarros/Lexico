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

		TypeVariant cast_copy(DataType type) const;
		TypeVariant cast_move(DataType type);

		int value() const;
	};

	class Float
	{
		float _value;

	public:
		Float(float value);

		static Float make_from_literal(std::string_view resolved);

		TypeVariant cast_copy(DataType type) const;
		TypeVariant cast_move(DataType type);

		float value() const;
	};

	class Bool
	{
		bool _value;

	public:
		Bool(bool value);

		static Bool make_from_literal(std::string_view resolved);

		TypeVariant cast_copy(DataType type) const;
		TypeVariant cast_move(DataType type);

		bool value() const;
	};

	class String
	{
		std::string _value;

	public:
		String(const std::string& value);
		String(std::string&& value);

		static String make_from_literal(std::string_view resolved);

		TypeVariant cast_copy(DataType type) const;
		TypeVariant cast_move(DataType type);

		std::string_view value() const;
	};

	class Void
	{
	public:
		TypeVariant cast_copy(DataType type) const;
		TypeVariant cast_move(DataType type);
	};

	class Match
	{
	public:
		TypeVariant cast_copy(DataType type) const;
		TypeVariant cast_move(DataType type);

	};

	class Matches
	{
	public:
		TypeVariant cast_copy(DataType type) const;
		TypeVariant cast_move(DataType type);
	};

	class CapId
	{
		unsigned int _uid;

	public:
		CapId(unsigned int uid);

		TypeVariant cast_copy(DataType type) const;
		TypeVariant cast_move(DataType type);
	};

	class Cap
	{
	public:
		TypeVariant cast_copy(DataType type) const;
		TypeVariant cast_move(DataType type);
	};

	class IRange
	{
		std::optional<int> _min;
		std::optional<int> _max;

	public:
		IRange(std::optional<int> min, std::optional<int> max);

		TypeVariant cast_copy(DataType type) const;
		TypeVariant cast_move(DataType type);

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

		TypeVariant cast_copy(DataType type) const;
		TypeVariant cast_move(DataType type);

		std::optional<char> min() const;
		std::optional<char> max() const;
		std::string string() const;
	};

	class List
	{
		std::vector<Unresolved> _elements;

	public:
		TypeVariant cast_copy(DataType type) const;
		TypeVariant cast_move(DataType type);

		void push(const Unresolved& element);
		void push(Unresolved&& element);
	};

	enum class MarkerIdentifier
	{
		Any,
		Cap,
		End,
		Start,
	};

	extern MarkerIdentifier marker(BuiltinSymbol symbol);

	class Marker
	{
		MarkerIdentifier _identifier;

	public:
		TypeVariant cast_copy(DataType type) const;
		TypeVariant cast_move(DataType type);
	};

	class Scope
	{
		std::optional<unsigned int> _lines;

	public:
		Scope(std::optional<unsigned int> lines);

		TypeVariant cast_copy(DataType type) const;
		TypeVariant cast_move(DataType type);
	};

	enum class HighlightColor
	{
		Yellow,
		Red,
		Green,
		Blue,
		Grey,
		Purple,
		Orange,
		Mono,
	};

	class Color
	{
		HighlightColor _color;

	public:
		Color(BuiltinSymbol symbol);

		TypeVariant cast_copy(DataType type) const;
		TypeVariant cast_move(DataType type);
	};
}
