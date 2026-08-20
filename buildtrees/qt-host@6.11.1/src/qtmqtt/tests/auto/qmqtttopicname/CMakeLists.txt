# Copyright (C) 2022 The Qt Company Ltd.
# SPDX-License-Identifier: BSD-3-Clause

#####################################################################
## tst_qmqtttopicname Test:
#####################################################################

qtmqtt_add_test(
    TARGET
        tst_qmqtttopicname
    SOURCES
        tst_qmqtttopicname.cpp
    DEFINES
        SRCDIR="${CMAKE_CURRENT_SOURCE_DIR}/"
    LIBRARIES
        Qt::MqttPrivate
        Qt::Mqtt
)
