#pragma once

#include "errors.h"
#include "variable.h"

namespace lx
{
	class Runtime;
	struct ScriptSegment;
	class DataType;
	class DataPoint;

	struct EvalContext
	{
		Runtime& runtime;
		const ScriptSegment* segment;

		LxError runtime_error(const std::string_view message) const;
		LxError internal_error(const std::string_view message) const;

		[[noreturn]] void throw_bad_set_expression(const DataType& from, const DataType& to) const;
		[[noreturn]] void throw_bad_cast(const DataType& from, const DataType& to) const;
	};

	struct VarContext
	{
		const EvalContext& env;
		Variable self;

		VarContext(const EvalContext& env, Variable self);

		Variable variable(DataPoint&& dp) const;

		[[noreturn]] void throw_bad_set_expression(const DataType& to) const;
		[[noreturn]] void throw_bad_set_expression(const DataPoint& to) const;
		[[noreturn]] void throw_no_data_member(const std::string_view member) const;
		[[noreturn]] void throw_no_method(const std::string_view method, const std::vector<Variable>& args) const;
		[[noreturn]] void throw_bad_cast(const DataType& to) const;
		[[noreturn]] void throw_bad_cast(const DataPoint& to) const;
	};
}
