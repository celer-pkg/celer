// Copyright (C) 2013 Aaron McCarthy <mccarthy.aaron@gmail.com>
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR LGPL-3.0-only OR GPL-2.0-only OR GPL-3.0-only
// Qt-Security score:significant reason:default

#ifndef QPROPOSEDSEARCHRESULT_H
#define QPROPOSEDSEARCHRESULT_H

#include <QtLocation/QPlaceSearchResult>

QT_BEGIN_NAMESPACE

class QPlaceProposedSearchResultPrivate;

class Q_LOCATION_EXPORT QPlaceProposedSearchResult : public QPlaceSearchResult
{
public:
    QPlaceProposedSearchResult();

#ifdef Q_QDOC
    QPlaceProposedSearchResult(const QPlaceSearchRequest &other);
#else
    Q_DECLARE_SEARCHRESULT_COPY_CTOR(QPlaceProposedSearchResult)
#endif

    ~QPlaceProposedSearchResult();

    QPlaceSearchRequest searchRequest() const;
    void setSearchRequest(const QPlaceSearchRequest &request);

private:
    Q_DECLARE_SEARCHRESULT_D_FUNC(QPlaceProposedSearchResult)
};

Q_DECLARE_TYPEINFO(QPlaceProposedSearchResult, Q_RELOCATABLE_TYPE);

QT_END_NAMESPACE

#endif // QPROPOSEDSEARCHRESULT_H
