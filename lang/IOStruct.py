class InputStruct:
	def __init__(self, input_text: str, script_text: str):
		self.input_text = input_text
		self.script_text = script_text  # TODO script variables


class OutputStruct:
	def __init__(self, output_text: str, success: bool, log_text: str):
		self.output_text = output_text
		self.success = success
		self.log_text = log_text
