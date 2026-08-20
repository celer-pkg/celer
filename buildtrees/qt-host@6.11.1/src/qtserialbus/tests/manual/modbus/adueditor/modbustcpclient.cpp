// Copyright (C) 2017 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR GPL-3.0-only

#include "mainwindow.h"
#include "modbustcpclient.h"
#include "modbustcpclient_p.h"

QT_USE_NAMESPACE

ModbusTcpClient::ModbusTcpClient(QObject *parent)
    : QModbusTcpClient(*new ModbusTcpClientPrivate, parent)
{
}

ModbusTcpClient::ModbusTcpClient(ModbusTcpClientPrivate &dd, QObject *parent)
    : QModbusTcpClient(dd, parent)
{
}

void ModbusTcpClient::valueChanged(int value)
{
    Q_UNUSED(value);
    Q_D(ModbusTcpClient);

    if (auto instance = MainWindow::instance()) {
        d->m_tId = quint16(instance->ti1Spin->value() << 8) | quint8(instance->ti2Spin->value());
        d->m_pId = quint16(instance->pi1Spin->value() << 8) | quint8(instance->pi2Spin->value());
        d->m_length = quint16(instance->l1Spin->value() << 8) | quint8(instance->l2Spin->value());
        d->m_uId = quint8(instance->ui1Spin->value());
    }
}
