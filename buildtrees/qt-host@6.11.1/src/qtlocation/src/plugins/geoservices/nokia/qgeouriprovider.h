// Copyright (C) 2015 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR LGPL-3.0-only OR GPL-2.0-only OR GPL-3.0-only
#ifndef QGEO_MOBILE_COUNTRY_TRACKER_H
#define QGEO_MOBILE_COUNTRY_TRACKER_H

#include <QObject>
#include <QPointer>

QT_BEGIN_NAMESPACE

class QNetworkInfo;

class QGeoUriProvider : public QObject
{
    Q_OBJECT
    Q_DISABLE_COPY(QGeoUriProvider)

public:
    QGeoUriProvider(QObject *parent,
                    const QVariantMap &parameters,
                    const QString &hostParameterName,
                    const QString &internationalHost,
                    const QString &localizedHost = QString());

    QString getCurrentHost() const;

private Q_SLOTS:
    void mobileCountryCodeChanged(int interfaceId, const QString& mcc);

private:
    bool isInternationalNetwork() const;
    void setCurrentHost(const QString &host);

    const QString m_internationalHost;
    const QString m_localizedHost;
    QString m_currentHost;
    QChar m_firstSubdomain;
    unsigned char m_maxSubdomains;
};

QT_END_NAMESPACE

#endif // QGEO_MOBILE_COUNTRY_TRACKER_H
