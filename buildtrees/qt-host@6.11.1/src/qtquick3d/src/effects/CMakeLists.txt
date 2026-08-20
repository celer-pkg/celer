# Copyright (C) 2022 The Qt Company Ltd.
# SPDX-License-Identifier: BSD-3-Clause

#####################################################################
## qtquick3deffectplugin Plugin:
#####################################################################

set(qml_files
    "Vignette.qml"
    "TiltShift.qml"
    "SCurveTonemap.qml"
    "Scatter.qml"
    "MotionBlur.qml"
    "HDRBloomTonemap.qml"
    "GaussianBlur.qml"
    "Fxaa.qml"
    "Flip.qml"
    "Emboss.qml"
    "EdgeDetect.qml"
    "DistortionSpiral.qml"
    "DistortionSphere.qml"
    "DistortionRipple.qml"
    "Desaturate.qml"
    "DepthOfFieldHQBlur.qml"
    "ColorMaster.qml"
    "ChromaticAberration.qml"
    "BrushStrokes.qml"
    "Blur.qml"
    "AdditiveColorGradient.qml"
)

qt_internal_add_qml_module(Quick3DEffects
    URI "QtQuick3D.Effects"
    VERSION "${PROJECT_VERSION}"
    DESIGNER_SUPPORTED
    CLASS_NAME QtQuick3DEffectPlugin
    PLUGIN_TARGET qtquick3deffectplugin
    DEPENDENCIES
        QtQuick3D/auto
        QtQuick.Window/auto
    NO_SYNC_QT
    NO_PRIVATE_MODULE
    QML_FILES
        ${qml_files}
    LIBRARIES
        Qt::Qml
        Qt::Quick
        Qt::Quick3DPrivate
)

# Resources:
set(qteffectlibrary_resource_files
    "maps/brushnoise.png"
    "maps/white.png"
    "shaders/additivecolorgradient.frag"
    "shaders/blend.frag"
    "shaders/blur.frag"
    "shaders/blurhorizontal.vert"
    "shaders/blurvertical.vert"
    "shaders/brushstrokes.frag"
    "shaders/chromaticaberration.frag"
    "shaders/colormaster.frag"
    "shaders/combiner.frag"
    "shaders/depthoffieldblur.frag"
    "shaders/depthoffieldblur.vert"
    "shaders/desaturate.frag"
    "shaders/distortion.vert"
    "shaders/distortionripple.frag"
    "shaders/distortionsphere.frag"
    "shaders/distortionspiral.frag"
    "shaders/downsample.frag"
    "shaders/downsample.vert"
    "shaders/downsampletiltshift.frag"
    "shaders/edgedetect.frag"
    "shaders/edgedetect.vert"
    "shaders/emboss.frag"
    "shaders/flip.frag"
    "shaders/fxaaBlur.frag"
    "shaders/fxaaRgbl.frag"
    "shaders/gaussianblur.frag"
    "shaders/luminosity.frag"
    "shaders/luminosity.vert"
    "shaders/motionblurhorizontal.frag"
    "shaders/motionblurhorizontal.vert"
    "shaders/motionblurvertical.frag"
    "shaders/motionblurvertical.vert"
    "shaders/poissonblur.frag"
    "shaders/poissonblur.vert"
    "shaders/poissonblurtiltshift.frag"
    "shaders/poissonblurtiltshift.vert"
    "shaders/scatter.frag"
    "shaders/scurvetonemap.frag"
    "shaders/vignette.frag"
)

qt_internal_add_resource(Quick3DEffects "qteffectlibrary"
    PREFIX
        "/qtquick3deffects"
    FILES
        ${qteffectlibrary_resource_files}
)


#### Keys ignored in scope 2:.:.:plugin.pro:<TRUE>:
# CXX_MODULE = "qml"
# DISTFILES = "qmldir"
# EFFECT_IMAGE_FILES = "maps/brushnoise.png" "maps/white.png"
# OTHER_FILES = "$$QML_FILES"
# QMLTYPES_FILENAME = "plugins.qmltypes"
# QMLTYPES_INSTALL_DIR = "$$[QT_INSTALL_QML]/QtQuick3D/Effects"
# QML_FILES = "Vignette.qml" "TiltShift.qml" "SCurveTonemap.qml" "Scatter.qml" "MotionBlur.qml" "HDRBloomTonemap.qml" "GaussianBlur.qml" "Fxaa.qml" "Flip.qml" "Emboss.qml" "EdgeDetect.qml" "DistortionSpiral.qml" "DistortionSphere.qml" "DistortionRipple.qml" "Desaturate.qml" "DepthOfFieldHQBlur.qml" "ColorMaster.qml" "ChromaticAberration.qml" "BrushStrokes.qml" "Blur.qml" "AdditiveColorGradient.qml" "$$EFFECT_IMAGE_FILES"
# QML_IMPORT_NAME = "QtQuick3D.Effects"
# QML_IMPORT_VERSION = "$$QT_VERSION"
# TARGETPATH = "QtQuick3D/Effects"

## Scopes:
#####################################################################

if(QT_FEATURE_quick_designer AND QT_BUILD_SHARED_LIBS) # special case handle unconverted static
    add_subdirectory(designer)
endif()
