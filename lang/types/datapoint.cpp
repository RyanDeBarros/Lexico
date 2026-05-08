#include "datapoint.h"

#include <sstream>

namespace lx
{
	DataPoint::DataPoint(const TypeVariant& v)
		: _storage(v)
	{
	}

	DataPoint::DataPoint(TypeVariant&& v)
		: _storage(std::move(v))
	{
	}

	DataPoint DataPoint::make_from_literal(DataType type, std::string_view resolved)
	{
		switch (type.simple())
		{
		case SimpleType::Int:
			return Int::make_from_literal(resolved);
		case SimpleType::Float:
			return Float::make_from_literal(resolved);
		case SimpleType::Bool:
			return Bool::make_from_literal(resolved);
		case SimpleType::String:
			return String::make_from_literal(resolved);
		default:
		{
			std::stringstream ss;
			ss << __FUNCTION__ << ": " << type << " does not support literal construction";
			throw LxError(ErrorType::Internal, ss.str());
		}
		}
	}

	const TypeVariant& DataPoint::variant() const
	{
		return _storage;
	}

	TypeVariant& DataPoint::variant()
	{
		return _storage;
	}

	DataPoint DataPoint::cast_copy(const DataType& type) const
	{
		return std::visit([&type](const auto& v) { return DataPoint(v.cast_copy(type)); }, _storage);
	}

	DataPoint DataPoint::cast_move(const DataType& type)
	{
		return std::visit([&type](auto&& v) { return DataPoint(v.cast_move(type)); }, std::move(_storage));
	}

	void DataPoint::set(const DataPoint& other)
	{
		std::visit([this](const auto& o) { setval(o); }, other._storage);
	}

	void DataPoint::set(DataPoint&& other)
	{
		std::visit([this](auto&& o) { setval(std::forward<decltype(o)>(o)); }, std::move(other._storage));
	}

	bool DataPoint::equals(const DataPoint& other) const
	{
		if (other.can_cast_implicit(data_type()))
			return std::visit([o = other.cast_copy(data_type())](const auto& v) { return v.equals(o.get<std::decay_t<decltype(v)>>()); }, _storage);
		else
			return false;
	}

	bool DataPoint::equals(DataPoint&& other) const
	{
		if (other.can_cast_implicit(data_type()))
			return std::visit([o = other.cast_move(data_type())](const auto& v) { return v.equals(o.get<std::decay_t<decltype(v)>>()); }, _storage);
		else
			return false;
	}

	bool DataPoint::can_cast_implicit(const DataType& to) const
	{
		return data_type().can_cast_implicit(to);
	}

	bool DataPoint::can_cast_explicit(const DataType& to) const
	{
		return data_type().can_cast_explicit(to);
	}

	DataType DataPoint::data_type() const
	{
		return std::visit([](const auto& v) -> DataType { return v.data_type(); }, _storage);
	}

	void DataPoint::print(std::stringstream& ss) const
	{
		std::visit([&ss](const auto& v) { v.print(ss); }, _storage);
	}

	size_t DataPoint::iterlen() const
	{
		if (data_type().is_iterable())
			return std::visit([](const auto& v) -> size_t {
				if constexpr (requires { v.iterlen(); })
					return v.iterlen();
				else
					throw LxError(ErrorType::Internal, v.data_type().repr() + " should implement 'iterlen' but it doesn't");
			}, _storage);
		else
			throw LxError(ErrorType::Internal, "iterlen(): " + data_type().repr() + " is not iterable");
	}

	DataPoint DataPoint::iterget(size_t i) const
	{
		if (data_type().is_iterable())
			return std::visit([i](const auto& v) -> DataPoint {
				if constexpr (requires { v.iterget(i); })
					return v.iterget(i);
				else
					throw LxError(ErrorType::Internal, v.data_type().repr() + " should implement 'iterget' but it doesn't");
			}, _storage);
		else
			throw LxError(ErrorType::Internal, "iterget(): " + data_type().repr() + " is not iterable");
	}

	std::string DataPoint::page_content() const
	{
		if (data_type().is_pageable())
			return std::visit([](const auto& v) -> std::string {
			if constexpr (requires { v.page_content(); })
				return v.page_content();
			else
				throw LxError(ErrorType::Internal, v.data_type().repr() + " should implement 'page_content' but it doesn't");
				}, _storage);
		else
			throw LxError(ErrorType::Internal, "page_content(): " + data_type().repr() + " is not pageable");
	}

	Variable DataPoint::data_member(Runtime& env, const ScriptSegment& segment, const std::string_view member) const
	{
		return std::visit([&env, &segment, member](const auto& v) -> Variable { return v.data_member(env, segment, member); }, _storage);
	}

	Variable DataPoint::invoke_method(Runtime& env, const ScriptSegment& segment, const std::string_view method, std::vector<Variable>&& args) const
	{
		return std::visit([&env, &segment, method, &args](const auto& v) -> Variable { return v.invoke_method(env, segment, method, std::move(args)); }, _storage);
	}
}
