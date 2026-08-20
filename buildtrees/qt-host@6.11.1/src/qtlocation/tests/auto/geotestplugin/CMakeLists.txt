qt_internal_add_plugin(TestGeoServicePlugin
    OUTPUT_NAME qtgeoservices_qmltestplugin
    CLASS_NAME QGeoServiceProviderFactoryTest
    PLUGIN_TYPE geoservices
    TEST_PLUGIN
    DEFAULT_IF FALSE
    SOURCES
        qgeocodingmanagerengine_test.h
        qgeoserviceproviderplugin_test.h
        qgeoroutingmanagerengine_test.h
        qplacemanagerengine_test.h
        qgeotiledmappingmanagerengine_test.h
        qgeotiledmap_test.h
        qgeotilefetcher_test.h
        qgeoserviceproviderplugin_test.cpp
        qgeotiledmap_test.cpp
    LIBRARIES
        Qt::Core
        Qt::LocationPrivate
        Qt::PositioningPrivate
        Qt::Test
)

set(plugin_resource_files
    "geotestplugin.json"
    "place_data.json"
)

qt_internal_add_resource(TestGeoServicePlugin "testdata"
    PREFIX
        "/"
    FILES
        ${plugin_resource_files}
)
