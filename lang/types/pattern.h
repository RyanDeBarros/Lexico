#pragma once

#include "declarations.h"

namespace lx
{
	class Pattern
	{
	public:
		static Pattern make_from(const Int& v);
		static Pattern make_from(const Float& v);
		static Pattern make_from(const Bool& v);
		static Pattern make_from(const String& v);
		static Pattern make_from(String&& v);
	};
}
