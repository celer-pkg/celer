QT += quick quick3d

target.path = $$[QT_INSTALL_EXAMPLES]/quick3d/runtimeloader
INSTALLS += target

qtHaveModule(widgets)  {
    QT += widgets
    DEFINES += HAS_MODULE_QT_WIDGETS
}

TARGET = runtimeloader

SOURCES += \
        main.cpp

RESOURCES += qml.qrc
