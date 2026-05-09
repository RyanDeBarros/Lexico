#include "basic.h"

#include "pattern.h"
#include "errors.h"
#include "heap.h"
#include "runtime.h"
#include "constants.h"

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
			ss << "could not convert \"" << resolved << "\" to " << DataType::Int();
			throw LxError(ErrorType::Runtime, ss.str());
		}
	}

	DataType Int::data_type()
	{
		return DataType::Int();
	}

	TypeVariant Int::cast_copy(const DataType& type) const
	{
		switch (type.simple())
		{
		case SimpleType::Int:
			return *this;
		case SimpleType::Float:
			return Float(static_cast<float>(_value));
		case SimpleType::Bool:
			return Bool(static_cast<bool>(_value));
		case SimpleType::String:
			return String(std::to_string(_value));
		case SimpleType::IRange:
			return IRange(_value, _value);
		case SimpleType::Pattern:
			return Pattern::make_from_subpattern<SubpatternString>(std::to_string(_value));
		case SimpleType::Void:
			return Void();
		default:
			throw_bad_cast(data_type(), type);
		}
	}
	
	TypeVariant Int::cast_move(const DataType& type)
	{
		(void*)this; // ignore const warning
		return cast_copy(type);
	}

	void Int::print(std::stringstream& ss) const
	{
		ss << _value;
	}

	Variable Int::data_member(VarContext& ctx, const std::string_view member) const
	{
		ctx.throw_no_data_member(member);
	}

	Variable Int::invoke_method(VarContext& ctx, const std::string_view method, std::vector<Variable>&& args) const
	{
		ctx.throw_no_method(method, args);
	}

	bool Int::equals(const Int& o) const
	{
		return _value == o._value;
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
			ss << "could not convert \"" << resolved << "\" to " << DataType::Float();
			throw LxError(ErrorType::Runtime, ss.str());
		}
	}

	DataType Float::data_type()
	{
		return DataType::Float();
	}

	TypeVariant Float::cast_copy(const DataType& type) const
	{
		switch (type.simple())
		{
		case SimpleType::Int:
			return Int(static_cast<int>(_value));
		case SimpleType::Float:
			return *this;
		case SimpleType::Bool:
			return Bool(static_cast<bool>(_value));
		case SimpleType::String:
			return String(std::to_string(_value));
		case SimpleType::Pattern:
			Pattern::make_from_subpattern<SubpatternString>(std::to_string(_value));
		case SimpleType::Void:
			return Void();
		default:
			throw_bad_cast(data_type(), type);
		}
	}

	TypeVariant Float::cast_move(const DataType& type)
	{
		(void*)this; // ignore const warning
		return cast_copy(type);
	}

	void Float::print(std::stringstream& ss) const
	{
		ss << _value;
	}

	Variable Float::data_member(VarContext& ctx, const std::string_view member) const
	{
		ctx.throw_no_data_member(member);
	}

	Variable Float::invoke_method(VarContext& ctx, const std::string_view method, std::vector<Variable>&& args) const
	{
		ctx.throw_no_method(method, args);
	}

	bool Float::equals(const Float& o) const
	{
		return _value == o._value;
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
			ss << "could not convert \"" << resolved << "\" to " << DataType::Bool();
			throw LxError(ErrorType::Runtime, ss.str());
		}
	}

	DataType Bool::data_type()
	{
		return DataType::Bool();
	}

	TypeVariant Bool::cast_copy(const DataType& type) const
	{
		switch (type.simple())
		{
		case SimpleType::Int:
			return Int(static_cast<int>(_value));
		case SimpleType::Float:
			return Float(static_cast<float>(_value));
		case SimpleType::Bool:
			return *this;
		case SimpleType::String:
			return String(_value ? "true" : "false");
		case SimpleType::Void:
			return Void();
		default:
			throw_bad_cast(data_type(), type);
		}
	}

	TypeVariant Bool::cast_move(const DataType& type)
	{
		(void*)this; // ignore const warning
		return cast_copy(type);
	}

	void Bool::print(std::stringstream& ss) const
	{
		ss << (_value ? "true" : "false");
	}

	Variable Bool::data_member(VarContext& ctx, const std::string_view member) const
	{
		ctx.throw_no_data_member(member);
	}

	Variable Bool::invoke_method(VarContext& ctx, const std::string_view method, std::vector<Variable>&& args) const
	{
		ctx.throw_no_method(method, args);
	}

	bool Bool::equals(const Bool& o) const
	{
		return _value == o._value;
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

	DataType String::data_type()
	{
		return DataType::String();
	}

	TypeVariant String::cast_copy(const DataType& type) const
	{
		switch (type.simple())
		{
		case SimpleType::Int:
			return Int::make_from_literal(_value);
		case SimpleType::Float:
			return Float::make_from_literal(_value);
		case SimpleType::Bool:
			return Bool::make_from_literal(_value);
		case SimpleType::String:
			return *this;
		case SimpleType::Pattern:
			return Pattern::make_from_subpattern<SubpatternString>(_value);
		case SimpleType::Void:
			return Void();
		default:
			throw_bad_cast(data_type(), type);
		}
	}

	TypeVariant String::cast_move(const DataType& type)
	{
		if (type.simple() == SimpleType::String)
			return std::move(*this);
		else if (type.simple() == SimpleType::Pattern)
			return Pattern::make_from_subpattern<SubpatternString>(std::move(_value));
		else
			return cast_copy(type);
	}

	void String::print(std::stringstream& ss) const
	{
		ss << _value;
	}

	static void assert_valid_string_subscript(VarContext& ctx, const std::string_view value, const Int& index)
	{
		if (index.value() < 0 || index.value() >= value.size())
		{
			std::stringstream ss;
			ss << "index ";
			index.print(ss);
			ss << " is out of range for " << DataType::String() << " of length " << value.size();
			throw ctx.env.runtime_error(ss.str());
		}
	}

	static std::pair<int, int> convert_string_subscript_range(const std::string_view value, const IRange& range)
	{
		return { range.min() ? *range.min() : 0, range.max() ? *range.max() : static_cast<int>(value.size()) - 1 };
	}

	static void assert_valid_string_subscript(VarContext& ctx, const std::string_view value, const IRange& range)
	{
		const auto [min, max] = convert_string_subscript_range(value, range);
		if (value.empty() || min < 0 || min >= value.size() || max < 0 || max >= value.size())
		{
			std::stringstream ss;
			ss << "range ";
			range.print(ss);
			ss << " is out of range for " << DataType::String() << " of length " << value.size();
			throw ctx.env.runtime_error(ss.str());
		}
	}

	DataPoint String::resolve_path(VarContext& ctx, const PathStep& step)
	{
		if (step.symbol == ctx.symbolize(constants::SUBSCRIPT_OP))
		{
			const DataPoint& aux = step.aux;
			if (const Int* index = aux.as<Int>())
			{
				assert_valid_string_subscript(ctx, _value, *index);
				return String({ _value[index->value()] });
			}
			else if (const IRange* range = aux.as<IRange>())
			{
				assert_valid_string_subscript(ctx, _value, *range);
				const auto [min, max] = convert_string_subscript_range(_value, *range);
				if (min <= max)
					return String(_value.substr(min, static_cast<size_t>(max - min + 1)));
				else
					return String(std::string(_value.rbegin() + _value.size() - 1 - min, _value.rbegin() + _value.size() - max));
			}
			else
				ctx.throw_unsupported_aux_type_get(aux);
		}
		else
			ctx.throw_unsupported_datapath_symbol_get();
	}

	DataPoint String::consume_path(VarContext& ctx, const PathStep& step) &&
	{
		if (step.symbol == ctx.symbolize(constants::SUBSCRIPT_OP))
		{
			const DataPoint& aux = step.aux;
			if (const Int* index = aux.as<Int>())
			{
				assert_valid_string_subscript(ctx, _value, *index);
				return String({ _value[index->value()] });
			}
			else if (const IRange* range = aux.as<IRange>())
			{
				assert_valid_string_subscript(ctx, _value, *range);
				const auto [min, max] = convert_string_subscript_range(_value, *range);
				if (min == 0 && max == _value.size() - 1)
					return String(std::move(_value));
				else if (max == 0 && min == _value.size() - 1)
				{
					std::reverse(_value.begin(), _value.end());
					return String(std::move(_value));
				}
				else if (min <= max)
					return String(_value.substr(min, static_cast<size_t>(max - min + 1)));
				else
					return String(std::string(_value.rbegin() + _value.size() - 1 - min, _value.rbegin() + _value.size() - max));
			}
			else
				ctx.throw_unsupported_aux_type_get(aux);
		}
		else
			ctx.throw_unsupported_datapath_symbol_get();
	}

	void String::assign_path(VarContext& ctx, const PathStep& step, DataPoint&& to)
	{
		// TODO cache data symbols somehow
		if (step.symbol == ctx.symbolize(constants::SUBSCRIPT_OP))
		{
			if (to.can_cast_implicit(DataType::String()))
			{
				const DataPoint& aux = step.aux;
				if (const Int* index = aux.as<Int>())
				{
					assert_valid_string_subscript(ctx, _value, *index);
					std::string s = std::move(to.move_as<String>()._value);
					if (s.size() == 1)
						_value[index->value()] = s[0];
					else
						throw ctx.env.runtime_error("cannot set character to multi-character string");
				}
				else if (const IRange* range = aux.as<IRange>())
				{
					assert_valid_string_subscript(ctx, _value, *range);
					std::string s = std::move(to.move_as<String>()._value);
					const auto [min, max] = convert_string_subscript_range(_value, *range);
					_value.erase(std::min(min, max), static_cast<size_t>(std::abs(max - min) + 1));
					if (min > max)
						std::reverse(s.begin(), s.end());
					_value.insert(std::min(min, max), s);
				}
				else
					ctx.throw_unsupported_aux_type_set(aux);
			}
			else
				ctx.throw_bad_set_expression(to);
		}
		else
			ctx.throw_unsupported_datapath_symbol_set();
	}

	Variable String::data_member(VarContext& ctx, const std::string_view member) const
	{
		if (member == "len")
			return ctx.variable(Int(_value.size()));

		ctx.throw_no_data_member(member);
	}

	Variable String::invoke_method(VarContext& ctx, const std::string_view method, std::vector<Variable>&& args) const
	{
		if (method == constants::SUBSCRIPT_OP)
		{
			if (args.size() == 1)
			{
				if (args[0].ref().data_type() == DataType::Int())
				{
					Int index = std::move(args[0]).consume().move_as<Int>();
					assert_valid_string_subscript(ctx, _value, index);
					return ctx.self.subpath({ .steps = { { .symbol = ctx.symbolize(constants::SUBSCRIPT_OP), .aux = std::move(index) }}});
				}
				else if (args[0].ref().data_type() == DataType::IRange())
				{
					IRange range = std::move(args[0]).consume().move_as<IRange>();
					assert_valid_string_subscript(ctx, _value, range);
					return ctx.self.subpath({ .steps = { {.symbol = ctx.symbolize(constants::SUBSCRIPT_OP), .aux = std::move(range) }} });
				}
			}
		}

		ctx.throw_no_method(method, args);
	}

	bool String::equals(const String& o) const
	{
		return _value == o._value;
	}

	size_t String::iterlen() const
	{
		return _value.size();
	}

	DataPoint String::iterget(size_t i) const
	{
		return String({ _value[i] });
	}

	std::string String::page_content() const
	{
		return _value;
	}

	std::string_view String::value() const
	{
		return _value;
	}

	DataType Void::data_type()
	{
		return DataType::Void();
	}

	TypeVariant Void::cast_copy(const DataType& type) const
	{
		if (type.simple() == SimpleType::Void)
			return Void();
		else
			throw_bad_cast(data_type(), type);
	}

	TypeVariant Void::cast_move(const DataType& type)
	{
		(void*)this; // ignore const warning
		return cast_copy(type);
	}

	void Void::print(std::stringstream& ss) const
	{
		ss << "";
	}

	Variable Void::data_member(VarContext& ctx, const std::string_view member) const
	{
		ctx.throw_no_data_member(member);
	}

	Variable Void::invoke_method(VarContext& ctx, const std::string_view method, std::vector<Variable>&& args) const
	{
		ctx.throw_no_method(method, args);
	}

	bool Void::equals(const Void& o) const
	{
		return true;
	}

	DataType Match::data_type()
	{
		return DataType::Match();
	}

	TypeVariant Match::cast_copy(const DataType& type) const
	{
		if (type.simple() == SimpleType::Match)
			return *this;
		else if (type.simple() == SimpleType::Void)
			return Void();
		else
			throw_bad_cast(data_type(), type);
	}

	TypeVariant Match::cast_move(const DataType& type)
	{
		if (type.simple() == SimpleType::Match)
			return std::move(*this);
		else
			return cast_copy(type);
	}

	void Match::print(std::stringstream& ss) const
	{
		// TODO v0.2 string representation of match
		ss << DataType::Match();
	}

	Variable Match::data_member(VarContext& ctx, const std::string_view member) const
	{
		// TODO use constants for these member names
		if (member == "caps")
		{
			// TODO
			return ctx.variable(List::make_nonvoid_list(DataType::Cap()));
		}
		else if (member == "start")
		{
			// TODO
			return ctx.variable(Int(0));
		}
		else if (member == "end")
		{
			// TODO
			return ctx.variable(Int(0));
		}
		else if (member == "len")
		{
			// TODO
			return ctx.variable(Int(0));
		}
		else if (member == "range")
		{
			// TODO
			return ctx.variable(IRange(0, 0));
		}
		else if (member == "str")
		{
			// TODO
			return ctx.variable(String(""));
		}

		ctx.throw_no_data_member(member);
	}

	Variable Match::invoke_method(VarContext& ctx, const std::string_view method, std::vector<Variable>&& args) const
	{
		if (method == constants::SUBSCRIPT_OP)
		{
			if (args.size() == 1)
			{
				if (args[0].ref().data_type().simple() == SimpleType::CapId)
				{
					// TODO
					return ctx.variable(Cap());
				}
				else if (args[0].ref().data_type().simple() == SimpleType::Int)
				{
					// TODO
					return ctx.variable(Cap());
				}
			}
		}

		ctx.throw_no_method(method, args);
	}

	bool Match::equals(const Match& o) const
	{
		// TODO
		return false;
	}

	size_t Match::iterlen() const
	{
		// TODO
		return 0;
	}

	DataPoint Match::iterget(size_t i) const
	{
		// TODO
		return Cap();
	}

	DataType Matches::data_type()
	{
		return DataType::Matches();
	}

	TypeVariant Matches::cast_copy(const DataType& type) const
	{
		if (type.simple() == SimpleType::Matches)
			return *this;
		else if (type.simple() == SimpleType::Void)
			return Void();
		else
			throw_bad_cast(data_type(), type);
	}

	TypeVariant Matches::cast_move(const DataType& type)
	{
		if (type.simple() == SimpleType::Matches)
			return std::move(*this);
		else
			return cast_copy(type);
	}

	void Matches::print(std::stringstream& ss) const
	{
		// TODO v0.2 string representation of matches
		ss << DataType::Matches();
	}

	Variable Matches::data_member(VarContext& ctx, const std::string_view member) const
	{
		ctx.throw_no_data_member(member);
	}

	Variable Matches::invoke_method(VarContext& ctx, const std::string_view method, std::vector<Variable>&& args) const
	{
		ctx.throw_no_method(method, args);
	}

	bool Matches::equals(const Matches& o) const
	{
		// TODO
		return false;
	}

	size_t Matches::iterlen() const
	{
		// TODO
		return 0;
	}

	DataPoint Matches::iterget(size_t i) const
	{
		// TODO
		return Match();
	}

	CapId::CapId(unsigned int uid)
		: _uid(uid)
	{
	}

	DataType CapId::data_type()
	{
		return DataType::CapId();
	}

	TypeVariant CapId::cast_copy(const DataType& type) const
	{
		if (type.simple() == SimpleType::CapId)
			return *this;
		else if (type.simple() == SimpleType::Void)
			return Void();
		else
			throw_bad_cast(data_type(), type);
	}

	TypeVariant CapId::cast_move(const DataType& type)
	{
		(void*)this; // ignore const warning
		return cast_copy(type);
	}

	void CapId::print(std::stringstream& ss) const
	{
		ss << DataType::CapId();
	}

	Variable CapId::data_member(VarContext& ctx, const std::string_view member) const
	{
		ctx.throw_no_data_member(member);
	}

	Variable CapId::invoke_method(VarContext& ctx, const std::string_view method, std::vector<Variable>&& args) const
	{
		ctx.throw_no_method(method, args);
	}

	bool CapId::equals(const CapId& o) const
	{
		return _uid == o._uid;
	}

	DataType Cap::data_type()
	{
		return DataType::Cap();
	}

	TypeVariant Cap::cast_copy(const DataType& type) const
	{
		if (type.simple() == SimpleType::Cap)
			return *this;
		else if (type.simple() == SimpleType::Void)
			return Void();
		else
			throw_bad_cast(data_type(), type);
	}

	TypeVariant Cap::cast_move(const DataType& type)
	{
		if (type.simple() == SimpleType::Cap)
			return std::move(*this);
		else
		{
			(void*)this; // ignore const warning
			return cast_copy(type);
		}
	}

	void Cap::print(std::stringstream& ss) const
	{
		// TODO v0.2 string representation of cap
		ss << DataType::Cap();
	}

	Variable Cap::data_member(VarContext& ctx, const std::string_view member) const
	{
		if (member == "exists")
		{
			// TODO
			return ctx.variable(Bool(true));
		}
		else if (member == "start")
		{
			// TODO
			return ctx.variable(Int(0));
		}
		else if (member == "end")
		{
			// TODO
			return ctx.variable(Int(0));
		}
		else if (member == "len")
		{
			// TODO
			return ctx.variable(Int(0));
		}
		else if (member == "range")
		{
			// TODO
			return ctx.variable(IRange(0, 0));
		}
		else if (member == "str")
		{
			// TODO
			return ctx.variable(String(""));
		}
		else if (member == "sub")
		{
			// TODO
			return ctx.variable(Match());
		}

		ctx.throw_no_data_member(member);
	}

	Variable Cap::invoke_method(VarContext& ctx, const std::string_view method, std::vector<Variable>&& args) const
	{
		ctx.throw_no_method(method, args);
	}

	bool Cap::equals(const Cap& o) const
	{
		// TODO
		return false;
	}

	IRange::IRange(std::optional<int> min, std::optional<int> max)
		: _min(min), _max(max)
	{
	}

	DataType IRange::data_type()
	{
		return DataType::IRange();
	}

	TypeVariant IRange::cast_copy(const DataType& type) const
	{
		if (type.simple() == SimpleType::IRange)
			return *this;
		else if (type.simple() == SimpleType::Void)
			return Void();
		else
			throw_bad_cast(data_type(), type);
	}

	TypeVariant IRange::cast_move(const DataType& type)
	{
		(void*)this; // ignore const warning
		return cast_copy(type);
	}

	void IRange::print(std::stringstream& ss) const
	{
		ss << '<';
		if (_min)
		{
			if (_max)
				ss << *_min << " to " << *_max;
			else
				ss << "min " << *_min;
		}
		else if (_max)
			ss << "max " << *_max;
		ss << '>';
	}

	Variable IRange::data_member(VarContext& ctx, const std::string_view member) const
	{
		ctx.throw_no_data_member(member);
	}

	Variable IRange::invoke_method(VarContext& ctx, const std::string_view method, std::vector<Variable>&& args) const
	{
		ctx.throw_no_method(method, args);
	}

	bool IRange::equals(const IRange& o) const
	{
		return _min == o._min && _max == o._max;
	}

	size_t IRange::iterlen() const
	{
		if (!_min || !_max)
			throw LxError(ErrorType::Runtime, "cannot iterate over unbounded range");

		return static_cast<size_t>(std::abs(*_max - *_min) + 1);
	}

	DataPoint IRange::iterget(size_t i) const
	{
		if (!_min || !_max)
			throw LxError(ErrorType::Runtime, "cannot iterate over unbounded range");

		int dir = *_max >= *_min ? 1 : -1;
		return Int(*_min + dir * i);
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

	static void assert_valid_srange_char(const std::string_view m, const ScriptSegment* segment)
	{
		if (m.size() != 1)
		{
			std::stringstream ss;
			ss << "\"" << m << "\" should only consist of one character";
			if (segment)
				throw LxError::segment_error(*segment, ErrorType::Runtime, ss.str());
			else
				throw LxError(ErrorType::Runtime, ss.str());
		}
		else if (m[0] < LOWER_A || (m[0] > LOWER_Z && m[0] < UPPER_A) || m[0] > UPPER_Z)
		{
			std::stringstream ss;
			ss << "\"" << m[0] << "\" out of range \"a-z\" and \"A-Z\"";
			if (segment)
				throw LxError::segment_error(*segment, ErrorType::Runtime, ss.str());
			else
				throw LxError(ErrorType::Runtime, ss.str());
		}
	}

	static void assert_valid_srange(std::optional<std::string_view> min, const ScriptSegment* min_segment, std::optional<std::string_view> max, const ScriptSegment* max_segment)
	{
		std::vector<LxError> errors;

		if (min)
		{
			try
			{
				assert_valid_srange_char(*min, min_segment);
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
				assert_valid_srange_char(*max, max_segment);
			}
			catch (LxError& e)
			{
				errors.push_back(std::move(e));
			}
		}

		if (!errors.empty())
			throw LxErrorList(errors);
	}
	
	SRange::SRange(std::optional<std::string> min, std::optional<std::string> max)
		: _min(min && !min->empty() ? std::make_optional((*min)[0]) : std::nullopt), _max(max && !max->empty() ? std::make_optional((*max)[0]) : std::nullopt)
	{
		assert_valid_srange(min, nullptr, max, nullptr);
	}

	SRange::SRange(std::optional<std::string_view> min, std::optional<std::string_view> max)
		: _min(min && !min->empty() ? std::make_optional((*min)[0]) : std::nullopt), _max(max && !max->empty() ? std::make_optional((*max)[0]) : std::nullopt)
	{
		assert_valid_srange(min, nullptr, max, nullptr);
	}

	SRange::SRange(std::optional<std::string> min, const ScriptSegment* min_segment, std::optional<std::string> max, const ScriptSegment* max_segment)
		: _min(min && !min->empty() ? std::make_optional((*min)[0]) : std::nullopt), _max(max && !max->empty() ? std::make_optional((*max)[0]) : std::nullopt)
	{
		assert_valid_srange(min, min_segment, max, max_segment);
	}

	SRange::SRange(std::optional<std::string_view> min, const ScriptSegment* min_segment, std::optional<std::string_view> max, const ScriptSegment* max_segment)
		: _min(min && !min->empty() ? std::make_optional((*min)[0]) : std::nullopt), _max(max && !max->empty() ? std::make_optional((*max)[0]) : std::nullopt)
	{
		assert_valid_srange(min, min_segment, max, max_segment);
	}

	DataType SRange::data_type()
	{
		return DataType::SRange();
	}

	TypeVariant SRange::cast_copy(const DataType& type) const
	{
		switch (type.simple())
		{
		case SimpleType::String:
			return String(string());
		case SimpleType::SRange:
			return *this;
		case SimpleType::Pattern:
		{
			Pattern ptn;
			auto& sub = ptn.make_root<SubpatternDisjunction>();
			for (char c : string())
				sub.append(ptn.make_node<SubpatternChar>(c));
			return ptn;
		}
		case SimpleType::Void:
			return Void();
		default:
			throw_bad_cast(data_type(), type);
		}
	}

	TypeVariant SRange::cast_move(const DataType& type)
	{
		(void*)this; // ignore const warning
		return cast_copy(type);
	}

	void SRange::print(std::stringstream& ss) const
	{
		ss << '<';
		if (_min)
		{
			if (_max)
				ss << *_min << " to " << *_max;
			else
				ss << "min " << *_min;
		}
		else if (_max)
			ss << "max " << *_max;
		ss << '>';
	}

	Variable SRange::data_member(VarContext& ctx, const std::string_view member) const
	{
		ctx.throw_no_data_member(member);
	}

	Variable SRange::invoke_method(VarContext& ctx, const std::string_view method, std::vector<Variable>&& args) const
	{
		ctx.throw_no_method(method, args);
	}

	bool SRange::equals(const SRange& o) const
	{
		return _min == o._min && _max == o._max;
	}

	static constexpr bool is_lower(char c)
	{
		return c <= LOWER_Z;
	}

	static size_t range_iterlen(int min, int max)
	{
		return static_cast<size_t>(max - min + 1);
	}

	size_t SRange::iterlen() const
	{
		if (!_min && !_max)
			return 0;

		if (_min && !_max)
			return range_iterlen(*_min, is_lower(*_min) ? 'z' : 'Z');

		if (_max && !_min)
			return range_iterlen(is_lower(*_max) ? 'a' : 'A', *_max);

		if (*_min <= *_max)
			return range_iterlen(*_min, *_max);

		if (is_lower(*_min))
			return range_iterlen(*_min, 'z') + range_iterlen('A', *_max);
		else
			return range_iterlen(*_min, 'Z') + range_iterlen('a', *_max);
	}

	static String to_string(size_t c)
	{
		return String({ static_cast<char>(c) });
	}

	DataPoint SRange::iterget(size_t i) const
	{
		if (!_min && !_max)
			return String("");

		std::stringstream ss;

		if (_min && !_max)
			return to_string(*_min + i);

		if (_max && !_min)
			return to_string((is_lower(*_max) ? 'a' : 'A') + i);

		if (*_min <= *_max)
			return to_string(*_min + i);

		if (is_lower(*_min))
		{
			if (i < range_iterlen(*_min, 'z'))
				return to_string(*_min + i);
			else
				return to_string('A' + i - range_iterlen(*_min, 'z'));
		}
		else
		{
			if (i < range_iterlen(*_min, 'Z'))
				return to_string(*_min + i);
			else
				return to_string('a' + i - range_iterlen(*_min, 'Z'));
		}
	}

	std::optional<char> SRange::min() const
	{
		return _min;
	}

	std::optional<char> SRange::max() const
	{
		return _max;
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

	static DataType underlying_of(const std::vector<Variable>& elements, const ScriptSegment* segment)
	{
		if (elements.empty())
		{
			if (segment)
				throw LxError::segment_error(*segment, ErrorType::Runtime, "unresolved list cannot initialize with no elements");
			else
				return DataType::Void();
		}

		std::vector<LxError> errors;
		DataType underlying = elements[0].ref().data_type();
		for (size_t i = 1; i < elements.size(); ++i)
		{
			if (elements[i].ref().data_type() != underlying)
			{
				std::stringstream ss;
				ss << __FUNCTION__ << ": element [" << i << "] has type " << elements[i].ref().data_type() << ", which doesn't match type of first element: " << underlying;
				if (segment)
					errors.push_back(LxError::segment_error(*segment, ErrorType::Runtime, ss.str()));
				else
					return DataType::Void();
			}
		}

		if (errors.empty())
			return underlying;
		else
			throw LxErrorList(errors);
	}

	List::List(const DataType& underlying)
		: _underlying(underlying)
	{
	}

	List::List(DataType&& underlying)
		: _underlying(std::move(underlying))
	{
	}

	List::List(std::vector<Variable>&& elements)
		: _underlying(underlying_of(elements, nullptr))
	{
		_elements = std::move(elements);
	}

	List::List(const DataType& underlying, const ScriptSegment& segment)
		: _underlying(underlying)
	{
		if (_underlying.simple() == SimpleType::Void)
			throw LxError::segment_error(segment, ErrorType::Runtime, "list cannot have void underlying type");
	}

	List::List(DataType&& underlying, const ScriptSegment& segment)
		: _underlying(std::move(underlying))
	{
		if (_underlying.simple() == SimpleType::Void)
			throw LxError::segment_error(segment, ErrorType::Runtime, "list cannot have void underlying type");
	}

	List::List(std::vector<Variable>&& elements, const ScriptSegment& segment)
		: _underlying(underlying_of(elements, &segment))
	{
		_elements = std::move(elements);
	}

	List List::make_nonvoid_list(const DataType& underlying)
	{
		return List(underlying);
	}

	List List::make_nonvoid_list(DataType&& underlying)
	{
		return List(std::move(underlying));
	}

	List List::make_nonvoid_list(std::vector<Variable>&& elements)
	{
		return List(std::move(elements));
	}

	DataType List::data_type() const
	{
		return DataType::List(_underlying);
	}

	TypeVariant List::cast_copy(const DataType& type) const
	{
		if (type == DataType::List(_underlying))
			return *this;
		else if (type.simple() == SimpleType::Void)
			return Void();
		else
			throw_bad_cast(data_type(), type);
	}

	TypeVariant List::cast_move(const DataType& type)
	{
		if (type == DataType::List(_underlying))
			return std::move(*this);
		else
			return cast_copy(type);
	}

	void List::print(std::stringstream& ss) const
	{
		ss << "[";
		for (size_t i = 0; i < _elements.size(); ++i)
		{
			_elements[i].ref().print(ss);
			if (i + 1 < _elements.size())
				ss << ", ";
		}
		ss << "]";
	}

	Variable List::data_member(VarContext& ctx, const std::string_view member) const
	{
		if (member == "len")
			return ctx.variable(Int(_elements.size()));

		ctx.throw_no_data_member(member);
	}

	Variable List::invoke_method(VarContext& ctx, const std::string_view method, std::vector<Variable>&& args) const
	{
		if (method == constants::SUBSCRIPT_OP)
		{
			if (args.size() == 1)
			{
				if (args[0].ref().data_type().simple() == SimpleType::Int)
					return _elements[std::move(args[0]).consume().move_as<Int>().value()];
			}
		}

		ctx.throw_no_method(method, args);
	}

	bool List::equals(const List& o) const
	{
		if (_elements.size() != o._elements.size())
			return false;
		
		for (size_t i = 0; i < _elements.size(); ++i)
			if (!_elements[i].ref().equals(o._elements[i].ref()))
				return false;

		return true;
	}

	size_t List::iterlen() const
	{
		return _elements.size();
	}

	DataPoint List::iterget(size_t i) const
	{
		return _elements[i].ref();
	}

	bool List::push(Variable element)
	{
		if (element.ref().data_type() == _underlying)
		{
			_elements.push_back(std::move(element));
			return true;
		}
		else
			return false;
	}
}
