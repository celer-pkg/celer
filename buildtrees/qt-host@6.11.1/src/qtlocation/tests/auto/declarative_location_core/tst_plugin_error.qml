// Copyright (C) 2016 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR GPL-3.0-only

import QtQuick
import QtTest
import QtLocation

Item {

    Plugin { id: testPlugin;
            name: "qmlgeo.test.plugin"
            allowExperimental: true
            parameters: [
                // Parms to guide the test plugin
                PluginParameter { name: "error"; value: "1"},
                PluginParameter { name: "errorString"; value: "This error was expected. No worries !"}
            ]
        }

    Map {
        id: map
    }

    SignalSpy {id: errorSpy; target: map; signalName: "errorChanged"}

    TestCase {
        name: "MappingManagerError"
        function test_error() {
            verify (map.error === Map.NoError);
            map.plugin = testPlugin;
            verify (map.error === Map.NotSupportedError);
            verify (map.errorString == "This error was expected. No worries !");
            compare(errorSpy.count, 1);
        }
    }
}
