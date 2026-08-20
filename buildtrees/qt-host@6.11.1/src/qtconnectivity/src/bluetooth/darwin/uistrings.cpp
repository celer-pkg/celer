// Copyright (C) 2022 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR LGPL-3.0-only OR GPL-2.0-only OR GPL-3.0-only

#include "uistrings_p.h"

#include <qttranslation.h>

// Translatable messages should go into this .cpp file for them to
// be picked up by lupdate.

QT_BEGIN_NAMESPACE

const char DEV_DISCOVERY[] = "QBluetoothDeviceDiscoveryAgent";
const char DD_POWERED_OFF[] = QT_TRANSLATE_NOOP("QBluetoothDeviceDiscoveryAgent", "Device is powered off");
const char DD_INVALID_ADAPTER[] = QT_TRANSLATE_NOOP("QBluetoothDeviceDiscoveryAgent", "Cannot find valid Bluetooth adapter.");
const char DD_IO[] = QT_TRANSLATE_NOOP("QBluetoothDeviceDiscoveryAgent", "Input Output Error");
const char DD_NOTSUPPORTED[] = QT_TRANSLATE_NOOP("QBluetoothDeviceDiscoveryAgent", "Bluetooth LE is not supported");
const char DD_UNKNOWN_ERROR[] = QT_TRANSLATE_NOOP("QBluetoothDeviceDiscoveryAgent", "Unknown error");
const char DD_MISSING_PERMISSION[] = QT_TRANSLATE_NOOP("QBluetoothDeviceDiscoveryAgent", "Missing permission");
const char DD_NOT_STARTED[] = QT_TRANSLATE_NOOP("QBluetoothDeviceDiscoveryAgent", "Cannot start device inquiry");
const char DD_NOT_STARTED_LE[] = QT_TRANSLATE_NOOP("QBluetoothDeviceDiscoveryAgent", "Cannot start low energy device inquiry");
const char DD_NOT_STOPPED[] = QT_TRANSLATE_NOOP("QBluetoothDeviceDiscoveryAgent", "Discovery cannot be stopped");

const char SERVICE_DISCOVERY[] = "QBluetoothServiceDiscoveryAgent";
const char SD_LOCAL_DEV_OFF[] = QT_TRANSLATE_NOOP("QBluetoothServiceDiscoveryAgent", "Local device is powered off");
const char SD_MINIMAL_FAILED[] = QT_TRANSLATE_NOOP("QBluetoothServiceDiscoveryAgent", "Minimal service discovery failed");
const char SD_INVALID_ADDRESS[] = QT_TRANSLATE_NOOP("QBluetoothServiceDiscoveryAgent", "Invalid Bluetooth adapter address");

const char SOCKET[] = "QBluetoothSocket";
const char SOC_NETWORK_ERROR[] = QT_TRANSLATE_NOOP("QBluetoothSocket", "Network Error");
const char SOC_NOWRITE[] = QT_TRANSLATE_NOOP("QBluetoothSocket", "Cannot write while not connected");
const char SOC_CONNECT_IN_PROGRESS[] = QT_TRANSLATE_NOOP("QBluetoothSocket", "Trying to connect while connection is in progress");
const char SOC_SERVICE_NOT_FOUND[] = QT_TRANSLATE_NOOP("QBluetoothSocket", "Service cannot be found");
const char SOC_INVAL_DATASIZE[] = QT_TRANSLATE_NOOP("QBluetoothSocket", "Invalid data/data size");
const char SOC_NOREAD[] = QT_TRANSLATE_NOOP("QBluetoothSocket", "Cannot read while not connected");

const char TRANSFER_REPLY[] = "QBluetoothTransferReply";
const char TR_INVAL_TARGET[] = QT_TRANSLATE_NOOP("QBluetoothTransferReply", "Invalid target address");
const char TR_SESSION_NO_START[] = QT_TRANSLATE_NOOP("QBluetoothTransferReply", "Push session cannot be started");
const char TR_CONNECT_FAILED[] = QT_TRANSLATE_NOOP("QBluetoothTransferReply", "Push session cannot connect");
const char TR_FILE_NOT_EXIST[] = QT_TRANSLATE_NOOP("QBluetoothTransferReply", "Source file does not exist");
const char TR_NOT_READ_IODEVICE[] = QT_TRANSLATE_NOOP("QBluetoothTransferReply", "QIODevice cannot be read. Make sure it is open for reading.");
const char TR_SESSION_FAILED[] = QT_TRANSLATE_NOOP("QBluetoothTransferReply", "Push session failed");
const char TR_INVALID_DEVICE[] = QT_TRANSLATE_NOOP("QBluetoothTransferReply", "Invalid input device (null)");
const char TR_OP_CANCEL[] = QT_TRANSLATE_NOOP("QBluetoothTransferReply", "Operation canceled");
const char TR_IN_PROGRESS[] = QT_TRANSLATE_NOOP("QBluetoothTransferReply", "Transfer already started");
const char TR_SERVICE_NO_FOUND[] = QT_TRANSLATE_NOOP("QBluetoothTransferReply", "Push service not found");

const char LE_CONTROLLER[] = "QLowEnergyController";
const char LEC_RDEV_NO_FOUND[] = QT_TRANSLATE_NOOP("QLowEnergyController", "Remote device cannot be found");
const char LEC_NO_LOCAL_DEV[] = QT_TRANSLATE_NOOP("QLowEnergyController", "Cannot find local adapter");
const char LEC_IO_ERROR[] = QT_TRANSLATE_NOOP("QLowEnergyController", "Error occurred during connection I/O");
const char LEC_UNKNOWN_ERROR[] = QT_TRANSLATE_NOOP("QLowEnergyController", "Unknown Error");
const char LEC_MISSING_PERMISSION[] = QT_TRANSLATE_NOOP("QLowEnergyController", "Missing permission");

QT_END_NAMESPACE
