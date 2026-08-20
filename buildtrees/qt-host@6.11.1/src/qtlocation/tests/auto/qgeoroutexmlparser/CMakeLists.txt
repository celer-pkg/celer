set(plugin_path "../../../src/plugins/geoservices/nokia")

qt_internal_add_test(tst_qgeoroutexmlparser
    SOURCES
        tst_qgeoroutexmlparser.cpp
    INCLUDE_DIRECTORIES
        ${plugin_path}
    LIBRARIES
        Qt::Core
        Qt::LocationPrivate
)

qt_add_resources(resources_file fixtures.qrc)

qt_internal_extend_target(tst_qgeoroutexmlparser
    SOURCES
        ${resources_file}
)

if(NOT QT_BUILD_MINIMAL_STATIC_TESTS)
    qt_internal_extend_target(tst_qgeoroutexmlparser
        SOURCES
            ${plugin_path}/qgeoroutexmlparser.cpp
            ${plugin_path}/qgeoroutexmlparser.h
    )
endif()
