pragma Strict
import QtQml
import QtQml as QQ
import TestTypes
import TestTypes as TT

QtObject {
    property rect r: ({x: 1, y: 2, width: 3, height: 4})
    property rect r2: { var x = 42; return {x}; }
    property rect r3: new QQ.rect({x: 4, y: 3, width: 2, height: 1})
    property rect r4: { var x = 43; return new QQ.rect({x}); }
    property weatherModelUrl w: ({ strings: ["one", "two", "three"] })
    property weatherModelUrlDerived w1: ({ strings: ["four", "five", "six"] })
    property weatherModelUrl w2: new TT.weatherModelUrl({ strings: ["three", "two", "one"] })
}
