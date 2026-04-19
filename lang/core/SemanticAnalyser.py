from lang.core import DefineFunctionNode, DefineVariableNode, CallFunctionNode
from lang.core.InstructionNode import GlobalNode


class FunctionSignature:
	def __init__(self, name: str, argv: int):
		self.name = name
		self.argv = argv


class SymbolsTable:
	def __init__(self):
		self.variables: dict[str, DefineVariableNode] = {}
		self.functions: dict[FunctionSignature, DefineFunctionNode] = {}


class Scope:
	def __init__(self):
		self.table = SymbolsTable()
		self.parent: Scope | None = None

	def variable_exists(self, variable: str) -> bool:
		if variable in self.table.variables:
			return True
		elif self.parent is not None:
			return self.parent.variable_exists(variable)
		else:
			return False

	def add_variable(self, var: DefineVariableNode):
		self.table.variables[var.name.data] = var

	def function_exists(self, function: FunctionSignature) -> bool:
		if function in self.table.functions:
			return True
		elif self.parent is not None:
			return self.parent.function_exists(function)
		else:
			return False

	def valid_invocation(self, function: CallFunctionNode):
		return self.function_exists(FunctionSignature(function.name.data, len(function.args)))

	def add_function(self, fun: DefineFunctionNode):
		self.table.functions[FunctionSignature(fun.name.data, len(fun.args))] = fun


class SemanticAnalyser:
	def __init__(self, script_lines: list[str], global_node: GlobalNode):
		self.script_lines = script_lines
		self.global_node = global_node
		self.global_scope = Scope()

	def run(self):
		self._build_tables()
		self._validate_references()

	def _build_tables(self):
		pass  # TODO

	def _validate_references(self):
		pass  # TODO
