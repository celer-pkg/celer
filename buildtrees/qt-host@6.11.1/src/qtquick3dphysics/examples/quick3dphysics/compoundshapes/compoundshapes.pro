QT += quick quick3d

target.path = $$[QT_INSTALL_EXAMPLES]/quick3dphysics/compoundshapes
INSTALLS += target

SOURCES += \
    main.cpp

RESOURCES += \
    qml.qrc \
    resources.qrc

OTHER_FILES += \
    doc/src/*.*
