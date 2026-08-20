# Copyright (C) 2022 The Qt Company Ltd.
# SPDX-License-Identifier: BSD-3-Clause

#####################################################################
## Nfc Module:
#####################################################################

qt_internal_add_module(Nfc
    SOURCES
        qndeffilter.cpp qndeffilter.h
        qndefmessage.cpp qndefmessage.h
        qndefnfcsmartposterrecord.cpp qndefnfcsmartposterrecord.h qndefnfcsmartposterrecord_p.h
        qndefnfctextrecord.cpp qndefnfctextrecord.h
        qndefnfcurirecord.cpp qndefnfcurirecord.h
        qndefrecord.cpp qndefrecord.h qndefrecord_p.h
        qnearfieldmanager.cpp qnearfieldmanager.h qnearfieldmanager_p.h
        qnearfieldtarget.cpp qnearfieldtarget.h qnearfieldtarget_p.cpp qnearfieldtarget_p.h
        qtnfcglobal.h qtnfcglobal_p.h
    PUBLIC_LIBRARIES
        Qt::Core
)

## Scopes:
#####################################################################

if(ANDROID AND (ANDROID AND NOT ANDROID_EMBEDDED))
    set_property(TARGET Nfc APPEND PROPERTY QT_ANDROID_BUNDLED_JAR_DEPENDENCIES
        jar/Qt${QtConnectivity_VERSION_MAJOR}AndroidNfc.jar
    )
    qt_internal_add_android_permission(Nfc NAME android.permission.NFC)
    set(NFC_BACKEND_AVAILABLE ON)
endif()

qt_internal_extend_target(Nfc CONDITION ANDROID AND NOT ANDROID_EMBEDDED
    SOURCES
        android/androidjninfc.cpp android/androidjninfc_p.h
        android/androidmainnewintentlistener.cpp android/androidmainnewintentlistener_p.h
        qnearfieldmanager_android.cpp qnearfieldmanager_android_p.h
        qnearfieldtarget_android.cpp qnearfieldtarget_android_p.h
    DEFINES
        ANDROID_NFC
        QT_ANDROID_NFC
    LIBRARIES
        Qt::CorePrivate
        Qt::Gui
)

if(IOS)
    # special case begin
    set(NFC_BACKEND_AVAILABLE ON)
    qt_disable_apple_app_extension_api_only(Nfc)
    qt_internal_find_apple_system_framework(FWCoreNFC CoreNFC)
    # special case end
endif()

qt_internal_extend_target(Nfc CONDITION IOS
    SOURCES
        ios/qiostagreaderdelegate.mm ios/qiostagreaderdelegate_p.h
        ios/qiosnfcndefsessiondelegate.mm ios/qiosnfcndefsessiondelegate_p.h
        ios/qiosndefnotifier.cpp ios/qiosndefnotifier_p.h
        qnearfieldmanager_ios.mm qnearfieldmanager_ios_p.h
        qnearfieldtarget_ios.mm qnearfieldtarget_ios_p.h
    DEFINES
        IOS_NFC
    LIBRARIES
        ${FWCoreNFC}
        Qt::CorePrivate
)

if(QT_FEATURE_pcsclite)
    set(NFC_BACKEND_AVAILABLE ON)
endif()

qt_internal_extend_target(Nfc CONDITION QT_FEATURE_pcsclite
    SOURCES
        qnearfieldmanager_pcsc.cpp qnearfieldmanager_pcsc_p.h
        qnearfieldtarget_pcsc.cpp qnearfieldtarget_pcsc_p.h
        qapduutils.cpp qapduutils_p.h
        pcsc/qpcsc.cpp pcsc/qpcsc_p.h
        pcsc/qpcscmanager.cpp pcsc/qpcscmanager_p.h
        pcsc/qpcscslot.cpp pcsc/qpcscslot_p.h
        pcsc/qpcsccard.cpp pcsc/qpcsccard_p.h
        ndef/qndefaccessfsm_p.h
        ndef/qnfctagtype4ndeffsm.cpp ndef/qnfctagtype4ndeffsm_p.h
    DEFINES
        PCSC_NFC
    LIBRARIES
        PkgConfig::PCSCLITE
)

if(QT_FEATURE_neard)
    set(NFC_BACKEND_AVAILABLE ON)
endif()

qt_internal_extend_target(Nfc CONDITION QT_FEATURE_neard
    SOURCES
        qnearfieldmanager_neard.cpp qnearfieldmanager_neard_p.h
        qnearfieldtarget_neard_p.cpp qnearfieldtarget_neard_p.h
        neard/neard_helper.cpp neard/neard_helper_p.h
        neard/neard_dbus_types_p.h
    DBUS_INTERFACE_SOURCES
        neard/org.freedesktop.dbus.objectmanager.xml
        neard/org.freedesktop.dbus.properties.xml
        neard/org.neard.Adapter.xml
        neard/org.neard.Tag.xml
    DBUS_INTERFACE_FLAGS
        -i neard/neard_dbus_types_p.h
        --namespace QtNfcPrivate
    DEFINES
        NEARD_NFC
    LIBRARIES
        Qt::DBus
)

qt_internal_extend_target(Nfc CONDITION NOT NFC_BACKEND_AVAILABLE
    SOURCES
        qnearfieldmanager_generic.cpp qnearfieldmanager_generic_p.h
)
qt_internal_add_docs(Nfc
    doc/qtnfc.qdocconf
)
