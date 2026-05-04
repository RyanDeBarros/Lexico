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
		switch (type)
		{
		case DataType::Int:
			return Int::make_from_literal(resolved);
		case DataType::Float:
			return Float::make_from_literal(resolved);
		case DataType::Bool:
			return Bool::make_from_literal(resolved);
		case DataType::String:
			return String::make_from_literal(resolved);
		default:
		{
			std::stringstream ss;
			ss << __FUNCTION__ << ": " << type << " does not support literal construction";
			throw LxError(ErrorType::Internal, ss.str());
		}
		}
	}

	DataPoint DataPoint::cast_copy(DataType type) const
	{
		return std::visit([type](const auto& v) { return DataPoint(v.cast_copy(type)); }, _storage);
	}

	DataPoint DataPoint::cast_move(DataType type)
	{
		return std::visit([type](auto&& v) { return DataPoint(v.cast_move(type)); }, std::move(_storage));
	}

	void DataPoint::set(const DataPoint& other)
	{
		std::visit([this](const auto& o) { setval(o); }, other._storage);
	}

	void DataPoint::set(DataPoint&& other)
	{
		std::visit([this](auto&& o) { setval(std::forward<decltype(o)>(o)); }, std::move(other._storage));
	}

	bool DataPoint::can_cast_implicit(DataType to) const
	{
		return lx::can_cast_implicit(static_cast<DataType>(_storage.index()), to);
	}

	bool DataPoint::can_cast_explicit(DataType to) const
	{
		return lx::can_cast_explicit(static_cast<DataType>(_storage.index()), to);
	}
}
