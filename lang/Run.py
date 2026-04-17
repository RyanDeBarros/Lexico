from lang import InputStruct, OutputStruct
from lang.core import Lexer, Parser


def run(inp: InputStruct) -> OutputStruct:
	lexer = Lexer(inp.script_text)
	lexer.run()
	parser = Parser(lexer.statements)
	parser.run()

	output_text = ""
	for statement in parser.statements:
		output_text += "Statement:\n"
		for token in statement.tokens:
			output_text += f"\t{token.data} ({token.pos.row}:{token.pos.col})\n"
		output_text += "\n"

	return OutputStruct(output_text, True, "success!")  # TODO
