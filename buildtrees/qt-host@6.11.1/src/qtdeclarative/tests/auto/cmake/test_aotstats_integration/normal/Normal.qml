import QtQml

QtObject {
    function f(i: int) : int { return i }
    property string s: "hello" + " world"
    function g(x) : int { return x }
}
