#include "processing.h"

#include "evalcontext.h"
#include "find.h"

namespace lx
{
	Variable operate(const EvalContext& env, BinaryOperator op, Variable&& lhs, Variable&& rhs)
	{
		switch (op)
		{
		case BinaryOperator::And:
			if (lhs.ref().can_cast_implicit(DataType::Bool()) && rhs.ref().can_cast_implicit(DataType::Bool()))
				return env.runtime.unbound_variable(Bool(std::move(lhs).consume_as<Bool>(env).value() && std::move(rhs).consume_as<Bool>(env).value()));
			else
				break;

		case BinaryOperator::Assign:
			if (lhs.temporary())
				env.log_runtime_warning("cannot assign temporary variable");
			
			if (rhs.ref().can_cast_implicit(lhs.ref().data_type()))
			{
				lhs.ref().assign(env, std::move(rhs));
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
				int l = std::move(lhs).consume_as<Int>(env).value();
				int r = std::move(rhs).consume_as<Int>(env).value();
				
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
				return env.runtime.unbound_variable(Int(res));
			}
			else
			{
				if ((lint || lhs.ref().can_cast_implicit(DataType::Float())) && (rint || rhs.ref().can_cast_implicit(DataType::Float())))
				{
					float l = lint ? static_cast<float>(std::move(lhs).consume_as<Int>(env).value()) : std::move(lhs).consume_as<Float>(env).value();
					float r = rint ? static_cast<float>(std::move(rhs).consume_as<Int>(env).value()) : std::move(rhs).consume_as<Float>(env).value();

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
					return env.runtime.unbound_variable(Float(res));
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
				cat.append(res.take(std::move(lhs).consume_as<Pattern>(env)));
				cat.append(res.take(std::move(rhs).consume_as<Pattern>(env)));
				return env.runtime.unbound_variable(std::move(res));
			}
			else
				break;

		case BinaryOperator::EqualTo:
		case BinaryOperator::NotEqualTo:
			if (lhs.ref().can_cast_implicit(rhs.ref().data_type()))
			{
				bool eq = lhs.ref().equals(env, std::move(rhs));
				return env.runtime.unbound_variable(Bool(op == BinaryOperator::EqualTo ? eq : !eq));
			}
			else if (lhs.ref().can_cast_implicit(rhs.ref().data_type()))
			{
				bool eq = rhs.ref().equals(env, std::move(lhs));
				return env.runtime.unbound_variable(Bool(op == BinaryOperator::EqualTo ? eq : !eq));
			}
			else
				break;

		case BinaryOperator::Except:
			if (lhs.ref().can_cast_implicit(DataType::Pattern()) && rhs.ref().can_cast_implicit(DataType::Pattern()))
			{
				Pattern res;
				res.make_root<SubpatternException>(
					res.take(std::move(lhs).consume_as<Pattern>(env)),
					res.take(std::move(rhs).consume_as<Pattern>(env))
				);
				return env.runtime.unbound_variable(std::move(res));
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
				float l = lint ? static_cast<float>(std::move(lhs).consume_as<Int>(env).value()) : std::move(lhs).consume_as<Float>(env).value();
				float r = rint ? static_cast<float>(std::move(rhs).consume_as<Int>(env).value()) : std::move(rhs).consume_as<Float>(env).value();

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
				return env.runtime.unbound_variable(Bool(res));
			}
			else
				break;
		}

		case BinaryOperator::Or:
			if (lhs.ref().can_cast_implicit(DataType::Bool()) && rhs.ref().can_cast_implicit(DataType::Bool()))
				return env.runtime.unbound_variable(Bool(std::move(lhs).consume_as<Bool>(env).value() && std::move(rhs).consume_as<Bool>(env).value()));
			else if (lhs.ref().can_cast_implicit(DataType::Pattern()) && rhs.ref().can_cast_implicit(DataType::Pattern()))
			{
				Pattern res;
				auto& disj = res.make_root<SubpatternDisjunction>();
				disj.append(res.take(std::move(lhs).consume_as<Pattern>(env)));
				disj.append(res.take(std::move(rhs).consume_as<Pattern>(env)));
				return env.runtime.unbound_variable(std::move(res));
			}
			else
				break;

		case BinaryOperator::Repeat:
			if (lhs.ref().can_cast_implicit(DataType::Pattern()) && rhs.ref().can_cast_implicit(DataType::IRange()))
				return env.runtime.unbound_variable(Pattern::make_repeat(std::move(lhs).consume_as<Pattern>(env), std::move(rhs).consume_as<IRange>(env)));
			else
				break;

		case BinaryOperator::To:
			if (lhs.ref().can_cast_implicit(DataType::Int()) && rhs.ref().can_cast_implicit(DataType::Int()))
				return env.runtime.unbound_variable(IRange(std::move(lhs).consume_as<Int>(env).value(), std::move(rhs).consume_as<Int>(env).value()));
			else if (lhs.ref().can_cast_implicit(DataType::String()) && rhs.ref().can_cast_implicit(DataType::String()))
				return env.runtime.unbound_variable(SRange(std::move(lhs).consume_as<String>(env).value(), std::move(rhs).consume_as<String>(env).value()));
			else
				break;
		}

		std::stringstream ss;
		ss << "operator not supported for types " << lhs.ref().data_type().repr() << " and " << rhs.ref().data_type().repr();
		throw env.runtime_error(ss.str());
	}

	Variable operate(const EvalContext& env, PrefixOperator op, Variable&& var)
	{
		switch (op)
		{
		case PrefixOperator::Ahead:
			if (var.ref().can_cast_implicit(DataType::Pattern()))
			{
				Pattern res;
				res.make_root<SubpatternLookaround>(LookaroundMode::Ahead, res.take(std::move(var).consume_as<Pattern>(env)));
				return env.runtime.unbound_variable(std::move(res));
			}
			else
				break;

		case PrefixOperator::Behind:
			if (var.ref().can_cast_implicit(DataType::Pattern()))
			{
				Pattern res;
				res.make_root<SubpatternLookaround>(LookaroundMode::Behind, res.take(std::move(var).consume_as<Pattern>(env)));
				return env.runtime.unbound_variable(std::move(res));
			}
			else
				break;

		case PrefixOperator::Max:
			if (var.ref().can_cast_implicit(DataType::Int()))
				return env.runtime.unbound_variable(IRange(std::nullopt, std::move(var).consume_as<Int>(env).value()));
			else if (var.ref().can_cast_implicit(DataType::String()))
				return env.runtime.unbound_variable(SRange(std::nullopt, nullptr, std::move(var).consume_as<String>(env).value(), env.segment));
			else
				break;

		case PrefixOperator::Min:
			if (var.ref().can_cast_implicit(DataType::Int()))
				return env.runtime.unbound_variable(IRange(std::move(var).consume_as<Int>(env).value(), std::nullopt));
			else if (var.ref().can_cast_implicit(DataType::String()))
				return env.runtime.unbound_variable(SRange(std::move(var).consume_as<String>(env).value(), env.segment, std::nullopt, nullptr));
			else
				break;

		case PrefixOperator::Minus:
			if (var.ref().can_cast_implicit(DataType::Int()))
				return env.runtime.unbound_variable(Int(-std::move(var).consume_as<Int>(env).value()));
			else if (var.ref().can_cast_implicit(DataType::Float()))
				return env.runtime.unbound_variable(Float(-std::move(var).consume_as<Float>(env).value()));
			else
				break;

		case PrefixOperator::Not:
			if (var.ref().can_cast_implicit(DataType::Bool()))
				return env.runtime.unbound_variable(Bool(!std::move(var).consume_as<Bool>(env).value()));
			else
				break;

		case PrefixOperator::NotAhead:
			if (var.ref().can_cast_implicit(DataType::Pattern()))
			{
				Pattern res;
				res.make_root<SubpatternLookaround>(LookaroundMode::NotAhead, res.take(std::move(var).consume_as<Pattern>(env)));
				return env.runtime.unbound_variable(std::move(res));
			}
			else
				break;

		case PrefixOperator::NotBehind:
			if (var.ref().can_cast_implicit(DataType::Pattern()))
			{
				Pattern res;
				res.make_root<SubpatternLookaround>(LookaroundMode::NotBehind, res.take(std::move(var).consume_as<Pattern>(env)));
				return env.runtime.unbound_variable(std::move(res));
			}
			else
				break;

		case PrefixOperator::Optional:
			if (var.ref().can_cast_implicit(DataType::Pattern()))
			{
				Pattern res;
				res.make_root<SubpatternOptional>(res.take(std::move(var).consume_as<Pattern>(env)));
				return env.runtime.unbound_variable(std::move(res));
			}
			else
				break;

		case PrefixOperator::Ref:
			if (var.ref().can_cast_implicit(DataType::CapId()))
			{
				Pattern res;
				res.make_root<SubpatternBackRef>(std::move(var).consume_as<CapId>(env));
				return env.runtime.unbound_variable(std::move(res));
			}
			else
				break;
		}

		throw env.runtime_error("operator not supported for type " + var.ref().data_type().repr());
	}

	Variable operate(const EvalContext& env, PatternSimpleRepeatOperator op, Variable&& var)
	{
		if (!var.ref().can_cast_implicit(DataType::Pattern()))
		{
			std::stringstream ss;
			ss << "expected " << DataType::Pattern() << " but resolved to " << var.ref().data_type() << " instead";
			throw env.runtime_error(ss.str());
		}

		switch (op)
		{
		case PatternSimpleRepeatOperator::Asterisk:
			return env.runtime.unbound_variable(Pattern::make_repeat(std::move(var).consume_as<Pattern>(env), IRange(0, std::nullopt)));
		case PatternSimpleRepeatOperator::Plus:
			return env.runtime.unbound_variable(Pattern::make_repeat(std::move(var).consume_as<Pattern>(env), IRange(1, std::nullopt)));
		default:
		{
			std::stringstream ss;
			ss << __FUNCTION__ << ": operator \"" << static_cast<int>(op) << "\" not supported";
			throw env.internal_error(ss.str());
		}
		}
	}
}
