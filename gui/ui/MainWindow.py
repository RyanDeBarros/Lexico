# -*- coding: utf-8 -*-

################################################################################
## Form generated from reading UI file 'MainWindow.ui'
##
## Created by: Qt User Interface Compiler version 6.9.1
##
## WARNING! All changes made in this file will be lost when recompiling UI file!
################################################################################

from PySide6.QtCore import (QCoreApplication, QDate, QDateTime, QLocale,
    QMetaObject, QObject, QPoint, QRect,
    QSize, QTime, QUrl, Qt)
from PySide6.QtGui import (QAction, QBrush, QColor, QConicalGradient,
    QCursor, QFont, QFontDatabase, QGradient,
    QIcon, QImage, QKeySequence, QLinearGradient,
    QPainter, QPalette, QPixmap, QRadialGradient,
    QTransform)
from PySide6.QtWidgets import (QApplication, QDockWidget, QMainWindow, QMenu,
    QMenuBar, QPlainTextEdit, QSizePolicy, QStatusBar,
    QVBoxLayout, QWidget)

class Ui_MainWindow(object):
    def setupUi(self, MainWindow):
        if not MainWindow.objectName():
            MainWindow.setObjectName(u"MainWindow")
        MainWindow.resize(870, 630)
        MainWindow.setDockOptions(QMainWindow.DockOption.AllowNestedDocks|QMainWindow.DockOption.AllowTabbedDocks|QMainWindow.DockOption.AnimatedDocks)
        self.actionOpenInputFile = QAction(MainWindow)
        self.actionOpenInputFile.setObjectName(u"actionOpenInputFile")
        self.actionSaveOutputFile = QAction(MainWindow)
        self.actionSaveOutputFile.setObjectName(u"actionSaveOutputFile")
        self.actionOpenScript = QAction(MainWindow)
        self.actionOpenScript.setObjectName(u"actionOpenScript")
        self.actionSaveScript = QAction(MainWindow)
        self.actionSaveScript.setObjectName(u"actionSaveScript")
        self.actionRunScript = QAction(MainWindow)
        self.actionRunScript.setObjectName(u"actionRunScript")
        self.actionSelect = QAction(MainWindow)
        self.actionSelect.setObjectName(u"actionSelect")
        self.actionNew = QAction(MainWindow)
        self.actionNew.setObjectName(u"actionNew")
        self.actionClose = QAction(MainWindow)
        self.actionClose.setObjectName(u"actionClose")
        self.centralwidget = QWidget(MainWindow)
        self.centralwidget.setObjectName(u"centralwidget")
        self.centralwidget.setEnabled(True)
        self.centralwidget.setMaximumSize(QSize(0, 0))
        MainWindow.setCentralWidget(self.centralwidget)
        self.menubar = QMenuBar(MainWindow)
        self.menubar.setObjectName(u"menubar")
        self.menubar.setGeometry(QRect(0, 0, 870, 33))
        self.menuFile = QMenu(self.menubar)
        self.menuFile.setObjectName(u"menuFile")
        self.menuRun = QMenu(self.menubar)
        self.menuRun.setObjectName(u"menuRun")
        self.menuWindow = QMenu(self.menubar)
        self.menuWindow.setObjectName(u"menuWindow")
        self.menuHelp = QMenu(self.menubar)
        self.menuHelp.setObjectName(u"menuHelp")
        MainWindow.setMenuBar(self.menubar)
        self.statusbar = QStatusBar(MainWindow)
        self.statusbar.setObjectName(u"statusbar")
        MainWindow.setStatusBar(self.statusbar)
        self.dockWidget = QDockWidget(MainWindow)
        self.dockWidget.setObjectName(u"dockWidget")
        self.dockWidget.setDockLocation(Qt.DockWidgetArea.TopDockWidgetArea)
        self.dockWidgetContents = QWidget()
        self.dockWidgetContents.setObjectName(u"dockWidgetContents")
        self.verticalLayout = QVBoxLayout(self.dockWidgetContents)
        self.verticalLayout.setObjectName(u"verticalLayout")
        self.inputText = QPlainTextEdit(self.dockWidgetContents)
        self.inputText.setObjectName(u"inputText")
        self.inputText.setTabStopDistance(20.000000000000000)

        self.verticalLayout.addWidget(self.inputText)

        self.dockWidget.setWidget(self.dockWidgetContents)
        MainWindow.addDockWidget(Qt.DockWidgetArea.TopDockWidgetArea, self.dockWidget)
        self.dockWidget_2 = QDockWidget(MainWindow)
        self.dockWidget_2.setObjectName(u"dockWidget_2")
        self.dockWidget_2.setDockLocation(Qt.DockWidgetArea.BottomDockWidgetArea)
        self.dockWidgetContents_2 = QWidget()
        self.dockWidgetContents_2.setObjectName(u"dockWidgetContents_2")
        self.verticalLayout_6 = QVBoxLayout(self.dockWidgetContents_2)
        self.verticalLayout_6.setObjectName(u"verticalLayout_6")
        self.scriptText = QPlainTextEdit(self.dockWidgetContents_2)
        self.scriptText.setObjectName(u"scriptText")
        self.scriptText.setTabStopDistance(20.000000000000000)

        self.verticalLayout_6.addWidget(self.scriptText)

        self.dockWidget_2.setWidget(self.dockWidgetContents_2)
        MainWindow.addDockWidget(Qt.DockWidgetArea.BottomDockWidgetArea, self.dockWidget_2)
        self.dockWidget_3 = QDockWidget(MainWindow)
        self.dockWidget_3.setObjectName(u"dockWidget_3")
        self.dockWidget_3.setDockLocation(Qt.DockWidgetArea.BottomDockWidgetArea)
        self.dockWidgetContents_3 = QWidget()
        self.dockWidgetContents_3.setObjectName(u"dockWidgetContents_3")
        self.verticalLayout_7 = QVBoxLayout(self.dockWidgetContents_3)
        self.verticalLayout_7.setObjectName(u"verticalLayout_7")
        self.logText = QPlainTextEdit(self.dockWidgetContents_3)
        self.logText.setObjectName(u"logText")
        font = QFont()
        font.setFamilies([u"Courier New"])
        self.logText.setFont(font)
        self.logText.setTabStopDistance(20.000000000000000)

        self.verticalLayout_7.addWidget(self.logText)

        self.dockWidget_3.setWidget(self.dockWidgetContents_3)
        MainWindow.addDockWidget(Qt.DockWidgetArea.BottomDockWidgetArea, self.dockWidget_3)
        self.dockWidget_4 = QDockWidget(MainWindow)
        self.dockWidget_4.setObjectName(u"dockWidget_4")
        self.dockWidget_4.setDockLocation(Qt.DockWidgetArea.TopDockWidgetArea)
        self.dockWidgetContents_4 = QWidget()
        self.dockWidgetContents_4.setObjectName(u"dockWidgetContents_4")
        self.verticalLayout_2 = QVBoxLayout(self.dockWidgetContents_4)
        self.verticalLayout_2.setObjectName(u"verticalLayout_2")
        self.outputText = QPlainTextEdit(self.dockWidgetContents_4)
        self.outputText.setObjectName(u"outputText")
        self.outputText.setReadOnly(True)
        self.outputText.setTabStopDistance(20.000000000000000)

        self.verticalLayout_2.addWidget(self.outputText)

        self.dockWidget_4.setWidget(self.dockWidgetContents_4)
        MainWindow.addDockWidget(Qt.DockWidgetArea.TopDockWidgetArea, self.dockWidget_4)

        self.menubar.addAction(self.menuFile.menuAction())
        self.menubar.addAction(self.menuRun.menuAction())
        self.menubar.addAction(self.menuWindow.menuAction())
        self.menubar.addAction(self.menuHelp.menuAction())
        self.menuFile.addAction(self.actionOpenInputFile)
        self.menuFile.addAction(self.actionSaveOutputFile)
        self.menuFile.addSeparator()
        self.menuFile.addAction(self.actionOpenScript)
        self.menuFile.addAction(self.actionSaveScript)
        self.menuRun.addAction(self.actionRunScript)

        self.retranslateUi(MainWindow)

        QMetaObject.connectSlotsByName(MainWindow)
    # setupUi

    def retranslateUi(self, MainWindow):
        MainWindow.setWindowTitle(QCoreApplication.translate("MainWindow", u"MainWindow", None))
        self.actionOpenInputFile.setText(QCoreApplication.translate("MainWindow", u"Open Input File", None))
#if QT_CONFIG(tooltip)
        self.actionOpenInputFile.setToolTip(QCoreApplication.translate("MainWindow", u"Ctrl+O", None))
#endif // QT_CONFIG(tooltip)
        self.actionSaveOutputFile.setText(QCoreApplication.translate("MainWindow", u"Save Output File", None))
#if QT_CONFIG(tooltip)
        self.actionSaveOutputFile.setToolTip(QCoreApplication.translate("MainWindow", u"Ctrl+S", None))
#endif // QT_CONFIG(tooltip)
        self.actionOpenScript.setText(QCoreApplication.translate("MainWindow", u"Open Script", None))
#if QT_CONFIG(tooltip)
        self.actionOpenScript.setToolTip(QCoreApplication.translate("MainWindow", u"Ctrl+Alt+O", None))
#endif // QT_CONFIG(tooltip)
        self.actionSaveScript.setText(QCoreApplication.translate("MainWindow", u"Save Script", None))
#if QT_CONFIG(tooltip)
        self.actionSaveScript.setToolTip(QCoreApplication.translate("MainWindow", u"Ctrl+Alt+S", None))
#endif // QT_CONFIG(tooltip)
        self.actionRunScript.setText(QCoreApplication.translate("MainWindow", u"Run Script", None))
#if QT_CONFIG(tooltip)
        self.actionRunScript.setToolTip(QCoreApplication.translate("MainWindow", u"F5", None))
#endif // QT_CONFIG(tooltip)
        self.actionSelect.setText(QCoreApplication.translate("MainWindow", u"Select", None))
        self.actionNew.setText(QCoreApplication.translate("MainWindow", u"New", None))
        self.actionClose.setText(QCoreApplication.translate("MainWindow", u"Close", None))
        self.menuFile.setTitle(QCoreApplication.translate("MainWindow", u"File", None))
        self.menuRun.setTitle(QCoreApplication.translate("MainWindow", u"Run", None))
        self.menuWindow.setTitle(QCoreApplication.translate("MainWindow", u"Window", None))
        self.menuHelp.setTitle(QCoreApplication.translate("MainWindow", u"Help", None))
        self.dockWidget.setWindowTitle(QCoreApplication.translate("MainWindow", u"Input", None))
        self.dockWidget_2.setWindowTitle(QCoreApplication.translate("MainWindow", u"Script", None))
        self.dockWidget_3.setWindowTitle(QCoreApplication.translate("MainWindow", u"Log", None))
        self.dockWidget_4.setWindowTitle(QCoreApplication.translate("MainWindow", u"Output", None))
    # retranslateUi

