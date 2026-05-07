#pragma once

#include "declarations.h"
#include "variable.h"
#include "public.h"

namespace lx
{
	class Int
	{
		int _value;

	public:
		explicit Int(int value);

		static Int make_from_literal(std::string_view resolved);

		static DataType data_type();
		TypeVariant cast_copy(const DataType& type) const;
		TypeVariant cast_move(const DataType& type);
		void print(std::stringstream& ss) const;

		int value() const;
	};

	class Float
	{
		float _value;

	public:
		explicit Float(float value);

		static Float make_from_literal(std::string_view resolved);

		static DataType data_type();
		TypeVariant cast_copy(const DataType& type) const;
		TypeVariant cast_move(const DataType& type);
		void print(std::stringstream& ss) const;

		float value() const;
	};

	class Bool
	{
		bool _value;

	public:
		explicit Bool(bool value);

		static Bool make_from_literal(std::string_view resolved);

		static DataType data_type();
		TypeVariant cast_copy(const DataType& type) const;
		TypeVariant cast_move(const DataType& type);
		void print(std::stringstream& ss) const;

		bool value() const;
	};

	class String
	{
		std::string _value;

	public:
		explicit String(const std::string& value);
		explicit String(std::string&& value);

		static String make_from_literal(std::string_view resolved);

		static DataType data_type();
		TypeVariant cast_copy(const DataType& type) const;
		TypeVariant cast_move(const DataType& type);
		void print(std::stringstream& ss) const;

		size_t iterlen() const;
		DataPoint iterget(size_t i) const;

		std::string_view value() const;
	};

	class Void
	{
	public:
		static DataType data_type();
		TypeVariant cast_copy(const DataType& type) const;
		TypeVariant cast_move(const DataType& type);
		void print(std::stringstream& ss) const;
	};

	class Match
	{
	public:
		static DataType data_type();
		TypeVariant cast_copy(const DataType& type) const;
		TypeVariant cast_move(const DataType& type);
		void print(std::stringstream& ss) const;

		size_t iterlen() const;
		DataPoint iterget(size_t i) const;

	};

	class Matches
	{
	public:
		static DataType data_type();
		TypeVariant cast_copy(const DataType& type) const;
		TypeVariant cast_move(const DataType& type);
		void print(std::stringstream& ss) const;

		size_t iterlen() const;
		DataPoint iterget(size_t i) const;
	};

	class CapId
	{
		unsigned int _uid;

	public:
		explicit CapId(unsigned int uid);

		static DataType data_type();
		TypeVariant cast_copy(const DataType& type) const;
		TypeVariant cast_move(const DataType& type);
		void print(std::stringstream& ss) const;
	};

	class Cap
	{
	public:
		static DataType data_type();
		TypeVariant cast_copy(const DataType& type) const;
		TypeVariant cast_move(const DataType& type);
		void print(std::stringstream& ss) const;
	};

	class IRange
	{
		std::optional<int> _min;
		std::optional<int> _max;

	public:
		IRange(std::optional<int> min, std::optional<int> max);

		static DataType data_type();
		TypeVariant cast_copy(const DataType& type) const;
		TypeVariant cast_move(const DataType& type);
		void print(std::stringstream& ss) const;

		size_t iterlen() const;
		DataPoint iterget(size_t i) const;

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

		static DataType data_type();
		TypeVariant cast_copy(const DataType& type) const;
		TypeVariant cast_move(const DataType& type);
		void print(std::stringstream& ss) const;

		size_t iterlen() const;
		DataPoint iterget(size_t i) const;

		std::optional<char> min() const;
		std::optional<char> max() const;
		std::string string() const;
	};

	class List
	{
		DataType _underlying;
		std::vector<Variable> _elements;

	public:
		List(const DataType& underlying, const ScriptSegment& segment);
		List(DataType&& underlying, const ScriptSegment& segment);
		List(std::vector<Variable>&& elements, const ScriptSegment& segment);

		DataType data_type() const;
		TypeVariant cast_copy(const DataType& type) const;
		TypeVariant cast_move(const DataType& type);
		void print(std::stringstream& ss) const;

		size_t iterlen() const;
		DataPoint iterget(size_t i) const;

		bool push(const Variable& element);
		bool push(Variable&& element);
	};
}
