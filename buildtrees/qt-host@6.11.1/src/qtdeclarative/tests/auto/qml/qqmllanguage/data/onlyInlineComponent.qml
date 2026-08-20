import QtQml
import People

BirthdayParty {
    component MyGuest : QtObject {
        objectName: "green"
        property string text: "My Text"
    }

    QtObject {
        id: notMyGuest
    }

    MyGuest {
        id: myGuest
    }

    objectName: {
        if (notMyGuest as MyGuest)
            return "no"
        else if (myGuest as MyGuest)
            return "yes"
        else
            return "no"
    }
}
