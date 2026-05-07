#include "processing.h"

namespace lx
{
	Variable operate(Runtime& env, BinaryOperator op, Variable lhs, const Variable& rhs)
	{
		// TODO note that '=' shouldn't allow for assigning to temporary Variables
		return env.temporary_variable(Void());
	}

	Variable operate(Runtime& env, PrefixOperator op, Variable var)
	{
		// TODO
		return env.temporary_variable(Void());
	}

	Variable operate(Runtime& env, PatternSimpleRepeatOperator op, Variable var, const ScriptSegment& var_segment)
	{
		if (!var.ref().holds<Pattern>())
		{
			std::stringstream ss;
			ss << "expected " << DataType::Pattern() << " but resolved to " << var.ref().data_type() << " instead";
			throw LxError::segment_error(var_segment, ErrorType::Runtime, ss.str());
		}

		switch (op)
		{
		case PatternSimpleRepeatOperator::Asterisk:
			return env.temporary_variable(Pattern::make_repeat(var.dp().move_as<Pattern>(), IRange(0, std::nullopt)));
		case PatternSimpleRepeatOperator::Plus:
			return env.temporary_variable(Pattern::make_repeat(var.dp().move_as<Pattern>(), IRange(1, std::nullopt)));
		default:
		{
			std::stringstream ss;
			ss << __FUNCTION__ << ": operator \"" << static_cast<int>(op) << "\" not supported";
			throw LxError(ErrorType::Internal, ss.str());
		}
		}
	}
}
