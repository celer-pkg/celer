QT += quick quick3d

target.path = $$[QT_INSTALL_EXAMPLES]/quick3d/skinning
INSTALLS += target

CONFIG += qmltypes
QML_IMPORT_NAME = Example
QML_IMPORT_MAJOR_VERSION = 1

SOURCES += \
    main.cpp \
    skingeometry.cpp

RESOURCES += \
    qml.qrc

OTHER_FILES += \
    main.qml

HEADERS += \
    skingeometry.h

OTHER_FILES += \
    doc/src/*.*
