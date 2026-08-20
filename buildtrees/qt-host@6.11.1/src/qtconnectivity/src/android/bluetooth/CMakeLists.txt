# Copyright (C) 2022 The Qt Company Ltd.
# SPDX-License-Identifier: BSD-3-Clause

set(java_sources
    src/org/qtproject/qt/android/bluetooth/QtBluetoothBroadcastReceiver.java
    src/org/qtproject/qt/android/bluetooth/QtBluetoothInputStreamThread.java
    src/org/qtproject/qt/android/bluetooth/QtBluetoothLE.java
    src/org/qtproject/qt/android/bluetooth/QtBluetoothLEServer.java
    src/org/qtproject/qt/android/bluetooth/QtBluetoothSocketServer.java
    src/org/qtproject/qt/android/bluetooth/QtBluetoothGattCharacteristic.java
    src/org/qtproject/qt/android/bluetooth/QtBluetoothGattDescriptor.java
    src/org/qtproject/qt/android/bluetooth/QtBluetoothUtility.java
)

qt_internal_add_jar(Qt${QtConnectivity_VERSION_MAJOR}AndroidBluetooth
    INCLUDE_JARS ${QT_ANDROID_JAR}
    SOURCES ${java_sources}
    OUTPUT_DIR "${QT_BUILD_DIR}/jar"
)

qt_path_join(destination ${INSTALL_DATADIR} "jar")

install_jar(Qt${QtConnectivity_VERSION_MAJOR}AndroidBluetooth
    DESTINATION ${destination}
    COMPONENT Devel
)

add_dependencies(Bluetooth Qt${QtConnectivity_VERSION_MAJOR}AndroidBluetooth)
