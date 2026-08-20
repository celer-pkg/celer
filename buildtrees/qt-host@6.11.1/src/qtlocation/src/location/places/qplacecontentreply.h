// Copyright (C) 2015 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR LGPL-3.0-only OR GPL-2.0-only OR GPL-3.0-only
// Qt-Security score:significant reason:default

#ifndef QPLACECONTENTREPLY_H
#define QPLACECONTENTREPLY_H

#include <QtLocation/QPlaceReply>
#include <QtLocation/QPlaceContent>

QT_BEGIN_NAMESPACE

class QPlaceContentRequest;
class QPlaceContentReplyPrivate;
class Q_LOCATION_EXPORT QPlaceContentReply : public QPlaceReply
{
    Q_OBJECT

public:
    explicit QPlaceContentReply(QObject *parent = nullptr);
    virtual ~QPlaceContentReply();

    QPlaceReply::Type type() const override;

    QPlaceContent::Collection content() const;

    int totalCount() const;

    QPlaceContentRequest request() const;

    QPlaceContentRequest previousPageRequest() const;
    QPlaceContentRequest nextPageRequest() const;

protected:
    void setContent(const QPlaceContent::Collection &content);
    void setTotalCount(int total);
    void setRequest(const QPlaceContentRequest &request);
    void setPreviousPageRequest(const QPlaceContentRequest &previous);
    void setNextPageRequest(const QPlaceContentRequest &next);

private:
    Q_DISABLE_COPY(QPlaceContentReply)
    Q_DECLARE_PRIVATE(QPlaceContentReply)
};

QT_END_NAMESPACE

#endif
