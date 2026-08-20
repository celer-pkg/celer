set(plugin_path "../../../../src/plugins/geoservices/nokia")

qt_internal_add_test(tst_nokia_routing
    SOURCES
        tst_routing.cpp
    INCLUDE_DIRECTORIES
        ${plugin_path}
    LIBRARIES
        Qt::Core
        Qt::Network
        Qt::Location
        Qt::Positioning
)

if(NOT QT_BUILD_MINIMAL_STATIC_TESTS)
    qt_internal_extend_target(tst_nokia_routing
        SOURCES
            ${plugin_path}/qgeonetworkaccessmanager.h
    )
endif()
