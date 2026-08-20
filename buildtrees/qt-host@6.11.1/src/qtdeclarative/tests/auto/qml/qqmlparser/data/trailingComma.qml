import QtQml

QtObject {
    property list<QtObject> list1: [
        QtObject {},
        QtObject {},
    ]

    property list<QtObject> list3
    list3: [ QtObject{}, ]
}
