# Copyright (C) 2025 The Qt Company Ltd.
# SPDX-License-Identifier: LicenseRef-Qt-Commercial OR BSD-3-Clause

set(assets_images
    appLogo.svg
    builtWithQt.svg
    dialogBackground.svg
    Bear.png
    Koala.png
    Lion.png
    Monkey.png
    Cat.png
    Deer.png
    Panda.png
    Pig.png
    Sheep.png
    Rabbit.png
    Raccoon.png
    Sloth.png
    Squirrel.png
    Tiger.png
    teddyBear.png
    Bear_BaseColor.png
    Bear_NormalMap.png
    Cat_BaseColor.png
    Cat_NormalMap.png
    Deer_BaseColor.png
    Deer_NormalMap.png
    Koala_BaseColor.png
    Koala_NormalMap.png
    Lion_BaseColor.png
    Lion_NormalMap.png
    Monkey_BaseColor.png
    Monkey_NormalMap.png
    Panda_BaseColor.png
    Panda_NormalMap.png
    Pig_BaseColor.png
    Pig_NormalMap.png
    Rabbit_BaseColor.png
    Rabbit_NormalMap.png
    Raccoon_BaseColor.png
    Raccoon_NormalMap.png
    Sheep_BaseColor.png
    Sheep_NormalMap.png
    Sloth_BaseColor.png
    Sloth_NormalMap.png
    Squirrel_BaseColor.png
    Squirrel_NormalMap.png
    studio_small_01_4k.hdr
    Tiger_BaseColor.png
    Tiger_NormalMap.png
    HeadwearImages/BandanaHatImage1.png
    HeadwearImages/BeanieImage1.png
    HeadwearImages/CapImage1.png
    HeadwearImages/HeadphonesImage1.png
    HeadwearImages/PartyHatImage1.png
    HeadwearImages/WhiskersImage1.png
    HeadwearImages/WizardHatImage1.png
    EyewearImages/EyepatchImage1.png
    EyewearImages/GlassesImage1.png
    EyewearImages/IncognitoImage1.png
    EyewearImages/MonacleImage1.png
    EyewearImages/NightVisionGogglesImage1.png
    EyewearImages/SunglassesImage1.png
    EyesImages/AnnoyedEyesImage1.png
    EyesImages/ConfusedEyesImage1.png
    EyesImages/CuteEyesImage1.png
    EyesImages/PowerPuffEyesImage1.png
    EyesImages/SmallEyesImage1.png
    EyesImages/SurprisedEyesImage1.png
    EyesImages/WideEyesImage1.png
    ItemsImages/AngelWingsImage1.png
    ItemsImages/BackpackImage1.png
    ItemsImages/BowtieImage1.png
    ItemsImages/BracletsImage1.png
    ItemsImages/ButterflyWingsImage1.png
    ItemsImages/NecktieImage1.png
    Generic_ColorAtlas2_BaseColor.png
    Generic_ColorAtlas2_Metallic.png
    Generic_ColorAtlas2_Roughness.png
    crazyMouth1.png
    quteEyes.png
    PowerpuffEye.png
    wowmouth.png
    smallmouth.png
    incognito_color.png
    BdayHat_BaseColor.png
    Wizardhat_BaseColor.png
    AngelWings_BaseColor.png
    Backpack_BaseColor.png
    Butterflywings_BaseColor.png
)

set(assets_icons
    icons/back.svg
    icons/maximize_circle_fill.svg
    icons/headwear.svg
    icons/eyewear.svg
    icons/eyes.svg
    icons/items.svg
    icons/names.svg
    icons/currency.svg
    icons/check.svg
    icons/exit.svg
)

qt_target_qml_sources(${TARGET_NAME}
    PREFIX "/qt/qml/ToyCustomizer/qml/images/"
    RESOURCES ${assets_images}
)

qt_target_qml_sources(${TARGET_NAME}
    PREFIX "/qt/qml/ToyCustomizer/qml/"
    RESOURCES "${assets_icons}"
)

set(clean_images "${assets_icons}" "${assets_images}")
list(TRANSFORM clean_images PREPEND "${CMAKE_CURRENT_SOURCE_DIR}/")
set_property(DIRECTORY APPEND PROPERTY ADDITIONAL_CLEAN_FILES ${clean_images})
