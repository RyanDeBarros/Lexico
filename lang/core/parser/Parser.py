from lang.core import Statement, parse_instruction_node, LxSyntaxError, LxSyntaxErrorList
from lang.core.parser.InstructionNode import EndBlockNode, GlobalNode, BlockNode


class Parser:
	def __init__(self, script_lines: list[str], statements: list[Statement]):
		self.script_lines = script_lines
		self.statements = statements
		self.global_node = GlobalNode()

	def run(self):
		e = LxSyntaxErrorList()
		current_node = self.global_node
		for statement in self.statements:
			node = parse_instruction_node(self.script_lines, statement)
			if isinstance(node, LxSyntaxError):
				e.errors.append(node)
			elif isinstance(node, BlockNode):
				current_node.append(node)
				current_node = node
			elif isinstance(node, EndBlockNode):
				if isinstance(current_node, GlobalNode):
					e.errors.append(LxSyntaxError("[Parser Error] unexpected block ending - no block currently opened:\n" + node.block_name.pos.message_pointer(self.script_lines)))
					continue

				if current_node.keyword.data != node.block_name.data:
					e.errors.append(LxSyntaxError("[Parser Error] block ending instruction does not match block opening instruction:\n"
												  + node.block_name.pos.message_pointer(self.script_lines) + '\n'
												  + current_node.keyword.pos.message_pointer(self.script_lines)))

				current_node = current_node.parent
			else:
				current_node.append(node)

		while not isinstance(current_node, GlobalNode):
			e.errors.append(LxSyntaxError("[Parser Error] missing block ending instruction for:\n" + current_node.keyword.pos.message_pointer(self.script_lines)))
			current_node = current_node.parent

		if len(e.errors) > 0:
			raise e
