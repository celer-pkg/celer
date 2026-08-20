pragma Strict
import QtQml
import TestTypes

BirthdayParty {
    id: self

    guests: [
        Person { name: "Horst 1" },
        Person { name: "Horst 2" },
        Person { name: "Horst 3" }
    ]

    property list<QtObject> o: self.guests
    property list<string> s: self.guestNames
    property list<var> v: self.stuffs

    component DataSource : QtObject {
        property list<int> numbers: [1, 2]
        property list<QtObject> objects: [
            QtObject { objectName: "a" },
            QtObject { objectName: "b" }
        ]
        property list<Binding> bindings: [
            Binding { objectName: "c" },
            Binding { objectName: "d" }
        ]
    }

    property DataSource src: DataSource {}
    property list<int> numbers: src.numbers
    property list<QtObject> objects: src.objects
    property list<Binding> bindings: src.bindings
    property list<QtObject> objectsFromBindings: src.bindings
    property list<Binding> nulls: src.objects

    function shuffle() {
        numbers[1] = 3;
        objects[1] = self;
        bindings[1] = null;
        objectsFromBindings[1] = self;
        nulls[1] = null;
    }
}
