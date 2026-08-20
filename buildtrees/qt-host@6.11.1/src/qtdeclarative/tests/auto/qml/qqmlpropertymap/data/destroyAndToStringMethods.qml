import QtQml

QtObject {
    Component.onCompleted: {
        const person = Utils.createObject(this);
        person.groan = "114514";
        objectName = person.toString() + " " + person.groan;
        person.destroy();
    }
}
