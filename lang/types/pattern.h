#pragma once

#include "declarations.h"

namespace lx
{
	class Pattern
	{
	public:
		static String make_from(const Int& v);
		static String make_from(const Float& v);
		static String make_from(const Bool& v);
		static String make_from(const String& v);
	};
}
