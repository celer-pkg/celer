qt_internal_add_plugin(UnsupportedPlacesGeoServicePlugin
    OUTPUT_NAME qtgeoservices_placesplugin_unsupported
    CLASS_NAME QGeoServiceProviderFactoryTest
    PLUGIN_TYPE geoservices
    TEST_PLUGIN
    DEFAULT_IF FALSE
    SOURCES
        qgeoserviceproviderplugin_test.h
        qgeoserviceproviderplugin_test.cpp
    LIBRARIES
        Qt::Core
        Qt::Location
)
