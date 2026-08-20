// Copyright (C) 2015 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR LGPL-3.0-only OR GPL-2.0-only OR GPL-3.0-only
#include "qgeouriprovider.h"

#include <QMap>
#include <QVariant>
#include <QRandomGenerator>
#include <QSet>
#include <QString>

QT_BEGIN_NAMESPACE

QGeoUriProvider::QGeoUriProvider(
                QObject *parent,
                const QVariantMap &parameters,
                const QString &hostParameterName,
                const QString &internationalHost,
                const QString &localizedHost)
    : QObject(parent)
    , m_internationalHost(parameters.value(hostParameterName, internationalHost).toString())
    , m_localizedHost(localizedHost)
    , m_firstSubdomain(QChar::Null)
    , m_maxSubdomains(0)
{
    setCurrentHost(isInternationalNetwork() || m_localizedHost.isEmpty() ? m_internationalHost : m_localizedHost);
}

QString QGeoUriProvider::getCurrentHost() const
{
    if (m_maxSubdomains) {
        auto result = QString("%1").arg(static_cast<char>(m_firstSubdomain.toLatin1() + QRandomGenerator::global()->bounded(m_maxSubdomains)));
        result += '.' + m_currentHost;
        return result;
    }
    return m_currentHost;
}

void QGeoUriProvider::setCurrentHost(const QString &host)
{
    if (host.length() > 4 && host.at(1) == QChar('-') && host.at(3) == QChar('.')) {
        QString realHost = host.right(host.length() - 4);
        m_firstSubdomain = host.at(0);
        m_maxSubdomains = host.at(2).toLatin1() - host.at(0).toLatin1() + 1;
        m_currentHost = realHost;
    } else {
        m_currentHost = host;
        m_firstSubdomain = QChar::Null;
        m_maxSubdomains = 0;
    }
}

void QGeoUriProvider::mobileCountryCodeChanged(int interfaceId, const QString& mcc)
{
    Q_UNUSED(interfaceId);
    Q_UNUSED(mcc);

    setCurrentHost(isInternationalNetwork() || m_localizedHost.isEmpty() ? m_internationalHost : m_localizedHost);
}

bool QGeoUriProvider::isInternationalNetwork() const
{
    return true;
}

QT_END_NAMESPACE
