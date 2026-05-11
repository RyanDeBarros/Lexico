#include "irange.h"

#include "include.h"
#include "runtime.h"

namespace lx
{
	IRange::IRange(std::optional<int> min, std::optional<int> max)
		: _min(min), _max(max)
	{
	}

	DataType IRange::data_type()
	{
		return DataType::IRange();
	}

	TypeVariant IRange::cast_copy(const EvalContext& env, const DataType& type) const
	{
		if (type.simple() == SimpleType::IRange)
			return *this;
		else if (type.simple() == SimpleType::Void)
			return Void();
		else
			env.throw_bad_cast(data_type(), type);
	}

	TypeVariant IRange::cast_move(const EvalContext& env, const DataType& type)
	{
		(void*)this; // ignore const warning
		return cast_copy(env, type);
	}

	void IRange::print(const EvalContext& env, std::stringstream& ss) const
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

	void IRange::assign(const EvalContext& env, IRange&& o)
	{
		// TODO
	}

	bool IRange::equals(const EvalContext& env, const IRange& o) const
	{
		return _min == o._min && _max == o._max;
	}

	size_t IRange::iterlen(const EvalContext& env) const
	{
		if (!_min || !_max)
			throw env.runtime_error("cannot iterate over unbounded range");

		return static_cast<size_t>(std::abs(*_max - *_min) + 1);
	}

	DataPoint IRange::iterget(const EvalContext& env, size_t i) const
	{
		if (!_min || !_max)
			throw env.runtime_error("cannot iterate over unbounded range");

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
}
