# Copyright (C) 2022 The Qt Company Ltd.
# SPDX-License-Identifier: BSD-3-Clause

#####################################################################
## IIOSensorProxySensorPlugin Plugin:
#####################################################################

qt_internal_add_plugin(IIOSensorProxySensorPlugin
    OUTPUT_NAME qtsensors_iio-sensor-proxy
    PLUGIN_TYPE sensors
    SOURCES
        iiosensorproxycompass.cpp iiosensorproxycompass.h
        iiosensorproxylightsensor.cpp iiosensorproxylightsensor.h
        iiosensorproxyorientationsensor.cpp iiosensorproxyorientationsensor.h
        iiosensorproxysensorbase.cpp iiosensorproxysensorbase.h
        iiosensorproxyproximitysensor.cpp iiosensorproxyproximitysensor.h
        main.cpp
    NO_UNITY_BUILD_SOURCES
        # Duplicate symbol definition between, iiosensorproxycompass.cpp, and
        # the followings
        # TODO: dbusPath() can probably be implemented as a static data member
        iiosensorproxylightsensor.cpp
        iiosensorproxyorientationsensor.cpp
        iiosensorproxyproximitysensor.cpp
    DBUS_INTERFACE_SOURCES
        org.freedesktop.DBus.Properties.xml
        net.hadess.SensorProxy.xml
        net.hadess.SensorProxy.Compass.xml
    DBUS_INTERFACE_FLAGS
        "-N"
        --namespace QtSensorsPrivate
    LIBRARIES
        Qt::Core
        Qt::DBus
        Qt::Sensors
)

#### Keys ignored in scope 1:.:.:iio-sensor-proxy.pro:<TRUE>:
# OTHER_FILES = "plugin.json" "$$DBUS_INTERFACES"
# dbus_properties.files = "org.freedesktop.DBus.Properties.xml"
# dbus_properties.header_flags = "-N"
# sensor_proxy.files = "net.hadess.SensorProxy.xml"
# sensor_proxy.header_flags = "-N"
# sensor_proxy_compass.files = "net.hadess.SensorProxy.Compass.xml"
# sensor_proxy_compass.header_flags = "-N"

## Scopes:
#####################################################################

qt_internal_extend_target(IIOSensorProxySensorPlugin CONDITION NOT ANDROID
    LIBRARIES
        rt
)
