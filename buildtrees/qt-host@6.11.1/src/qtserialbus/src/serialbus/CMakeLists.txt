# Copyright (C) 2022 The Qt Company Ltd.
# SPDX-License-Identifier: BSD-3-Clause

#####################################################################
## SerialBus Module:
#####################################################################

qt_internal_add_module(SerialBus
    PLUGIN_TYPES canbus
    SOURCES
        qcanbus.cpp qcanbus.h
        qcanbusdevice.cpp qcanbusdevice.h qcanbusdevice_p.h
        qcanbusdeviceinfo.cpp qcanbusdeviceinfo.h qcanbusdeviceinfo_p.h
        qcanbusfactory.cpp qcanbusfactory.h
        qcanbusframe.cpp qcanbusframe.h
        qcancommondefinitions.cpp qcancommondefinitions.h
        qcandbcfileparser.cpp qcandbcfileparser.h qcandbcfileparser_p.h
        qcanframeprocessor.cpp qcanframeprocessor.h qcanframeprocessor_p.h
        qcanmessagedescription.cpp qcanmessagedescription.h qcanmessagedescription_p.h
        qcansignaldescription.cpp qcansignaldescription.h qcansignaldescription_p.h
        qcanuniqueiddescription.cpp qcanuniqueiddescription.h qcanuniqueiddescription_p.h
        qmodbus_symbols_p.h
        qmodbusadu_p.h
        qmodbusclient.cpp qmodbusclient.h qmodbusclient_p.h
        qmodbuscommevent_p.h
        qmodbusdataunit.cpp qmodbusdataunit.h
        qmodbusdevice.cpp qmodbusdevice.h qmodbusdevice_p.h
        qmodbusdeviceidentification.cpp qmodbusdeviceidentification.h
        qmodbuspdu.cpp qmodbuspdu.h
        qmodbusreply.cpp qmodbusreply.h
        qmodbusserver.cpp qmodbusserver.h qmodbusserver_p.h
        qmodbustcpclient.cpp qmodbustcpclient.h qmodbustcpclient_p.h
        qmodbustcpserver.cpp qmodbustcpserver.h qmodbustcpserver_p.h
        qtserialbusglobal.h
    LIBRARIES
        Qt::CorePrivate
    PUBLIC_LIBRARIES
        Qt::Core
        Qt::Network  # TODO: QTBUG-86533 workaround, Qt::Network should be moved to LIBRARIES
    PRIVATE_MODULE_INTERFACE
        Qt::CorePrivate
        Qt::Network
)

## Scopes:
#####################################################################

qt_internal_extend_target(SerialBus CONDITION QT_FEATURE_modbus_serialport
    SOURCES
        qmodbusrtuserialclient.cpp qmodbusrtuserialclient.h qmodbusrtuserialclient_p.h
        qmodbusrtuserialserver.cpp qmodbusrtuserialserver.h qmodbusrtuserialserver_p.h
    PUBLIC_LIBRARIES
        Qt::SerialPort
    PRIVATE_MODULE_INTERFACE
        Qt::SerialPort
)

qt_internal_add_docs(SerialBus
    doc/qtserialbus.qdocconf
)

