// Copyright (C) 2015 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR LGPL-3.0-only OR GPL-2.0-only OR GPL-3.0-only
// Qt-Security score:significant reason:default

#ifndef QPLACEMATCHREPLY_H
#define QPLACEMATCHREPLY_H

#include <QtLocation/QPlaceReply>

QT_BEGIN_NAMESPACE

class QPlace;
class QPlaceMatchRequest;
class QPlaceMatchReplyPrivate;
class Q_LOCATION_EXPORT QPlaceMatchReply : public QPlaceReply
{
    Q_OBJECT
public:
    explicit QPlaceMatchReply(QObject *parent = nullptr);
    ~QPlaceMatchReply();

    QPlaceReply::Type type() const override;

    QList<QPlace> places() const;
    QPlaceMatchRequest request() const;

protected:
    void setPlaces(const QList<QPlace> &results);
    void setRequest(const QPlaceMatchRequest &request);
private:
    Q_DISABLE_COPY(QPlaceMatchReply)
    Q_DECLARE_PRIVATE(QPlaceMatchReply)
};

QT_END_NAMESPACE

#endif
