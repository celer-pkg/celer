// Copyright (C) 2015 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR LGPL-3.0-only OR GPL-2.0-only OR GPL-3.0-only
// Qt-Security score:significant reason:default

#ifndef QGEOCODEREPLY_H
#define QGEOCODEREPLY_H

#include <QtLocation/qlocationglobal.h>
#include <QtPositioning/QGeoShape>
#include <QtCore/QObject>
#include <QtCore/QList>

QT_BEGIN_NAMESPACE

class QGeoLocation;
class QGeoCodeReplyPrivate;

class Q_LOCATION_EXPORT QGeoCodeReply : public QObject
{
    Q_OBJECT

public:
    enum Error {
        NoError,
        EngineNotSetError,
        CommunicationError,
        ParseError,
        UnsupportedOptionError,
        CombinationError,
        UnknownError
    };

    explicit QGeoCodeReply(Error error, const QString &errorString, QObject *parent = nullptr);
    virtual ~QGeoCodeReply();

    bool isFinished() const;
    Error error() const;
    QString errorString() const;

    QGeoShape viewport() const;
    QList<QGeoLocation> locations() const;

    qsizetype limit() const;
    qsizetype offset() const;

    virtual void abort();

Q_SIGNALS:
    void finished();
    void aborted();
    void errorOccurred(QGeoCodeReply::Error error, const QString &errorString = QString());

protected:
    explicit QGeoCodeReply(QObject *parent = nullptr);

    void setError(Error error, const QString &errorString);
    void setFinished(bool finished);

    void setViewport(const QGeoShape &viewport);
    void addLocation(const QGeoLocation &location);
    void setLocations(const QList<QGeoLocation> &locations);

    void setLimit(qsizetype limit);
    void setOffset(qsizetype offset);

private:
    QGeoCodeReplyPrivate *d_ptr;
    Q_DISABLE_COPY(QGeoCodeReply)
    friend class QGeoCodeReplyPrivate;
};

QT_END_NAMESPACE

#endif
