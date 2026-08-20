TEMPLATE = app

QT += quick qml

SOURCES += main.cpp

QML_IMPORT_NAME = XmlListModel
QML_IMPORT_MAJOR_VERSION = 1

RESOURCES += rssnews.qrc

OTHER_FILES = rssnews.qml \
              content/*.qml \
              content/*.js \
              content/images/*

target.path = $$[QT_INSTALL_EXAMPLES]/demos/rssnews
INSTALLS += target
