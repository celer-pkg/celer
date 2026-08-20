// Copyright (C) 2021 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR GPL-3.0-only

import QtTest
import QtSensors

TestCase {
    id: testCase
    name: "SensorTest"

    SignalSpy {
        id: sensorActiveSpy
        signalName: "activeChanged"
    }

    SignalSpy {
        id: sensorReadingSpy
        signalName: "readingChanged"
    }

    SignalSpy {
        id: sensorBusySpy
        signalName: "busyChanged"
    }

    SignalSpy {
        id: sensorIdentifierSpy
        signalName: "identifierChanged"
    }

    function init() {
        TestControl.registerTestBackends()
    }

    function cleanup() {
        TestControl.unregisterTestBackends()
        sensorBusySpy.clear()
        sensorActiveSpy.clear()
        sensorReadingSpy.clear()
        sensorIdentifierSpy.clear()
    }

    function test_activate() {

        // create sensor without proper identifier and verify activation fails
        var sensor = Qt.createQmlObject("import QtSensors; Accelerometer {identifier: \"nonexistent\"}",testCase);
        sensorActiveSpy.target = sensor
        sensorIdentifierSpy.target = sensor
        verify(!sensor.active)
        compare(sensor.identifier, "nonexistent")
        sensor.active = true
        verify(!sensor.active)
        compare(sensorActiveSpy.count, 0)

        // set proper identifier and verify activation succeeds
        sensor.identifier = "QAccelerometer"
        compare(sensor.identifier, "QAccelerometer")
        compare(sensorIdentifierSpy.count, 1)
        sensor.active = true
        compare(sensorActiveSpy.count, 1)
        verify(sensor.active)
        compare(sensor.reading.x, 1.0)

        // set identifier again, verify no impact
        sensor.identifier = "QAccelerometer"
        compare(sensor.identifier, "QAccelerometer")
        compare(sensorIdentifierSpy.count, 1)

        // set activate again, verify no impact
        sensor.active = true
        sensor.start()
        compare(sensorActiveSpy.count, 1)
        verify(sensor.active)

        // deactivate
        sensor.active = false
        compare(sensorActiveSpy.count, 2)
        verify(!sensor.active)

        // reactivate and stop
        sensor.active = true
        compare(sensorActiveSpy.count, 3)
        verify(sensor.active)
        sensor.stop()
        compare(sensorActiveSpy.count, 4)
        verify(!sensor.active)

        // create sensor with proper id and active 'true' on creation time
        var sensor2 = Qt.createQmlObject("import QtSensors; Accelerometer {identifier: \"QAccelerometer\"; active: true}", testCase);
        verify(sensor2.active)

        // create sensor with nonexistent id and active 'true' on creation time
        var sensor3 = Qt.createQmlObject("import QtSensors; Accelerometer {identifier: \"nonexistent\"; active: true}", testCase);
        verify(!sensor3.active)
        sensor3.identifier = "QAccelerometer"
        sensor3.start()
        verify(sensor3.active)

        // create sensor with empty id, and check that a default is used
        var sensor4 = Qt.createQmlObject("import QtSensors; Accelerometer {active: true}", testCase);
        verify(sensor4.active)
        compare(sensor4.identifier, QmlSensors.defaultSensorForType("QAccelerometer"));

        // same as previous but with delayed activation
        var sensor5 = Qt.createQmlObject("import QtSensors; Accelerometer {}", testCase);
        verify(!sensor5.active)
        sensor5.active = true
        verify(sensor5.active)
        compare(sensor5.identifier, QmlSensors.defaultSensorForType("QAccelerometer"));

        // tidy up
        sensor.destroy()
        sensor2.destroy()
        sensor3.destroy()
        sensor4.destroy()
        sensor5.destroy()
    }

    function test_busy() {
        var sensor = Qt.createQmlObject("import QtSensors; Accelerometer {identifier: \"QAccelerometer\"}", testCase);
        sensorBusySpy.target = sensor
        compare(sensor.busy, false)
        verify(sensor.start())

        // set sensor busy and verify 'busy' property and its signaling
        TestControl.setSensorBusy(sensor, true)
        compare(sensorBusySpy.count, 1)
        TestControl.setSensorBusy(sensor, false)
        compare(sensorBusySpy.count, 2)
        TestControl.setSensorBusy(sensor, false)
        compare(sensorBusySpy.count, 2)

        // tidy up
        sensor.destroy()
    }

    function test_reading(data) {

        var sensor = Qt.createQmlObject(
                    "import QtSensors; "
                    + data.tag + "{"
                    + "identifier: " + "\"Q" + data.tag + "\""
                    + "}"
                    ,testCase)
        sensorActiveSpy.target = sensor
        sensorReadingSpy.target = sensor

        // verify initial values of sensor
        // note: 'reading' values are 'undefined by design' before activation, and therefore aren't tested
        compare(sensor.type, "Q" + data.tag)
        compare(sensor.active, false)
        compare(sensor.alwaysOn, false )
        compare(sensor.busy, false)
        compare(sensor.description, "")
        compare(sensor.error, 0)
        compare(sensor.skipDuplicates, false)

        // start the sensor and verify activation
        sensor.start()
        compare(sensor.active, true)
        compare(sensorActiveSpy.count, 1)
        compare(sensorReadingSpy.count, 1)

        // verify the initial reading values
        for (var prop in data.initialReading)
            fuzzyCompare(sensor.reading[prop], data.initialReading[prop], 0.0001, data.tag + "::" + prop)

        // change reading values and verify them
        TestControl.setSensorReading(sensor, data.newReading)
        compare(sensorReadingSpy.count, 2)
        for (prop in data.newReading)
            fuzzyCompare(sensor.reading[prop], data.newReading[prop], 0.0001, data.tag + "::" + prop)

        // stop the sensor and verify deactivation
        sensor.stop()
        compare(sensor.active, false)
        compare(sensorActiveSpy.count, 2)
        compare(sensorReadingSpy.count, 2)

        // tidy up
        sensor.destroy()
    }

    function test_reading_data() {
        return [
                    {tag: "Accelerometer", initialReading: {timestamp: 1, x: 1.0, y: 1.0, z: 1.0}, newReading: {timestamp: 2, x: 2.0, y: 3.0, z: 4.0}},
                    {tag: "PressureSensor", initialReading: {pressure: 1.0, temperature: 1.0}, newReading: {pressure: 2.0, temperature: 3.0}},
                    {tag: "Gyroscope", initialReading: {x : 1.0, y: 1.0, z: 1.0}, newReading: {x : 2.0, y: 3.0, z: 4.0}},
                    {tag: "TapSensor", initialReading: {doubleTap: true, tapDirection: TapReading.Z_Both}, newReading: {doubleTap: false, tapDirection: TapReading.X_Both}},
                    {tag: "Compass", initialReading: {azimuth: 1.0, calibrationLevel: 1.0}, newReading: {azimuth: 2.0, calibrationLevel: 3.0}},
                    {tag: "ProximitySensor", initialReading: {near: true}, newReading: {near: false}},
                    {tag: "OrientationSensor", initialReading: {orientation: OrientationReading.LeftUp}, newReading: {orientation: OrientationReading.RightUp}},
                    {tag: "AmbientLightSensor", initialReading: {lightLevel: AmbientLightReading.Twilight}, newReading: {lightLevel: AmbientLightReading.Sunny}},
                    {tag: "Magnetometer", initialReading: {x : 1.0, y: 1.0, z: 1.0, calibrationLevel: 1.0}, newReading:  {x : 2.0, y: 3.0, z: 4.0, calibrationLevel: 5.0}},
                    {tag: "LidSensor", initialReading: {backLidClosed:true, frontLidClosed: true}, newReading:  {backLidClosed:false, frontLidClosed: false}},
                    {tag: "TiltSensor", initialReading: {yRotation: 1.0, xRotation: 1.0}, newReading: {yRotation: 2.0, xRotation: 3.0}},
                    {tag: "RotationSensor", initialReading: {x: 1.0, y: 1.0, z: 1.0}, newReading: {x: 2.0, y: 3.0, z: 4.0}},
                    {tag: "HumiditySensor", initialReading: {relativeHumidity: 1.0, absoluteHumidity: 1.0}, newReading: {relativeHumidity: 2.0, absoluteHumidity: 3.0}},
                    {tag: "AmbientTemperatureSensor", initialReading: {temperature: 30.0}, newReading: {temperature: 40.0}},
                    {tag: "LightSensor", initialReading: {illuminance: 1.0}, newReading: {illuminance: 2.0}},
                    {tag: "IRProximitySensor", initialReading: {reflectance: 0.5}, newReading: {reflectance: 0.6}}
               ];
    }

    function test_SupportedFeatures()
    {
        var sensor = Qt.createQmlObject("import QtSensors; Accelerometer \
                                         {identifier: \"QAccelerometer\"}",
                                         testCase);
        verify(sensor.start())
        verify(sensor.connectedToBackend)

        // According to isFeatureSupported() override implementation in test_backends.h,
        // only SkipDuplicates should be supported afterwards
        verify(!sensor.isFeatureSupported(Sensor.Buffering))
        verify(!sensor.isFeatureSupported(Sensor.AlwaysOn))
        verify(!sensor.isFeatureSupported(Sensor.GeoValues))
        verify(!sensor.isFeatureSupported(Sensor.FieldOfView))
        verify(!sensor.isFeatureSupported(Sensor.AccelerationMode))
        verify(sensor.isFeatureSupported(Sensor.SkipDuplicates))
        verify(!sensor.isFeatureSupported(Sensor.AxesOrientation))
        verify(!sensor.isFeatureSupported(Sensor.PressureSensorTemperature))

        sensor.destroy()
    }
}
