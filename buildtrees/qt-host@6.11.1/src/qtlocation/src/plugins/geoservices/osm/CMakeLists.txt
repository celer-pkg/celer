qt_internal_add_plugin(QGeoServiceProviderFactoryOsmPlugin
    OUTPUT_NAME qtgeoservices_osm
    CLASS_NAME QGeoServiceProviderFactoryOsm
    PLUGIN_TYPE geoservices
    SOURCES
        qgeoserviceproviderpluginosm.h qgeoserviceproviderpluginosm.cpp
        qgeotiledmappingmanagerengineosm.h qgeotiledmappingmanagerengineosm.cpp
        qgeotilefetcherosm.h qgeotilefetcherosm.cpp
        qgeomapreplyosm.h qgeomapreplyosm.cpp
        qgeocodingmanagerengineosm.h qgeocodingmanagerengineosm.cpp
        qgeocodereplyosm.h qgeocodereplyosm.cpp
        qgeoroutingmanagerengineosm.h qgeoroutingmanagerengineosm.cpp
        qgeoroutereplyosm.h qgeoroutereplyosm.cpp
        qplacemanagerengineosm.h qplacemanagerengineosm.cpp
        qplacesearchreplyosm.h qplacesearchreplyosm.cpp
        qplacecategoriesreplyosm.h qplacecategoriesreplyosm.cpp
        qgeotiledmaposm.h qgeotiledmaposm.cpp
        qgeofiletilecacheosm.h qgeofiletilecacheosm.cpp
        qgeotileproviderosm.h qgeotileproviderosm.cpp
        qgeorouteparserosrmv4_p.h qgeorouteparserosrmv4.cpp
    LIBRARIES
        Qt::Core
        Qt::Network
        Qt::LocationPrivate
        Qt::PositioningPrivate
)
