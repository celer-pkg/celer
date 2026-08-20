QT += qml quick quick3d

CONFIG += qmltypes
QML_IMPORT_NAME = Volume
QML_IMPORT_MAJOR_VERSION = 1

SOURCES += \
    main.cpp \
    volumetexturedata.cpp

HEADERS += \
    volumetexturedata.h

RESOURCES += \
    assets.qrc \
    qml.qrc

OTHER_FILES += \
    doc/src/*.*
