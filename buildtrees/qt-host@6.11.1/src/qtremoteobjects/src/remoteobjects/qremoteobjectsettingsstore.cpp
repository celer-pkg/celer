// Copyright (C) 2017 Ford Motor Company
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR LGPL-3.0-only OR GPL-2.0-only OR GPL-3.0-only
// Qt-Security score:significant reason:default

#include "qremoteobjectsettingsstore.h"

#include "qremoteobjectnode_p.h"

#include <QtCore/private/qobject_p.h>
#include <QtCore/qsettings.h>

QT_BEGIN_NAMESPACE

/*!
    \qmltype SettingsStore
    \inqmlmodule QtRemoteObjects
    \brief A basic store for persisted properties.

    This type provides simple QSettings-based storage for properties marked as PERSISTED. It is used in
    conjunction with Node::persistedStore:

    \code
    Node {
        persistedStore: SettingsStore {}
    }
    \endcode
*/

class QRemoteObjectSettingsStorePrivate : public QRemoteObjectAbstractPersistedStorePrivate
{
public:
    QRemoteObjectSettingsStorePrivate();
    virtual ~QRemoteObjectSettingsStorePrivate();

    QSettings settings;
    Q_DECLARE_PUBLIC(QRemoteObjectSettingsStore)
};

QRemoteObjectSettingsStorePrivate::QRemoteObjectSettingsStorePrivate()
{
}

QRemoteObjectSettingsStorePrivate::~QRemoteObjectSettingsStorePrivate()
{
}

QRemoteObjectSettingsStore::QRemoteObjectSettingsStore(QObject *parent)
    : QRemoteObjectAbstractPersistedStore(*new QRemoteObjectSettingsStorePrivate, parent)
{
}

QRemoteObjectSettingsStore::~QRemoteObjectSettingsStore()
{
}

QVariantList QRemoteObjectSettingsStore::restoreProperties(const QString &repName, const QByteArray &repSig)
{
    Q_D(QRemoteObjectSettingsStore);
    d->settings.beginGroup(repName + QLatin1Char('/') + QString::fromLatin1(repSig));
    QVariantList values = d->settings.value(QStringLiteral("values")).toList();
    d->settings.endGroup();
    return values;
}

void QRemoteObjectSettingsStore::saveProperties(const QString &repName, const QByteArray &repSig, const QVariantList &values)
{
    Q_D(QRemoteObjectSettingsStore);
    d->settings.beginGroup(repName + QLatin1Char('/') + QString::fromLatin1(repSig));
    d->settings.setValue(QStringLiteral("values"), values);
    d->settings.endGroup();
    d->settings.sync();
}

QT_END_NAMESPACE
