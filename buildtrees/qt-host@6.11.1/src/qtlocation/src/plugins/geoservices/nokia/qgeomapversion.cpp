// Copyright (C) 2014 Appello Systems AB.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR LGPL-3.0-only OR GPL-2.0-only OR GPL-3.0-only

#include "qgeomapversion.h"

#include <QJsonDocument>

QT_BEGIN_NAMESPACE

QGeoMapVersion::QGeoMapVersion()
    : m_version(-1) {}

bool QGeoMapVersion::isNewVersion(const QJsonObject &newVersionData) const
{
    return m_versionData != newVersionData;
}

int QGeoMapVersion::version() const
{
    return m_version;
}

void QGeoMapVersion::setVersion(int version)
{
    m_version = version;
}

void QGeoMapVersion::setVersionData(const QJsonObject &versionData)
{
    m_versionData = versionData;
}


QByteArray QGeoMapVersion::toJson() const
{

    QJsonObject object;
    object[QLatin1String("version")] = m_version;
    object[QLatin1String("data")] = m_versionData;

    QJsonDocument document(object);

    return document.toJson();
}

QT_END_NAMESPACE
