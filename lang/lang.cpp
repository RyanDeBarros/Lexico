#include "lang.h"

#include "lexer.h"
#include "parser.h"

namespace lx
{
	bool execute(const std::string_view script, const std::string_view input, std::string& output, std::string& log)
	{
		Lexer lexer;
		lexer.tokenize(script);

		Parser parser;
		parser.parse(lexer.stream());

		output = input;
		log = "success!";
		return true;
	}
}
