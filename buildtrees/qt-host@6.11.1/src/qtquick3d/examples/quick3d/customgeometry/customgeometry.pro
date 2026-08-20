QT += quick quick3d

CONFIG += qmltypes
QML_IMPORT_NAME = CustomGeometryExample
QML_IMPORT_MAJOR_VERSION = 1

target.path = $$[QT_INSTALL_EXAMPLES]/quick3d/customgeometry
INSTALLS += target

SOURCES += \
    main.cpp \
    examplegeometry.cpp

HEADERS += \
    examplegeometry.h

qml_resources.files = \
    qmldir \
    Main.qml \
    TorusMesh.qml \
    qt_logo_rect.png

qml_resources.prefix = /qt/qml/CustomGeometryExample/

RESOURCES += qml_resources

OTHER_FILES += \
    doc/src/*.*
