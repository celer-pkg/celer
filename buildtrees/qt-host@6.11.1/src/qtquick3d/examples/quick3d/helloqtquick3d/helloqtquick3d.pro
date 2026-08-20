QT += quick quick3d

target.path = $$[QT_INSTALL_EXAMPLES]/quick3d/helloqtquick3d
INSTALLS += target

SOURCES += \
    main.cpp \
    imageinstancetable.cpp

HEADERS += \
    imageinstancetable.h

RESOURCES += \
    qml.qrc

OTHER_FILES += \
    doc/src/*.*

CONFIG += qmltypes
QML_IMPORT_NAME = HelloExample
QML_IMPORT_MAJOR_VERSION = 1
