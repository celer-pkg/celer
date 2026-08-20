import QtQml
import QtQml.Models

QtObject {
    property DelegateModel m: DelegateModel {
        id: delegateModel
        model: ['one', 'two']
        MyButton {
            objectName: modelData
        }
    }
    component FirstComponent : QtObject {
        required property var control
    }
    component SecondComponent : FirstComponent {}
    component MyButton : QtObject {
        id: btn
        property SecondComponent s: SecondComponent {
            id: myContentItem
            control: btn
        }
    }


    component ThirdComponent : QtObject {
        required property int row
        required property var model
    }

    component FourthComponent: ThirdComponent {}

    property DelegateModel n: DelegateModel {
        id: delegateModel2
        model: ['three', 'four']
        FourthComponent {
            objectName: model.modelData
        }
    }


    component MyButton2 : QtObject {
        property SecondComponent s: SecondComponent {
            // forgetting the required property here
        }
    }

    property DelegateModel o: DelegateModel {
        id: delegateModel3
        model: ['five', 'six']
        MyButton2 {
            objectName: modelData
        }
    }
}

