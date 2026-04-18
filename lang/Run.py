from io import StringIO

from lang import InputStruct, OutputStruct
from lang.core import Lexer, Parser, LxSyntaxErrorList


def run(inp: InputStruct) -> OutputStruct:
	success = True
	log = StringIO()
	out = StringIO()

	try:
		lexer = Lexer(inp.script_text)
		lexer.run()

		parser = Parser(inp.script_text, lexer.statements)
		parser.run()

		for statement in parser.statements:
			out.write("Statement:\n")
			for token in statement.tokens:
				out.write(f"\t{token.data} ({token.pos.line}:{token.pos.col})\n")
			out.write('\n')
	except LxSyntaxErrorList as err:
		success = False
		log.writelines(e.message + '\n' for e in err.errors)

	return OutputStruct(out.getvalue(), success, log.getvalue())
