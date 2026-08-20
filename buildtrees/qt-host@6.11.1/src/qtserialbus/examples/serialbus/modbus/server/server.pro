QT += serialbus widgets
requires(qtConfig(combobox))

qtConfig(modbus-serialport): QT += serialport

TARGET = modbusserver
TEMPLATE = app
CONFIG += c++11

SOURCES += \
    main.cpp \
    mainwindow.cpp \
    settingsdialog.cpp

HEADERS += \
    mainwindow.h \
    settingsdialog.h

FORMS += \
    mainwindow.ui \
    settingsdialog.ui

RESOURCES += server.qrc

target.path = $$[QT_INSTALL_EXAMPLES]/serialbus/modbus/server
INSTALLS += target
