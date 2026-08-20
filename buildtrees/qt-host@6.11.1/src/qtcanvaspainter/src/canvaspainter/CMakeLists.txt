# Copyright (C) 2025 The Qt Company Ltd.
# SPDX-License-Identifier: BSD-3-Clause

qt_internal_add_module(CanvasPainter
    SOURCES
        qcanvaspainter.cpp qcanvaspainter.h qcanvaspainter_p.h
        qcanvaslineargradient.cpp qcanvaslineargradient.h
        qcanvasimagepattern.cpp qcanvasimagepattern.h qcanvasimagepattern_p.h
        qcanvasgridpattern.cpp qcanvasgridpattern.h qcanvasgridpattern_p.h
        qcanvasradialgradient.cpp qcanvasradialgradient.h
        qcanvasboxgradient.cpp qcanvasboxgradient.h
        qcanvasboxshadow.cpp qcanvasboxshadow.h qcanvasboxshadow_p.h
        qcanvasimage.cpp qcanvasimage.h qcanvasimage_p.h
        qcanvasdebugcountervisualizer.cpp qcanvasdebugcountervisualizer_p.h
        qcanvasbrush.cpp qcanvasbrush.h qcanvasbrush_p.h
        qcanvasgradient.cpp qcanvasgradient.h qcanvasgradient_p.h
        qcanvasconicalgradient.cpp qcanvasconicalgradient.h
        qcanvascustombrush.cpp qcanvascustombrush.h qcanvascustombrush_p.h
        qcanvaspath.cpp qcanvaspath.h qcanvaspath_p.h
        qcanvasoffscreencanvas.cpp qcanvasoffscreencanvas.h qcanvasoffscreencanvas_p.h
        qcanvaspainterfactory.cpp qcanvaspainterfactory.h qcanvaspainterfactory_p.h
        qcanvasrhipaintdriver.cpp qcanvasrhipaintdriver.h qcanvasrhipaintdriver_p.h

        engine/qcpainterengine.cpp engine/qcpainterengine_p.h engine/qcpainterengineutils_p.h
        engine/qcpainterrhirenderer.cpp engine/qcpainterrhirenderer_p.h
        engine/qcdistancefieldglyphcache.cpp engine/qcdistancefieldglyphcache_p.h
        engine/qcrhidistancefieldglyphcache.cpp engine/qcrhidistancefieldglyphcache_p.h
        engine/qcareaallocator.cpp engine/qcareaallocator_p.h
        engine/qctextlayout.cpp engine/qctextlayout_p.h

        qtcanvaspainterglobal.h qtcanvaspainterglobal_p.h

    DEFINES
        QT_BUILD_CANVASPAINTER_LIB
        # QCPAINTER_EQUAL_POINTS_CHECKING_ENABLED
        # QCPAINTER_FAST_BEVEL_CALCULATIONS
        QCPAINTER_CACHE_GLYPH_RUNS
        QCPAINTER_GLYPH_RUNS_CACHE_SIZE=1024
        # QCPAINTER_DISABLE_TEXT_SUPPORT
        NVG_NO_STB
    LIBRARIES
        Qt::CorePrivate
        Qt::GuiPrivate
    PUBLIC_LIBRARIES
        Qt::Core
        Qt::Gui
    PRIVATE_MODULE_INTERFACE
        Qt::GuiPrivate
    QT_LICENSE_ID QT_COMMERCIAL_OR_GPL3
    ATTRIBUTION_FILE_DIR_PATHS
        engine
)

qt_internal_extend_target(CanvasPainter CONDITION TARGET Qt::Quick
    SOURCES
        qcanvaspainteritem.cpp qcanvaspainteritem.h qcanvaspainteritem_p.h
        qcanvaspainteritemrenderer.cpp qcanvaspainteritemrenderer.h qcanvaspainteritemrenderer_p.h
    LIBRARIES
        Qt::QuickPrivate
    PUBLIC_LIBRARIES
        Qt::Quick
    PRIVATE_MODULE_INTERFACE
        Qt::QuickPrivate
)

qt_internal_extend_target(CanvasPainter CONDITION TARGET Qt::Widgets
    SOURCES
        qcanvaspainterwidget.cpp qcanvaspainterwidget.h qcanvaspainterwidget_p.h
    LIBRARIES
        Qt::WidgetsPrivate
    PUBLIC_LIBRARIES
        Qt::Widgets
    PRIVATE_MODULE_INTERFACE
        Qt::WidgetsPrivate
)

qt_internal_add_docs(CanvasPainter
    doc/qtcanvaspainter.qdocconf
)

qc_internal_add_shaders(CanvasPainter "qcshaders"
    PREFIX
        "/qcshaders"
    BASE
        engine
    OUTPUTS
        qcpainter.vert.qsb
        qcpainter.frag.qsb
    FILES
        engine/qcpainter.vert
        engine/qcpainter.frag
)

qc_internal_add_shaders(CanvasPainter "qcshaders_v_custommatrix"
    PREFIX
        "/qcshaders"
    BASE
        engine
    OUTPUTS
        qcpainter_custommat.vert.qsb
    FILES
        engine/qcpainter.vert
    DEFINES
        CUSTOM_MATRIX
)

qc_internal_add_shaders(CanvasPainter "qcshaders_t"
    PREFIX
        "/qcshaders"
    FILES
        engine/qcpainter.frag
    OUTPUTS
        qcpainter_t.frag.qsb
    DEFINES
        TEXT_A
)

qc_internal_add_shaders(CanvasPainter "qcshaders_aa"
    PREFIX
        "/qcshaders"
    FILES
        engine/qcpainter.frag
    OUTPUTS
        qcpainter_aa.frag.qsb
    DEFINES
        EDGE_AA
)

qc_internal_add_shaders(CanvasPainter "qcshaders_aat"
    PREFIX
        "/qcshaders"
    FILES
        engine/qcpainter.frag
    OUTPUTS
        qcpainter_aat.frag.qsb
    DEFINES
        EDGE_AA
        TEXT_A
)

qc_internal_add_shaders(CanvasPainter "qcshaders_sc"
    PREFIX
        "/qcshaders"
    FILES
        engine/qcpainter.frag
    OUTPUTS
        qcpainter_sc.frag.qsb
    DEFINES
        SCISSORING
)

qc_internal_add_shaders(CanvasPainter "qcshaders_sct"
    PREFIX
        "/qcshaders"
    FILES
        engine/qcpainter.frag
    OUTPUTS
        qcpainter_sct.frag.qsb
    DEFINES
        SCISSORING
        TEXT_A
)

qc_internal_add_shaders(CanvasPainter "qcshaders_aass"
    PREFIX
        "/qcshaders"
    FILES
        engine/qcpainter.frag
    OUTPUTS
        qcpainter_aa_ss.frag.qsb
    DEFINES
        EDGE_AA
        STENCIL_STROKES
)

qc_internal_add_shaders(CanvasPainter "qcshaders_aasst"
    PREFIX
        "/qcshaders"
    FILES
        engine/qcpainter.frag
    OUTPUTS
        qcpainter_aa_sst.frag.qsb
    DEFINES
        EDGE_AA
        STENCIL_STROKES
        TEXT_A
)

qc_internal_add_shaders(CanvasPainter "qcshaders_aasc"
    PREFIX
        "/qcshaders"
    FILES
        engine/qcpainter.frag
    OUTPUTS
        qcpainter_aa_sc.frag.qsb
    DEFINES
        EDGE_AA
        SCISSORING
)

qc_internal_add_shaders(CanvasPainter "qcshaders_aasct"
    PREFIX
        "/qcshaders"
    FILES
        engine/qcpainter.frag
    OUTPUTS
        qcpainter_aa_sct.frag.qsb
    DEFINES
        EDGE_AA
        SCISSORING
        TEXT_A
)

qc_internal_add_shaders(CanvasPainter "qcshaders_aasssc"
    PREFIX
        "/qcshaders"
    FILES
        engine/qcpainter.frag
    OUTPUTS
        qcpainter_aa_ss_sc.frag.qsb
    DEFINES
        EDGE_AA
        SCISSORING
        STENCIL_STROKES
)

qc_internal_add_shaders(CanvasPainter "qcshaders_aasssct"
    PREFIX
        "/qcshaders"
    FILES
        engine/qcpainter.frag
    OUTPUTS
        qcpainter_aa_ss_sct.frag.qsb
    DEFINES
        EDGE_AA
        SCISSORING
        STENCIL_STROKES
        TEXT_A
)
