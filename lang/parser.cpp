#include "parser.h"

namespace lx
{
	void Parser::parse(TokenStream& stream)
	{
		stream.seek();

		// TODO
	}

	const AbstractSyntaxTree& Parser::tree() const
	{
		return _tree;
	}

	AbstractSyntaxTree& Parser::tree()
	{
		return _tree;
	}
}
