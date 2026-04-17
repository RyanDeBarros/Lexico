class ScriptPosition:
	def __init__(self, line: int, col: int):
		self.line = line
		self.col = col

	def message_pointer(self, script_text: str, tabs: int = 1):
		source = script_text.splitlines()[self.line - 1]
		cursor = f"{' ' * (self.col - 1)}^"
		return f"{'\t' * tabs}{source}\n{'\t' * tabs}{cursor}"


class Token:
	def __init__(self, pos: ScriptPosition, data: str):
		self.pos = pos
		self.data = data


class Statement:
	def __init__(self):
		self.tokens: list[Token] = []
