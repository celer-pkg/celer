QT += serialbus widgets
requires(qtConfig(combobox))

qtConfig(modbus-serialport): QT += serialport

TARGET = modbusclient
TEMPLATE = app
CONFIG += c++11

SOURCES += \
    main.cpp\
    mainwindow.cpp \
    settingsdialog.cpp \
    writeregistermodel.cpp

HEADERS += \
    mainwindow.h \
    settingsdialog.h \
    writeregistermodel.h

FORMS += \
    mainwindow.ui \
    settingsdialog.ui

RESOURCES += \
    client.qrc

target.path = $$[QT_INSTALL_EXAMPLES]/serialbus/modbus/client
INSTALLS += target
