// Copyright (C) 2015 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR LGPL-3.0-only OR GPL-2.0-only OR GPL-3.0-only
// Qt-Security score:significant reason:default

#ifndef QGEOTILEDMAPREPLY_H
#define QGEOTILEDMAPREPLY_H

//
//  W A R N I N G
//  -------------
//
// This file is not part of the Qt API.  It exists purely as an
// implementation detail.  This header file may change from version to
// version without notice, or even be removed.
//
// We mean it.
//

#include <QtLocation/private/qlocationglobal_p.h>

#include <QObject>

QT_BEGIN_NAMESPACE

class QGeoTileSpec;
class QGeoTiledMapReplyPrivate;

class Q_LOCATION_EXPORT QGeoTiledMapReply : public QObject
{
    Q_OBJECT

public:
    enum Error {
        NoError,
        CommunicationError,
        ParseError,
        UnknownError
    };

    QGeoTiledMapReply(const QGeoTileSpec &spec, QObject *parent = nullptr);
    QGeoTiledMapReply(Error error, const QString &errorString, QObject *parent = nullptr);
    virtual ~QGeoTiledMapReply();

    bool isFinished() const;
    Error error() const;
    QString errorString() const;

    bool isCached() const;

    QGeoTileSpec tileSpec() const;

    QByteArray mapImageData() const;
    QString mapImageFormat() const;

    virtual void abort();

Q_SIGNALS:
    void finished();
    void aborted();
    void errorOccurred(QGeoTiledMapReply::Error error, const QString &errorString = QString());

protected:
    void setError(Error error, const QString &errorString);
    void setFinished(bool finished);

    void setCached(bool cached);

    void setMapImageData(const QByteArray &data);
    void setMapImageFormat(const QString &format);

private:
    QGeoTiledMapReplyPrivate *d_ptr;
    Q_DISABLE_COPY(QGeoTiledMapReply)
};

QT_END_NAMESPACE

#endif
