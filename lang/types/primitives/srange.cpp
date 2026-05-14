#include "srange.h"

#include "include.h"
#include "runtime.h"
#include "find.h"

namespace lx
{
	constexpr unsigned char MIN_RANGE_CHAR = 1;
	constexpr unsigned char MAX_RANGE_CHAR = 62;

	static unsigned char to_range_char(char chr)
	{
		if (chr >= '0' && chr <= '9')
			return chr - '0' + 1;
		else if (chr >= 'a' && chr <= 'z')
			return chr - 'a' + 11;
		else if (chr >= 'A' && chr <= 'Z')
			return chr - 'A' + 37;
		else
			return 0;
	}

	static char from_range_char(unsigned char index)
	{
		if (index >= 1 && index <= 10)
			return index - 1 + '0';
		else if (index >= 11 && index <= 36)
			return index - 11 + 'a';
		else if (index >= 37 && index <= 62)
			return index - 37 + 'A';
		else
			return '\0';
	}

	static char upper_bound(char min)
	{
		if (min >= '0' && min <= '9')
			return '9';
		else if (min >= 'a' && min <= 'z')
			return 'z';
		else if (min >= 'A' && min <= 'Z')
			return 'Z';
		else
			return '\0';
	}

	static char lower_bound(char max)
	{
		if (max >= '0' && max <= '9')
			return '0';
		else if (max >= 'a' && max <= 'z')
			return 'a';
		else if (max >= 'A' && max <= 'Z')
			return 'A';
		else
			return '\0';
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
		else if (to_range_char(m[0]) == 0)
		{
			std::stringstream ss;
			ss << "\"" << m[0] << "\" is not a valid range character (must be \"0\"-\"9\", \"a\"-\"z\" and \"A\"-\"Z\"";
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

	TypeVariant SRange::cast_copy(const VarContext& ctx, const DataType& type) const
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
			ptn.make_root<SubpatternSRange>(*this);
			return ptn;
		}
		case SimpleType::Void:
			return Void();
		default:
			ctx.env.throw_bad_cast(data_type(), type);
		}
	}

	TypeVariant SRange::cast_move(VarContext&& ctx, const DataType& type) &&
	{
		(void*)this; // ignore const warning
		return cast_copy(ctx, type);
	}

	void SRange::print(const EvalContext& env, std::stringstream& ss) const
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
		else
			ss << "full " << data_type();
		ss << '>';
	}

	Variable SRange::data_member(VarContext& ctx, const std::string_view member)
	{
		ctx.throw_no_data_member(member);
	}

	Variable SRange::invoke_method(VarContext& ctx, const std::string_view method, std::vector<Variable>&& args)
	{
		ctx.throw_no_method(method, args);
	}

	void SRange::assign(const EvalContext& env, SRange&& o)
	{
		_min = std::move(o._min);
		_max = std::move(o._max);
	}

	bool SRange::equals(const EvalContext& env, const SRange& o) const
	{
		return _min == o._min && _max == o._max;
	}
	
	static size_t range_iterlen(char min, char max)
	{
		unsigned char m1 = to_range_char(min);
		unsigned char m2 = to_range_char(max);
		if (m1 > 0 && m2 > 0)
			return static_cast<size_t>(m2 - m1 + 1);
		else
			return 0;
	}

	size_t SRange::iterlen(const EvalContext& env) const
	{
		if (!_min && !_max)
			return MAX_RANGE_CHAR - MIN_RANGE_CHAR + 1;

		if (_min && !_max)
			return range_iterlen(*_min, upper_bound(*_min));

		if (_max && !_min)
			return range_iterlen(lower_bound(*_max), *_max);

		if (*_min <= *_max)
			return range_iterlen(*_min, *_max);
		else
			return range_iterlen(*_max, *_min);
	}

	static String to_string(char c, signed char off)
	{
		unsigned char m = to_range_char(c);
		if (m > 0)
			return String({ from_range_char(m + off) });
		else
			return String("");
	}

	DataPoint SRange::iterget(const EvalContext& env, size_t i) const
	{
		if (!_min && !_max)
			return String({ from_range_char(MIN_RANGE_CHAR + i) });

		std::stringstream ss;

		if (_min && !_max)
			return to_string(*_min, i);

		if (_max && !_min)
			return to_string(lower_bound(*_max), i);

		if (*_min <= *_max)
			return to_string(*_min, i);
		else
			return to_string(*_min, -i);
	}

	std::optional<char> SRange::min() const
	{
		return _min;
	}

	std::optional<char> SRange::max() const
	{
		return _max;
	}

	std::string SRange::string() const
	{
		std::stringstream ss;
		unsigned char min = MIN_RANGE_CHAR;
		unsigned char max = MAX_RANGE_CHAR;

		if (_min || _max)
		{
			min = to_range_char(_min ? *_min : lower_bound(*_max));
			max = to_range_char(_max ? *_max : upper_bound(*_min));
		}

		if (min <= max)
		{
			for (unsigned char i = min; i <= max; ++i)
				ss << from_range_char(i);
		}
		else
		{
			for (unsigned char i = min; i >= max; --i)
				ss << from_range_char(i);
		}
		return ss.str();
	}

	bool SRange::contains(char c) const
	{
		if (!_min && !_max)
			return to_range_char(c) > 0;

		unsigned char min = to_range_char(_min ? *_min : lower_bound(*_max));
		unsigned char max = to_range_char(_max ? *_max : upper_bound(*_min));
		const unsigned char ch = to_range_char(c);
		return ch >= std::min(min, max) && ch <= std::max(min, max);
	}

	bool SRange::empty() const
	{
		if (!_min && !_max)
			return false;

		unsigned char min = to_range_char(_min ? *_min : lower_bound(*_max));
		unsigned char max = to_range_char(_max ? *_max : upper_bound(*_min));
		return min == 0 || max == 0;
	}
}
