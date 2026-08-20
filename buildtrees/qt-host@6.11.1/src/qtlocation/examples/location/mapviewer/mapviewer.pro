TARGET = qml_location_mapviewer
TEMPLATE = app

QT += qml network quick positioning location
SOURCES += main.cpp

# Workaround for QTBUG-38735
QT_FOR_CONFIG += location-private

qml_resources.files = \
    qmldir \
    Main.qml \
    helper.js \
    map/MapComponent.qml \
    map/MapSliders.qml \
    map/Marker.qml \
    map/MiniMap.qml \
    menus/ItemPopupMenu.qml \
    menus/MainMenu.qml \
    menus/MapPopupMenu.qml \
    menus/MarkerPopupMenu.qml \
    forms/Geocode.qml \
    forms/GeocodeForm.ui.qml\
    forms/Message.qml \
    forms/MessageForm.ui.qml \
    forms/ReverseGeocode.qml \
    forms/ReverseGeocodeForm.ui.qml \
    forms/RouteCoordinate.qml \
    forms/Locale.qml \
    forms/LocaleForm.ui.qml \
    forms/RouteAddress.qml \
    forms/RouteAddressForm.ui.qml \
    forms/RouteCoordinateForm.ui.qml \
    forms/RouteList.qml \
    forms/RouteListDelegate.qml \
    forms/RouteListHeader.qml \
    resources/marker.png \
    resources/marker_blue.png \
    resources/scale.png \
    resources/scale_end.png

qml_resources.prefix = /qt/qml/MapViewer

RESOURCES = qml_resources

target.path = $$[QT_INSTALL_EXAMPLES]/location/mapviewer
INSTALLS += target
