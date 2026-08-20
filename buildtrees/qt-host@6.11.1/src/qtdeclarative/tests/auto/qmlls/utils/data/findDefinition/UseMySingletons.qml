import QtQuick
import ModuleWithSingleton

Item {
    property int fromQml: MySingleton.myValue
    property int fromCpp: MySingletonFromCpp.myValue
}
