# Generated from mappolyline_tester.pro.

#####################################################################
## mappolyline_tester Binary:
#####################################################################

qt_internal_add_manual_test(mappolyline_tester
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
set(qml_resource_files
    "LongPolyline.qml"
    "main.qml"
)

qt_internal_add_resource(mappolyline_tester "qml"
    PREFIX
        "/"
    FILES
        ${qml_resource_files}
)


#### Keys ignored in scope 1:.:.:mappolyline_tester.pro:<TRUE>:
# QML_DESIGNER_IMPORT_PATH = <EMPTY>
# QML_IMPORT_PATH = <EMPTY>

## Scopes:
#####################################################################

#### Keys ignored in scope 2:.:.:mappolyline_tester.pro:QNX:
# target.path = "/tmp/$${TARGET}/bin"

#### Keys ignored in scope 4:.:.:mappolyline_tester.pro:UNIX AND NOT ANDROID:
# target.path = "/opt/$${TARGET}/bin"

#### Keys ignored in scope 5:.:.:mappolyline_tester.pro:NOT target.path_ISEMPTY:
# INSTALLS = "target"
