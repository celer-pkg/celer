// Copyright (C) 2015 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR LGPL-3.0-only OR GPL-2.0-only OR GPL-3.0-only

#ifndef QGEOCODEJSONPARSER_H
#define QGEOCODEJSONPARSER_H

#include <QtPositioning/QGeoShape>
#include <QtPositioning/QGeoLocation>

#include <QtCore/QObject>
#include <QtCore/QRunnable>
#include <QtCore/QJsonDocument>
#include <QtCore/QByteArray>
#include <QtCore/QString>
#include <QtCore/QList>

QT_BEGIN_NAMESPACE

class QGeoCodeJsonParser : public QObject, public QRunnable
{
    Q_OBJECT

public:
    void setBounds(const QGeoShape &bounds);
    void parse(const QByteArray &data);
    void run() override;

signals:
    void results(const QList<QGeoLocation> &locations);
    void errorOccurred(const QString &errorString);

private:
    QJsonDocument m_document;
    QByteArray m_data;
    QGeoShape m_bounds;
    QList<QGeoLocation> m_results;
    QString m_errorString;
};

QT_END_NAMESPACE

#endif
