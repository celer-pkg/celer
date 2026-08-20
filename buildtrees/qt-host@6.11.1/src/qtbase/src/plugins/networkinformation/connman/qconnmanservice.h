// Copyright (C) 2025 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR LGPL-3.0-only OR GPL-2.0-only OR GPL-3.0-only
// Qt-Security score:significant reason:default

#ifndef QCONNMANSERVICE_H
#define QCONNMANSERVICE_H

//
//  W A R N I N G
//  -------------
//
// This file is not part of the Qt API. It exists purely as an
// implementation detail. This header file may change from version to
// version without notice, or even be removed.
//
// We mean it.
//

#include <QtDBus/qdbusabstractinterface.h>

QT_BEGIN_NAMESPACE

class QDBusObjectPath;
class QConnManNetworkInformationBackend;

// This tiny class exists for the purpose of seeing if ConnMan is available without
// initializing everything the derived/full class needs.
class QConnManInterfaceBase : public QDBusAbstractInterface
{
    Q_OBJECT
public:
    explicit QConnManInterfaceBase(QObject *parent = nullptr);
    ~QConnManInterfaceBase() = default;

    static bool connmanAvailable();

private:
    Q_DISABLE_COPY_MOVE(QConnManInterfaceBase)
};

class QConnManInterface final : public QConnManInterfaceBase
{
    Q_OBJECT

public:
    explicit QConnManInterface(QObject *parent = nullptr);
    ~QConnManInterface();

    void setBackend(QConnManNetworkInformationBackend *ourBackend);

    QString state() const;
    QString type() const;

    bool isValid() const { return QDBusAbstractInterface::isValid() && validDBusConnection; }

private Q_SLOTS:
    void propertyChanged(const QString &name, const QDBusVariant &value);

private:
    Q_DISABLE_COPY_MOVE(QConnManInterface)
    QString findServiceType();

    QConnManNetworkInformationBackend *backend = nullptr;
    bool validDBusConnection = true;
    QString m_state;
    QString m_type;
};

QT_END_NAMESPACE

#endif
