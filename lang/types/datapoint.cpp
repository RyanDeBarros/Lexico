#include "datapoint.h"

namespace lx
{
	bool DataPoint::can_cast_implicit(DataType to) const
	{
		return lx::can_cast_implicit(static_cast<DataType>(_storage.index()), to);
	}

	bool DataPoint::can_cast_explicit(DataType to) const
	{
		return lx::can_cast_explicit(static_cast<DataType>(_storage.index()), to);
	}
}
