QT += quick quick3d-private

target.path = $$[QT_INSTALL_EXAMPLES]/quick3d/principledmaterial
INSTALLS += target

SOURCES += \
    main.cpp

RESOURCES += \
    qml.qrc \
    materials.qrc

OTHER_FILES += \
    doc/src/*.*
