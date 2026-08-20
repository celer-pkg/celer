# Copyright (C) 2022 The Qt Company Ltd.
# SPDX-License-Identifier: BSD-3-Clause

qt_internal_extend_target(Lottie
    SOURCES
        qbeziereasing.cpp qbeziereasing_p.h
        qlottiebase.cpp qlottiebase_p.h
        qlottiebasictransform.cpp qlottiebasictransform_p.h
        qlottieconstants_p.h
        qlottieellipse.cpp qlottieellipse_p.h
        qlottiefill.cpp qlottiefill_p.h
        qlottiefilleffect.cpp qlottiefilleffect_p.h
        qlottiefreeformshape.cpp qlottiefreeformshape_p.h
        qlottiegfill.cpp qlottiegfill_p.h
        qlottiegroup.cpp qlottiegroup_p.h
        qlottieimage.cpp qlottieimage_p.h
        qlottieflatlayers.cpp qlottieflatlayers_p.h
        qlottielayer.cpp qlottielayer_p.h
        qlottiepolystar.cpp qlottiepolystar_p.h
        qlottieprecomplayer.cpp qlottieprecomplayer_p.h
        qlottieproperty_p.h
        qlottierect.cpp qlottierect_p.h
        qlottierepeater.cpp qlottierepeater_p.h
        qlottierepeatertransform.cpp qlottierepeatertransform_p.h
        qlottieroot.cpp qlottieroot_p.h
        qlottieround.cpp qlottieround_p.h
        qlottieshape.cpp qlottieshape_p.h
        qlottieshapelayer.cpp qlottieshapelayer_p.h
        qlottieshapetransform.cpp qlottieshapetransform_p.h
        qlottiespatialproperty_p.h
        qlottiestroke.cpp qlottiestroke_p.h
        qlottietrimpath.cpp qlottietrimpath_p.h
        renderer/qbatchrenderer.cpp renderer/qbatchrenderer_p.h
        renderer/qlottierenderer.cpp renderer/qlottierenderer_p.h
        renderer/qlottierasterrenderer.cpp renderer/qlottierasterrenderer_p.h
    INCLUDE_DIRECTORIES
        renderer
)
