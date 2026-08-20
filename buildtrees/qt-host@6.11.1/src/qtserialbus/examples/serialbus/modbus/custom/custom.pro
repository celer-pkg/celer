QT += serialbus widgets

TARGET = customcommand
TEMPLATE = app
CONFIG += c++11

SOURCES += main.cpp \
    mainwindow.cpp \
    modbusclient.cpp \
    modbusserver.cpp \
    registermodel.cpp

HEADERS += \
     mainwindow.h \
     modbusclient.h \
     modbusserver.h \
     registermodel.h

FORMS += \
    mainwindow.ui

RESOURCES += images.qrc

target.path = $$[QT_INSTALL_EXAMPLES]/serialbus/modbus/custom
INSTALLS += target
