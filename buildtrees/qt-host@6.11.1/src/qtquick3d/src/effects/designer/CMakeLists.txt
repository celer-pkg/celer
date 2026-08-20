# Copyright (C) 2022 The Qt Company Ltd.
# SPDX-License-Identifier: BSD-3-Clause

qt_path_join(destination ${QT_INSTALL_DIR} "${INSTALL_QMLDIR}/QtQuick3D/Effects/designer")
qt_copy_or_install(
    FILES
        AdditiveColorGradientSection.qml
        AdditiveColorGradientSpecifics.qml
        BlurSection.qml
        BlurSpecifics.qml
        BrushStrokesSection.qml
        BrushStrokesSpecifics.qml
        ChromaticAberrationSection.qml
        ChromaticAberrationSpecifics.qml
        ColorMasterSection.qml
        ColorMasterSpecifics.qml
        DepthOfFieldHQBlurSection.qml
        DepthOfFieldHQBlurSpecifics.qml
        DesaturateSection.qml
        DesaturateSpecifics.qml
        DistortionRippleSection.qml
        DistortionRippleSpecifics.qml
        DistortionSphereSection.qml
        DistortionSphereSpecifics.qml
        DistortionSpiralSection.qml
        DistortionSpiralSpecifics.qml
        EdgeDetectSection.qml
        EdgeDetectSpecifics.qml
        EmbossSection.qml
        EmbossSpecifics.qml
        FlipSection.qml
        FlipSpecifics.qml
        FxaaSection.qml
        FxaaSpecifics.qml
        GaussianBlurSection.qml
        GaussianBlurSpecifics.qml
        HDRBloomTonemapSection.qml
        HDRBloomTonemapSpecifics.qml
        MotionBlurSection.qml
        MotionBlurSpecifics.qml
        SCurveTonemapSection.qml
        SCurveTonemapSpecifics.qml
        ScatterSection.qml
        ScatterSpecifics.qml
        TiltShiftSection.qml
        TiltShiftSpecifics.qml
        VignetteSection.qml
        VignetteSpecifics.qml
        effectlib.metainfo
    DESTINATION "${destination}"
)

add_subdirectory(images)
