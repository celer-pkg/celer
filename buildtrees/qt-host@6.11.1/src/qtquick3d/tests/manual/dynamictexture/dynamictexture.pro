QT += quick quick3d

target.path = $$[QT_INSTALL_EXAMPLES]/quick3d/dynamictexture
INSTALLS += target

SOURCES += \
    main.cpp

RESOURCES += \
    qml.qrc \
    content/Panel.qml \
    content/cork.jpg \
    content/note-yellow.png \
    content/tack.png \
    meshes/door1.mesh \
    meshes/door2.mesh \
    meshes/wall.mesh

OTHER_FILES += \
    doc/src/*.*


