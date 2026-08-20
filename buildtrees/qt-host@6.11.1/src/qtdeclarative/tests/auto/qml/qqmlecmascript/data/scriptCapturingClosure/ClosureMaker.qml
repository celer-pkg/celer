import "myscript.js" as JS
import QtQml

QtObject {
    function getClosure() {
        function inner() {
            return JS.val1 === JS.val2 + 1;
        }
        return inner;
    }
}
