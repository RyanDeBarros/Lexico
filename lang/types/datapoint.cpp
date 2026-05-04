#include "datapoint.h"

#include <sstream>

namespace lx
{
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

	bool DataPoint::can_cast_implicit(DataType to) const
	{
		return lx::can_cast_implicit(static_cast<DataType>(_storage.index()), to);
	}

	bool DataPoint::can_cast_explicit(DataType to) const
	{
		return lx::can_cast_explicit(static_cast<DataType>(_storage.index()), to);
	}
}
