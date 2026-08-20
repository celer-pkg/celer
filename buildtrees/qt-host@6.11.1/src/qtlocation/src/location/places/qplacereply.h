// Copyright (C) 2015 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR LGPL-3.0-only OR GPL-2.0-only OR GPL-3.0-only
// Qt-Security score:significant reason:default

#ifndef QPLACEREPLY_H
#define QPLACEREPLY_H

#include <QtCore/QMetaType>
#include <QtCore/QObject>
#include <QtLocation/qlocationglobal.h>

QT_BEGIN_NAMESPACE

class QPlaceReplyPrivate;
class Q_LOCATION_EXPORT QPlaceReply : public QObject
{
    Q_OBJECT
public:
    enum Error {
        NoError,
        PlaceDoesNotExistError,
        CategoryDoesNotExistError,
        CommunicationError,
        ParseError,
        PermissionsError,
        UnsupportedError,
        BadArgumentError,
        CancelError,
        UnknownError
    };

    enum Type {
        Reply,
        DetailsReply,
        SearchReply,
        SearchSuggestionReply,
        ContentReply,
        IdReply,
        MatchReply
    };

    explicit QPlaceReply(QObject *parent = nullptr);
    ~QPlaceReply();

    bool isFinished() const;

    virtual Type type() const;

    QString errorString() const;
    QPlaceReply::Error error() const;

public Q_SLOTS:
    virtual void abort();

Q_SIGNALS:
    void finished();
    void contentUpdated();
    void aborted();
    void errorOccurred(QPlaceReply::Error error, const QString &errorString = QString());

protected:
    explicit QPlaceReply(QPlaceReplyPrivate *, QObject *parent = nullptr);
    void setFinished(bool finished);
    void setError(QPlaceReply::Error error, const QString &errorString);
    QPlaceReplyPrivate *d_ptr;

private:
    Q_DISABLE_COPY(QPlaceReply)
};

QT_END_NAMESPACE

Q_DECLARE_METATYPE(QPlaceReply::Error)
Q_DECLARE_METATYPE(QPlaceReply *)

#endif // QPLACEREPLY_H
