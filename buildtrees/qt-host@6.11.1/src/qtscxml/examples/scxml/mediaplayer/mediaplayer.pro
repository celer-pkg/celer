TEMPLATE = app

QT += qml scxml
CONFIG += c++11
CONFIG += qmltypes

SOURCES += main.cpp \
    thedatamodel.cpp

HEADERS += thedatamodel.h \
           mediaplayer-qml.h

QML_IMPORT_NAME = Mediaplayer
QML_IMPORT_MAJOR_VERSION = 1

qml_resources.files = \
    qmldir \
    MainWindow.qml

qml_resources.prefix = /qt/qml/Mediaplayer

RESOURCES += qml_resources

STATECHARTS = mediaplayer.scxml

target.path = $$[QT_INSTALL_EXAMPLES]/scxml/mediaplayer
INSTALLS += target
