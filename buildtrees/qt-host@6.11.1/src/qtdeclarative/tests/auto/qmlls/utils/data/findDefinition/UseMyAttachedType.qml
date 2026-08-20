import ModuleWithAttachedType
import QtQuick

Item {
    MessageBoard.expired: true
    function f() {
        MessageBoard.published()
        return MessageBoard.expired
    }

    MessageBoard.onPublished: {}
}
