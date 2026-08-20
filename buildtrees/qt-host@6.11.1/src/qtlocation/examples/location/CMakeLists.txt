# Generated from location.pro.

if(TARGET Qt::Quick)
    qt_internal_add_example(places)
    qt_internal_add_example(places_list)
    qt_internal_add_example(places_map)
    qt_internal_add_example(mapviewer)
    qt_internal_add_example(minimal_map)
    qt_internal_add_example(itemview_transitions)
    qt_internal_add_example(planespotter)
endif()
if(TARGET Qt::Quick AND TARGET Qt::Widgets)
    qt_internal_add_example(geojson_viewer)
endif()
