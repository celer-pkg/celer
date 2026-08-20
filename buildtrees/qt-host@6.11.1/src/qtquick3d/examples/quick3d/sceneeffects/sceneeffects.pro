QT += quick quick3d

target.path = $$[QT_INSTALL_EXAMPLES]/quick3d/sceneeffects

INSTALLS += target

SOURCES += \
    main.cpp

RESOURCES += \
    qml.qrc \
    assets.qrc \
    luts.qrc

OTHER_FILES += \
    doc/src/*.*
