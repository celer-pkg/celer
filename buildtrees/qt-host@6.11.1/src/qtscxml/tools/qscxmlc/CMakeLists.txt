# Copyright (C) 2022 The Qt Company Ltd.
# SPDX-License-Identifier: BSD-3-Clause

#####################################################################
## qscxmlc Tool:
#####################################################################

qt_internal_include_in_repo_target_set(qtscxml)

qt_get_tool_target_name(target_name qscxmlc)
qt_internal_add_tool(${target_name}
    TOOLS_TARGET Scxml
    INSTALL_DIR "${INSTALL_LIBEXECDIR}"
    SOURCES
        ../../src/scxml/qscxmlcompiler.cpp ../../src/scxml/qscxmlcompiler.h ../../src/scxml/qscxmlcompiler_p.h
        ../../src/scxml/qscxmlerror.cpp ../../src/scxml/qscxmlerror.h
        ../../src/scxml/qscxmlexecutablecontent.cpp ../../src/scxml/qscxmlexecutablecontent.h ../../src/scxml/qscxmlexecutablecontent_p.h
        ../../src/scxml/qscxmlglobals.h
        ../../src/scxml/qscxmltabledata.cpp ../../src/scxml/qscxmltabledata.h
        generator.cpp generator.h
        main.cpp
        moc.cpp moc.h
        outputrevision.h
        qscxmlc.cpp qscxmlc.h
        scxmlcppdumper.cpp scxmlcppdumper.h
        utils.h
    DEFINES
        BUILD_QSCXMLC
        QT_NO_CAST_FROM_ASCII
        QT_NO_CAST_TO_ASCII
    INCLUDE_DIRECTORIES
        $<TARGET_PROPERTY:Qt::Scxml,INTERFACE_INCLUDE_DIRECTORIES>
        $<TARGET_PROPERTY:Qt::ScxmlPrivate,INTERFACE_INCLUDE_DIRECTORIES>
    LIBRARIES
        Qt::CorePrivate
)
qt_internal_return_unless_building_tools()

# qscxmlc uses header and source files from Scxml library instead of linking it. These sources use
# includes from the module(using the module include style) so need to sync the module header
# files first to avoid compiler errors.
add_dependencies(${target_name} Scxml_sync_headers)

set_property(SOURCE ../../src/scxml/qscxmlerror.h PROPERTY SKIP_AUTOMOC ON)
set_property(SOURCE ../../src/scxml/qscxmlcompiler.cpp PROPERTY SKIP_AUTOMOC ON)

# Resources:
set(templates_resource_files
    "cppdatamodel.t"
    "data.t"
    "decl.t"
)

qt_internal_add_resource(${target_name} "templates"
    PREFIX
        "/"
    FILES
        ${templates_resource_files}
    OPTIONS --no-compress
)


#### Keys ignored in scope 1:.:.:qscxmlc.pro:<TRUE>:
# _OPTION = "host_build"
