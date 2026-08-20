import QtQml

QtObject {
    id: root
    property int changes: 0

    component MyCustomType : QtObject {
        property int myPropA: 0
    }

    property list<MyCustomType> testProp: []
    onTestPropChanged: ++changes

    function testF() {
        const l = makeMyCustomTypes()
        console.log("Return value (l):", l.length)
        console.log("root.testProp before assigning l:", root.testProp.length)
        root.testProp = l
        console.log("root.testProp after assigning l:", root.testProp.length)
    }

    function testG() {
        const l = getMyCustomTypes()
        console.log("Return value (l):", l.length)
        console.log("root.testProp before assigning l:", root.testProp.length)
        root.testProp = l
        console.log("root.testProp after assigning l:", root.testProp.length)
    }

    property Component myCustomTypeComp: Component {
        MyCustomType {}
    }

    function makeMyCustomTypes(): list<MyCustomType> {
        return [root.myCustomTypeComp.createObject(root, { "myPropA": 7 }),
                root.myCustomTypeComp.createObject(root, { "myPropA": 8 })]
    }

    function getMyCustomTypes(): list<MyCustomType> {
        return [testProp[0], testProp[1]]
    }
}
