QT += quick quick3d

target.path = $$[QT_INSTALL_EXAMPLES]/quick3d/principledmaterial
INSTALLS += target

SOURCES += \
    main.cpp \
    imagehelper.cpp

HEADERS += \
    imagehelper.h

RESOURCES += \
    assets.qrc \
    qml.qrc

OTHER_FILES += \
    doc/src/*.*

CONFIG += qmltypes

QML_IMPORT_NAME = Example
QML_IMPORT_MAJOR_VERSION = 1
