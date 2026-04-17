class ScriptPosition:
	def __init__(self, row: int, col: int):
		self.row = row
		self.col = col


class Token:
	def __init__(self, pos: ScriptPosition, data: str):
		self.pos = pos
		self.data = data


class Statement:
	def __init__(self):
		self.tokens: list[Token] = []
