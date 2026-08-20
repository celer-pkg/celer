# Copyright (C) 2022 The Qt Company Ltd.
# SPDX-License-Identifier: BSD-3-Clause



#### Inputs



#### Libraries



#### Tests


qt_config_compile_test("socketcan"
                   LABEL "Socket CAN"
                   PROJECT_PATH "${CMAKE_CURRENT_SOURCE_DIR}/../../config.tests/socketcan"
)

qt_config_compile_test("socketcan_fd"
                   LABEL "Socket CAN FD"
                   PROJECT_PATH "${CMAKE_CURRENT_SOURCE_DIR}/../../config.tests/socketcan_fd"
)


#### Features

qt_feature("socketcan" PRIVATE
    LABEL "Socket CAN"
    CONDITION LINUX AND TEST_socketcan
)
qt_feature("socketcan_fd" PRIVATE
    LABEL "Socket CAN FD"
    CONDITION LINUX AND QT_FEATURE_socketcan AND TEST_socketcan_fd
)
qt_feature("modbus-serialport" PUBLIC
    LABEL "SerialPort Support"
    PURPOSE "Enables Serial-based Modbus Support"
    CONDITION TARGET Qt::SerialPort
)
qt_configure_add_summary_section(NAME "Qt SerialBus")
qt_configure_add_summary_entry(ARGS "socketcan")
qt_configure_add_summary_entry(ARGS "socketcan_fd")
qt_configure_add_summary_entry(ARGS "modbus-serialport")
qt_configure_end_summary_section() # end of "Qt SerialBus" section
qt_configure_add_report_entry(
    TYPE NOTE
    MESSAGE "QtSerialBus: Cannot find linux/can.h and linux/can/raw.h Linux headers for socketCAN support."
    CONDITION LINUX AND NOT QT_FEATURE_socketcan
)
qt_configure_add_report_entry(
    TYPE NOTE
    MESSAGE "QtSerialBus: Newer kernel needed for flexible data-rate frame support (canfd_frame)."
    CONDITION LINUX AND QT_FEATURE_socketcan AND NOT QT_FEATURE_socketcan_fd
)
