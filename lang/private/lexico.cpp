#include "lexico.h"

#include "lexer.h"
#include "parser.h"
#include "semantics.h"
#include "executor.h"

#include <sstream>

namespace lx
{
	// TODO v0.2 serialize token stream + validated AST: will need to switch to ids instead of raw pointers for node relationships

	template<typename T>
	static bool log_messages(const std::vector<T>& messages, std::stringstream& log) requires (std::is_base_of_v<LxStatusMessage, T>)
	{
		if (!messages.empty())
		{
			for (const auto& e : messages)
				log << e.what() << "\n\n";

			return true;
		}
		else
			return false;
	}

	static bool log_errors_and_warnings(const std::vector<LxError>& errors, const std::vector<LxWarning>& warnings, std::stringstream& log)
	{
		bool err = log_messages(errors, log);
		log_messages(warnings, log);
		return err;
	}

	static bool exec(const std::string_view script, const std::string_view input, std::stringstream& output, std::stringstream& log, HighlightMap& highlights)
	{
		// TODO v0.2 delete lexer, parser, analyser, etc. resources once done with them to save resources for executor. Need to be careful, since Token references persist in AST.

		try
		{
			Lexer lexer;
			lexer.tokenize(script);
			if (log_messages(lexer.errors(), log))
				return false;

			Parser parser(lexer.start_token());
			parser.parse(lexer);
			if (log_messages(parser.errors(), log))
				return false;

			SemanticAnalyser analyser;
			analyser.analyse(parser);
			if (log_errors_and_warnings(analyser.errors(), analyser.warnings(), log))
				return false;

			Executor executor(parser.tree().root());
			executor.execute(analyser, input);

			output << executor.output().str();
			log << executor.log().str();
			highlights = std::move(executor.highlights());
			return true;
		}
		catch (const LxError& e)
		{
			log << e.what();
			return false;
		}
	}

	ExecResult execute(const ExecInput& input)
	{
		ExecResult res;
		std::stringstream out;
		std::stringstream lg;
		res.success = exec(input.script, input.input, out, lg, res.highlights);
		res.output = out.str();
		res.log = lg.str();
		return res;
	}
}
