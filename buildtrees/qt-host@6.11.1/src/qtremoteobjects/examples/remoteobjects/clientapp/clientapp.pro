TEMPLATE = app
TARGET  = clientapp

QT += remoteobjects quick

CONFIG += qmltypes

REPC_REPLICA += $$PWD/../timemodel.rep

QML_IMPORT_NAME = TimeExample
QML_IMPORT_MAJOR_VERSION = 1

SOURCES += main.cpp
HEADERS += plugin.h

qml_resources.files = \
    qmldir \
    Plugins.qml \
    plugins0.qml \
    plugins1.qml \
    plugins2.qml \
    Clock.qml \
    center.png \
    clock.png \
    hour.png \
    minute.png

qml_resources.prefix = /qt/qml/TimeExample

RESOURCES += qml_resources

contains(QT_CONFIG, c++11): CONFIG += c++11

target.path = $$[QT_INSTALL_EXAMPLES]/remoteobjects/clientapp
INSTALLS += target
