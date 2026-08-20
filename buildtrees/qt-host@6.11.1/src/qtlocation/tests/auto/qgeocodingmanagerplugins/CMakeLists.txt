qt_internal_add_plugin(GeocodingTestGeoServicePlugin
    OUTPUT_NAME qtgeoservices_geocodingplugin
    CLASS_NAME QGeoServiceProviderFactoryTest
    PLUGIN_TYPE geoservices
    TEST_PLUGIN
    DEFAULT_IF FALSE
    SOURCES
        qgeocodingmanagerengine_test.h
        qgeoserviceproviderplugin_test.h
        qgeoserviceproviderplugin_test.cpp
    LIBRARIES
        Qt::Core
        Qt::Location
        Qt::Positioning
)
