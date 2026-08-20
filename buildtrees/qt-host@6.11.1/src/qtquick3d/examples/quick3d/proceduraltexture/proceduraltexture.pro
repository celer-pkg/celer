QT += quick quick3d

target.path = $$[QT_INSTALL_EXAMPLES]/quick3d/proceduraltexture
INSTALLS += target

SOURCES += \
    gradienttexture.cpp \
    main.cpp

HEADERS += \
    gradienttexture.h

qml_resources.files = \
    qmldir \
    Main.qml

qml_resources.prefix = /qt/qml/ProceduralTextureExample/

RESOURCES += qml_resources

CONFIG += qmltypes
QML_IMPORT_NAME = ProceduralTextureExample
QML_IMPORT_MAJOR_VERSION = 1
