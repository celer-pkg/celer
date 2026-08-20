# Copyright (C) 2022 The Qt Company Ltd.
# SPDX-License-Identifier: BSD-3-Clause

#####################################################################
## tst_scion Test:
#####################################################################

qt_internal_add_test(tst_scion
    SOURCES
        tst_scion.cpp
    DEFINES
        QT_NO_CAST_FROM_ASCII
        QT_NO_CAST_TO_ASCII
    LIBRARIES
        Qt::Gui
        Qt::Qml
        Qt::Scxml
)
#### Ignored generated resource: ${CMAKE_CURRENT_BINARY_DIR}/scion.qrc
#### Keys ignored in scope 1:.:.:scion.pro:<TRUE>:
# ALLFILES = "$$SCXMLS_DIR/*.*,"
# ALLSCXMLS = "$$SCXMLS_DIR/*.scxml,"
# BLACKLISTED = "test216sub1.scxml" "test226sub1.txml" "test239sub1.scxml" "test242sub1.scxml" "test276sub1.scxml" "test530.txml.scxml" "test301.txml.scxml" "test441a.txml.scxml" "test441b.txml.scxml" "test557.txml.scxml"
# QMAKE_EXTRA_COMPILERS = "myscxml" "myscxml_hdr"
# SCXMLS_DIR = "$$absolute_path($$PWD/../../3rdparty/scion-tests/scxml-test-framework/test)"
# TEMPLATE = "app"
# contents = "'<!DOCTYPE" "RCC><RCC" "version=\"1.0\">'" "'<qresource>'" "$$qrc" "'</qresource></RCC>'"
# myscxml.commands = "$$QMAKE_QSCXMLC" "--header" "scxml/${QMAKE_FUNC_nameTheNamespace}_${QMAKE_FILE_IN_BASE}.h" "--impl" "${QMAKE_FILE_OUT}" "--namespace" "${QMAKE_FUNC_nameTheNamespace}" "--classname" "${QMAKE_FUNC_nameTheClass}" "${QMAKE_FILE_IN}"
# myscxml.depends = "$$QMAKE_QSCXMLC_EXE"
# myscxml.input = "SCXMLS"
# myscxml.output = "scxml/${QMAKE_FUNC_nameTheNamespace}_${QMAKE_FILE_IN_BASE}.cpp"
# myscxml.variable_out = "SOURCES"
# myscxml_hdr.commands = "$$escape_expand(\\n)"
# myscxml_hdr.depends = "scxml/${QMAKE_FUNC_nameTheNamespace}_${QMAKE_FILE_IN_BASE}.cpp"
# myscxml_hdr.input = "SCXMLS"
# myscxml_hdr.output = "scxml/${QMAKE_FUNC_nameTheNamespace}_${QMAKE_FILE_IN_BASE}.h"
# myscxml_hdr.variable_out = "SCXML_HEADERS"

## Scopes:
#####################################################################

#### Keys ignored in scope 2:.:.:scion.pro:defineReplace(nameTheNamespace):
# sn = "__relative_path___absolute_path___dirname_1_____OUT_PWD____SCXMLS_DIR_"

#### Keys ignored in scope 3:.:.:scion.pro:defineReplace(nameTheClass):
# cn = "1"

qt_internal_extend_target(tst_scion CONDITION MSVC AND WIN32
    COMPILE_OPTIONS
        /bigobj
)

# For a better explanation about the "blacklisted" tests, see tst_scion.cpp
# <invoke>. The files with sub in their file name are loaded from other
# tests, they are not to be run alone.

set(blacklisted
    test216sub1.scxml
    test226sub1.txml
    test239sub1.scxml
    test242sub1.scxml
    test276sub1.scxml
    # 530: Scxml children element and expr attribute in content, not supported
    # at the moment by the scxml compiler.
    test530.txml.scxml)

# other
list(APPEND blacklisted
    # 301: The scxml is trying to use a nonexistent file as a script souce,
    # script compilation fails.
    test301.txml.scxml
    # 441a and b tests the exmode scxml element attribute that is not
    # supported by the scxml compiler. This attribute was also removed
    # from the latest scxml specification.
    test441a.txml.scxml
    test441b.txml.scxml
    # 557: Compilation fails at parsing books as data element content and loading
    # data content from external file.
    test557.txml.scxml)

set(scxmls_dir ../../3rdparty/scion-tests/scxml-test-framework/test)
get_filename_component(scxmls_dir ${scxmls_dir} ABSOLUTE)
file(GLOB_RECURSE allscxmls ${scxmls_dir}/*.scxml)

_qt_internal_get_tool_wrapper_script_path(tool_wrapper)
set(qscxmlc_bin "${tool_wrapper}" "$<TARGET_FILE:${QT_CMAKE_EXPORT_NAMESPACE}::qscxmlc>")

set(scxmls)
set(inc_list)
set(func_list)
set(test_bases)
foreach(f ${allscxmls})
    get_filename_component(cn ${f} NAME)
    if(NOT cn IN_LIST blacklisted)
        list(APPEND scxmls ${f})

        string(REGEX REPLACE "\\.scxml$" "" cn ${cn})
        set(hn ${cn})
        string(REGEX REPLACE "\\.txml$" "" cn ${cn})
        get_filename_component(sn ${f} DIRECTORY)
        file(RELATIVE_PATH sn ${scxmls_dir} ${sn})
        string(REGEX REPLACE "[^a-zA-Z_0-9]" "_" sn ${sn})

        set(out_h scxml/${sn}_${hn}.h)
        set(out_cpp scxml/${sn}_${hn}.cpp)
        string(APPEND inc_list "#include \"${out_h}\"\n")
        string(APPEND func_list "    []()->QScxmlStateMachine*{return new ${sn}::${cn};},\n")

        file(RELATIVE_PATH tn ${scxmls_dir} ${f})
        string(REGEX REPLACE "\\.scxml$" "" tn ${tn})
        string(APPEND test_bases "    \"${tn}\",\n")

        file(TO_NATIVE_PATH ${out_h} native_out_h)
        file(TO_NATIVE_PATH ${out_cpp} native_out_cpp)
        add_custom_command(
            OUTPUT ${out_cpp} ${out_h}
            COMMAND
                ${qscxmlc_bin} --header ${native_out_h} --impl ${native_out_cpp}
                --namespace ${sn} --classname ${cn} ${f}
            DEPENDS ${QT_CMAKE_EXPORT_NAMESPACE}::qscxmlc
            VERBATIM
        )
        set_source_files_properties(${out_h} ${out_cpp} PROPERTIES SKIP_AUTOMOC TRUE)
        qt_internal_extend_target(tst_scion SOURCES ${out_h} ${out_cpp})
    endif()
endforeach()

file(GLOB_RECURSE allfiles ${scxmls_dir}/*.*)
foreach(f ${allfiles})
    file(RELATIVE_PATH base ${scxmls_dir} ${f})
    set_source_files_properties(${f}
        PROPERTIES
            QT_SKIP_QUICKCOMPILER ON
            QT_RESOURCE_ALIAS ${base}
    )
endforeach()

qt_internal_add_resource(tst_scion "scion"
    PREFIX "/"
    FILES ${allfiles}
)

file(WRITE ${CMAKE_CURRENT_BINARY_DIR}/scxml/compiled_tests.h
    "${inc_list}\nstd::function<QScxmlStateMachine *()> creators[] = {\n${func_list}};")
file(WRITE ${CMAKE_CURRENT_BINARY_DIR}/scxml/scion.h
    "const char *testBases[] = {\n${test_bases}};")
qt_internal_extend_target(tst_scion SOURCES
    ${CMAKE_CURRENT_BINARY_DIR}/scxml/compiled_tests.h
    ${CMAKE_CURRENT_BINARY_DIR}/scxml/scion.h)
