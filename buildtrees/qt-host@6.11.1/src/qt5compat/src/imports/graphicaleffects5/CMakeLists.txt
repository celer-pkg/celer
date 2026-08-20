# Copyright (C) 2022 The Qt Company Ltd.
# SPDX-License-Identifier: BSD-3-Clause

#####################################################################
## qtgraphicaleffectsplugin Plugin:
#####################################################################

set(qml_files
    "Blend.qml"
    "BrightnessContrast.qml"
    "ColorOverlay.qml"
    "Colorize.qml"
    "ConicalGradient.qml"
    "Desaturate.qml"
    "DirectionalBlur.qml"
    "Displace.qml"
    "DropShadow.qml"
    "FastBlur.qml"
    "GammaAdjust.qml"
    "GaussianBlur.qml"
    "Glow.qml"
    "HueSaturation.qml"
    "InnerShadow.qml"
    "LevelAdjust.qml"
    "LinearGradient.qml"
    "MaskedBlur.qml"
    "OpacityMask.qml"
    "RadialBlur.qml"
    "RadialGradient.qml"
    "RectangularGlow.qml"
    "RecursiveBlur.qml"
    "ThresholdMask.qml"
    "ZoomBlur.qml"
)

qt_internal_add_qml_module(qtgraphicaleffectsplugin
    URI "Qt5Compat.GraphicalEffects"
    VERSION "${PROJECT_VERSION}"
    DESIGNER_SUPPORTED
    CLASS_NAME QtGraphicalEffectsPlugin
    PLUGIN_TARGET qtgraphicaleffectsplugin
    PAST_MAJOR_VERSIONS 1
    DEPENDENCIES
        Qt5Compat.GraphicalEffects.private
        QtQuick.Window
    QML_FILES
        ${qml_files}
    LIBRARIES
        Qt::Qml
        Qt::Quick
)

# Resources:
set(qtgraphicaleffectsshaders_resource_files
    "shaders_ng/brightnesscontrast.frag.qsb"
    "shaders_ng/colorize.frag.qsb"
    "shaders_ng/coloroverlay.frag.qsb"
    "shaders_ng/conicalgradient_mask.frag.qsb"
    "shaders_ng/conicalgradient_nomask.frag.qsb"
    "shaders_ng/desaturate.frag.qsb"
    "shaders_ng/displace.frag.qsb"
    "shaders_ng/fastblur.frag.qsb"
    "shaders_ng/fastblur_internal.frag.qsb"
    "shaders_ng/fastblur_internal.vert.qsb"
    "shaders_ng/fastinnershadow.frag.qsb"
    "shaders_ng/fastinnershadow_level0.frag.qsb"
    "shaders_ng/fastglow.frag.qsb"
    "shaders_ng/gammaadjust.frag.qsb"
    "shaders_ng/gaussianinnershadow.frag.qsb"
    "shaders_ng/gaussianinnershadow_shadow.frag.qsb"
    "shaders_ng/huesaturation.frag.qsb"
    "shaders_ng/leveladjust.frag.qsb"
    "shaders_ng/lineargradient.vert.qsb"
    "shaders_ng/lineargradient_mask.frag.qsb"
    "shaders_ng/lineargradient_nomask.frag.qsb"
    "shaders_ng/opacitymask.frag.qsb"
    "shaders_ng/opacitymask_invert.frag.qsb"
    "shaders_ng/radialgradient.vert.qsb"
    "shaders_ng/radialgradient_mask.frag.qsb"
    "shaders_ng/radialgradient_nomask.frag.qsb"
    "shaders_ng/recursiveblur.vert.qsb"
    "shaders_ng/recursiveblur.frag.qsb"
    "shaders_ng/rectangularglow.frag.qsb"
    "shaders_ng/thresholdmask.frag.qsb"
)

qt_internal_add_resource(qtgraphicaleffectsplugin "qtgraphicaleffectsshaders"
    PREFIX
        "/qt-project.org/imports/Qt5Compat/GraphicalEffects"
    FILES
        ${qtgraphicaleffectsshaders_resource_files}
)
qt_internal_add_docs(qtgraphicaleffectsplugin
    doc/qtgraphicaleffects5compat.qdocconf
)

add_subdirectory(private)
