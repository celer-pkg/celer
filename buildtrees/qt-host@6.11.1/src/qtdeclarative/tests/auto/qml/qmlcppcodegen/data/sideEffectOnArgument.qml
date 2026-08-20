import QtQml

QtObject {
    function foo(data: var) : void {
        data.count = 1
    }

    function check(data : var) : void {
        console.info("check", data.count === 1 ? "ok" : "fail")
    }

    function bugTest() : void {
        const data = ({})
        foo(data)
        check(data)
    }

    Component.onCompleted: bugTest()
}

