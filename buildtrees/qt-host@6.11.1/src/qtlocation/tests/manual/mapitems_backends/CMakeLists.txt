# Generated from mapitems_backends.pro.

#####################################################################
## mapitems_backends Binary:
#####################################################################

qt_internal_add_manual_test(mapitems_backends
    GUI
    SOURCES
        main.cpp
    DEFINES
        QT_DEPRECATED_WARNINGS
    LIBRARIES
        Qt::Gui
        Qt::Quick
)

# Resources:
set_source_files_properties("../mappolyline_tester/LongPolyline.qml"
    PROPERTIES QT_RESOURCE_ALIAS "LongPolyline.qml"
)
set(qml_resource_files
    "../mappolyline_tester/LongPolyline.qml"
    "main.qml"
)

qt_internal_add_resource(mapitems_backends "qml"
    PREFIX
        "/"
    FILES
        ${qml_resource_files}
)


#### Keys ignored in scope 1:.:.:mapitems_backends.pro:<TRUE>:
# QML_DESIGNER_IMPORT_PATH = <EMPTY>
# QML_IMPORT_PATH = <EMPTY>

## Scopes:
#####################################################################

#### Keys ignored in scope 2:.:.:mapitems_backends.pro:QNX:
# target.path = "/tmp/$${TARGET}/bin"

#### Keys ignored in scope 4:.:.:mapitems_backends.pro:UNIX AND NOT ANDROID:
# target.path = "/opt/$${TARGET}/bin"

#### Keys ignored in scope 5:.:.:mapitems_backends.pro:NOT target.path_ISEMPTY:
# INSTALLS = "target"
