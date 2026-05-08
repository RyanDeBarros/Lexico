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

		Variable data_member(Variable self, Runtime& env, const ScriptSegment& segment, const std::string_view member) const;
		Variable invoke_method(Variable self, Runtime& env, const ScriptSegment& segment, const std::string_view method, std::vector<Variable>&& args) const;
		bool equals(const Int& o) const;

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

		Variable data_member(Variable self, Runtime& env, const ScriptSegment& segment, const std::string_view member) const;
		Variable invoke_method(Variable self, Runtime& env, const ScriptSegment& segment, const std::string_view method, std::vector<Variable>&& args) const;
		bool equals(const Float& o) const;

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

		Variable data_member(Variable self, Runtime& env, const ScriptSegment& segment, const std::string_view member) const;
		Variable invoke_method(Variable self, Runtime& env, const ScriptSegment& segment, const std::string_view method, std::vector<Variable>&& args) const;
		bool equals(const Bool& o) const;

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

		Variable data_member(Variable self, Runtime& env, const ScriptSegment& segment, const std::string_view member) const;
		Variable invoke_method(Variable self, Runtime& env, const ScriptSegment& segment, const std::string_view method, std::vector<Variable>&& args) const;
		bool equals(const String& o) const;

		size_t iterlen() const;
		DataPoint iterget(size_t i) const;
		std::string page_content() const;

		std::string_view value() const;
	};

	class Void
	{
	public:
		static DataType data_type();
		TypeVariant cast_copy(const DataType& type) const;
		TypeVariant cast_move(const DataType& type);
		void print(std::stringstream& ss) const;

		Variable data_member(Variable self, Runtime& env, const ScriptSegment& segment, const std::string_view member) const;
		Variable invoke_method(Variable self, Runtime& env, const ScriptSegment& segment, const std::string_view method, std::vector<Variable>&& args) const;
		bool equals(const Void& o) const;
	};

	class Match
	{
	public:
		static DataType data_type();
		TypeVariant cast_copy(const DataType& type) const;
		TypeVariant cast_move(const DataType& type);
		void print(std::stringstream& ss) const;

		Variable data_member(Variable self, Runtime& env, const ScriptSegment& segment, const std::string_view member) const;
		Variable invoke_method(Variable self, Runtime& env, const ScriptSegment& segment, const std::string_view method, std::vector<Variable>&& args) const;
		bool equals(const Match& o) const;

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

		Variable data_member(Variable self, Runtime& env, const ScriptSegment& segment, const std::string_view member) const;
		Variable invoke_method(Variable self, Runtime& env, const ScriptSegment& segment, const std::string_view method, std::vector<Variable>&& args) const;
		bool equals(const Matches& o) const;

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

		Variable data_member(Variable self, Runtime& env, const ScriptSegment& segment, const std::string_view member) const;
		Variable invoke_method(Variable self, Runtime& env, const ScriptSegment& segment, const std::string_view method, std::vector<Variable>&& args) const;
		bool equals(const CapId& o) const;
	};

	class Cap
	{
	public:
		static DataType data_type();
		TypeVariant cast_copy(const DataType& type) const;
		TypeVariant cast_move(const DataType& type);
		void print(std::stringstream& ss) const;

		Variable data_member(Variable self, Runtime& env, const ScriptSegment& segment, const std::string_view member) const;
		Variable invoke_method(Variable self, Runtime& env, const ScriptSegment& segment, const std::string_view method, std::vector<Variable>&& args) const;
		bool equals(const Cap& o) const;
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

		Variable data_member(Variable self, Runtime& env, const ScriptSegment& segment, const std::string_view member) const;
		Variable invoke_method(Variable self, Runtime& env, const ScriptSegment& segment, const std::string_view method, std::vector<Variable>&& args) const;
		bool equals(const IRange& o) const;

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
		SRange(std::optional<std::string_view> min, std::optional<std::string_view> max);
		SRange(std::optional<std::string> min, const ScriptSegment* min_segment, std::optional<std::string> max, const ScriptSegment* max_segment);
		SRange(std::optional<std::string_view> min, const ScriptSegment* min_segment, std::optional<std::string_view> max, const ScriptSegment* max_segment);

		static DataType data_type();
		TypeVariant cast_copy(const DataType& type) const;
		TypeVariant cast_move(const DataType& type);
		void print(std::stringstream& ss) const;

		Variable data_member(Variable self, Runtime& env, const ScriptSegment& segment, const std::string_view member) const;
		Variable invoke_method(Variable self, Runtime& env, const ScriptSegment& segment, const std::string_view method, std::vector<Variable>&& args) const;
		bool equals(const SRange& o) const;

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

		explicit List(const DataType& underlying);
		explicit List(DataType&& underlying);
		explicit List(std::vector<Variable>&& elements);

	public:
		List(const DataType& underlying, const ScriptSegment& segment);
		List(DataType&& underlying, const ScriptSegment& segment);
		List(std::vector<Variable>&& elements, const ScriptSegment& segment);

		static List make_nonvoid_list(const DataType& underlying);
		static List make_nonvoid_list(DataType&& underlying);
		static List make_nonvoid_list(std::vector<Variable>&& elements);

		DataType data_type() const;
		TypeVariant cast_copy(const DataType& type) const;
		TypeVariant cast_move(const DataType& type);
		void print(std::stringstream& ss) const;

		Variable data_member(Variable self, Runtime& env, const ScriptSegment& segment, const std::string_view member) const;
		Variable invoke_method(Variable self, Runtime& env, const ScriptSegment& segment, const std::string_view method, std::vector<Variable>&& args) const;
		bool equals(const List& o) const;

		size_t iterlen() const;
		DataPoint iterget(size_t i) const;

		bool push(Variable element);
	};
}
