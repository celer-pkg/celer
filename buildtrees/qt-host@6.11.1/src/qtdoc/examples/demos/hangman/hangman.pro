QT += quick qml quickcontrols2

INCLUDEPATH += purchasing/qmltypes \
    purchasing/inapp

CONFIG += qmltypes
QML_IMPORT_PATH = $$PWD/hangmangame.h
QML_IMPORT_NAME = Hangman
QML_IMPORT_MAJOR_VERSION = 1

SOURCES += $$PWD/main.cpp \
    $$PWD/hangmangame.cpp

HEADERS += $$PWD/hangmangame.h

RESOURCES += resources.qrc

target.path = $$[QT_INSTALL_EXAMPLES]/demos/hangman
INSTALLS += target

include($$PWD/purchasing/purchasing.pri)
