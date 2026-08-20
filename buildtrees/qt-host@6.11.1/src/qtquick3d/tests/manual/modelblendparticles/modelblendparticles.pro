QT += quick quick3d

target.path = $$[QT_INSTALL_EXAMPLES]/quick3d/modelblendparticles
INSTALLS += target

SOURCES += \
    main.cpp

HEADERS += \
    testgeometry.h

RESOURCES += \
    qml.qrc

OTHER_FILES += \
    doc/src/*.*
