# Generated from mapobjects_tester.pro.

#####################################################################
## mapobjects_tester Binary:
#####################################################################

qt_internal_add_manual_test(mapobjects_tester
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

qt_internal_add_resource(mapobjects_tester "qml"
    PREFIX
        "/"
    FILES
        ${qml_resource_files}
)


#### Keys ignored in scope 1:.:.:mapobjects_tester.pro:<TRUE>:
# QML_DESIGNER_IMPORT_PATH = <EMPTY>
# QML_IMPORT_PATH = <EMPTY>

## Scopes:
#####################################################################

#### Keys ignored in scope 2:.:.:mapobjects_tester.pro:QNX:
# target.path = "/tmp/$${TARGET}/bin"

#### Keys ignored in scope 4:.:.:mapobjects_tester.pro:UNIX AND NOT ANDROID:
# target.path = "/opt/$${TARGET}/bin"

#### Keys ignored in scope 5:.:.:mapobjects_tester.pro:NOT target.path_ISEMPTY:
# INSTALLS = "target"
