qt_feature("geoservices_osm" PRIVATE
    LABEL "OpenStreetMap Geoservice"
    PURPOSE "Provides access to OpenStreetMap geoservices"
    CONDITION TRUE
)

qt_feature("geoservices_esri" PRIVATE
    LABEL "ESRI Geoservice"
    PURPOSE "Provides access to ESRI geoservices"
    CONDITION FALSE
)

qt_feature("geoservices_mapbox" PRIVATE
    LABEL "Mapbox Geoservice"
    PURPOSE "Provides access to Mapbox geoservices"
    CONDITION FALSE
)

qt_feature("geoservices_nokia" PRIVATE
    LABEL "Nokia Geoservice"
    PURPOSE "Provides access to Nokia geoservices"
    CONDITION FALSE
)
