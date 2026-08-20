# Copyright (C) 2022 The Qt Company Ltd.
# SPDX-License-Identifier: BSD-3-Clause


#####################################################################
## Scxml Module:
#####################################################################

qt_internal_include_in_repo_target_set(qtscxml)

qt_internal_add_module(Scxml
    QMAKE_MODULE_CONFIG c++11 qscxmlc
    PLUGIN_TYPES scxmldatamodel
    SOURCES
        qscxmlcompiler.cpp qscxmlcompiler.h qscxmlcompiler_p.h
        qscxmlcppdatamodel.cpp qscxmlcppdatamodel.h qscxmlcppdatamodel_p.h
        qscxmldatamodel.cpp qscxmldatamodel.h qscxmldatamodel_p.h
        qscxmlerror.cpp qscxmlerror.h
        qscxmlevent.cpp qscxmlevent.h qscxmlevent_p.h
        qscxmlexecutablecontent.cpp qscxmlexecutablecontent.h qscxmlexecutablecontent_p.h
        qscxmlglobals.h qscxmlglobals_p.h
        qscxmlinvokableservice.cpp qscxmlinvokableservice.h qscxmlinvokableservice_p.h
        qscxmlnulldatamodel.cpp qscxmlnulldatamodel.h
        qscxmlstatemachine.cpp qscxmlstatemachine.h qscxmlstatemachine_p.h
        qscxmlstatemachineinfo.cpp qscxmlstatemachineinfo_p.h
        qscxmltabledata.cpp qscxmltabledata.h qscxmltabledata_p.h
        qscxmldatamodelplugin_p.h qscxmldatamodelplugin.cpp
    DEFINES
        QT_NO_CAST_FROM_ASCII
        QT_NO_CAST_TO_ASCII
    LIBRARIES
        Qt::CorePrivate
        Qt::ScxmlGlobalPrivate
    PUBLIC_LIBRARIES
        Qt::Core
    PRIVATE_MODULE_INTERFACE
        Qt::CorePrivate
        Qt::ScxmlGlobalPrivate
)

# Install the public qscxlmc.prf file that is used by the qmake
set(scxml_mkspecs "${CMAKE_CURRENT_SOURCE_DIR}/../../mkspecs/features/qscxmlc.prf")
set(mkspecs_install_dir "${INSTALL_MKSPECSDIR}")
qt_path_join(mkspecs_install_dir "${QT_INSTALL_DIR}" "${mkspecs_install_dir}" "features")
qt_copy_or_install(FILES "${scxml_mkspecs}" DESTINATION ${mkspecs_install_dir})

#### Keys ignored in scope 3:.:.:scxml.pro:NOT force_independent AND ( NOT debug_and_release OR NOT build_all OR CONFIG(release,debug OR release) ):
# QMAKE_EXTRA_COMPILERS = "prf2build"
# prf2build.CONFIG = "no_link" "no_clean" "target_predeps"
# prf2build.commands = "$$QMAKE_COPY" "${QMAKE_FILE_IN}" "${QMAKE_FILE_OUT}"
# prf2build.input = "FEATURES"
# prf2build.name = "COPY" "${QMAKE_FILE_IN}"
# prf2build.output = "$$[QT_INSTALL_DATA/get]/mkspecs/features/${QMAKE_FILE_BASE}${QMAKE_FILE_EXT}"
qt_internal_add_docs(Scxml
    doc/qtscxml.qdocconf
)

include(Qt6ScxmlMacros.cmake)
