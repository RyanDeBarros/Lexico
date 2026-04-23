#include "lang.h"

#include "lexer.h"
#include "parser.h"

#include <sstream>

namespace lx
{
	bool execute(const std::string_view script, const std::string_view input, std::string& output, std::string& log)
	{
		Lexer lexer;
		lexer.tokenize(script);

		Parser parser;
		parser.parse(lexer.stream());

		std::stringstream out;
		while (!lexer.stream().eof())
		{
			const Token& token = lexer.stream().peek();
			out << (int)token.type << ": " << token.lexeme << " (" << token.start_line << ":" << token.start_column << ")\n";
			lexer.stream().advance();
		}

		output = out.str();
		log = "success!";
		return true;
	}
}
