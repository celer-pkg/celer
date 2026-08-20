// Copyright (C) 2015 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR LGPL-3.0-only OR GPL-2.0-only OR GPL-3.0-only
// Qt-Security score:significant reason:default

#ifndef QPLACESEARCHRESULT_H
#define QPLACESEARCHRESULT_H

#include <QtCore/QSharedDataPointer>
#include <QtCore/QVariant>
#include <QtCore/QString>
#include <QtLocation/qlocationglobal.h>

QT_BEGIN_NAMESPACE

#define Q_DECLARE_SEARCHRESULT_D_FUNC(Class) \
    inline Class##Private *d_func(); \
    inline const Class##Private *d_func() const;\
    friend class Class##Private;

#define Q_DECLARE_SEARCHRESULT_COPY_CTOR(Class) \
    Class(const QPlaceSearchResult &other);

class QPlaceSearchRequest;
class QPlaceSearchResultPrivate;
class QPlaceIcon;

class Q_LOCATION_EXPORT QPlaceSearchResult
{
public:
    QPlaceSearchResult();
    QPlaceSearchResult(const QPlaceSearchResult &other);

    virtual ~QPlaceSearchResult();

    QPlaceSearchResult &operator=(const QPlaceSearchResult &other);

    bool operator==(const QPlaceSearchResult &other) const;
    bool operator!=(const QPlaceSearchResult &other) const {
        return !(other == *this);
    }

    enum SearchResultType {
        UnknownSearchResult = 0,
        PlaceResult,
        ProposedSearchResult
    };

    SearchResultType type() const;

    QString title() const;
    void setTitle(const QString &title);

    QPlaceIcon icon() const;
    void setIcon(const QPlaceIcon &icon);

protected:
    explicit QPlaceSearchResult(QPlaceSearchResultPrivate *d);
    QSharedDataPointer<QPlaceSearchResultPrivate> d_ptr;

private:
    inline QPlaceSearchResultPrivate *d_func();
    inline const QPlaceSearchResultPrivate *d_func() const;

    friend class QPlaceSearchResultPrivate;
};

Q_DECLARE_TYPEINFO(QPlaceSearchResult, Q_RELOCATABLE_TYPE);

QT_END_NAMESPACE

Q_DECLARE_METATYPE(QPlaceSearchResult)
Q_DECLARE_METATYPE(QPlaceSearchResult::SearchResultType)

#endif // QPLACESEARCHRESULT_H
