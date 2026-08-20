// Copyright (C) 2016 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR GPL-3.0-only

import QtQuick 2.0
import QtTest 1.0
import QtPositioning 5.14

TestCase {
    id: testCase

    name: "PositionSource"

    PositionSource { id: defaultSource }
    PositionSource
    {
        id: activeDefaultSource
        active: true
    }

    SignalSpy { id: defaultSourceSpy; target: defaultSource; signalName: "positionChanged" }

    function test_activeDefaultSource() {
        wait(0);
        verify(activeDefaultSource.name !== "");
        compare(activeDefaultSource.active, true);
    }

    function test_invalidSource() {
        activeDefaultSource.name = "invalid_positioning_source";
        verify(!activeDefaultSource.active);
        verify(!activeDefaultSource.valid);
    }

    function test_defaults() {
        // at least the test.source plugin should be available
        verify(defaultSource.name != "");
        compare(defaultSource.active, false);
    }

    function test_inactive() {
        defaultSourceSpy.clear();
        compare(defaultSourceSpy.count, 0);
        wait(1000);
        compare(defaultSourceSpy.count, 0);
    }

    PositionSource { id: testSetSource; name: "nonexistent bogus plugin" }
    SignalSpy { id: testingSourcePluginSpy; target: testSetSource; signalName: "nameChanged" }

    function test_setplugin() {
        testingSourcePluginSpy.clear();

        // On construction, if the provided source name is invalid, the default source will be
        // used. Test that the source is valid as expected.
        verify(testSetSource.name !== "");
        //we don't really know what the default source is named.
        //It may not be "test.source"
        var defaultSourceName = testSetSource.name;
        verify(testSetSource.valid);

        // Test that setting name to "" will still use the default.
        testSetSource.name = "";
        compare(testingSourcePluginSpy.count, 0);
        compare(testSetSource.name, defaultSourceName);
        verify(testSetSource.valid);

        testSetSource.name = "test.source";
        if (defaultSourceName === "test.source")
            compare(testingSourcePluginSpy.count, 0);
        compare(testSetSource.name, "test.source");
        verify(testSetSource.valid);
        testingSourcePluginSpy.clear();

        testSetSource.name = "bogus";
        compare(testingSourcePluginSpy.count, 1);
        verify(!testSetSource.valid);
    }

    PositionSource { id: testingSource; name: "test.source"; updateInterval: 1000 }

    PositionSource {
        id: testingSourceWParams
        name: "test.source"
        updateInterval: 200
        PluginParameter {
            id: altitudeParameter
            name: "test.source.altitude"
            value: 42.42
        }
    }

    // use property bindings instead of SignalSpy
    property bool directionWParamsValid: testingSourceWParams.position.directionValid
    property int directionWParamsValidChangedCount: 0
    onDirectionWParamsValidChanged: {
        ++directionWParamsValidChangedCount
    }

    property real directionWParams: testingSourceWParams.position.direction
    property int directionWParamsChangedCount:  0
    onDirectionWParamsChanged: {
        ++directionWParamsChangedCount
    }

    SignalSpy { id: updateSpyWParams; target: testingSourceWParams; signalName: "positionChanged" }

    PositionSource { id: testingSourceV1; name: "test.source"; updateInterval: 200 }

    // use property bindings instead of SignalSpy
    property bool directionV1Valid: testingSourceV1.position.directionValid
    property int directionV1ValidChangedCount: 0
    onDirectionV1ValidChanged: {
        ++directionV1ValidChangedCount
    }

    property real directionV1: testingSourceV1.position.direction
    property int directionV1ChangedCount:  0
    onDirectionV1Changed: {
        ++directionV1ChangedCount
    }

    SignalSpy { id: updateSpyV1; target: testingSourceV1; signalName: "positionChanged" }

    function test_updateInterval() {
        testingSource.updateInterval = 200;
        compare(testingSource.updateInterval, 200);
        testingSource.updateInterval = 300;
        compare(testingSource.updateInterval, 300);
        testingSource.updateInterval = 100;
        compare(testingSource.updateInterval, 200);
    }

    function test_preferredPositioningMethods() {
        testingSource.preferredPositioningMethods = PositionSource.AllPositioningMethods;
        compare(testingSource.preferredPositioningMethods, PositionSource.AllPositioningMethods);
        testingSource.preferredPositioningMethods = PositionSource.SatellitePositioningMethods;
        compare(testingSource.preferredPositioningMethods, PositionSource.SatellitePositioningMethods);
        testingSource.preferredPositioningMethods = PositionSource.NonSatellitePositioningMethods;
        compare(testingSource.preferredPositioningMethods, PositionSource.NonSatellitePositioningMethods);
    }

    function test_updates() {
        updateSpyV1.clear();

        directionV1ChangedCount = 0
        directionV1ValidChangedCount = 0

        testingSourceV1.active = true;

        tryCompare(updateSpyV1, "count", 1, 300);
        compare(testingSourceV1.position.coordinate.longitude, 0.1);
        compare(testingSourceV1.position.coordinate.latitude, 0.1);
        compare(directionV1ValidChangedCount, 1)
        compare(directionV1ChangedCount, 1)
        fuzzyCompare(testingSourceV1.position.direction, 45, 0.1)
        verify(!testingSourceV1.position.speedValid)
        verify(isNaN(testingSourceV1.position.speed))

        tryCompare(updateSpyV1, "count", 2, 300);
        compare(testingSourceV1.position.coordinate.longitude, 0.2);
        compare(testingSourceV1.position.coordinate.latitude, 0.2);
        compare(directionV1ValidChangedCount, 1)
        compare(directionV1ChangedCount, 2)
        fuzzyCompare(testingSourceV1.position.direction, 45, 0.1)
        verify(testingSourceV1.position.speedValid)
        verify(testingSourceV1.position.speed > 10000)

        testingSourceV1.active = false;
        wait(300);
        compare(updateSpyV1.count, 2);
        compare(testingSourceV1.position.coordinate.longitude, 0.2);
        compare(testingSourceV1.position.coordinate.latitude, 0.2);
        compare(directionV1ValidChangedCount, 1)
        compare(directionV1ChangedCount, 2)
        fuzzyCompare(testingSourceV1.position.direction, 45, 0.1)
        verify(testingSourceV1.position.speedValid)
        verify(testingSourceV1.position.speed > 10000)
    }

    function test_updates_w_params() {
        updateSpyWParams.clear();

        directionWParamsChangedCount = 0
        directionWParamsValidChangedCount = 0
        compare(testingSourceWParams.backendProperty("altitude"), altitudeParameter.value)
        testingSourceWParams.active = true;

        tryCompare(updateSpyWParams, "count", 1, 300);
        compare(testingSourceWParams.position.coordinate.longitude, 0.1);
        compare(testingSourceWParams.position.coordinate.latitude, 0.1);
        compare(testingSourceWParams.position.coordinate.altitude, altitudeParameter.value);
        compare(directionWParamsValidChangedCount, 1)
        compare(directionWParamsChangedCount, 1)
        fuzzyCompare(testingSourceWParams.position.direction, 45, 0.1)
        verify(!testingSourceWParams.position.speedValid)
        verify(isNaN(testingSourceWParams.position.speed))
        testingSourceWParams.setBackendProperty("altitude", 24.24)

        tryCompare(updateSpyWParams, "count", 2, 300);
        compare(testingSourceWParams.position.coordinate.longitude, 0.2);
        compare(testingSourceWParams.position.coordinate.latitude, 0.2);
        compare(testingSourceWParams.position.coordinate.altitude, 24.24);
        compare(directionWParamsValidChangedCount, 1)
        compare(directionWParamsChangedCount, 2)
        fuzzyCompare(testingSourceWParams.position.direction, 45, 0.1)
        verify(testingSourceWParams.position.speedValid)
        verify(testingSourceWParams.position.speed > 10000)
        compare(testingSourceWParams.backendProperty("altitude"), 24.24)

        testingSourceWParams.active = false;
        wait(300);
        compare(updateSpyWParams.count, 2);
        compare(testingSourceWParams.position.coordinate.longitude, 0.2);
        compare(testingSourceWParams.position.coordinate.latitude, 0.2);
        compare(testingSourceWParams.position.coordinate.altitude, 24.24);
        compare(directionWParamsValidChangedCount, 1)
        compare(directionWParamsChangedCount, 2)
        fuzzyCompare(testingSourceWParams.position.direction, 45, 0.1)
        verify(testingSourceWParams.position.speedValid)
        verify(testingSourceWParams.position.speed > 10000)
    }

    // tests for bindings
    PositionSource {
        id: testSourceForBindings
        name: "test.source"
    }

    property var positionObserver: testSourceForBindings.position
    SignalSpy {
        id: positionObserverSpy; target: testSourceForBindings; signalName: "positionChanged"
    }

    property bool activeObserver: testSourceForBindings.active
    SignalSpy {
        id: activeObserverSpy; target: testSourceForBindings; signalName: "activeChanged"
    }

    property bool validObserver: testSourceForBindings.valid
    SignalSpy {
        id: validObserverSpy; target: testSourceForBindings; signalName: "validityChanged"
    }

    property int updateIntervalObserver: testSourceForBindings.updateInterval
    SignalSpy {
        id: updateIntervalObserverSpy; target: testSourceForBindings; signalName: "updateIntervalChanged"
    }

    property int supportedMethodsObserver: testSourceForBindings.supportedPositioningMethods
    SignalSpy {
        id: supportedMethodsObserverSpy; target: testSourceForBindings; signalName: "supportedPositioningMethodsChanged"
    }

    property int preferredMethodsObserver: testSourceForBindings.preferredPositioningMethods
    SignalSpy {
        id: preferredMethodsObserverSpy;
        target: testSourceForBindings;
        signalName: "preferredPositioningMethodsChanged"
    }

    property int errorObserver: testSourceForBindings.sourceError
    SignalSpy {
        id: errorObserverSpy; target: testSourceForBindings; signalName: "sourceErrorChanged"
    }

    property string nameObserver: testSourceForBindings.name
    SignalSpy {
        id: nameObserverSpy; target: testSourceForBindings; signalName: "nameChanged"
    }

    function test_bindingsToPositionSource() {
        positionObserverSpy.clear()
        activeObserverSpy.clear()
        validObserverSpy.clear()
        updateIntervalObserverSpy.clear()
        supportedMethodsObserverSpy.clear()
        preferredMethodsObserverSpy.clear()
        errorObserverSpy.clear()
        nameObserverSpy.clear()

        verify(testSourceForBindings.valid)
        compare(testSourceForBindings.supportedPositioningMethods, PositionSource.AllPositioningMethods)

        testSourceForBindings.preferredPositioningMethods = PositionSource.SatellitePositioningMethods
        compare(preferredMethodsObserver, PositionSource.SatellitePositioningMethods)
        compare(preferredMethodsObserverSpy.count, 1)

        testSourceForBindings.updateInterval = 210;
        compare(updateIntervalObserver, 210)
        compare(updateIntervalObserverSpy.count, 1)

        testSourceForBindings.start();
        compare(activeObserver, true)
        compare(activeObserverSpy.count, 1)

        tryCompare(positionObserverSpy, "count", 1, 300);
        verify(testSourceForBindings.position.coordinate !== QtPositioning.coordinate())

        testSourceForBindings.update(50) // small timeout will result in an error
        tryCompare(errorObserverSpy, "count", 1, 200)
        compare(errorObserver, PositionSource.UpdateTimeoutError)

        var newSourceName = "some invalid name"
        testSourceForBindings.name = newSourceName

        compare(validObserver, false)
        compare(validObserverSpy.count, 1)

        compare(nameObserver, newSourceName)
        compare(nameObserverSpy.count, 1)

        compare(supportedMethodsObserver, PositionSource.NoPositioningMethods)
        compare(supportedMethodsObserverSpy.count, 1)
    }

    property bool activeSetter: false
    property string nameSetter: "test.source"
    property int updateIntervalSetter: 200
    property int preferredMethodsSetter: PositionSource.NonSatellitePositioningMethods

    PositionSource {
        id: sourceWithBindings
        name: nameSetter
        active: activeSetter
        updateInterval: updateIntervalSetter
        preferredPositioningMethods: preferredMethodsSetter
    }

    function test_bindPositionSourceProperties() {
        compare(sourceWithBindings.name, "test.source")
        compare(sourceWithBindings.active, false)
        compare(sourceWithBindings.updateInterval, 200)
        compare(sourceWithBindings.preferredPositioningMethods, PositionSource.NonSatellitePositioningMethods)

        updateIntervalSetter = 210;
        preferredMethodsSetter = PositionSource.AllPositioningMethods
        activeSetter = true

        wait(0) // to trigger event processing

        compare(sourceWithBindings.active, true)
        compare(sourceWithBindings.updateInterval, 210)
        compare(sourceWithBindings.preferredPositioningMethods, PositionSource.AllPositioningMethods)

        activeSetter = false;

        compare(sourceWithBindings.active, false)

        // check how call to update() effects the binding
        sourceWithBindings.update(updateIntervalSetter)
        compare(sourceWithBindings.active, true)

        wait(300)
        compare(sourceWithBindings.active, false)

        activeSetter = true;
        wait(0) // to trigger event processing
        // the binding is *not* broken by calling update()
        compare(sourceWithBindings.active, true)
    }

    function test_updateIntervalBindings() {
        var updateIntervalSpy = Qt.createQmlObject('import QtTest; SignalSpy { }', testCase)
        updateIntervalSpy.target = sourceWithBindings
        updateIntervalSpy.signalName = "updateIntervalChanged"
        verify(updateIntervalSpy.valid)

        nameSetter = "invalid name"
        updateIntervalSetter = 100;
        compare(sourceWithBindings.updateInterval, 100)
        compare(updateIntervalSpy.count, 1)

        nameSetter = "test.source"
        // "test.source" has a minimum update interval of 200
        compare(sourceWithBindings.updateInterval, 200)
        compare(updateIntervalSpy.count, 2)

        nameSetter = "dummy.source"
        // "dummy.source" has a minimum udpate interval of 100, so we expect
        // the updateInterval to be set to 100
        compare(sourceWithBindings.updateInterval, 100)
        compare(updateIntervalSpy.count, 3)

        // The binding still works
        updateIntervalSetter = 110
        compare(sourceWithBindings.updateInterval, 110)
        compare(updateIntervalSpy.count, 4)
    }

    function test_preferredPositioningMethodsBindings() {
        var preferredMethodsSpy = Qt.createQmlObject('import QtTest; SignalSpy { }', testCase)
        preferredMethodsSpy.target = sourceWithBindings
        preferredMethodsSpy.signalName = "preferredPositioningMethodsChanged"
        verify(preferredMethodsSpy.valid)

        nameSetter = "invalid name"
        preferredMethodsSetter = PositionSource.SatellitePositioningMethods
        compare(sourceWithBindings.preferredPositioningMethods, PositionSource.SatellitePositioningMethods)
        compare(preferredMethodsSpy.count, 1)

        nameSetter = "dummy.source"
        // "dummy.source" has only NonSatellitePositioningMethods, so we expect
        // the value to be set to NonSatellitePositioningMethods (see
        // QGeoPositionInfoSource::setPreferredPositioningMethods for details)
        compare(sourceWithBindings.preferredPositioningMethods, PositionSource.NonSatellitePositioningMethods)
        compare(preferredMethodsSpy.count, 2)

        nameSetter = "test.source"
        // "test.source" has all positioning methods, so we expect the value to
        // be set to the desired SatellitePositioningMethods
        compare(sourceWithBindings.preferredPositioningMethods, PositionSource.SatellitePositioningMethods)
        compare(preferredMethodsSpy.count, 3)

        // The binding still works
        preferredMethodsSetter = PositionSource.AllPositioningMethods
        compare(sourceWithBindings.preferredPositioningMethods, PositionSource.AllPositioningMethods)
        compare(preferredMethodsSpy.count, 4)
    }

}
