QT += widgets remoteobjects websockets
requires(qtConfig(treeview))

SOURCES += main.cpp

include(../common/common.pri)

target.path = $$[QT_INSTALL_EXAMPLES]/remoteobjects/websockets/wsserver
INSTALLS += target
