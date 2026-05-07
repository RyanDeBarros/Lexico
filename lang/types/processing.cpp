#include "processing.h"

namespace lx
{
	Variable operate(Runtime& env, BinaryOperator op, Variable&& lhs, const ScriptSegment& lhs_segment, Variable&& rhs, const ScriptSegment& rhs_segment)
	{
		// TODO note that '=' shouldn't allow for assigning to temporary Variables
		return env.temporary_variable(Void());
	}

	Variable operate(Runtime& env, PrefixOperator op, Variable&& var, const ScriptSegment& segment)
	{
		switch (op)
		{
		case PrefixOperator::Ahead:
		{
			if (var.ref().can_cast_implicit(DataType::Pattern()))
			{
				Pattern res;
				Pattern& subject = res.add(var.dp().move_as<Pattern>());
				res.append(res.add(std::make_unique<SubpatternLookaround>(LookaroundMode::Ahead, subject)));
				return env.temporary_variable(std::move(res));
			}
			else
				break;
		}

		case PrefixOperator::Behind:
		{
			if (var.ref().can_cast_implicit(DataType::Pattern()))
			{
				Pattern res;
				Pattern& subject = res.add(var.dp().move_as<Pattern>());
				res.append(res.add(std::make_unique<SubpatternLookaround>(LookaroundMode::Behind, subject)));
				return env.temporary_variable(std::move(res));
			}
			else
				break;
		}

		case PrefixOperator::Max:
		{
			if (var.ref().can_cast_implicit(DataType::Int()))
				return env.temporary_variable(IRange(std::nullopt, var.dp().move_as<Int>().value()));
			else if (var.ref().can_cast_implicit(DataType::String()))
				return env.temporary_variable(SRange(std::nullopt, nullptr, std::string(var.dp().move_as<String>().value()), &segment));
			else
				break;
		}

		case PrefixOperator::Min:
		{
			if (var.ref().can_cast_implicit(DataType::Int()))
				return env.temporary_variable(IRange(var.dp().move_as<Int>().value(), std::nullopt));
			else if (var.ref().can_cast_implicit(DataType::String()))
				return env.temporary_variable(SRange(std::string(var.dp().move_as<String>().value()), &segment, std::nullopt, nullptr));
			else
				break;
		}

		case PrefixOperator::Minus:
			if (var.ref().can_cast_implicit(DataType::Int()))
				return env.temporary_variable(Int(-var.dp().move_as<Int>().value()));
			else if (var.ref().can_cast_implicit(DataType::Float()))
				return env.temporary_variable(Float(-var.dp().move_as<Float>().value()));
			else
				break;

		case PrefixOperator::Not:
			if (var.ref().can_cast_implicit(DataType::Bool()))
				return env.temporary_variable(Bool(!var.dp().move_as<Bool>().value()));
			else
				break;

		case PrefixOperator::NotAhead:
		{
			if (var.ref().can_cast_implicit(DataType::Pattern()))
			{
				Pattern res;
				Pattern& subject = res.add(var.dp().move_as<Pattern>());
				res.append(res.add(std::make_unique<SubpatternLookaround>(LookaroundMode::NotAhead, subject)));
				return env.temporary_variable(std::move(res));
			}
			else
				break;
		}

		case PrefixOperator::NotBehind:
		{
			if (var.ref().can_cast_implicit(DataType::Pattern()))
			{
				Pattern res;
				Pattern& subject = res.add(var.dp().move_as<Pattern>());
				res.append(res.add(std::make_unique<SubpatternLookaround>(LookaroundMode::NotBehind, subject)));
				return env.temporary_variable(std::move(res));
			}
			else
				break;
		}

		case PrefixOperator::Optional:
		{
			if (var.ref().can_cast_implicit(DataType::Pattern()))
			{
				Pattern res;
				Pattern& subject = res.add(var.dp().move_as<Pattern>());
				res.append(res.add(std::make_unique<SubpatternOptional>(subject)));
				return env.temporary_variable(std::move(res));
			}
			else
				break;
		}

		case PrefixOperator::Ref:
			if (var.ref().can_cast_implicit(DataType::CapId()))
			{
				Pattern res;
				res.append(res.add(std::make_unique<SubpatternBackRef>(var.dp().move_as<CapId>())));
				return env.temporary_variable(std::move(res));
			}
			else
				break;
		}

		throw LxError::segment_error(segment, ErrorType::Runtime, "operator not supported for type " + var.ref().data_type().repr());
	}

	Variable operate(Runtime& env, PatternSimpleRepeatOperator op, Variable&& var, const ScriptSegment& var_segment)
	{
		if (!var.ref().can_cast_implicit(DataType::Pattern()))
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
