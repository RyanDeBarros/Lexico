#include "processing.h"

namespace lx
{
	Variable operate(Runtime& env, BinaryOperator op, Variable&& lhs, const ScriptSegment& lhs_segment, Variable&& rhs, const ScriptSegment& rhs_segment)
	{
		switch (op)
		{
		case BinaryOperator::And:
			if (lhs.ref().can_cast_implicit(DataType::Bool()) && rhs.ref().can_cast_implicit(DataType::Bool()))
				return env.unbound_variable(Bool(lhs.dp().move_as<Bool>().value() && rhs.dp().move_as<Bool>().value()));
			else
				break;

		case BinaryOperator::Assign:
			if (lhs.unbound())
				throw LxError::segment_error(lhs_segment, ErrorType::Runtime, "cannot assign temporary variable");
			else if (rhs.ref().can_cast_implicit(lhs.ref().data_type()))
			{
				lhs.dp().set(rhs.dp());
				return std::move(lhs);
			}
			else
				break;

		case BinaryOperator::Asterisk:
		case BinaryOperator::Minus:
		case BinaryOperator::Mod:
		case BinaryOperator::Plus:
		case BinaryOperator::Slash:
		{
			bool lint = lhs.ref().can_cast_implicit(DataType::Int());
			bool rint = rhs.ref().can_cast_implicit(DataType::Int());
			if (lint && rint)
			{
				int l = lhs.dp().move_as<Int>().value();
				int r = rhs.dp().move_as<Int>().value();
				
				int res = 0;
				switch (op)
				{
				case BinaryOperator::Asterisk:
					res = l * r;
					break;
				case BinaryOperator::Minus:
					res = l - r;
					break;
				case BinaryOperator::Mod:
					res = l % r;
					break;
				case BinaryOperator::Plus:
					res = l + r;
					break;
				case BinaryOperator::Slash:
					res = l / r;
					break;
				}
				return env.unbound_variable(Int(res));
			}
			else
			{
				if ((lint || lhs.ref().can_cast_implicit(DataType::Float())) && (rint || rhs.ref().can_cast_implicit(DataType::Float())))
				{
					float l = lint ? static_cast<float>(lhs.dp().move_as<Int>().value()) : lhs.dp().move_as<Float>().value();
					float r = rint ? static_cast<float>(rhs.dp().move_as<Int>().value()) : rhs.dp().move_as<Float>().value();

					float res = 0.f;
					switch (op)
					{
					case BinaryOperator::Asterisk:
						res = l * r;
						break;
					case BinaryOperator::Minus:
						res = l - r;
						break;
					case BinaryOperator::Mod:
						res = std::fmod(l, r);
						break;
					case BinaryOperator::Plus:
						res = l + r;
						break;
					case BinaryOperator::Slash:
						res = l / r;
						break;
					}
					return env.unbound_variable(Float(res));
				}
				else
					break;
			}
		}

		case BinaryOperator::Comma:
			if (lhs.ref().can_cast_implicit(DataType::Pattern()) && rhs.ref().can_cast_implicit(DataType::Pattern()))
			{
				Pattern res;
				auto& cat = res.make_root<SubpatternCatenation>();
				cat.append(res.take(lhs.dp().move_as<Pattern>()));
				cat.append(res.take(rhs.dp().move_as<Pattern>()));
				return env.unbound_variable(std::move(res));
			}
			else
				break;

		case BinaryOperator::EqualTo:
		case BinaryOperator::NotEqualTo:
			if (lhs.ref().can_cast_implicit(rhs.ref().data_type()))
			{
				bool eq = lhs.ref().equals(rhs.dp());
				return env.unbound_variable(Bool(op == BinaryOperator::EqualTo ? eq : !eq));
			}
			else if (lhs.ref().can_cast_implicit(rhs.ref().data_type()))
			{
				bool eq = rhs.ref().equals(lhs.dp());
				return env.unbound_variable(Bool(op == BinaryOperator::EqualTo ? eq : !eq));
			}
			else
				break;

		case BinaryOperator::Except:
			if (lhs.ref().can_cast_implicit(DataType::Pattern()) && rhs.ref().can_cast_implicit(DataType::Pattern()))
			{
				Pattern res;
				res.make_root<SubpatternException>(
					res.take(lhs.dp().move_as<Pattern>()),
					res.take(rhs.dp().move_as<Pattern>())
				);
				return env.unbound_variable(std::move(res));
			}
			else
				break;

		case BinaryOperator::GreaterThan:
		case BinaryOperator::GreaterThanOrEqualTo:
		case BinaryOperator::LessThan:
		case BinaryOperator::LessThanOrEqualTo:
		{
			bool lint = lhs.ref().can_cast_implicit(DataType::Int());
			bool rint = rhs.ref().can_cast_implicit(DataType::Int());

			if ((lint || lhs.ref().can_cast_implicit(DataType::Float())) && (rint || rhs.ref().can_cast_implicit(DataType::Float())))
			{
				float l = lint ? static_cast<float>(lhs.dp().move_as<Int>().value()) : lhs.dp().move_as<Float>().value();
				float r = rint ? static_cast<float>(rhs.dp().move_as<Int>().value()) : rhs.dp().move_as<Float>().value();

				bool res = false;
				switch (op)
				{
				case BinaryOperator::GreaterThan:
					res = l > r;
					break;
				case BinaryOperator::GreaterThanOrEqualTo:
					res = l >= r;
					break;
				case BinaryOperator::LessThan:
					res = l < r;
					break;
				case BinaryOperator::LessThanOrEqualTo:
					res = l <= r;
					break;
				}
				return env.unbound_variable(Bool(res));
			}
			else
				break;
		}

		case BinaryOperator::Or:
			if (lhs.ref().can_cast_implicit(DataType::Bool()) && rhs.ref().can_cast_implicit(DataType::Bool()))
				return env.unbound_variable(Bool(lhs.dp().move_as<Bool>().value() && rhs.dp().move_as<Bool>().value()));
			else if (lhs.ref().can_cast_implicit(DataType::Pattern()) && rhs.ref().can_cast_implicit(DataType::Pattern()))
			{
				Pattern res;
				auto& disj = res.make_root<SubpatternDisjunction>();
				disj.append(res.take(lhs.dp().move_as<Pattern>()));
				disj.append(res.take(rhs.dp().move_as<Pattern>()));
				return env.unbound_variable(std::move(res));
			}
			else
				break;

		case BinaryOperator::Repeat:
			if (lhs.ref().can_cast_implicit(DataType::Pattern()) && rhs.ref().can_cast_implicit(DataType::IRange()))
				return env.unbound_variable(Pattern::make_repeat(lhs.dp().move_as<Pattern>(), rhs.dp().move_as<IRange>()));
			else
				break;

		case BinaryOperator::To:
			if (lhs.ref().can_cast_implicit(DataType::Int()) && rhs.ref().can_cast_implicit(DataType::Int()))
				return env.unbound_variable(IRange(lhs.dp().move_as<Int>().value(), rhs.dp().move_as<Int>().value()));
			else if (lhs.ref().can_cast_implicit(DataType::String()) && rhs.ref().can_cast_implicit(DataType::String()))
				return env.unbound_variable(SRange(lhs.dp().move_as<String>().value(), rhs.dp().move_as<String>().value()));
			else
				break;
		}

		std::stringstream ss;
		ss << "operator not supported for types " << lhs.ref().data_type().repr() << " and " << rhs.ref().data_type().repr();
		throw LxError::segment_error(lhs_segment.combined_right(rhs_segment), ErrorType::Runtime, ss.str());
	}

	Variable operate(Runtime& env, PrefixOperator op, Variable&& var, const ScriptSegment& segment)
	{
		switch (op)
		{
		case PrefixOperator::Ahead:
			if (var.ref().can_cast_implicit(DataType::Pattern()))
			{
				Pattern res;
				res.make_root<SubpatternLookaround>(LookaroundMode::Ahead, res.take(var.dp().move_as<Pattern>()));
				return env.unbound_variable(std::move(res));
			}
			else
				break;

		case PrefixOperator::Behind:
			if (var.ref().can_cast_implicit(DataType::Pattern()))
			{
				Pattern res;
				res.make_root<SubpatternLookaround>(LookaroundMode::Behind, res.take(var.dp().move_as<Pattern>()));
				return env.unbound_variable(std::move(res));
			}
			else
				break;

		case PrefixOperator::Max:
			if (var.ref().can_cast_implicit(DataType::Int()))
				return env.unbound_variable(IRange(std::nullopt, var.dp().move_as<Int>().value()));
			else if (var.ref().can_cast_implicit(DataType::String()))
				return env.unbound_variable(SRange(std::nullopt, nullptr, var.dp().move_as<String>().value(), &segment));
			else
				break;

		case PrefixOperator::Min:
			if (var.ref().can_cast_implicit(DataType::Int()))
				return env.unbound_variable(IRange(var.dp().move_as<Int>().value(), std::nullopt));
			else if (var.ref().can_cast_implicit(DataType::String()))
				return env.unbound_variable(SRange(var.dp().move_as<String>().value(), &segment, std::nullopt, nullptr));
			else
				break;

		case PrefixOperator::Minus:
			if (var.ref().can_cast_implicit(DataType::Int()))
				return env.unbound_variable(Int(-var.dp().move_as<Int>().value()));
			else if (var.ref().can_cast_implicit(DataType::Float()))
				return env.unbound_variable(Float(-var.dp().move_as<Float>().value()));
			else
				break;

		case PrefixOperator::Not:
			if (var.ref().can_cast_implicit(DataType::Bool()))
				return env.unbound_variable(Bool(!var.dp().move_as<Bool>().value()));
			else
				break;

		case PrefixOperator::NotAhead:
			if (var.ref().can_cast_implicit(DataType::Pattern()))
			{
				Pattern res;
				res.make_root<SubpatternLookaround>(LookaroundMode::NotAhead, res.take(var.dp().move_as<Pattern>()));
				return env.unbound_variable(std::move(res));
			}
			else
				break;

		case PrefixOperator::NotBehind:
			if (var.ref().can_cast_implicit(DataType::Pattern()))
			{
				Pattern res;
				res.make_root<SubpatternLookaround>(LookaroundMode::NotBehind, res.take(var.dp().move_as<Pattern>()));
				return env.unbound_variable(std::move(res));
			}
			else
				break;

		case PrefixOperator::Optional:
			if (var.ref().can_cast_implicit(DataType::Pattern()))
			{
				Pattern res;
				res.make_root<SubpatternOptional>(res.take(var.dp().move_as<Pattern>()));
				return env.unbound_variable(std::move(res));
			}
			else
				break;

		case PrefixOperator::Ref:
			if (var.ref().can_cast_implicit(DataType::CapId()))
			{
				Pattern res;
				res.make_root<SubpatternBackRef>(var.dp().move_as<CapId>());
				return env.unbound_variable(std::move(res));
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
			return env.unbound_variable(Pattern::make_repeat(var.dp().move_as<Pattern>(), IRange(0, std::nullopt)));
		case PatternSimpleRepeatOperator::Plus:
			return env.unbound_variable(Pattern::make_repeat(var.dp().move_as<Pattern>(), IRange(1, std::nullopt)));
		default:
		{
			std::stringstream ss;
			ss << __FUNCTION__ << ": operator \"" << static_cast<int>(op) << "\" not supported";
			throw LxError(ErrorType::Internal, ss.str());
		}
		}
	}
}
