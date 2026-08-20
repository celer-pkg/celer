import QtQml
import Qt.test

QtObject {
    component IC: MyDeferredObject {

        objectProperty: QtObject {
            objectName: "foo"
        }
    }
}
