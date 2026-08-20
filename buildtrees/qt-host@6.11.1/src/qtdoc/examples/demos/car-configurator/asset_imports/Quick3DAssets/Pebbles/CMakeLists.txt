qt_add_library(Ast_Pebbles STATIC)

qt_add_qml_module(Ast_Pebbles
    URI "Quick3DAssets.Pebbles"
    CLASS_NAME "PebblesPlugin"
    RESOURCE_PREFIX "/qt/qml"
    QML_FILES
        Pebbles.qml
)
