// Copyright (C) 2015 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR LGPL-3.0-only OR GPL-2.0-only OR GPL-3.0-only
// Qt-Security score:significant reason:default

#ifndef UNSUPPORTEDREPLIES_P_H
#define UNSUPPORTEDREPLIES_P_H

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
#include "qplacedetailsreply.h"
#include "qplacecontentreply.h"
#include "qplacesearchreply.h"
#include "qplacesearchsuggestionreply.h"
#include "qplaceidreply.h"

#include "qplacematchreply.h"
#include "qplacemanagerengine.h"

QT_BEGIN_NAMESPACE

class Q_LOCATION_EXPORT QPlaceDetailsReplyUnsupported : public QPlaceDetailsReply
{
    Q_OBJECT

public:
    QPlaceDetailsReplyUnsupported(QPlaceManagerEngine *parent)
    :   QPlaceDetailsReply(parent)
    {
        setError(QPlaceReply::UnsupportedError,
                 QStringLiteral("Getting place details is not supported."));
        setFinished(true);
        QMetaObject::invokeMethod(this, "errorOccurred", Qt::QueuedConnection,
                                  Q_ARG(QPlaceReply::Error, error()),
                                  Q_ARG(QString, errorString()));
        QMetaObject::invokeMethod(parent, "errorOccurred", Qt::QueuedConnection,
                                  Q_ARG(QPlaceReply *, this),
                                  Q_ARG(QPlaceReply::Error, error()),
                                  Q_ARG(QString, errorString()));
        QMetaObject::invokeMethod(this, "finished", Qt::QueuedConnection);
        QMetaObject::invokeMethod(parent, "finished", Qt::QueuedConnection,
                                  Q_ARG(QPlaceReply *, this));
    }
};

class Q_LOCATION_EXPORT QPlaceContentReplyUnsupported : public QPlaceContentReply
{
    Q_OBJECT

public:
    QPlaceContentReplyUnsupported(QPlaceManagerEngine *parent)
    :   QPlaceContentReply(parent)
    {
        setError(QPlaceReply::UnsupportedError,
                 QStringLiteral("Place content is not supported."));
        setFinished(true);
        QMetaObject::invokeMethod(this, "errorOccurred", Qt::QueuedConnection,
                                  Q_ARG(QPlaceReply::Error, error()),
                                  Q_ARG(QString, errorString()));
        QMetaObject::invokeMethod(parent, "errorOccurred", Qt::QueuedConnection,
                                  Q_ARG(QPlaceReply *, this),
                                  Q_ARG(QPlaceReply::Error, error()),
                                  Q_ARG(QString, errorString()));
        QMetaObject::invokeMethod(this, "finished", Qt::QueuedConnection);
        QMetaObject::invokeMethod(parent, "finished", Qt::QueuedConnection,
                                  Q_ARG(QPlaceReply *, this));
    }
};

class Q_LOCATION_EXPORT QPlaceSearchReplyUnsupported : public QPlaceSearchReply
{
    Q_OBJECT

public:
    QPlaceSearchReplyUnsupported(QPlaceReply::Error errorCode, const QString &message,
                                 QPlaceManagerEngine *parent)
    :   QPlaceSearchReply(parent)
    {
        setError(errorCode, message);
        setFinished(true);
        QMetaObject::invokeMethod(this, "errorOccurred", Qt::QueuedConnection,
                                  Q_ARG(QPlaceReply::Error, error()),
                                  Q_ARG(QString, errorString()));
        QMetaObject::invokeMethod(parent, "errorOccurred", Qt::QueuedConnection,
                                  Q_ARG(QPlaceReply *, this),
                                  Q_ARG(QPlaceReply::Error, error()),
                                  Q_ARG(QString, errorString()));
        QMetaObject::invokeMethod(this, "finished", Qt::QueuedConnection);
        QMetaObject::invokeMethod(parent, "finished", Qt::QueuedConnection,
                                  Q_ARG(QPlaceReply *, this));
    }
};

class Q_LOCATION_EXPORT QPlaceSearchSuggestionReplyUnsupported : public QPlaceSearchSuggestionReply
{
    Q_OBJECT

public:
    QPlaceSearchSuggestionReplyUnsupported(QPlaceManagerEngine *parent)
    :   QPlaceSearchSuggestionReply(parent)
    {
        setError(QPlaceReply::UnsupportedError,
                 QStringLiteral("Place search suggestions are not supported."));
        setFinished(true);
        QMetaObject::invokeMethod(this, "errorOccurred", Qt::QueuedConnection,
                                  Q_ARG(QPlaceReply::Error, error()),
                                  Q_ARG(QString, errorString()));
        QMetaObject::invokeMethod(parent, "errorOccurred", Qt::QueuedConnection,
                                  Q_ARG(QPlaceReply *, this),
                                  Q_ARG(QPlaceReply::Error, error()),
                                  Q_ARG(QString, errorString()));
        QMetaObject::invokeMethod(this, "finished", Qt::QueuedConnection);
        QMetaObject::invokeMethod(parent, "finished", Qt::QueuedConnection,
                                  Q_ARG(QPlaceReply *, this));
    }
};

class Q_LOCATION_EXPORT QPlaceIdReplyUnsupported : public QPlaceIdReply
{
    Q_OBJECT

public:
    QPlaceIdReplyUnsupported(const QString &message, QPlaceIdReply::OperationType type,
                             QPlaceManagerEngine *parent)
    :   QPlaceIdReply(type, parent)
    {
        setError(QPlaceReply::UnsupportedError, message);
        setFinished(true);
        QMetaObject::invokeMethod(this, "errorOccurred", Qt::QueuedConnection,
                                  Q_ARG(QPlaceReply::Error, error()),
                                  Q_ARG(QString, errorString()));
        QMetaObject::invokeMethod(parent, "errorOccurred", Qt::QueuedConnection,
                                  Q_ARG(QPlaceReply *, this),
                                  Q_ARG(QPlaceReply::Error, error()),
                                  Q_ARG(QString, errorString()));
        QMetaObject::invokeMethod(this, "finished", Qt::QueuedConnection);
        QMetaObject::invokeMethod(parent, "finished", Qt::QueuedConnection,
                                  Q_ARG(QPlaceReply *, this));
    }
};

class Q_LOCATION_EXPORT QPlaceReplyUnsupported : public QPlaceReply
{
    Q_OBJECT

public:
    QPlaceReplyUnsupported(const QString &message, QPlaceManagerEngine *parent)
    :   QPlaceReply(parent)
    {
        setError(QPlaceReply::UnsupportedError, message);
        setFinished(true);
        QMetaObject::invokeMethod(this, "errorOccurred", Qt::QueuedConnection,
                                  Q_ARG(QPlaceReply::Error, error()),
                                  Q_ARG(QString, errorString()));
        QMetaObject::invokeMethod(parent, "errorOccurred", Qt::QueuedConnection,
                                  Q_ARG(QPlaceReply *, this),
                                  Q_ARG(QPlaceReply::Error, error()),
                                  Q_ARG(QString, errorString()));
        QMetaObject::invokeMethod(this, "finished", Qt::QueuedConnection);
        QMetaObject::invokeMethod(parent, "finished", Qt::QueuedConnection,
                                  Q_ARG(QPlaceReply *, this));
    }
};

class Q_LOCATION_EXPORT QPlaceMatchReplyUnsupported : public QPlaceMatchReply
{
    Q_OBJECT

public:
    QPlaceMatchReplyUnsupported(QPlaceManagerEngine *parent)
    :   QPlaceMatchReply(parent)
    {
        setError(QPlaceReply::UnsupportedError,
                 QStringLiteral("Place matching is not supported."));
        setFinished(true);
        QMetaObject::invokeMethod(this, "errorOccurred", Qt::QueuedConnection,
                                  Q_ARG(QPlaceReply::Error, error()),
                                  Q_ARG(QString, errorString()));
        QMetaObject::invokeMethod(parent, "errorOccurred", Qt::QueuedConnection,
                                  Q_ARG(QPlaceReply *, this),
                                  Q_ARG(QPlaceReply::Error, error()),
                                  Q_ARG(QString, errorString()));
        QMetaObject::invokeMethod(this, "finished", Qt::QueuedConnection);
        QMetaObject::invokeMethod(parent, "finished", Qt::QueuedConnection,
                                  Q_ARG(QPlaceReply *, this));
    }
};

QT_END_NAMESPACE

#endif
