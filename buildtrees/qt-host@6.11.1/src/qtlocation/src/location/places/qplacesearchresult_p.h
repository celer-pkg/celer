// Copyright (C) 2021 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR LGPL-3.0-only OR GPL-2.0-only OR GPL-3.0-only
// Qt-Security score:significant reason:default

#ifndef QPLACESEARCHRESULT_P_H
#define QPLACESEARCHRESULT_P_H

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

#include "qplacesearchresult.h"
#include "qplacesearchrequest.h"

#include <QSharedData>
#include <QtLocation/QPlaceIcon>

QT_BEGIN_NAMESPACE

// defines must be in sync with class below
#define Q_IMPLEMENT_SEARCHRESULT_D_FUNC(Class) \
    Class##Private *Class::d_func() { return reinterpret_cast<Class##Private *>(d_ptr.data()); } \
    const Class##Private *Class::d_func() const { return reinterpret_cast<const Class##Private *>(d_ptr.constData()); } \

#define Q_IMPLEMENT_SEARCHRESULT_COPY_CTOR(Class) \
    Class::Class(const QPlaceSearchResult &other) : QPlaceSearchResult() { Class##Private::copyIfPossible(d_ptr, other); }

#define Q_DEFINE_SEARCHRESULT_PRIVATE_HELPER(Class, ResultType) \
    QPlaceSearchResultPrivate *clone() const override { return new Class##Private(*this); } \
    QPlaceSearchResult::SearchResultType type() const override {return ResultType;} \
    static void copyIfPossible(QSharedDataPointer<QPlaceSearchResultPrivate> &d_ptr, const QPlaceSearchResult &other) \
    { \
        if (other.type() == ResultType) \
            d_ptr = extract_d(other); \
        else \
            d_ptr = new Class##Private; \
    }

class QPlaceSearchResultPrivate : public QSharedData
{
public:
    QPlaceSearchResultPrivate() {}
    virtual ~QPlaceSearchResultPrivate() {}

    virtual bool compare(const QPlaceSearchResultPrivate *other) const;

    static const QSharedDataPointer<QPlaceSearchResultPrivate>
            &extract_d(const QPlaceSearchResult &other) { return other.d_ptr; }

    virtual QPlaceSearchResultPrivate *clone() const { return new QPlaceSearchResultPrivate(*this); }
    virtual QPlaceSearchResult::SearchResultType type() const  { return QPlaceSearchResult::UnknownSearchResult; }
    static void copyIfPossible(QSharedDataPointer<QPlaceSearchResultPrivate> &d_ptr, const QPlaceSearchResult &other)
    {
        if (other.type() == QPlaceSearchResult::UnknownSearchResult)
            d_ptr = extract_d(other);
        else
            d_ptr = new QPlaceSearchResultPrivate;
    }

    QString title;
    QPlaceIcon icon;
};

template<> QPlaceSearchResultPrivate *QSharedDataPointer<QPlaceSearchResultPrivate>::clone();

QT_END_NAMESPACE

#endif // QPLACESEARCHRESULT_P_H
