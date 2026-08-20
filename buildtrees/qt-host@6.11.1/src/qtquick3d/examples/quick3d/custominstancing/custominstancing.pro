QT += quick quick3d

target.path = $$[QT_INSTALL_EXAMPLES]/quick3d/custominstancing
INSTALLS += target

SOURCES += \
    main.cpp \
    cppinstancetable.cpp

HEADERS += \
    cppinstancetable.h

RESOURCES += \
    qml.qrc

OTHER_FILES += \
    doc/src/*.*

CONFIG += qmltypes
QML_IMPORT_NAME = InstancingExample
QML_IMPORT_MAJOR_VERSION = 1
