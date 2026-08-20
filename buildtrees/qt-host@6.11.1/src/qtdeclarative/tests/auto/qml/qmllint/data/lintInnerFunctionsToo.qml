import QtQml

QtObject {
    function f() {
        () => { a };
        (function() { a });
        function f1() { a }
        function* f2() { a }
        function f3() {
            function f4() {
                a
            }
        }
    }

    property var p1: () => { a }
    property var p2: () => { () => { a } }
    property var p3: function() { a }
    property var p4: function() { function f5() { a } }
}
