// Copyright (C) 2015 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR LGPL-3.0-only OR GPL-2.0-only OR GPL-3.0-only

#ifndef QPLACESEARCHSUGGESTIONREPLYIMPL_H
#define QPLACESEARCHSUGGESTIONREPLYIMPL_H

#include <QtNetwork/QNetworkReply>
#include <QtLocation/QPlaceSearchSuggestionReply>

QT_BEGIN_NAMESPACE

class QPlaceSearchSuggestionReplyImpl : public QPlaceSearchSuggestionReply
{
    Q_OBJECT

public:
    explicit QPlaceSearchSuggestionReplyImpl(QNetworkReply *reply, QObject *parent = nullptr);
    ~QPlaceSearchSuggestionReplyImpl();

private slots:
    void setError(QPlaceReply::Error error_, const QString &errorString);
    void replyFinished();
    void replyError(QNetworkReply::NetworkError error);
};

QT_END_NAMESPACE

#endif // QPLACESEARCHSUGGESTIONREPLYIMPL_H
