import QtQml

QtObject {
    property Person person: Person { name: "horst" }
    objectName: person.getName()
}
