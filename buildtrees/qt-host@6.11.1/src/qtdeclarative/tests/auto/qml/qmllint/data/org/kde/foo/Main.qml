import QtQuick
import org.kde.foo

Item {

    width: t.map["width"]
    height: t.map.height

    x: t.derivedMap["x"]
    y: t.derivedMap.y

    Thing {
        id: t
    }
}
