qt_internal_add_plugin(QGeoServiceProviderFactoryItemsOverlayPlugin
    OUTPUT_NAME qtgeoservices_itemsoverlay
    CLASS_NAME QGeoServiceProviderFactoryItemsOverlay
    PLUGIN_TYPE geoservices
    SOURCES
        qgeomapitemsoverlay.h qgeomapitemsoverlay.cpp
        qgeomappingmanagerengineitemsoverlay.h qgeomappingmanagerengineitemsoverlay.cpp
        qgeoserviceproviderpluginitemsoverlay.h qgeoserviceproviderpluginitemsoverlay.cpp
    LIBRARIES
        Qt::Core
        Qt::LocationPrivate
        Qt::PositioningPrivate
)
