from lang.core import LxSyntaxError, LxSyntaxErrorList
from . import Statement, Token, ScriptPosition

COMMENT_CHAR = '#'
QUOTE_CHAR = '"'
ESCAPE_CHAR = '\\'
RUNOFF_LINE_TOKEN = '\\'
RUNOFF_WORD_TOKEN = '\\+'


class Lexer:
	def __init__(self, script_text: str):
		self.script_text = script_text
		self.statements: list[Statement] = []

	def run(self):
		e = LxSyntaxErrorList()

		quote_pos: ScriptPosition | None = None
		escaping_quote = False
		line_number = 0
		col_number = 0

		statement = Statement()
		token_data = ""
		token_pos: ScriptPosition | None = None

		def add_token_char(c: str):
			nonlocal token_data, token_pos
			token_data += c
			if token_pos is None:
				token_pos = ScriptPosition(line_number, col_number)

		def add_token():
			nonlocal token_data, token_pos

			if len(token_data) > 0:
				assert token_pos is not None
				token = Token(token_pos, token_data)
				token_data = ""
				token_pos = None
				statement.tokens.append(token)

		def add_statement():
			nonlocal statement

			if quote_pos is None:
				add_token()
				if len(statement.tokens) > 0:
					self.statements.append(statement)
					statement = Statement()
			else:
				add_token_char('\n')

		for line in self.script_text.splitlines():
			line_number += 1
			col_number = 0

			add_statement()

			for c in line:
				col_number += 1

				if quote_pos is not None:
					if c == QUOTE_CHAR:
						if escaping_quote:
							escaping_quote = False
							add_token_char(c)
						else:
							quote_pos = None
							add_token_char(c)
							add_token()
					elif c == ESCAPE_CHAR:
						if escaping_quote:
							add_token_char(ESCAPE_CHAR)
							escaping_quote = False
						else:
							escaping_quote = True
					else:
						if escaping_quote:
							add_token_char(ESCAPE_CHAR)
						add_token_char(c)
				elif c.isspace():
					add_token()
				elif c == COMMENT_CHAR:
					add_token()
					break
				elif c == QUOTE_CHAR:
					if len(token_data) > 0:
						pass  # TODO lexer error: " appears immediately after token -> unless doing string prefixes like f"" or r""
					quote_pos = ScriptPosition(line_number, col_number)
					add_token_char(c)
				else:
					add_token_char(c)

		add_statement()

		if quote_pos is not None:
			pass
		e.errors.append(LxSyntaxError(f"[Lexer Error] Missing closing quote character:\n{quote_pos.message_pointer(self.script_text)}"))

		self._combine_runoff_statements(e)

		if len(e.errors) > 0:
			raise e

	def _combine_runoff_statements(self, e: LxSyntaxErrorList):
		def _runoff_lines(e: LxSyntaxErrorList):
			i = len(self.statements) - 1
			while i > 0:
				current_statement = self.statements[i]
				previous_statement = self.statements[i - 1]
				if previous_statement.tokens[-1].data == RUNOFF_LINE_TOKEN:
					previous_statement.tokens = previous_statement.tokens[:-1] + current_statement.tokens
					self.statements.pop(i)
				i -= 1

		def _runoff_words(e: LxSyntaxErrorList):
			i = len(self.statements) - 1
			while i > 0:
				current_statement = self.statements[i]
				previous_statement = self.statements[i - 1]
				if previous_statement.tokens[-1].data == RUNOFF_WORD_TOKEN:
					if len(previous_statement.tokens) > 1:
						previous_statement.tokens[-2].data += current_statement.tokens[0].data
						previous_statement.tokens = previous_statement.tokens[:-1] + current_statement.tokens[1:]
						self.statements.pop(i)
					else:
						e.errors.append(
							LxSyntaxError(f"[Lexer Error] {RUNOFF_WORD_TOKEN} has no preceding operand to append to:\n{previous_statement.tokens[-1].pos.message_pointer(self.script_text)}"))
				i -= 1

		_runoff_lines(e)
		_runoff_words(e)
