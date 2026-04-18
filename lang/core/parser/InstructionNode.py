from typing import Protocol, Type

from .. import Statement, Token, LxSyntaxError
from ..common import SpecialTokens


def message_pointer(script_text: str, statement: Statement, i: int):
	return statement.tokens[i].pos.message_pointer(script_text, show_cursor=i > 0)


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


class ParsableInstruction(Protocol):
	@staticmethod
	def parse(script_text: str, statement: Statement) -> InstructionNode | LxSyntaxError:
		...


INSTRUCTION_MAP: dict[str, Type[ParsableInstruction]] = {}


def parsable_instruction(name: str):
	def decorator(cls: Type[ParsableInstruction]):
		assert name not in INSTRUCTION_MAP
		INSTRUCTION_MAP[name] = cls
		return cls

	return decorator


class BlockNode(InstructionNode):
	def __init__(self, keyword: Token):
		super().__init__(keyword)
		self.body: list[ASTNode] = []

	def append(self, node: ASTNode):
		self.body.append(node)
		node.parent = self


@parsable_instruction('push')
class PushStackNode(InstructionNode):
	def __init__(self, keyword: Token, frame: Token):
		super().__init__(keyword)
		self.frame = frame

	@staticmethod
	def parse(script_text: str, statement: Statement) -> InstructionNode | LxSyntaxError:
		if len(statement.tokens) == 2:
			return PushStackNode(statement.tokens[0], statement.tokens[1])
		elif len(statement.tokens) < 2:
			return LxSyntaxError("[Parser Error] missing push stack frame:\n" + message_pointer(script_text, statement, 0))
		else:
			return LxSyntaxError("[Parser Error] too many arguments for stack frame:\n" + message_pointer(script_text, statement, 2))


@parsable_instruction('pop')
class PopStackNode(InstructionNode):
	def __init__(self, keyword: Token):
		super().__init__(keyword)

	@staticmethod
	def parse(script_text: str, statement: Statement) -> InstructionNode | LxSyntaxError:
		if len(statement.tokens) == 1:
			return PopStackNode(statement.tokens[0])
		else:
			return LxSyntaxError("[Parser Error] expected no arguments for popping stack frame:\n" + message_pointer(script_text, statement, 1))


@parsable_instruction('fn')
class DefineFunctionNode(BlockNode):
	def __init__(self, keyword: Token, name: Token, args: list[Token]):
		super().__init__(keyword)
		self.name = name
		self.args = args

	@staticmethod
	def parse(script_text: str, statement: Statement) -> InstructionNode | LxSyntaxError:
		if len(statement.tokens) > 1:
			return DefineFunctionNode(statement.tokens[0], statement.tokens[1], statement.tokens[2:])
		else:
			return LxSyntaxError("[Parser Error] no function name in definition:\n" + message_pointer(script_text, statement, 0))


@parsable_instruction('var')
class DefineVariableNode(InstructionNode):
	def __init__(self, keyword: Token, name: Token, value: Token):
		super().__init__(keyword)
		self.name = name
		self.value = value

	@staticmethod
	def parse(script_text: str, statement: Statement) -> InstructionNode | LxSyntaxError:
		if len(statement.tokens) > 1:
			if len(statement.tokens) == 3:
				return DefineVariableNode(statement.tokens[0], statement.tokens[1], statement.tokens[2])
			elif len(statement.tokens) > 3:
				return LxSyntaxError("[Parser Error] too many operands in variable definition:\n" + message_pointer(script_text, statement, 3))
			else:
				return LxSyntaxError("[Parser Error] missing value in variable definition:\n" + message_pointer(script_text, statement, 0))
		else:
			return LxSyntaxError("[Parser Error] missing name in variable definition:\n" + message_pointer(script_text, statement, 0))


@parsable_instruction(SpecialTokens.INVOKE_INSTRUCTION)
class CallFunctionNode(InstructionNode):
	def __init__(self, keyword: Token, function: Token, args: list[Token]):
		super().__init__(keyword)
		self.function = function
		self.args = args

	@staticmethod
	def parse(script_text: str, statement: Statement) -> InstructionNode | LxSyntaxError:
		if len(statement.tokens) > 1:
			return CallFunctionNode(statement.tokens[0], statement.tokens[1], statement.tokens[2:])
		else:
			return LxSyntaxError("[Parser Error] no function name in call instruction:\n" + message_pointer(script_text, statement, 0))


@parsable_instruction('end')
class EndBlockNode(InstructionNode):
	def __init__(self, keyword: Token, block_name: Token):
		super().__init__(keyword)
		self.block_name = block_name

	@staticmethod
	def parse(script_text: str, statement: Statement) -> InstructionNode | LxSyntaxError:
		if len(statement.tokens) == 2:
			return EndBlockNode(statement.tokens[0], statement.tokens[1])
		elif len(statement.tokens) > 2:
			return LxSyntaxError("[Parser Error] too many operands in block end:\n" + message_pointer(script_text, statement, 2))
		else:
			return LxSyntaxError("[Parser Error] missing keyword in block end:\n" + message_pointer(script_text, statement, 0))


def parse_instruction_node(script_text: str, statement: Statement) -> InstructionNode | LxSyntaxError:
	instruction = statement.tokens[0].data
	if instruction in INSTRUCTION_MAP:
		return INSTRUCTION_MAP[instruction].parse(script_text, statement)
	else:
		return LxSyntaxError("[Parser Error] unrecognized instruction:\n" + statement.tokens[0].pos.message_pointer(script_text))
