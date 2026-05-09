#include "srange.h"

#include "include.h"
#include "runtime.h"

namespace lx
{
	static constexpr int LOWER_A = 'a';
	static constexpr int LOWER_Z = 'z';
	static constexpr int UPPER_A = 'A';
	static constexpr int UPPER_Z = 'Z';

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
}
