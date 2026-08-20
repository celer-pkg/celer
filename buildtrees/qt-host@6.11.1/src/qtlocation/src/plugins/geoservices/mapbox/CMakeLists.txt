qt_internal_add_plugin(QGeoServiceProviderFactoryMapboxPlugin
    OUTPUT_NAME qtgeoservices_mapbox
    CLASS_NAME QGeoServiceProviderFactoryMapbox
    PLUGIN_TYPE geoservices
    SOURCES
        qgeoserviceproviderpluginmapbox.cpp qgeoserviceproviderpluginmapbox.h
        qgeotiledmappingmanagerenginemapbox.cpp qgeotiledmappingmanagerenginemapbox.h
        qgeotilefetchermapbox.cpp qgeotilefetchermapbox.h
        qgeomapreplymapbox.cpp qgeomapreplymapbox.h
        qgeofiletilecachemapbox.cpp qgeofiletilecachemapbox.h
        qgeoroutingmanagerenginemapbox.cpp qgeoroutingmanagerenginemapbox.h
        qgeoroutereplymapbox.cpp qgeoroutereplymapbox.h
        qplacecategoriesreplymapbox.cpp qplacecategoriesreplymapbox.h
        qplacemanagerenginemapbox.cpp qplacemanagerenginemapbox.h
        qplacesearchsuggestionreplymapbox.cpp qplacesearchsuggestionreplymapbox.h
        qplacesearchreplymapbox.cpp qplacesearchreplymapbox.h
        qgeocodingmanagerenginemapbox.cpp qgeocodingmanagerenginemapbox.h
        qgeocodereplymapbox.cpp qgeocodereplymapbox.h
        qmapboxcommon.cpp qmapboxcommon.h
    LIBRARIES
        Qt::Core
        Qt::Network
        Qt::LocationPrivate
        Qt::PositioningPrivate
)

qt_add_resources(plugin_resource_files mapbox.qrc)

qt_internal_extend_target(QGeoServiceProviderFactoryMapboxPlugin
    SOURCES
        ${plugin_resource_files}
)
