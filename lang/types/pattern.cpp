#include "pattern.h"

#include "basic.h"

namespace lx
{
	Pattern Pattern::make_from(const Int& v)
	{
		return Pattern::make_from(String::make_from(v));
	}

	Pattern make_from(const Float& v)
	{
		return Pattern::make_from(String::make_from(v));
	}

	Pattern make_from(const Bool& v)
	{
		return Pattern::make_from(String::make_from(v));
	}

	Pattern make_from(const String& v)
	{
		// TODO
		return Pattern();
	}
	
	Pattern make_from(String&& v)
	{
		// TODO
		return Pattern();
	}
}
