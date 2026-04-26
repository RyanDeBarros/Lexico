#include "lang.h"

#include "lexer.h"
#include "parser.h"
#include "semantics.h"
#include "executor.h"

#include <sstream>

namespace lx
{
	bool execute(const std::string_view script, const std::string_view input, std::string& output, std::string& log)
	{
		Lexer lexer;
		lexer.tokenize(script);

		Parser parser;
		parser.parse(lexer);

		if (!parser.errors().empty())
		{
			std::stringstream out;
			for (const auto& e : parser.errors())
				out << e.what() << "\n\n";

			log = out.str();
			return false;
		}

		SemanticAnalyser analyser;
		analyser.analyse(parser);

		Executor executor;
		executor.execute(analyser, input);

		output = executor.output().str();
		log = executor.log().str();
		return true;
	}
}
