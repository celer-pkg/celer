qt_add_qml_module(onlybytecode_module
    URI OnlyBytecode
    VERSION 1.0
)

set_target_properties(onlybytecode_module
    PROPERTIES
    QT_QMLCACHEGEN_ARGUMENTS "--only-bytecode"
)
