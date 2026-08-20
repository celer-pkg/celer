QT += quick quick3d quick3dphysics

target.path = $$[QT_INSTALL_EXAMPLES]/demos/dice
INSTALLS += target

SOURCES += \
    main.cpp

qml.files = \
    Carpet.qml \
    DiceSpawner.qml \
    Dice_low.qml \
    PhysicalDie.qml \
    PhysicalTable.qml \
    RoundTable.qml \
    Scene.qml \
    Main.qml

qml.prefix = /qt/qml/DiceModule

resources.files = \
    Menu_Icon.svg \
    maps/M_Side_Table_Natural_Wenge_Wood_4K_baseColor_small.jpg \
    maps/M_Side_Table_Natural_Wenge_Wood_4K_metallicRoughness_small.png \
    maps/M_Side_Table_Natural_Wenge_Wood_4K_normal_small.png \
    maps/carpet_texture_baseColor.jpeg \
    meshes/cube_001.mesh \
    meshes/object_2.mesh \
    meshes/object_3.mesh \
    meshes/side_Table_Pine_LOD0_M_Side_Table_Natural_Wenge_Wood_4K_0.mesh \
    sounds/onedice.wav \
    sounds/rolling.wav \
    qmldir

resources.prefix = /qt/qml/DiceModule

RESOURCES += qml resources

OTHER_FILES += \
    doc/src/*.*
