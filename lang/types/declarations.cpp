#include "declarations.h"

#include "basic.h"
#include "pattern.h"

namespace lx
{
	void throw_bad_cast(DataType from, DataType to)
	{
		std::stringstream ss;
		ss << "bad cast from " << from << " to " << to;
		throw LxError(ErrorType::Internal, ss.str());
	}
}
