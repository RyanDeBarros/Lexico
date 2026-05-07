#include "datapoint.h"

#include <sstream>

namespace lx
{
	DataPoint::DataPoint(const DataPoint& other)
		: _storage(other._storage)
	{
	}

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

	bool DataPoint::can_cast_implicit(const DataType& to) const
	{
		return lx::can_cast_implicit(data_type(), to);
	}

	bool DataPoint::can_cast_explicit(const DataType& to) const
	{
		return lx::can_cast_explicit(data_type(), to);
	}

	DataType DataPoint::data_type() const
	{
		return std::visit([](const auto& v) -> DataType { return v.data_type(); }, _storage);
	}

	void DataPoint::print(std::stringstream& ss) const
	{
		std::visit([&ss](const auto& v) { v.print(ss); }, _storage);
	}
}
