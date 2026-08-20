TEMPLATE = app

QT += qml scxml
CONFIG += c++11
CONFIG += qmltypes

SOURCES += invoke.cpp
HEADERS += invoke-qml.h

QML_IMPORT_NAME = InvokeExample
QML_IMPORT_MAJOR_VERSION = 1

qml_resources.files = \
    qmldir \
    Button.qml \
    MainView.qml \
    SubView.qml

qml_resources.prefix = /qt/qml/InvokeExample

RESOURCES += qml_resources

STATECHARTS = statemachine.scxml

target.path = $$[QT_INSTALL_EXAMPLES]/scxml/invoke
INSTALLS += target

