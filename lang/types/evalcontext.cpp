#include "evalcontext.h"

#include "runtime.h"
#include "token.h"
#include "variable.h"
#include "util.h"

namespace lx
{
	LxError EvalContext::runtime_error(const std::string_view message) const
	{
		if (segment)
			return LxError::segment_error(*segment, ErrorType::Runtime, message);
		else
			return LxError(ErrorType::Runtime, std::string(message));
	}

	LxError EvalContext::internal_error(const std::string_view message) const
	{
		if (segment)
			return LxError::segment_error(*segment, ErrorType::Internal, message);
		else
			return LxError(ErrorType::Internal, std::string(message));
	}

	void EvalContext::throw_bad_set_expression(const DataType& from, const DataType& to) const
	{
		std::stringstream ss;
		ss << "cannot set " << from << " to expression of type " << to;
		throw runtime_error(ss.str());
	}

	void EvalContext::throw_bad_cast(const DataType& from, const DataType& to) const
	{
		std::stringstream ss;
		ss << "bad cast from " << from << " to " << to;
		throw runtime_error(ss.str());
	}

	VarContext::VarContext(const EvalContext& env, Variable self)
		: env(env), self(std::move(self))
	{
	}

	Variable VarContext::variable(DataPoint&& dp) const
	{
		return env.runtime.unbound_variable(std::move(dp));
	}

	Symbol VarContext::symbolize(const std::string_view name) const
	{
		return env.runtime.symbolize(name);
	}

	void VarContext::throw_bad_set_expression(const DataType& to) const
	{
		env.throw_bad_set_expression(self.ref().data_type(), to);
	}

	void VarContext::throw_bad_set_expression(const DataPoint& to) const
	{
		throw_bad_set_expression(to.data_type());
	}

	void VarContext::throw_no_data_member(const std::string_view member) const
	{
		std::stringstream ss;
		ss << self.ref().data_type() << " does not have a data member '" << member << "'";
		throw env.runtime_error(ss.str());
	}

	void VarContext::throw_no_method(const std::string_view method, const std::vector<Variable>& args) const
	{
		std::stringstream ss;
		ss << self.ref().data_type() << " does not have a method '" << method << "' that matches the argument list ";
		print_list(ss, args, [](const Variable& v) { return v.ref().data_type(); });
		throw env.runtime_error(ss.str());
	}

	void VarContext::throw_bad_cast(const DataType& to) const
	{
		env.throw_bad_cast(self.ref().data_type(), to);
	}

	void VarContext::throw_bad_cast(const DataPoint& to) const
	{
		throw_bad_cast(to.data_type());
	}
}
