from curses.ascii import isspace

from . import Statement, Token, ScriptPosition


COMMENT_CHAR = '#'
QUOTE_CHAR = '"'
ESCAPE_CHAR = '\\'
RUNOFF_CHAR = '\\'


class Lexer:
	def __init__(self, script_text: str):
		self.script_text = script_text
		self.statements: list[Statement] = []

	def run(self):
		in_quotes = False
		escaping_quote = False

		statement = Statement()
		token_data = ""
		token_pos: ScriptPosition | None = None

		def add_token():
			nonlocal token_data, token_pos

			if len(token_data) > 0:
				assert token_pos is not None  # TODO don't use asserts
				token = Token(token_pos, token_data)
				token_data = ""
				token_pos = None
				statement.tokens.append(token)

		def add_statement():
			nonlocal statement

			if not in_quotes:
				add_token()
				if len(statement.tokens) > 0:
					self.statements.append(statement)
					statement = Statement()

		line_number = 0
		for line in self.script_text.splitlines():
			line_number += 1
			col_number = 0

			add_statement()

			def add_token_char(c: str):
				nonlocal token_data, token_pos
				token_data += c
				if token_pos is None:
					token_pos = ScriptPosition(line_number, col_number)

			for c in line:
				col_number += 1

				if in_quotes:
					if c == QUOTE_CHAR:
						if escaping_quote:
							escaping_quote = False
							add_token_char(c)
						else:
							in_quotes = False
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
				elif isspace(c):
					add_token()
				elif c == COMMENT_CHAR:
					add_token()
					break
				elif c == QUOTE_CHAR:
					if len(token_data) > 0:
						pass  # TODO lexer error: " appears immediately after token -> unless doing string prefixes like f"" or r""
					in_quotes = True
					# TODO don't add quotes to token, unless doing string prefixes
				else:
					add_token_char(c)

		add_statement()

		if in_quotes:
			pass  # TODO lexer error

		self._combine_runoff_statements()

	def _combine_runoff_statements(self):
		i = len(self.statements) - 1
		while i > 0:
			current_statement = self.statements[i]
			previous_statement = self.statements[i - 1]
			if previous_statement.tokens[-1].data == RUNOFF_CHAR:
				previous_statement.tokens = previous_statement.tokens[:-1] + current_statement.tokens
				self.statements.pop(i)
