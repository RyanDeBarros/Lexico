from io import StringIO

from lang import InputStruct, OutputStruct
from lang.core import Lexer, Parser, SemanticAnalyser, LxSyntaxErrorList


def run(inp: InputStruct) -> OutputStruct:
	script_lines = inp.script_text.splitlines()

	success = True
	log = StringIO()
	out = StringIO()

	try:
		lexer = Lexer(script_lines)
		lexer.run()

		parser = Parser(script_lines, lexer.statements)
		parser.run()

		analyser = SemanticAnalyser(script_lines, parser.global_node)
		analyser.run()

		for statement in parser.statements:
			out.write("Statement:\n")
			for token in statement.tokens:
				out.write(f"\t{token.data} ({token.pos.line}:{token.pos.col})\n")
			out.write('\n')
	except LxSyntaxErrorList as err:
		success = False
		log.writelines(e.message + '\n' for e in err.errors)

	return OutputStruct(out.getvalue(), success, log.getvalue())
