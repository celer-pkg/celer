QT += widgets

SOURCES += main.cpp \
    connectpage.cpp \
    mainwindow.cpp \
    heaterview.cpp

HEADERS += \
    connectpage.h \
    mainwindow.h \
    heaterview.h

FORMS += \
    connectpage.ui \
    heaterview.ui

include(../common/common.pri)

REPC_REPLICA = ../common/heater.rep

target.path = $$[QT_INSTALL_EXAMPLES]/remoteobjects/ble/bleclient
INSTALLS += target
