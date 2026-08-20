SOURCES += main.cpp

include(../common/common.pri)

REPC_SOURCE = ../common/heater.rep

target.path = $$[QT_INSTALL_EXAMPLES]/remoteobjects/ble/bleserver
INSTALLS += target
