// Copyright (C) 2022 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR BSD-3-Clause

#include "bluetoothbaseclass.h"

BluetoothBaseClass::BluetoothBaseClass(QObject *parent) : QObject(parent)
{
}

QString BluetoothBaseClass::error() const
{
    return m_error;
}

QString BluetoothBaseClass::info() const
{
    return m_info;
}

void BluetoothBaseClass::setError(const QString &error)
{
    if (m_error != error) {
        m_error = error;
        emit errorChanged();
    }
}

void BluetoothBaseClass::setInfo(const QString &info)
{
    if (m_info != info) {
        m_info = info;
        emit infoChanged();
    }
}

BluetoothBaseClass::IconType BluetoothBaseClass::icon() const
{
    return m_icon;
}

void BluetoothBaseClass::setIcon(IconType icon)
{
    if (m_icon != icon) {
        m_icon = icon;
        emit iconChanged();
    }
}

void BluetoothBaseClass::clearMessages()
{
    setInfo("");
    setError("");
    setIcon(IconNone);
}
