qt_internal_add_plugin(RoutingTestGeoServicePlugin
    OUTPUT_NAME qtgeoservices_routingplugin
    CLASS_NAME QGeoServiceProviderFactoryTest
    PLUGIN_TYPE geoservices
    TEST_PLUGIN
    DEFAULT_IF FALSE
    SOURCES
        qgeoroutingmanagerengine_test.h
        qgeoserviceproviderplugin_test.h
        qgeoserviceproviderplugin_test.cpp
    LIBRARIES
        Qt::Core
        Qt::Location
        Qt::Positioning
)
