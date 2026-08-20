pragma Strict
import TestTypes
import QtQuick

Item {
    id: root
    property int i: moo.data.value
    property int j: moo.many[1].value
    property string foo: moo.data.foo

    VariantMapLookupFoo {
        id: moo
        data: {
            let result = { value: 42 };
            switch(root.visible) {
                case true:
                    result.foo = "blue";
                    break;
                case false:
                    result.foo = "green";
                    break;
            }
            return result;
        }
    }

    function doI() {
        moo.data.value = i + 1
    }
}
