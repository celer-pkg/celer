// Copyright (C) 2015 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR LGPL-3.0-only OR GPL-2.0-only OR GPL-3.0-only
// Qt-Security score:significant reason:default

#ifndef QPLACERESULT_H
#define QPLACERESULT_H

#include <QtLocation/QPlaceSearchResult>

QT_BEGIN_NAMESPACE

class QPlace;
class QPlaceResultPrivate;

class Q_LOCATION_EXPORT QPlaceResult : public QPlaceSearchResult
{
public:
    QPlaceResult();

#ifdef Q_QDOC
    QPlaceResult::QPlaceResult(const QPlaceSearchResult &other);
#else
     Q_DECLARE_SEARCHRESULT_COPY_CTOR(QPlaceResult)
#endif

    virtual ~QPlaceResult();

    qreal distance() const;
    void setDistance(qreal distance);

    QPlace place() const;
    void setPlace(const QPlace &place);

    bool isSponsored() const;
    void setSponsored(bool sponsored);

private:
    Q_DECLARE_SEARCHRESULT_D_FUNC(QPlaceResult)
};

Q_DECLARE_TYPEINFO(QPlaceResult, Q_RELOCATABLE_TYPE);

QT_END_NAMESPACE

#endif
