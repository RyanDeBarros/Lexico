from lang.core.InstructionNode import GlobalNode


class SemanticAnalyser:
	def __init__(self, script_lines: list[str], global_node: GlobalNode):
		self.script_lines = script_lines
		self.global_node = global_node


	def run(self):
		pass  # TODO
