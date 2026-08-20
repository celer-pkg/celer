import QtQml

QtObject {
    component A : QtObject {
        property int from
    }

    property QtObject o: A {
        from: {
            let from = 3
            return from
        }
    }
}
