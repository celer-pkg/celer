TEMPLATE = app
TARGET = quickspeech

QT += quick texttospeech

SOURCES += main.cpp

qml_resources.files = \
    qmldir \
    Main.qml

qml_resources.prefix = /qt/qml/main

RESOURCES += qml_resources

target.path = $$[QT_INSTALL_EXAMPLES]/speech/quickspeech
INSTALLS += target

DISTFILES += \
    qmldir
