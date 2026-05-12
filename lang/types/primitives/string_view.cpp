#include "string_view.h"

#include "include.h"
#include "runtime.h"
#include "constants.h"

namespace lx
{
	StringView::StringView(const EvalContext& env, Variable string, const Indexer& indexer)
		: _ref(std::move(string)), _indexer(indexer)
	{
		if (StringView* sv = _ref.ref().as<StringView>())
			*this = sv->substring(env, _indexer);
		else if (_ref.ref().data_type() != DataType::String())
			env.throw_bad_set_expression(_ref.ref().data_type(), DataType::StringView());

		_string = _ref.ref().as<String>();
	}

	StringView::StringView(const StringView& other) noexcept
		: _ref(other._ref), _string(other._string), _indexer(other._indexer)
	{
	}
	
	StringView::StringView(StringView&& other) noexcept
		: _ref(std::move(other._ref)), _string(other._string), _indexer(std::move(other._indexer))
	{
		other._string = nullptr;
	}
	
	StringView& StringView::operator=(const StringView& other)
	{
		if (this != &other)
		{
			_ref = other._ref;
			_string = other._string;
			_indexer = other._indexer;
		}
		return *this;
	}
	
	StringView& StringView::operator=(StringView&& other) noexcept
	{
		if (this != &other)
		{
			_ref = std::move(other._ref);
			_string = other._string;
			_indexer = std::move(other._indexer);
			other._string = nullptr;
		}
		return *this;
	}

	DataType StringView::data_type()
	{
		return DataType::StringView();
	}

	TypeVariant StringView::cast_copy(const VarContext& ctx, const DataType& type) const
	{
		// TODO v0.3 more efficient conversions over iterators without using temporaries
		switch (type.simple())
		{
		case SimpleType::Int:
			return Int::make_from_literal(ctx.env, copy_value(ctx.env));
		case SimpleType::Float:
			return Float::make_from_literal(ctx.env, copy_value(ctx.env));
		case SimpleType::Bool:
			return Bool::make_from_literal(ctx.env, copy_value(ctx.env));
		case SimpleType::String:
			return String(copy_value(ctx.env));
		case SimpleType::StringView:
			return *this;
		case SimpleType::Pattern:
			return Pattern::make_from_subpattern<SubpatternString>(copy_value(ctx.env));
		case SimpleType::Void:
			return Void();
		default:
			ctx.env.throw_bad_cast(data_type(), type);
		}
	}

	TypeVariant StringView::cast_move(VarContext&& ctx, const DataType& type) &&
	{
		if (type == DataType::String())
			return String(std::move(*this).consume_value(ctx.env));
		else if (type == DataType::Pattern())
			return Pattern::make_from_subpattern<SubpatternString>(std::move(*this).consume_value(ctx.env));

		(void*)this; // ignore const warning
		return cast_copy(ctx, type);
	}
	
	void StringView::print(const EvalContext& env, std::stringstream& ss) const
	{
		assert_valid(env);

		const int min = min_index();
		const int max = max_index();
		const int len = std::abs(max - min) + 1;

		if (min <= max)
			ss << string().substr(min, len);
		else
			for (size_t i = 0; i < len; ++i)
				ss << chr(i, min, max);
	}

	Variable StringView::data_member(VarContext& ctx, const std::string_view member) const
	{
		if (member == "len")
		{
			assert_valid(ctx.env);
			return ctx.variable(Int(max_index() - min_index() + 1));
		}

		ctx.throw_no_data_member(member);
	}

	Variable StringView::invoke_method(VarContext& ctx, const std::string_view method, std::vector<Variable>&& args) const
	{
		if (method == constants::SUBSCRIPT_OP)
		{
			if (args.size() == 1)
			{
				if (args[0].ref().data_type() == DataType::Int())
					return ctx.variable(substring(ctx.env, std::move(args[0]).consume_as<Int>(ctx.env)));
				else if (args[0].ref().data_type() == DataType::IRange())
					return ctx.variable(substring(ctx.env, std::move(args[0]).consume_as<IRange>(ctx.env)));
				else if (args[0].ref().can_cast_implicit(DataType::Int()))
					return ctx.variable(substring(ctx.env, std::move(args[0]).consume_as<Int>(ctx.env)));
				else if (args[0].ref().can_cast_implicit(DataType::IRange()))
					return ctx.variable(substring(ctx.env, std::move(args[0]).consume_as<IRange>(ctx.env)));
			}
		}

		ctx.throw_no_method(method, args);
	}

	void StringView::assign(const EvalContext& env, StringView&& o)
	{
		if (const IRange* range = std::get_if<IRange>(&_indexer))
		{
			assert_valid(env);
			o.assert_valid(env);

			const int min = min_index();
			const int max = max_index();

			_string->_value.erase(std::min(min, max), static_cast<size_t>(std::abs(max - min) + 1));
			std::string s = std::move(o).consume_value(env);
			if (min > max)
				std::reverse(s.begin(), s.end());
			_string->_value.insert(std::min(min, max), s);
		}
		else
		{
			const Int& index = std::get<Int>(_indexer);

			assert_valid(env);
			o.assert_valid(env);
			if (o.string().size() == 1)
				_string->_value[index.value()] = o.string()[0];
			else
				throw env.runtime_error("cannot set character to multi-character string");
		}
	}

	bool StringView::equals(const EvalContext& env, const StringView& o) const
	{
		assert_valid(env);
		o.assert_valid(env);

		const int min1 = min_index();
		const int max1 = max_index();
		const int min2 = o.min_index();
		const int max2 = o.max_index();

		if (std::abs(max1 - min1) != std::abs(max2 - min2))
			return false;

		const int len = std::abs(max1 - min1) + 1;
		for (size_t i = 0; i < len; ++i)
			if (chr(i, min1, max1) != o.chr(i, min2, max2))
				return false;

		return true;
	}

	size_t StringView::iterlen(const EvalContext& env) const
	{
		assert_valid(env);
		return static_cast<size_t>(std::abs(max_index() - min_index()) + 1);
	}

	DataPoint StringView::iterget(const EvalContext& env, size_t i) const
	{
		assert_valid(env);

		const int min = min_index();
		const int max = max_index();
		return String({ chr(i, min, max) });
	}

	std::string StringView::page_content(const EvalContext& env) const
	{
		return copy_value(env);
	}

	std::string StringView::copy_value(const EvalContext& env) const
	{
		assert_valid(env);
		const int min = min_index();
		const int max = max_index();

		if (min <= max)
			return std::string(string().substr(min, static_cast<size_t>(max - min + 1)));
		else
			return std::string(string().rbegin() + string().size() - 1 - min, string().rbegin() + string().size() - max);
	}

	std::string StringView::consume_value(const EvalContext& env) &&
	{
		if (_ref.unbound())
		{
			assert_valid(env);

			if (std::holds_alternative<IRange>(_indexer))
			{
				const int min = min_index();
				const int max = max_index();

				if (min == 0 && max == string().size() - 1)
					return std::move(_string->_value);
				else if (max == 0 && min == string().size() - 1)
				{
					std::reverse(_string->_value.begin(), _string->_value.end());
					return std::move(_string->_value);
				}
				else if (min <= max)
					return std::string(string().substr(min, static_cast<size_t>(max - min + 1)));
				else
					return std::string(string().rbegin() + string().size() - 1 - min, string().rbegin() + string().size() - max);
			}
			else
			{
				if (string().size() == 1)
					return std::move(_string->_value);
				else
					return std::string{ string()[std::get<Int>(_indexer).value()] };
			}
		}
		else
			return copy_value(env);
	}

	StringView StringView::substring(const EvalContext& env, const Int& index) const
	{
		const int min = min_index();
		const int max = max_index();
		assert_in_range(env, index, min, max);

		return StringView(env, _ref, Int(idx(index.value(), min, max)));
	}

	StringView StringView::substring(const EvalContext& env, const IRange& range) const
	{
		const int min = min_index();
		const int max = max_index();
		assert_in_range(env, range, min, max);

		if (const IRange* my_range = std::get_if<IRange>(&_indexer))
		{
			std::optional<int> submin = range.min() ? std::make_optional(idx(*range.min(), min, max)) : std::nullopt;
			std::optional<int> submax = range.max() ? std::make_optional(idx(*range.max(), min, max)) : std::nullopt;
			return StringView(env, _ref, IRange(std::move(submin), std::move(submax)));
		}
		else
			return *this;
	}
	
	StringView StringView::substring(const EvalContext& env, const Indexer& indexer) const
	{
		if (const Int* index = std::get_if<Int>(&indexer))
			return substring(env, *index);
		else
			return substring(env, std::get<IRange>(indexer));
	}

	void StringView::assert_valid(const EvalContext& env) const
	{
		const int min = min_index();
		const int max = max_index();

		if (string().empty() || min < 0 || min >= string().size() || max < 0 || max >= string().size())
			throw_out_of_range(env, _indexer, string().size());
	}

	void StringView::assert_in_range(const EvalContext& env, const Indexer& indexer, const int min, const int max)
	{
		const int len = std::abs(max - min) + 1;
		if (const Int* index = std::get_if<Int>(&indexer))
		{
			if (index->value() < 0 || index->value() >= len)
				throw_out_of_range(env, indexer, len);
		}
		else
		{
			const IRange& range = std::get<IRange>(indexer);
			if (range.min() && (*range.min() < 0 || *range.min() >= len))
				throw_out_of_range(env, indexer, len);
			else if (range.max() && (*range.max() < 0 || *range.max() >= len))
				throw_out_of_range(env, indexer, len);
		}
	}

	void StringView::throw_out_of_range(const EvalContext& env, const Indexer& indexer, const int len)
	{
		std::stringstream ss;
		if (const Int* index = std::get_if<Int>(&indexer))
		{
			ss << "index ";
			index->print(env, ss);
		}
		else
		{
			ss << "range ";
			std::get<IRange>(indexer).print(env, ss);
		}

		ss << " is out of range for " << DataType::String() << " of length " << len;
		throw env.runtime_error(ss.str());
	}

	int StringView::min_index() const
	{
		if (const IRange* range = std::get_if<IRange>(&_indexer))
			return range->min() ? *range->min() : 0;
		else
			return std::get<Int>(_indexer).value();
	}
	
	int StringView::max_index() const
	{
		if (const IRange* range = std::get_if<IRange>(&_indexer))
			return range->max() ? *range->max() : string().size() - 1;
		else
			return std::get<Int>(_indexer).value();
	}

	std::string_view StringView::string() const
	{
		if (_string)
			return _string->_value;
		else
			throw LxError(ErrorType::Internal, "StringView base string is null");
	}

	char StringView::chr(int i, int min, int max) const
	{
		return string()[idx(i, min, max)];
	}

	int StringView::idx(int i, int min, int max) const
	{
		return min + (min <= max ? i : -i);
	}
}
