# Copyright (C) 2022 The Qt Company Ltd.
# SPDX-License-Identifier: BSD-3-Clause

# Generated from graphicaleffects.pro.

#####################################################################
## graphicaleffects Binary:
#####################################################################

qt_internal_add_manual_test(graphicaleffects
    GUI
    SOURCES
        main.cpp
    LIBRARIES
        Qt::Gui
        Qt::Quick
)

# Resources:
set_source_files_properties("../../../src/imports/graphicaleffects5/doc/images/bug.jpg"
    PROPERTIES QT_RESOURCE_ALIAS "images/bug.jpg"
)
set_source_files_properties("../../../src/imports/graphicaleffects5/doc/images/butterfly.png"
    PROPERTIES QT_RESOURCE_ALIAS "images/butterfly.png"
)
set_source_files_properties("../../../src/imports/graphicaleffects5/doc/images/fog.png"
    PROPERTIES QT_RESOURCE_ALIAS "images/fog.png"
)
set_source_files_properties("../../../src/imports/graphicaleffects5/doc/images/glass_normal.png"
    PROPERTIES QT_RESOURCE_ALIAS "images/glass_normal.png"
)
set(qml_resource_files
    "../../../src/imports/graphicaleffects5/doc/images/bug.jpg"
    "../../../src/imports/graphicaleffects5/doc/images/butterfly.png"
    "../../../src/imports/graphicaleffects5/doc/images/fog.png"
    "../../../src/imports/graphicaleffects5/doc/images/glass_normal.png"
    "BlendEffect.qml"
    "BrightnessContrastEffect.qml"
    "ColorOverlayEffect.qml"
    "ColorizeEffect.qml"
    "ConicalGradientEffect.qml"
    "DesaturateEffect.qml"
    "DirectionalBlurEffect.qml"
    "DisplaceEffect.qml"
    "DropShadowEffect.qml"
    "FastBlurEffect.qml"
    "GammaAdjustEffect.qml"
    "GaussianBlurEffect.qml"
    "GlowEffect.qml"
    "HueSaturationEffect.qml"
    "InnerShadowEffect.qml"
    "InnerShadowFastEffect.qml"
    "LevelAdjustEffect.qml"
    "LinearGradientEffect.qml"
    "MaskedBlurEffect.qml"
    "OpacityMaskEffect.qml"
    "RadialBlurEffect.qml"
    "RadialGradientEffect.qml"
    "RectangularGlowEffect.qml"
    "RecursiveBlurEffect.qml"
    "ThresholdMaskEffect.qml"
    "ZoomBlurEffect.qml"
    "main.qml"
)

qt_internal_add_resource(graphicaleffects "qml"
    PREFIX
        "/"
    FILES
        ${qml_resource_files}
)


#### Keys ignored in scope 1:.:.:graphicaleffects.pro:<TRUE>:
# QML_DESIGNER_IMPORT_PATH = <EMPTY>
# QML_IMPORT_PATH = <EMPTY>

## Scopes:
#####################################################################

#### Keys ignored in scope 2:.:.:graphicaleffects.pro:QNX:
# target.path = "/tmp/$${TARGET}/bin"

#### Keys ignored in scope 4:.:.:graphicaleffects.pro:UNIX AND NOT ANDROID:
# target.path = "/opt/$${TARGET}/bin"

#### Keys ignored in scope 5:.:.:graphicaleffects.pro:NOT target.path_ISEMPTY:
# INSTALLS = "target"
