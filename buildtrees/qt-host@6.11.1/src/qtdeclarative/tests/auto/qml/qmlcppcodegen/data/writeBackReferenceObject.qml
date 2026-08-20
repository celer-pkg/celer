pragma Strict
import QtQml

RefuseWrite {
    id: self
    function idToId() {
        let v = self.things
        v[1] = 42.25
        self.things = v
    }

    function scopeToScope() {
        let v = things
        v[0] = 0.5
        things = v
    }

    function idToScope() {
        let v = self.things
        v[2] = 3
        self.things = v
    }

    function scopeToId() {
        let v = things
        v[3] = 4
        things = v
    }

    function scopeToUnrelated() {
        let v = things
        v[4] = 5
        a.things = v
    }

    function idToUnrelated() {
        let v = self.things
        v[5] = 6
        a.things = v
    }

    property QtObject a: QtObject {
        id: a
        property list<var> things
    }
}

