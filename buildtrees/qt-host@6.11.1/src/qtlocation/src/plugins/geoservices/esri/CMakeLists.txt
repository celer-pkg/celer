qt_internal_add_plugin(GeoServiceProviderFactoryEsriPlugin
    OUTPUT_NAME qtgeoservices_esri
    CLASS_NAME GeoServiceProviderFactoryEsri
    PLUGIN_TYPE geoservices
    SOURCES
        geocodereply_esri.h geocodereply_esri.cpp
        geocodingmanagerengine_esri.h geocodingmanagerengine_esri.cpp
        geomapsource.h geomapsource.cpp
        georoutejsonparser_esri.h georoutejsonparser_esri.cpp
        georoutereply_esri.h georoutereply_esri.cpp
        georoutingmanagerengine_esri.h georoutingmanagerengine_esri.cpp
        geoserviceproviderfactory_esri.h geoserviceproviderfactory_esri.cpp
        geotiledmap_esri.h geotiledmap_esri.cpp
        geotiledmappingmanagerengine_esri.h geotiledmappingmanagerengine_esri.cpp
        geotiledmapreply_esri.h geotiledmapreply_esri.cpp
        geotilefetcher_esri.h geotilefetcher_esri.cpp
        placemanagerengine_esri.h placemanagerengine_esri.cpp
        placesearchreply_esri.h placesearchreply_esri.cpp
        placecategoriesreply_esri.h placecategoriesreply_esri.cpp
    LIBRARIES
        Qt::Core
        Qt::Network
        Qt::LocationPrivate
        Qt::PositioningPrivate
)

qt_add_resources(plugin_resource_files esri.qrc)

qt_internal_extend_target(GeoServiceProviderFactoryEsriPlugin
    SOURCES
        ${plugin_resource_files}
)
