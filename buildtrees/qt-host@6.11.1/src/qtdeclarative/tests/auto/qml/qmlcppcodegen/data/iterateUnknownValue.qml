import QtQml
import TestTypes as PC

QtObject {
    Component.onCompleted: {
        const theList = PC.ListSingleton.get()
        if (theList) {
            for (let entry of theList) {
                console.log(entry)
            }
        }
    }
}
