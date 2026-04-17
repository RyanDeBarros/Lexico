class LxSyntaxError:
	def __init__(self, message: str):
		self.message = message


class LxSyntaxErrorList(RuntimeError):
	def __init__(self, errors: list[LxSyntaxError] | None = None):
		super().__init__()
		self.errors: list[LxSyntaxError] = errors if errors is not None else []
