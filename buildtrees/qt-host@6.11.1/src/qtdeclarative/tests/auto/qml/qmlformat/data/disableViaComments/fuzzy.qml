// Copyright (C) 2025 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR GPL-3.0-only

    // qmlformat off
                                        import QtQuick 2.15
    // qmlformat on

// indent 0
import QtQuick.Controls 2.15

    // qmlformat off
                                        Item {
                                            // Properties
                                            property int foo: 42
    // qmlformat on

    // indent 4
    property string bar: "baz"

    // Signals, indent 4
    // qmlformat off
                                        signal mySignal(int value)
    // qmlformat on

    // indent 4
    signal anotherSignal(string text)

    // Enums, indent 4
        enum MyEnum {
    // qmlformat off
                                        Value1, 
    // qmlformat on
    // Value2, indent 8
             Value2 }

    // inden 4
    enum AnotherEnum { ValueA, ValueB }

    // Components
    // qmlformat off
                                        Component {
                                            Rectangle {
                                                width: 100
                                                height: 100
                                                // qmlformat off
                                                        color: "red"
    // qmlformat on
        }
    }
    // indent 4
    // qmlformat on

    // indent 4
    Component {
        Rectangle {
            width: 50
            height: 50
        }
    }

    // Methods (functions), indent 4
    // qmlformat off
                                            function myFunction(a, b) {
                                                var x = a + b;
                                                return x;
                                            }
    // qmlformat on

    // indent 4
    function anotherFunction(x) {
        return x * 2;
    }

    // Advanced JS: destructuring, arrow functions, default params
    function destructureExample() {
    // qmlformat off
                                            let {c, d} = rest;
                                            let [first, ...remaining] = others;
    // qmlformat on
        // indent 8
                return a + b + c + d + first;
    }

    // indent 4
    function arrowFuncExample(x = 1, y = 2) {
        // indent 8
        const add = (a, b) => a + b;
        // qmlformat off
                                            const obj = {
        // qmlformat on
        // indent 12
                                                method: (p, q) => {
                                                        return add(p, q);
                                                }
        };
        // indent 8
        return obj.method(x, y);
    }
    // qmlformat on

    // JS signal handlers
    // qmlformat off
                                            onMySignal: {
                                                var x = 1;
                                                // Array destructuring with default
                                                let [a = 10, b = 20] = [1];
                                                // Object destructuring with renaming
                                                let {foo: renamedFoo, bar: renamedBar = "default"} = {foo: 5};
    // qmlformat on
    // indent 8
                var y = 2;
    // qmlformat off
                                                    var z = 3;
                                                }
    //  qmlformat on

    // indent 4
    onAnotherSignal: {
        // qmlformat off
                                                let arr = [1, 2, 3];
        // qmlformat on
        // indent 8
                                                let [x, ...rest] = arr;
                                                let obj = {a: 1, b: 2, c: 3};
        let total = x + a;
    }
    // qmlformat on
}

// qmlformat off
    // Unclosed region at end of file
    // Should turn off qmlformat until the end of file
