#include "lang.h"

#include "lexer.h"
#include "parser.h"
#include "semantics.h"
#include "executor.h"

#include <sstream>

namespace lx
{
	static bool log_errors(const std::vector<LxError>& errors, std::string& log)
	{
		if (!errors.empty())
		{
			std::stringstream out;
			for (const auto& e : errors)
				out << e.what() << "\n\n";

			log = out.str();
			return true;
		}
		else
			return false;
	}

	bool execute(const std::string_view script, const std::string_view input, std::string& output, std::string& log)
	{
		try
		{
			Lexer lexer;
			lexer.tokenize(script);

			Parser parser;
			parser.parse(lexer);
			if (log_errors(parser.errors(), log))
				return false;

			SemanticAnalyser analyser;
			analyser.analyse(parser);
			if (log_errors(analyser.errors(), log))
				return false;

			Executor executor;
			executor.execute(analyser, input);

			output = executor.output().str();
			log = executor.log().str();
			return true;
		}
		catch (const LxError& e)
		{
			log = e.what();
			return false;
		}
	}
}
