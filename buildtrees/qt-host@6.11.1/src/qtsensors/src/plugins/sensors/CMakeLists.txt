# Copyright (C) 2022 The Qt Company Ltd.
# SPDX-License-Identifier: BSD-3-Clause

# all get generic sensor
add_subdirectory(generic)

if(ANDROID AND NOT SENSORS_PLUGINS OR "sensorfw" IN_LIST SENSORS_PLUGINS)
    add_subdirectory(android)
endif()

# Currently disabled, see src/sensors/configure.cmake
#if (QT_FEATURE_sensorfw AND NOT SENSORS_PLUGINS OR "sensorfw" IN_LIST SENSORS_PLUGINS)
#     add_subdirectory(sensorfw)
#endif()

if (QT_FEATURE_winrt_sensors AND NOT SENSORS_PLUGINS OR "winrt" IN_LIST SENSORS_PLUGINS)
    add_subdirectory(winrt)
endif()

if (IOS AND NOT SENSORS_PLUGINS OR "ios" IN_LIST SENSORS_PLUGINS)
    add_subdirectory(ios)
endif()

if(LINUX AND TARGET Qt::DBus AND NOT SENSORS_PLUGINS OR "iio-sensor-proxy" IN_LIST SENSORS_PLUGINS)
   add_subdirectory(iio-sensor-proxy)
endif()

if("dummy" IN_LIST SENSORS_PLUGINS)
   add_subdirectory(dummy)
endif()
