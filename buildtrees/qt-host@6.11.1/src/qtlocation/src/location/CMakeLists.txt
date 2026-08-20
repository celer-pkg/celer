qt_internal_add_module(Location
    PLUGIN_TYPES geoservices
    SOURCES
        qlocation.h qlocation.cpp
        qlocationglobal.h qlocationglobal_p.h
        maps/qgeoserviceprovider.h maps/qgeoserviceprovider_p.h maps/qgeoserviceprovider.cpp
        maps/qgeoserviceproviderfactory.h maps/qgeoserviceproviderfactory.cpp
        maps/qcache3q_p.h
        maps/qabstractgeotilecache_p.h maps/qabstractgeotilecache.cpp
        maps/qgeofiletilecache_p.h maps/qgeofiletilecache.cpp
        maps/qgeotilespec_p.h maps/qgeotilespec_p_p.h maps/qgeotilespec.cpp
        maps/qgeotiledmapscene_p.h maps/qgeotiledmapscene_p_p.h maps/qgeotiledmapscene.cpp
        maps/qgeotilerequestmanager_p.h maps/qgeotilerequestmanager.cpp
        maps/qgeotilefetcher_p.h maps/qgeotilefetcher_p_p.h maps/qgeotilefetcher.cpp
        maps/qgeotiledmap_p.h maps/qgeotiledmap_p_p.h maps/qgeotiledmap.cpp
        maps/qgeotiledmapreply_p.h maps/qgeotiledmapreply_p_p.h maps/qgeotiledmapreply.cpp
        maps/qgeotiledmappingmanagerengine_p.h maps/qgeotiledmappingmanagerengine_p_p.h
        maps/qgeotiledmappingmanagerengine.cpp
        maps/qgeocameradata_p.h maps/qgeocameradata.cpp
        maps/qgeocameracapabilities_p.h maps/qgeocameracapabilities.cpp
        maps/qgeocameratiles_p.h maps/qgeocameratiles_p_p.h maps/qgeocameratiles.cpp
        maps/qgeocodingmanagerengine.h maps/qgeocodingmanagerengine_p.h
        maps/qgeocodingmanagerengine.cpp
        maps/qgeocodingmanager.h maps/qgeocodingmanager_p.h maps/qgeocodingmanager.cpp
        maps/qgeocodereply.h maps/qgeocodereply_p.h maps/qgeocodereply.cpp
        maps/qgeoroutingmanager.h maps/qgeoroutingmanager_p.h maps/qgeoroutingmanager.cpp
        maps/qgeoroutingmanagerengine_p.h maps/qgeoroutingmanagerengine.h
        maps/qgeoroutingmanagerengine.cpp
        maps/qgeorouterequest.h maps/qgeorouterequest_p.h maps/qgeorouterequest.cpp
        maps/qgeoroutereply.h maps/qgeoroutereply_p.h maps/qgeoroutereply.cpp
        maps/qgeoroute.h maps/qgeoroute_p.h maps/qgeoroute.cpp
        maps/qgeoroutesegment.h maps/qgeoroutesegment_p.h maps/qgeoroutesegment.cpp
        maps/qgeorouteparser_p.h maps/qgeorouteparser_p_p.h maps/qgeorouteparser.cpp
        maps/qgeorouteparserosrmv5_p.h maps/qgeorouteparserosrmv5.cpp
        maps/qgeomaneuver.h maps/qgeomaneuver_p.h maps/qgeomaneuver.cpp
        maps/qgeomaneuverderived_p.h
        maps/qgeomappingmanager_p.h maps/qgeomappingmanager_p_p.h maps/qgeomappingmanager.cpp
        maps/qgeomappingmanagerengine_p.h maps/qgeomappingmanagerengine_p_p.h
        maps/qgeomappingmanagerengine.cpp
        maps/qgeomaptype_p.h maps/qgeomaptype_p_p.h maps/qgeomaptype.cpp
        maps/qgeomap_p.h maps/qgeomap_p_p.h maps/qgeomap.cpp
        maps/qgeoprojection_p.h maps/qgeoprojection.cpp
        maps/qgeojson_p.h maps/qgeojson.cpp
        places/qplacemanager.h places/qplacemanager.cpp
        places/qplacemanagerengine.h places/qplacemanagerengine_p.h places/qplacemanagerengine.cpp
        places/unsupportedreplies_p.h
        places/qplacereply.h places/qplacereply_p.h places/qplacereply.cpp
        places/qplaceresult.h places/qplaceresult_p.h places/qplaceresult.cpp
        places/qplacesearchrequest.h places/qplacesearchrequest_p.h places/qplacesearchrequest.cpp
        places/qplacesearchreply.h places/qplacesearchreply.cpp
        places/qplacesearchresult.h places/qplacesearchresult_p.h places/qplacesearchresult.cpp
        places/qplaceproposedsearchresult.h places/qplaceproposedsearchresult_p.h
        places/qplaceproposedsearchresult.cpp
        places/qplacesearchsuggestionreply.h places/qplacesearchsuggestionreply.cpp
        places/qplacematchrequest.h places/qplacematchrequest.cpp
        places/qplacematchreply.h places/qplacematchreply.cpp
        places/qplacecontentrequest.h places/qplacecontentrequest_p.h
        places/qplacecontentrequest.cpp
        places/qplacecontentreply.h places/qplacecontentreply.cpp
        places/qplacedetailsreply.h places/qplacedetailsreply.cpp
        places/qplaceidreply.h places/qplaceidreply.cpp
        places/qplace.h places/qplace_p.h places/qplace.cpp
        places/qplaceicon.h places/qplaceicon_p.h places/qplaceicon.cpp
        places/qplacecategory.h places/qplacecategory_p.h places/qplacecategory.cpp
        places/qplaceuser_p.h places/qplaceuser.h places/qplaceuser.cpp
        places/qplacesupplier.h places/qplacesupplier_p.h places/qplacesupplier.cpp
        places/qplaceattribute.h places/qplaceattribute_p.h places/qplaceattribute.cpp
        places/qplaceratings.h places/qplaceratings_p.h places/qplaceratings.cpp
        places/qplacecontactdetail.h places/qplacecontactdetail_p.h places/qplacecontactdetail.cpp
        places/qplacecontent.h places/qplacecontent_p.h places/qplacecontent.cpp
    NO_UNITY_BUILD_SOURCES
        maps/qgeocameratiles.cpp
        maps/qgeotiledmapscene.cpp
    INCLUDE_DIRECTORIES
        maps
        places
        declarativemaps
        declarativeplaces
        quickmapitems
    LIBRARIES
        Qt::CorePrivate
        Qt::Gui
        Qt::QuickPrivate
        Qt::PositioningPrivate
        Qt::PositioningQuickPrivate
        Qt::Qml
        Qt::Quick
        Qt::QuickShapesPrivate
    PUBLIC_LIBRARIES
        Qt::Core
        Qt::Positioning
        Qt::PositioningQuick
    PRIVATE_MODULE_INTERFACE
        Qt::CorePrivate
        Qt::QuickPrivate
        Qt::QuickShapesPrivate
        Qt::PositioningPrivate
        Qt::PositioningQuickPrivate
)

set_source_files_properties(maps/MapView.qml PROPERTIES
     QT_RESOURCE_ALIAS MapView.qml
)

qt_internal_add_qml_module(Location
    URI QtLocation
    VERSION ${PROJECT_VERSION}
    PLUGIN_TARGET declarative_location
    NO_PLUGIN_OPTIONAL
    CLASS_NAME QtLocationDeclarativeModule
    QML_FILES
        maps/MapView.qml
    SOURCES
        # value types
        maps/qgeomaptype_p.h
        maps/qgeomaneuver.h
        maps/qgeoroute.h
        maps/qgeoroutesegment.h
        places/qplaceattribute.h
        places/qplacecontactdetail.h
        places/qplaceicon.h
        places/qplaceratings.h
        places/qplacesupplier.h
        places/qplaceuser.h
        # Element types
        declarativemaps/qdeclarativegeoserviceprovider_p.h
        declarativemaps/qdeclarativegeoserviceprovider.cpp
        declarativemaps/error_messages.cpp declarativemaps/error_messages_p.h
        declarativemaps/qdeclarativegeocodemodel.cpp declarativemaps/qdeclarativegeocodemodel_p.h
        declarativemaps/qdeclarativegeoroutemodel.cpp declarativemaps/qdeclarativegeoroutemodel_p.h
        declarativemaps/qdeclarativegeojsondata.cpp declarativemaps/qdeclarativegeojsondata_p.h
        quickmapitems/qgeomapitemgeometry.cpp quickmapitems/qgeomapitemgeometry_p.h
        quickmapitems/qdeclarativegeomap_p.h quickmapitems/qdeclarativegeomap.cpp
        quickmapitems/qdeclarativegeomapitembase_p.h
        quickmapitems/qdeclarativegeomapitembase.cpp
        quickmapitems/qdeclarativegeomapitemview_p.h
        quickmapitems/qdeclarativegeomapitemview.cpp
        quickmapitems/qdeclarativegeomapitemutils.cpp quickmapitems/qdeclarativegeomapitemutils_p.h
        quickmapitems/qdeclarativegeomapquickitem_p.h
        quickmapitems/qdeclarativegeomapquickitem.cpp
        quickmapitems/qdeclarativegeomapitemgroup_p.h
        quickmapitems/qdeclarativegeomapitemgroup.cpp
        quickmapitems/qdeclarativepolygonmapitem.cpp
        quickmapitems/qdeclarativepolygonmapitem_p.h
        quickmapitems/qdeclarativepolygonmapitem_p_p.h
        quickmapitems/qdeclarativepolylinemapitem.cpp
        quickmapitems/qdeclarativepolylinemapitem_p.h
        quickmapitems/qdeclarativepolylinemapitem_p_p.h
        quickmapitems/qdeclarativerectanglemapitem.cpp
        quickmapitems/qdeclarativerectanglemapitem_p.h
        quickmapitems/qdeclarativerectanglemapitem_p_p.h
        quickmapitems/qdeclarativecirclemapitem.cpp quickmapitems/qdeclarativecirclemapitem_p.h
        quickmapitems/qdeclarativecirclemapitem_p_p.h
        quickmapitems/qdeclarativeroutemapitem.cpp quickmapitems/qdeclarativeroutemapitem_p.h
        quickmapitems/qdeclarativegeomapcopyrightsnotice_p.h
        quickmapitems/qdeclarativegeomapcopyrightsnotice.cpp
        quickmapitems/qdeclarativegeomapitemtransitionmanager_p.h
        quickmapitems/qdeclarativegeomapitemtransitionmanager.cpp
        declarativeplaces/qdeclarativecategory.cpp
        declarativeplaces/qdeclarativecategory_p.h
        declarativeplaces/qdeclarativecontactdetails.cpp
        declarativeplaces/qdeclarativecontactdetails_p.h
        declarativeplaces/qdeclarativeplacecontentmodel.cpp
        declarativeplaces/qdeclarativeplacecontentmodel_p.h
        declarativeplaces/qdeclarativeplace.cpp
        declarativeplaces/qdeclarativeplace_p.h
        declarativeplaces/qdeclarativesearchmodelbase.cpp
        declarativeplaces/qdeclarativesearchmodelbase_p.h
        declarativeplaces/qdeclarativesearchresultmodel.cpp
        declarativeplaces/qdeclarativesearchresultmodel_p.h
        declarativeplaces/qdeclarativesearchsuggestionmodel.cpp
        declarativeplaces/qdeclarativesearchsuggestionmodel_p.h
        declarativeplaces/qdeclarativesupportedcategoriesmodel.cpp
        declarativeplaces/qdeclarativesupportedcategoriesmodel_p.h
    DEPENDENCIES
        QtQuick/auto
        QtPositioning/auto
    NO_GENERATE_CPP_EXPORTS
)

qt_internal_add_docs(Location
    doc/qtlocation.qdocconf
)
