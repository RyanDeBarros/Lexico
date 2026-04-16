from PySide6.QtGui import QKeySequence, Qt
from PySide6.QtWidgets import QMainWindow

import lang
from gui.ui import Ui_MainWindow


class MainWindow(QMainWindow):
	def __init__(self):
		super().__init__()
		self.ui = Ui_MainWindow()
		self.ui.setupUi(self)

		# TODO file actions

		self.ui.actionRunScript.triggered.connect(self.run_script)
		self.ui.actionRunScript.setShortcut(QKeySequence(Qt.Key.Key_F5))
		self.ui.actionRunScript.setShortcutContext(Qt.ShortcutContext.ApplicationShortcut)

	def run_script(self) -> None:
		input_text = self.ui.inputText.toPlainText()
		script_text = self.ui.scriptText.toPlainText()
		output_text = lang.Run.run(input_text, script_text)
		self.ui.outputText.setPlainText(output_text)
