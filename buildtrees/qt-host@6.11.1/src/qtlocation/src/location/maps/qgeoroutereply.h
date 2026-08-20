// Copyright (C) 2015 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR LGPL-3.0-only OR GPL-2.0-only OR GPL-3.0-only
// Qt-Security score:significant reason:default

#ifndef QGEOROUTEREPLY_H
#define QGEOROUTEREPLY_H

#include <QtLocation/QGeoRoute>

#include <QtCore/QList>
#include <QtCore/QObject>

QT_BEGIN_NAMESPACE

class QGeoRouteRequest;
class QGeoRouteReplyPrivate;

class Q_LOCATION_EXPORT QGeoRouteReply : public QObject
{
    Q_OBJECT
public:
    enum Error {
        NoError,
        EngineNotSetError,
        CommunicationError,
        ParseError,
        UnsupportedOptionError,
        UnknownError
    };
    Q_ENUM(Error)

    explicit QGeoRouteReply(Error error, const QString &errorString, QObject *parent = nullptr);
    virtual ~QGeoRouteReply();

    bool isFinished() const;
    Error error() const;
    QString errorString() const;

    QGeoRouteRequest request() const;
    QList<QGeoRoute> routes() const;

    virtual void abort();

Q_SIGNALS:
    void finished();
    void aborted();
    void errorOccurred(QGeoRouteReply::Error error, const QString &errorString = QString());

protected:
    explicit QGeoRouteReply(const QGeoRouteRequest &request, QObject *parent = nullptr);

    void setError(Error error, const QString &errorString);
    void setFinished(bool finished);

    void setRoutes(const QList<QGeoRoute> &routes);
    void addRoutes(const QList<QGeoRoute> &routes);

private:
    QGeoRouteReplyPrivate *d_ptr;
    Q_DISABLE_COPY(QGeoRouteReply)
};

QT_END_NAMESPACE

#endif
