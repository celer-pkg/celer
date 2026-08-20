TEMPLATE = app

QT += qml quick

SOURCES += main.cpp

EXAMPLE_FILES = \
    Main.qml

target.path = $$[QT_INSTALL_EXAMPLES]/quick/flexboxlayout
INSTALLS += target

