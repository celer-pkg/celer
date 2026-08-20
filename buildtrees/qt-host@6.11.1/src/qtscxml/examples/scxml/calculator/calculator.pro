QT += qml scxml

CONFIG += c++11
CONFIG += qmltypes

SOURCES += calculator.cpp

HEADERS += calculator-qml.h

QML_IMPORT_NAME = Calculator
QML_IMPORT_MAJOR_VERSION = 1

qml_resources.files = \
    qmldir \
    MainWindow.qml \
    Button.qml

qml_resources.prefix = /qt/qml/Calculator

RESOURCES += qml_resources

STATECHARTS = statemachine.scxml

# install
target.path = $$[QT_INSTALL_EXAMPLES]/scxml/calculator
INSTALLS += target
