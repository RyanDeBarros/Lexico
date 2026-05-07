#include "declarations.h"

#include "basic.h"
#include "pattern.h"

namespace lx
{
	void throw_bad_cast(const DataType& from, const DataType& to)
	{
		std::stringstream ss;
		ss << "bad cast from " << from << " to " << to;
		// TODO throw runtime error - pass script segment
		throw LxError(ErrorType::Internal, ss.str());
	}
}
