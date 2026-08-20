# Copyright (C) 2022 The Qt Company Ltd.
# SPDX-License-Identifier: BSD-3-Clause

#####################################################################
## Bluetooth Module:
#####################################################################

qt_internal_add_module(Bluetooth
    SOURCES
        lecmaccalculator_p.h
        removed_api.cpp
        qbluetooth.cpp qbluetooth.h
        qbluetoothaddress.cpp qbluetoothaddress.h
        qbluetoothdevicediscoveryagent.cpp qbluetoothdevicediscoveryagent.h qbluetoothdevicediscoveryagent_p.h
        qbluetoothdeviceinfo.cpp qbluetoothdeviceinfo.h qbluetoothdeviceinfo_p.h
        qbluetoothhostinfo.cpp qbluetoothhostinfo.h qbluetoothhostinfo_p.h
        qbluetoothlocaldevice.cpp qbluetoothlocaldevice.h qbluetoothlocaldevice_p.h
        qbluetoothserver.cpp qbluetoothserver.h qbluetoothserver_p.h
        qbluetoothservicediscoveryagent.cpp qbluetoothservicediscoveryagent.h qbluetoothservicediscoveryagent_p.h
        qbluetoothserviceinfo.cpp qbluetoothserviceinfo.h qbluetoothserviceinfo_p.h
        qbluetoothsocket.cpp qbluetoothsocket.h
        qbluetoothsocketbase.cpp qbluetoothsocketbase_p.h
        qbluetoothuuid.cpp qbluetoothuuid.h
        qlowenergyadvertisingdata.cpp qlowenergyadvertisingdata.h
        qlowenergyadvertisingparameters.cpp qlowenergyadvertisingparameters.h
        qlowenergycharacteristic.cpp qlowenergycharacteristic.h
        qlowenergycharacteristicdata.cpp qlowenergycharacteristicdata.h
        qlowenergyconnectionparameters.cpp qlowenergyconnectionparameters.h
        qlowenergycontroller.cpp qlowenergycontroller.h
        qlowenergycontrollerbase.cpp qlowenergycontrollerbase_p.h
        qlowenergydescriptor.cpp qlowenergydescriptor.h
        qlowenergydescriptordata.cpp qlowenergydescriptordata.h
        qlowenergyservice.cpp qlowenergyservice.h
        qlowenergyservicedata.cpp qlowenergyservicedata.h
        qlowenergyserviceprivate.cpp qlowenergyserviceprivate_p.h
        qprivatelinearbuffer_p.h
        qtbluetoothglobal.h qtbluetoothglobal_p.h
    LIBRARIES
        Qt::CorePrivate
        Qt::NetworkPrivate
    PUBLIC_LIBRARIES
        Qt::Core
        Qt::Network
    PRIVATE_MODULE_INTERFACE
        Qt::CorePrivate
        Qt::NetworkPrivate
)

#### Keys ignored in scope 1:.:.:bluetooth.pro:<TRUE>:
# OTHER_FILES = "doc/src/*.qdoc"

## Scopes:
#####################################################################

if(QT_FEATURE_bluez)
    qt_internal_extend_target(Bluetooth
        SOURCES
            bluez/adapter1_bluez5.cpp bluez/adapter1_bluez5_p.h
            bluez/battery1.cpp bluez/battery1_p.h
            bluez/bluetoothmanagement.cpp bluez/bluetoothmanagement_p.h
            bluez/bluez5_helper.cpp bluez/bluez5_helper_p.h
            bluez/bluez_data.cpp bluez/bluez_data_p.h
            bluez/device1_bluez5.cpp bluez/device1_bluez5_p.h
            bluez/gattchar1.cpp bluez/gattchar1_p.h
            bluez/gattdesc1.cpp bluez/gattdesc1_p.h
            bluez/gattservice1.cpp bluez/gattservice1_p.h
            bluez/hcimanager.cpp bluez/hcimanager_p.h
            bluez/objectmanager.cpp bluez/objectmanager_p.h
            bluez/profile1.cpp bluez/profile1_p.h
            bluez/profile1context.cpp bluez/profile1context_p.h
            bluez/profilemanager1.cpp bluez/profilemanager1_p.h
            bluez/properties.cpp bluez/properties_p.h
            bluez/remotedevicemanager.cpp bluez/remotedevicemanager_p.h
            bluez/servicemap.cpp bluez/servicemap_p.h
            bluez/gattmanager1.cpp bluez/gattmanager1_p.h
            bluez/leadvertisement1adaptor.cpp bluez/leadvertisement1adaptor_p.h
            bluez/leadvertisingmanager1.cpp bluez/leadvertisingmanager1_p.h
            bluez/objectmanageradaptor.cpp bluez/objectmanageradaptor_p.h
            bluez/propertiesadaptor.cpp bluez/propertiesadaptor_p.h
            bluez/gattcharacteristic1adaptor.cpp bluez/gattcharacteristic1adaptor_p.h
            bluez/gattdescriptor1adaptor.cpp bluez/gattdescriptor1adaptor_p.h
            bluez/gattservice1adaptor.cpp bluez/gattservice1adaptor_p.h
            bluez/bluezperipheralapplication.cpp bluez/bluezperipheralapplication_p.h
            bluez/bluezperipheralobjects.cpp bluez/bluezperipheralobjects_p.h
            bluez/bluezperipheralconnectionmanager.cpp bluez/bluezperipheralconnectionmanager_p.h
            qbluetoothdevicediscoveryagent_bluez.cpp
            qbluetoothlocaldevice_bluez.cpp
            qbluetoothserver_bluez.cpp
            qbluetoothservicediscoveryagent_bluez.cpp
            qbluetoothserviceinfo_bluez.cpp
            qbluetoothsocket_bluez.cpp qbluetoothsocket_bluez_p.h
            qbluetoothsocket_bluezdbus.cpp qbluetoothsocket_bluezdbus_p.h
        PUBLIC_LIBRARIES # for Linux QEMU (gcc-armv7) on Linux Ubuntu_20_04 (gcc-x86_64)
            Qt::DBus
    )

    if(QT_FEATURE_bluez_le)
        qt_internal_extend_target(Bluetooth
            SOURCES
                lecmaccalculator.cpp
                qleadvertiser_bluez.cpp qleadvertiser_bluez_p.h
                qleadvertiser_bluezdbus.cpp qleadvertiser_bluezdbus_p.h
                qlowenergycontroller_bluez.cpp qlowenergycontroller_bluez_p.h
                qlowenergycontroller_bluezdbus.cpp qlowenergycontroller_bluezdbus_p.h
        )

        if(QT_FEATURE_linux_crypto_api)
            qt_internal_extend_target(Bluetooth
                DEFINES
                    CONFIG_LINUX_CRYPTO_API
            )
        endif()
    else()
        qt_internal_extend_target(Bluetooth
        SOURCES
            dummy/dummy_helper.cpp dummy/dummy_helper_p.h
            qlowenergycontroller_dummy.cpp qlowenergycontroller_dummy_p.h
        DEFINES
            QT_BLUEZ_NO_BTLE
        )
    endif()

elseif(ANDROID AND NOT ANDROID_EMBEDDED)
    set_property(TARGET Bluetooth APPEND PROPERTY QT_ANDROID_BUNDLED_JAR_DEPENDENCIES
        jar/Qt${QtConnectivity_VERSION_MAJOR}AndroidBluetooth.jar
    )
    qt_internal_add_android_permission(Bluetooth
        NAME android.permission.ACCESS_FINE_LOCATION
        ATTRIBUTES maxSdkVersion 30
    )
    qt_internal_add_android_permission(Bluetooth
        NAME android.permission.BLUETOOTH
        ATTRIBUTES maxSdkVersion 30
    )
    qt_internal_add_android_permission(Bluetooth
        NAME android.permission.BLUETOOTH_ADMIN
        ATTRIBUTES maxSdkVersion 30
    )
    qt_internal_add_android_permission(Bluetooth
        NAME android.permission.BLUETOOTH_SCAN
        ATTRIBUTES
            minSdkVersion 31
            usesPermissionFlags neverForLocation
    )
    qt_internal_add_android_permission(Bluetooth
        NAME android.permission.BLUETOOTH_ADVERTISE
        ATTRIBUTES minSdkVersion 31
    )
    qt_internal_add_android_permission(Bluetooth
        NAME android.permission.BLUETOOTH_CONNECT
        ATTRIBUTES minSdkVersion 31
    )
    qt_internal_extend_target(Bluetooth
        SOURCES
            android/androidbroadcastreceiver.cpp android/androidbroadcastreceiver_p.h
            android/devicediscoverybroadcastreceiver.cpp android/devicediscoverybroadcastreceiver_p.h
            android/inputstreamthread.cpp android/inputstreamthread_p.h
            android/jni_android.cpp android/jni_android_p.h
            android/localdevicebroadcastreceiver.cpp android/localdevicebroadcastreceiver_p.h
            android/lowenergynotificationhub.cpp android/lowenergynotificationhub_p.h
            android/serveracceptancethread.cpp android/serveracceptancethread_p.h
            android/servicediscoverybroadcastreceiver.cpp android/servicediscoverybroadcastreceiver_p.h
            android/androidutils.cpp android/androidutils_p.h
            qbluetoothdevicediscoveryagent_android.cpp
            qbluetoothlocaldevice_android.cpp
            qbluetoothserver_android.cpp
            qbluetoothservicediscoveryagent_android.cpp
            qbluetoothserviceinfo_android.cpp
            qbluetoothsocket_android.cpp qbluetoothsocket_android_p.h
            qlowenergycontroller_android.cpp qlowenergycontroller_android_p.h
        DEFINES
            QT_ANDROID_BLUETOOTH
        LIBRARIES
            Qt::CorePrivate
        PRIVATE_MODULE_INTERFACE
            Qt::CorePrivate
    )
elseif(MACOS)
    qt_internal_extend_target(Bluetooth
        SOURCES
            darwin/btcentralmanager.mm darwin/btcentralmanager_p.h
            darwin/btconnectionmonitor.mm darwin/btconnectionmonitor_p.h
            darwin/btdelegates.cpp darwin/btdelegates_p.h
            darwin/btdeviceinquiry.mm darwin/btdeviceinquiry_p.h
            darwin/btdevicepair.mm darwin/btdevicepair_p.h
            darwin/btgcdtimer.mm darwin/btgcdtimer_p.h
            darwin/btl2capchannel.mm darwin/btl2capchannel_p.h
            darwin/btledeviceinquiry.mm darwin/btledeviceinquiry_p.h
            darwin/btnotifier.cpp darwin/btnotifier_p.h
            darwin/btperipheralmanager.mm darwin/btperipheralmanager_p.h
            darwin/btraii.mm darwin/btraii_p.h
            darwin/btrfcommchannel.mm darwin/btrfcommchannel_p.h
            darwin/btsdpinquiry.mm darwin/btsdpinquiry_p.h
            darwin/btservicerecord.mm darwin/btservicerecord_p.h
            darwin/btsocketlistener.mm darwin/btsocketlistener_p.h
            darwin/btutility.mm darwin/btutility_p.h
            darwin/uistrings.cpp darwin/uistrings_p.h
            qbluetoothdevicediscoveryagent_darwin.mm
            qbluetoothlocaldevice_macos.mm
            qbluetoothserver_macos.mm
            qbluetoothservicediscoveryagent_macos.mm
            qbluetoothserviceinfo_macos.mm
            qbluetoothsocket_macos.mm qbluetoothsocket_macos_p.h
            qlowenergycontroller_darwin.mm qlowenergycontroller_darwin_p.h
            qbluetoothuuid_darwin.mm
        DEFINES
            QT_OSX_BLUETOOTH
        LIBRARIES
            Qt::CorePrivate
            ${FWFoundation}
            ${FWIOBluetooth}
    )
elseif(IOS)
    qt_internal_extend_target(Bluetooth
        SOURCES
            darwin/btcentralmanager.mm darwin/btcentralmanager_p.h
            darwin/btdelegates.cpp darwin/btdelegates_p.h
            darwin/btgcdtimer.mm darwin/btgcdtimer_p.h
            darwin/btledeviceinquiry.mm darwin/btledeviceinquiry_p.h
            darwin/btnotifier.cpp darwin/btnotifier_p.h
            darwin/btraii.mm darwin/btraii_p.h
            darwin/btutility.mm darwin/btutility_p.h
            darwin/uistrings.cpp darwin/uistrings_p.h
            qbluetoothdevicediscoveryagent_darwin.mm
            qbluetoothlocaldevice_p.cpp
            qbluetoothserver_p.cpp
            qbluetoothservicediscoveryagent_p.cpp
            qbluetoothserviceinfo_p.cpp
            qbluetoothsocket_dummy.cpp qbluetoothsocket_dummy_p.h
            qlowenergycontroller_darwin.mm qlowenergycontroller_darwin_p.h
            darwin/btperipheralmanager.mm darwin/btperipheralmanager_p.h
            qbluetoothuuid_darwin.mm
        DEFINES
            QT_IOS_BLUETOOTH
        LIBRARIES
            Qt::CorePrivate
            ${FWCoreBluetooth}
            ${FWFoundation}
    )
elseif(QT_FEATURE_winrt_bt)
    qt_internal_extend_target(Bluetooth
        SOURCES
            qbluetoothdevicediscoveryagent_winrt.cpp
            qbluetoothlocaldevice_winrt.cpp
            qbluetoothserver_winrt.cpp
            qbluetoothservicediscoveryagent_winrt.cpp
            qbluetoothserviceinfo_winrt.cpp
            qbluetoothsocket_winrt.cpp qbluetoothsocket_winrt_p.h
            qbluetoothutils_winrt.cpp qbluetoothutils_winrt_p.h
            qlowenergycontroller_winrt.cpp qlowenergycontroller_winrt_p.h
            qbluetoothdevicewatcher_winrt.cpp qbluetoothdevicewatcher_winrt_p.h
        NO_UNITY_BUILD_SOURCES
            qbluetoothdevicediscoveryagent_winrt.cpp
            qbluetoothdevicewatcher_winrt.cpp
            qbluetoothlocaldevice_winrt.cpp
            qbluetoothservicediscoveryagent_winrt.cpp
            qlowenergycontroller_winrt.cpp
        DEFINES
            QT_WINRT_BLUETOOTH
        LIBRARIES
            Qt::CorePrivate
        PUBLIC_LIBRARIES
            Qt::Core
            runtimeobject
            user32
        PRIVATE_MODULE_INTERFACE
            Qt::CorePrivate
    )
else()
    qt_internal_extend_target(Bluetooth
        SOURCES
            dummy/dummy_helper.cpp dummy/dummy_helper_p.h
            qbluetoothdevicediscoveryagent_p.cpp
            qbluetoothlocaldevice_p.cpp
            qbluetoothserver_p.cpp
            qbluetoothservicediscoveryagent_p.cpp
            qbluetoothserviceinfo_p.cpp
            qbluetoothsocket_dummy.cpp qbluetoothsocket_dummy_p.h
            qlowenergycontroller_dummy.cpp qlowenergycontroller_dummy_p.h
    )
endif()
qt_update_ignore_pch_source(Bluetooth removed_api.cpp)

qt_internal_add_docs(Bluetooth
    doc/qtbluetooth.qdocconf
)

