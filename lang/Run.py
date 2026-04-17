from lang import InputStruct, OutputStruct
from lang.core import Lexer, Parser


def run(inp: InputStruct) -> OutputStruct:
	lexer = Lexer(inp.script_text)
	lexer.run()
	parser = Parser(lexer.statements)
	parser.run()
	return OutputStruct(inp.input_text, True, "success!")  # TODO
