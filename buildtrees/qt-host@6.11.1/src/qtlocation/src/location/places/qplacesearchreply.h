// Copyright (C) 2015 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR LGPL-3.0-only OR GPL-2.0-only OR GPL-3.0-only
// Qt-Security score:significant reason:default

#ifndef QPLACESEARCHREPLY_H
#define QPLACESEARCHREPLY_H

#include <QtLocation/QPlaceReply>

QT_BEGIN_NAMESPACE

class QPlaceSearchResult;
class QPlaceSearchRequest;
class QPlaceSearchReplyPrivate;

class Q_LOCATION_EXPORT QPlaceSearchReply : public QPlaceReply
{
    Q_OBJECT
public:
    explicit QPlaceSearchReply(QObject *parent = nullptr);
    ~QPlaceSearchReply();

    QPlaceReply::Type type() const override;

    QList<QPlaceSearchResult> results() const;
    QPlaceSearchRequest request() const;

    QPlaceSearchRequest previousPageRequest() const;
    QPlaceSearchRequest nextPageRequest() const;

protected:
    void setResults(const QList<QPlaceSearchResult> &results);
    void setRequest(const QPlaceSearchRequest &request);
    void setPreviousPageRequest(const QPlaceSearchRequest &previous);
    void setNextPageRequest(const QPlaceSearchRequest &next);

private:
    Q_DISABLE_COPY(QPlaceSearchReply)
    Q_DECLARE_PRIVATE(QPlaceSearchReply)
};

QT_END_NAMESPACE

#endif
