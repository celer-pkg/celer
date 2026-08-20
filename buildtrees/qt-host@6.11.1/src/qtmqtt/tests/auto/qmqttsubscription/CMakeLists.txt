# Copyright (C) 2022 The Qt Company Ltd.
# SPDX-License-Identifier: BSD-3-Clause

#####################################################################
## tst_qmqttsubscription Test:
#####################################################################

qtmqtt_add_test(
    TARGET
        tst_qmqttsubscription
    SOURCES
        ../../common/broker_connection.h
        tst_qmqttsubscription.cpp
    DEFINES
        SRCDIR="${CMAKE_CURRENT_SOURCE_DIR}/"
    INCLUDE_DIRECTORIES
        ../../common
    LIBRARIES
        Qt::MqttPrivate
        Qt::Mqtt
        Qt::Network
)
