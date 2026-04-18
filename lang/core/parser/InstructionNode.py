from .. import Statement, Token, LxSyntaxError


class ASTNode:
	def __init__(self):
		self.parent: BlockNode | GlobalNode | None = None


class GlobalNode(ASTNode):
	def __init__(self):
		super().__init__()
		self.body: list[ASTNode] = []

	def append(self, node: ASTNode):
		self.body.append(node)
		node.parent = self


class InstructionNode(ASTNode):
	def __init__(self, keyword: Token):
		super().__init__()
		self.keyword = keyword


class BlockNode(InstructionNode):
	def __init__(self, keyword: Token):
		super().__init__(keyword)
		self.body: list[ASTNode] = []

	def append(self, node: ASTNode):
		self.body.append(node)
		node.parent = self


class PushStackNode(InstructionNode):
	def __init__(self, keyword: Token, frame: Token):
		super().__init__(keyword)
		self.frame = frame


class PopStackNode(InstructionNode):
	def __init__(self, keyword: Token):
		super().__init__(keyword)


class DefineFunctionNode(BlockNode):
	def __init__(self, keyword: Token, name: Token, args: list[Token]):
		super().__init__(keyword)
		self.name = name
		self.args = args


class DefineVariableNode(InstructionNode):
	def __init__(self, keyword: Token, name: Token, value: Token):
		super().__init__(keyword)
		self.name = name
		self.value = value


class CallFunctionNode(InstructionNode):
	def __init__(self, keyword: Token, function: Token, args: list[Token]):
		super().__init__(keyword)
		self.function = function
		self.args = args


class EndBlockNode(InstructionNode):
	def __init__(self, keyword: Token, block_name: Token):
		super().__init__(keyword)
		self.block_name = block_name


def parse_instruction_node(script_text: str, statement: Statement) -> InstructionNode | LxSyntaxError:
	instruction = statement.tokens[0]
	if instruction.data.startswith('!'):
		return CallFunctionNode(instruction, statement.tokens[0], statement.tokens[1:])
	else:
		def mp(i: int):
			return statement.tokens[i].pos.message_pointer(script_text, show_cursor=i > 0)

		match instruction.data:
			case "push":
				if len(statement.tokens) == 2:
					return PushStackNode(instruction, statement.tokens[1])
				elif len(statement.tokens) < 2:
					return LxSyntaxError("[Parser Error] missing push stack frame:\n" + mp(0))
				else:
					return LxSyntaxError("[Parser Error] too many arguments for stack frame:\n" + mp(2))

			case "pop":
				if len(statement.tokens) == 1:
					return PopStackNode(instruction)
				else:
					return LxSyntaxError("[Parser Error] expected no arguments for popping stack frame:\n" + mp(1))

			case "fn":
				if len(statement.tokens) > 1:
					return DefineFunctionNode(instruction, statement.tokens[1], statement.tokens[2:])
				else:
					return LxSyntaxError("[Parser Error] no function name in definition:\n" + mp(0))

			case "var":
				if len(statement.tokens) > 1:
					if len(statement.tokens) == 3:
						return DefineVariableNode(instruction, statement.tokens[1], statement.tokens[2])
					elif len(statement.tokens) > 3:
						return LxSyntaxError("[Parser Error] too many operands in variable definition:\n" + mp(3))
					else:
						return LxSyntaxError("[Parser Error] missing value in variable definition:\n" + mp(0))
				else:
					return LxSyntaxError("[Parser Error] missing name in variable definition:\n" + mp(0))

			case "end":
				if len(statement.tokens) == 2:
					return EndBlockNode(instruction, statement.tokens[1])
				elif len(statement.tokens) > 2:
					return LxSyntaxError("[Parser Error] too many operands in block end:\n" + mp(2))
				else:
					return LxSyntaxError("[Parser Error] missing keyword in block end:\n" + mp(0))

			case _:
				return LxSyntaxError("[Parser Error] unrecognized instruction:\n" + statement.tokens[0].pos.message_pointer(script_text))
