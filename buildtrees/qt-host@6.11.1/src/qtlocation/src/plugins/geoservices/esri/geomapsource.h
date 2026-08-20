// Copyright (C) 2013-2018 Esri <contracts@esri.com>
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR LGPL-3.0-only OR GPL-2.0-only OR GPL-3.0-only

#ifndef GEOMAPSOURCE_H
#define GEOMAPSOURCE_H

#include <QtLocation/private/qgeomaptype_p.h>
#include <QtLocation/private/qgeocameracapabilities_p.h>

QT_BEGIN_NAMESPACE

class GeoMapSource : public QGeoMapType
{
public:
    GeoMapSource(QGeoMapType::MapStyle style, const QString &name,
                 const QString &description, bool mobile, bool night, int mapId,
                 const QString &url, const QString &copyright, const QGeoCameraCapabilities &cameraCapabilities);

    inline const QString &url() const;
    inline const QString &copyright() const;

    static QString toFormat(const QString &url);
    static QGeoMapType::MapStyle mapStyle(const QString &styleString);

private:
    QString m_url;
    QString m_copyright;
};

inline const QString &GeoMapSource::url() const
{
    return m_url;
}

inline const QString &GeoMapSource::copyright() const
{
    return m_copyright;
}

QT_END_NAMESPACE

#endif // GEOMAPSOURCE_H
