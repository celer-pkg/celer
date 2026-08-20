QT += quick quick3d

target.path = $$[QT_INSTALL_EXAMPLES]/quick3d/submeshes
INSTALLS += target

SOURCES += \
    main.cpp

RESOURCES += \
    meshes.qrc \
    qml.qrc

OTHER_FILES += \
    doc/src/*.*
