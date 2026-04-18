class ScriptPosition:
	def __init__(self, line: int, col: int):
		self.line = line
		self.col = col

	def message_pointer(self, script_lines: list[str], *, show_cursor: bool = True, tabs: int = 1):
		line_number = f"{self.line}. "
		source = line_number + '\t' * tabs + script_lines[self.line - 1]
		if show_cursor:
			cursor = ' ' * len(line_number) + '\t' * tabs + ' ' * (self.col - 1) + '^'
			return f"{source}\n{cursor}"
		else:
			return source


class Token:
	def __init__(self, pos: ScriptPosition, data: str):
		self.pos = pos
		self.data = data


class Statement:
	def __init__(self):
		self.tokens: list[Token] = []
