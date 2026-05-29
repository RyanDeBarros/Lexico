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

	DataPoint IRange::cast_copy(const VarContext& ctx, const DataType& type) const
	{
		if (type.simple() == SimpleType::IRange)
			return IRange(*this);
		else if (type.simple() == SimpleType::Void)
			return Void();
		else if (type == DataType::List(DataType::Int()))
		{
			if (!_min || !_max)
			{
				std::stringstream ss;
				ss << "cannot cast ";
				print(ctx.env, ss);
				ss << " to " << type;
				throw ctx.env.runtime_error(ss.str());
			}
			
			std::vector<Variable> elements;
			if (*_min <= *_max)
				for (int i = *_min; i <= *_max; ++i)
					elements.push_back(ctx.variable(Int(i)));
			else
				for (int i = *_min; i >= *_max; --i)
					elements.push_back(ctx.variable(Int(i)));

			if (!elements.empty())
				return List(ctx.env, std::move(elements));
			else
				return List(ctx.env, DataType::Int());
		}
		else
			ctx.env.throw_bad_cast(data_type(), type);
	}

	DataPoint IRange::cast_move(VarContext&& ctx, const DataType& type) &&
	{
		(void*)this; // ignore const warning
		return cast_copy(ctx, type);
	}

	Variable IRange::pass_arg(VarContext ctx)
	{
		return ctx.self.heap().add(IRange(*this));
	}

	void IRange::print(const EvalContext& env, std::ostream& ss) const
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

	Variable IRange::data_member(VarContext& ctx, const std::string_view member)
	{
		ctx.throw_no_data_member(member);
	}

	Variable IRange::invoke_method(VarContext& ctx, const std::string_view method, std::vector<Variable>&& args)
	{
		ctx.throw_no_method(method, args);
	}

	void IRange::assign(const EvalContext& env, Variable o)
	{
		IRange other = std::move(o).consume_as<IRange>(env);
		_min = std::move(other._min);
		_max = std::move(other._max);
	}

	bool IRange::equals(const EvalContext& env, Variable o) const
	{
		Variable casted = std::move(o).cast(env, data_type());
		const IRange& other = casted.ref().get<IRange>();
		return _min == other._min && _max == other._max;
	}

	size_t IRange::iterlen(const EvalContext& env) const
	{
		if (!_min || !_max)
			throw env.runtime_error("cannot iterate over unbounded range");

		return static_cast<size_t>(std::abs(*_max - *_min) + 1);
	}

	Variable IRange::iterget(VarContext& ctx, size_t i) const
	{
		if (!_min || !_max)
			throw ctx.env.runtime_error("cannot iterate over unbounded range");

		int dir = *_max >= *_min ? 1 : -1;
		return ctx.variable(Int(*_min + dir * i));
	}

	std::optional<int> IRange::min() const
	{
		return _min;
	}

	std::optional<int>& IRange::min()
	{
		return _min;
	}

	std::optional<int> IRange::max() const
	{
		return _max;
	}

	std::optional<int>& IRange::max()
	{
		return _max;
	}
}
