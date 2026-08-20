qt_internal_add_plugin(QGeoServiceProviderFactoryNokiaPlugin
    OUTPUT_NAME qtgeoservices_nokia
    CLASS_NAME QGeoServiceProviderFactoryNokia
    PLUGIN_TYPE geoservices
    SOURCES
        qgeocodereply_nokia.h qgeocodereply_nokia.cpp
        qgeocodejsonparser.h qgeocodejsonparser.cpp
        qgeocodingmanagerengine_nokia.h qgeocodingmanagerengine_nokia.cpp
        qgeotiledmappingmanagerengine_nokia.h qgeotiledmappingmanagerengine_nokia.cpp
        qgeotilefetcher_nokia.h qgeotilefetcher_nokia.cpp
        qgeomapreply_nokia.h qgeomapreply_nokia.cpp
        qgeoroutereply_nokia.h qgeoroutereply_nokia.cpp
        qgeoroutexmlparser.h qgeoroutexmlparser.cpp
        qgeoroutingmanagerengine_nokia.h qgeoroutingmanagerengine_nokia.cpp
        qgeoserviceproviderplugin_nokia.h qgeoserviceproviderplugin_nokia.cpp
        marclanguagecodes.h qgeonetworkaccessmanager.h
        qgeointrinsicnetworkaccessmanager.h qgeointrinsicnetworkaccessmanager.cpp
        qgeouriprovider.h qgeouriprovider.cpp
        uri_constants.h uri_constants.cpp
        qgeoerror_messages.h qgeoerror_messages.cpp
        qgeomapversion.h qgeomapversion.cpp
        qgeotiledmap_nokia.h qgeotiledmap_nokia.cpp
        qgeofiletilecachenokia.h qgeofiletilecachenokia.cpp
        qplacemanagerengine_nokiav2.h qplacemanagerengine_nokiav2.cpp
        placesv2/qplacecategoriesreplyhere.h placesv2/qplacecategoriesreplyhere.cpp
        placesv2/qplacecontentreplyimpl.h placesv2/qplacecontentreplyimpl.cpp
        placesv2/qplacedetailsreplyimpl.h placesv2/qplacedetailsreplyimpl.cpp
        placesv2/qplacesearchreplyhere.h placesv2/qplacesearchreplyhere.cpp
        placesv2/qplacesearchsuggestionreplyimpl.h placesv2/qplacesearchsuggestionreplyimpl.cpp
        placesv2/jsonparserhelpers.h placesv2/jsonparserhelpers.cpp
    INCLUDE_DIRECTORIES
            ../../../location/maps
    LIBRARIES
        Qt::Core
        Qt::Network
        Qt::LocationPrivate
        Qt::PositioningPrivate
)

qt_add_resources(plugin_resource_files nokia.qrc)

qt_internal_extend_target(QGeoServiceProviderFactoryNokiaPlugin
    SOURCES
        ${plugin_resource_files}
)
