pragma Strict
import QtQml

QtObject {
    property var layout: {
        const a = false
        const b = !true || a
        return {
            a,
            b,
        };
    }
}
