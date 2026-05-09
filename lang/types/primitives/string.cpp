#include "string.h"

#include "include.h"
#include "runtime.h"
#include "constants.h"

namespace lx
{
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
}
