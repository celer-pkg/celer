import QtQuick

Item {
    property int u: 50
    Item {
        function wait(timeout) {
            if (timeout === undefined)
                timeout = 5000

            var i = 0;
            while (i < timeout)
                i += u // unqualified, produces warning, but only once

            return i
        }
    }
}
