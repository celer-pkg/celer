// Copyright (C) 2022 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR GPL-3.0-only

import QtPositioning
import QtQml
import QtQuick
import QtTest

TestCase {
    name: "SatelliteSource"

    id: top

    Component {
        id: defaultSource
        SatelliteSource {
        }
    }

    Component {
        id: sourceWithParameters
        SatelliteSource {
            name: "satellitesource.test"
            PluginParameter { name: "satellitesystem"; value: GeoSatelliteInfo.GALILEO }
        }
    }

    Component {
        id: sourceWithIncompleteParameter
        SatelliteSource {
            name: "satellitesource.test"
            PluginParameter {
                objectName: "incompleteProperty"
                name: "satellitesystem"
            }
        }
    }

    SignalSpy { id: inViewSpy; signalName: "satellitesInViewChanged" }
    SignalSpy { id: inUseSpy; signalName: "satellitesInUseChanged" }
    SignalSpy { id: activeSpy; signalName: "activeChanged" }
    SignalSpy { id: validSpy; signalName: "validityChanged" }
    SignalSpy { id: updateIntervalSpy; signalName: "updateIntervalChanged" }
    SignalSpy { id: errorSpy; signalName: "sourceErrorChanged" }
    SignalSpy { id: nameSpy; signalName: "nameChanged" }

    function test_default_construct() {
        var source = defaultSource.createObject(top, {})
        // empty name resulted in picking a default source, which is valid
        compare(source.valid, true)
        compare(source.active, false)
        verify(source.name !== "")
        compare(source.sourceError, SatelliteSource.NoError)
        compare(source.satellitesInView.length, 0)
        compare(source.satellitesInUse.length, 0)
    }

    function test_unknown_plugin() {
        var source = defaultSource.createObject(top, { name: "invalid_plugin" })
        compare(source.valid, false)
        compare(source.active, false)
        compare(source.name, "invalid_plugin")
        compare(source.sourceError, SatelliteSource.NoError)
        compare(source.satellitesInView.length, 0)
        compare(source.satellitesInUse.length, 0)
    }

    function test_backend_properties() {
        var source = defaultSource.createObject(top, { name: "satellitesource.test" })

        var inView = source.backendProperty("desiredInViewCount")
        var inUse = source.backendProperty("desiredInUseCount")
        var useElevation = source.backendProperty("useElevation")
        var useAzimuth = source.backendProperty("useAzimuth")
        var unknownParam = source.backendProperty("unknown")

        // check defaults
        compare(inView, 5)
        compare(inUse, 3)
        compare(useElevation, false)
        compare(useAzimuth, false)
        compare(unknownParam, undefined)

        // modify
        compare(source.setBackendProperty("desiredInViewCount", 10), true)
        compare(source.setBackendProperty("desiredInUseCount", 5), true)
        compare(source.setBackendProperty("useElevation", true), true)
        compare(source.setBackendProperty("useAzimuth", true), true)
        compare(source.setBackendProperty("unknown", "val"), false)

        // check that parameters are updated
        inView = source.backendProperty("desiredInViewCount")
        inUse = source.backendProperty("desiredInUseCount")
        useElevation = source.backendProperty("useElevation")
        useAzimuth = source.backendProperty("useAzimuth")

        compare(inView, 10)
        compare(inUse, 5)
        compare(useElevation, true)
        compare(useAzimuth, true)
    }

    function test_regular_updates() {
        var source = defaultSource.createObject(top, { name: "satellitesource.test" })
        inViewSpy.target = source
        inUseSpy.target = source
        inViewSpy.clear()
        inUseSpy.clear()

        var expectedInView = source.backendProperty("desiredInViewCount")
        var expectedInUse = source.backendProperty("desiredInUseCount")
        verify(expectedInView > 0)
        verify(expectedInUse > 0)

        source.start()
        tryCompare(inViewSpy, "count", 1)
        tryCompare(inUseSpy, "count", 1)

        compare(source.satellitesInView.length, expectedInView)
        compare(source.satellitesInUse.length, expectedInUse)

        for (var idx = 0; idx < expectedInView; ++idx) {
            var satInView = source.satellitesInView[idx]
            compare(satInView.satelliteSystem, GeoSatelliteInfo.GPS)
            compare(satInView.satelliteIdentifier, idx + 1)
            compare(satInView.signalStrength, 5 * (idx + 1))
            compare(satInView.hasAttribute(GeoSatelliteInfo.Elevation), false)
            compare(satInView.attribute(GeoSatelliteInfo.Elevation), -1)
            compare(satInView.hasAttribute(GeoSatelliteInfo.Azimuth), false)
            compare(satInView.attribute(GeoSatelliteInfo.Azimuth), -1)

            if (idx < expectedInUse) {
                var satInUse = source.satellitesInUse[idx]
                compare(satInUse, satInView)
            }
        }
    }

    function test_update_with_plugin_parameters_and_properties() {
        var source = sourceWithParameters.createObject(top, {})
        inViewSpy.target = source
        inUseSpy.target = source
        inViewSpy.clear()
        inUseSpy.clear()

        source.setBackendProperty("useElevation", true)
        source.setBackendProperty("useAzimuth", true)

        var expectedInView = source.backendProperty("desiredInViewCount")
        var expectedInUse = source.backendProperty("desiredInUseCount")

        source.update()
        tryCompare(inViewSpy, "count", 1)
        tryCompare(inUseSpy, "count", 1)

        for (var idx = 0; idx < expectedInView; ++idx) {
            var satInView = source.satellitesInView[idx]
            compare(satInView.satelliteSystem, GeoSatelliteInfo.GALILEO)
            compare(satInView.satelliteIdentifier, idx + 1)
            compare(satInView.signalStrength, 5 * (idx + 1))
            compare(satInView.hasAttribute(GeoSatelliteInfo.Elevation), true)
            compare(satInView.attribute(GeoSatelliteInfo.Elevation), 3.0 * (idx + 1))
            compare(satInView.hasAttribute(GeoSatelliteInfo.Azimuth), true)
            compare(satInView.attribute(GeoSatelliteInfo.Azimuth), 0.5 * (idx + 1))

            if (idx < expectedInUse) {
                var satInUse = source.satellitesInUse[idx]
                compare(satInUse, satInView)
            }
        }
    }

    function test_signals() {
        var source = defaultSource.createObject(top, { name: "satellitesource.test" })
        activeSpy.target = source
        activeSpy.clear()
        validSpy.target = source
        validSpy.clear()
        updateIntervalSpy.target = source
        updateIntervalSpy.clear()
        nameSpy.target = source
        nameSpy.clear()
        errorSpy.target = source
        errorSpy.clear()

        compare(source.valid, true)

        source.updateInterval = 1000
        tryCompare(updateIntervalSpy, "count", 1)
        compare(source.updateInterval, 1000)

        source.active = true
        tryCompare(activeSpy, "count", 1)
        compare(source.active, true)

        // should trigger an error, because it's less than minimumUpdateInterval
        source.update(100)
        tryCompare(errorSpy, "count", 1)
        compare(source.sourceError, SatelliteSource.UpdateTimeoutError)

        source.name = "invalid_plugin"
        tryCompare(nameSpy, "count", 1)
        compare(source.name, "invalid_plugin")
        tryCompare(activeSpy, "count", 2)
        compare(source.active, false)
        tryCompare(validSpy, "count", 1)
        compare(source.valid, false)
    }

    function test_create_active() {
        // create a default source, active by default - should be working fine
        var source = defaultSource.createObject(top, { active: true })
        inViewSpy.target = source
        inViewSpy.clear()
        inUseSpy.target = source
        inUseSpy.clear()

        tryCompare(source, "active", true)
        tryCompare(source, "valid", true)

        tryVerify(function() { return inViewSpy.count > 0 && inUseSpy.count > 0 })

        verify(source.satellitesInView.length > 0)
        verify(source.satellitesInUse.length > 0)

        // create an invalid source, active by default - should fail
        var invalidSource = defaultSource.createObject(top, { name: "invalid", active: true })
        tryCompare(invalidSource, "active", false)
        tryCompare(invalidSource, "valid", false)
    }

    function test_set_plugin() {
        // When creating with no name specified, some default plugin will
        // be chosen
        var source = defaultSource.createObject(top, {})
        nameSpy.target = source
        nameSpy.clear()
        activeSpy.target = source
        activeSpy.clear()

        compare(source.valid, true)
        var defaultPluginName = source.name

        source.active = true
        tryCompare(activeSpy, "count", 1)

        // check that setting an empty name will do nothing, as we are already
        // using the default
        source.name = ""
        compare(nameSpy.count, 0)
        compare(source.name, defaultPluginName)
        compare(source.active, true)

        // Set another valid plugin name. Source will be set to inactive state,
        // but will be valid
        var otherPluginName = (defaultPluginName === "satellitesource.test")
                ? "dummy.source" : "satellitesource.test";
        source.name = otherPluginName
        compare(nameSpy.count, 1)
        compare(source.name, otherPluginName)
        tryCompare(activeSpy, "count", 2)
        compare(source.active, false)
        compare(source.valid, true)

        source.active = true
        tryCompare(activeSpy, "count", 3)
        compare(source.active, true)

        // Set invalid name. Source will be set to inactive state, and will be
        // invalid
        source.name = "invalid_name"
        compare(nameSpy.count, 2)
        compare(source.name, "invalid_name")
        tryCompare(activeSpy, "count", 4)
        compare(source.active, false)
        compare(source.valid, false)
    }

    function test_update_after_start() {
        // When update() is called after start(), it should not invalidate any
        // state. The active state must still be true when the single update()
        // is completed.
        var source = defaultSource.createObject(top, { name: "satellitesource.test" })
        errorSpy.target = source
        errorSpy.clear()
        inViewSpy.target = source
        inViewSpy.clear()
        compare(source.active, false)

        source.start()
        tryCompare(source, "active", true)

        source.update(300) // default update interval == 200
        tryVerify(function() { return errorSpy.count > 0 || inViewSpy.count > 0 })

        // at this point update() is completed, source should still be active
        compare(source.active, true)

        source.stop()
        tryCompare(source, "active", false)
    }

    function test_start_after_update() {
        // When start() is called after update(), the source should remain
        // active even when the single update is completed.
        var source = defaultSource.createObject(top, { name: "satellitesource.test" })
        errorSpy.target = source
        errorSpy.clear()
        inViewSpy.target = source
        inViewSpy.clear()
        compare(source.active, false)

        source.update(300)
        tryCompare(source, "active", true)

        source.start()
        tryVerify(function() { return errorSpy.count > 0 || inViewSpy.count > 0 })

        // at this point update() is completed, source should still be active
        compare(source.active, true)

        source.stop()
        tryCompare(source, "active", false)
    }

    function test_stop_after_update() {
        // When stop() is called after update(), and the update() is still in
        // progress, the source should remain active until the update()
        // is completed.
        var source = defaultSource.createObject(top, { name: "satellitesource.test" })
        errorSpy.target = source
        errorSpy.clear()
        inViewSpy.target = source
        inViewSpy.clear()
        compare(source.active, false)

        source.start()
        tryCompare(source, "active", true)

        source.update(300) // default update interval == 200
        source.stop();
        compare(source.active, true)

        tryVerify(function() { return errorSpy.count > 0 || inViewSpy.count > 0 })
        // at this point update() is completed, source should switch to inactive
        compare(source.active, false)
    }

    function test_start_stop_after_update() {
        // Calling start() and stop() after update(), while still waiting for
        // the update() to complete, should still result in the source to be
        // active until the update() is completed.
        var source = defaultSource.createObject(top, { name: "satellitesource.test" })
        errorSpy.target = source
        errorSpy.clear()
        inViewSpy.target = source
        inViewSpy.clear()
        compare(source.active, false)

        source.update(300)
        tryCompare(source, "active", true)

        source.start()
        source.stop()
        compare(source.active, true)

        tryVerify(function() { return errorSpy.count > 0 || inViewSpy.count > 0 })
        // at this point update() is completed, source should switch to inactive
        compare(source.active, false)
    }

    function test_update_timed_out() {
        // This test checks that source resets to inactive state when the single
        // update() request times out without providing the position info
        var source = defaultSource.createObject(top, { name: "satellitesource.test" })
        errorSpy.target = source
        errorSpy.clear()
        compare(source.active, false)

        source.update(50) // too small interval
        compare(source.active, true)

        tryCompare(errorSpy, "count", 1)
        compare(source.active, false)
    }

    function test_update_with_start_timed_out() {
        // This test checks that if single update() times out, but the regular
        // updates are running, we still remain in active state.
        var source = defaultSource.createObject(top, { name: "satellitesource.test" })
        errorSpy.target = source
        errorSpy.clear()
        compare(source.active, false)

        source.start()
        source.update(100) // too small interval
        tryCompare(source, "active", true)

        tryCompare(errorSpy, "count", 1)
        compare(source.active, true) // still active

        source.stop()
        tryCompare(source, "active", false)
    }

    function test_start_update_stop_with_no_intervals() {
        // This test checks that calls start() -> update() -> stop() without
        // any waits between them will result in expected behavior.
        // Specifically, the source should remain active until it gets
        // the satellite updates.
        var source = defaultSource.createObject(top, { name: "satellitesource.test" })
        inViewSpy.target = source
        inViewSpy.clear()
        compare(source.active, false)

        source.start()
        source.update(300)
        source.stop()
        tryCompare(source, "active", true)

        tryCompare(inViewSpy, "count", 1)
        // after the update is received, the source switches to inactive state
        compare(source.active, false)
    }

    function test_start_when_incomplete() {
        var source = sourceWithIncompleteParameter.createObject(top, {})
        inViewSpy.target = source
        inViewSpy.clear()
        var parameterObj = findChild(source, "incompleteProperty")

        source.start() // does nothing, as the parameter is not initialized
        compare(source.active, false)

        // init parameter
        parameterObj.value = GeoSatelliteInfo.BEIDOU
        // source created - should start providing updates
        tryCompare(source, "active", true)

        tryVerify(function() { return inViewSpy.count > 0 })

        source.stop()
        tryCompare(source, "active", false)
    }

    function test_start_stop_when_incomplete() {
        var source = sourceWithIncompleteParameter.createObject(top, {})
        var parameterObj = findChild(source, "incompleteProperty")

        source.start() // does nothing, as the parameter is not initialized
        compare(source.active, false)
        source.stop() // calling stop() should invalidate request for start

        // init parameter
        parameterObj.value = GeoSatelliteInfo.BEIDOU
        // source still should be inactive
        tryCompare(source, "active", false)
    }

    function test_update_when_incomplete() {
        var source = sourceWithIncompleteParameter.createObject(top, {})
        inViewSpy.target = source
        inViewSpy.clear()
        var parameterObj = findChild(source, "incompleteProperty")

        source.update() // does nothing, as the parameter is not initialized
        compare(source.active, false)

        // init parameter
        parameterObj.value = GeoSatelliteInfo.BEIDOU
        // source created - should provide exactly 1 update
        tryCompare(source, "active", true)

        tryVerify(function() { return inViewSpy.count == 1 })
        tryCompare(source, "active", false)
    }
}

