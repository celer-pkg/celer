TEMPLATE = app

QT += quick

SOURCES += main.cpp

RESOURCES += qml.qrc controls_conf.qrc

target.path = $$[QT_INSTALL_EXAMPLES]/demos/alarms
INSTALLS += target
