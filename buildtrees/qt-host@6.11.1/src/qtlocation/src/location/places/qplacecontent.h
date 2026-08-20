// Copyright (C) 2015 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR LGPL-3.0-only OR GPL-2.0-only OR GPL-3.0-only
// Qt-Security score:significant reason:default
#ifndef QPLACECONTENT_H
#define QPLACECONTENT_H

#include <QtLocation/qlocationglobal.h>

#include <QtCore/QExplicitlySharedDataPointer>
#include <QtCore/QMap>
#include <QtCore/QMetaType>
#include <QtCore/QVariant>

#include <QtLocation/QPlaceUser>
#include <QtLocation/QPlaceSupplier>

QT_BEGIN_NAMESPACE

class QPlaceContentPrivate;
QT_DECLARE_QESDP_SPECIALIZATION_DTOR_WITH_EXPORT(QPlaceContentPrivate, Q_LOCATION_EXPORT)

class Q_LOCATION_EXPORT QPlaceContent
{
public:
    typedef QMap<int, QPlaceContent> Collection;

    enum Type {
        NoType = 0,
        ImageType,
        ReviewType,
        EditorialType,
        CustomType = 0x0100
    };

    enum DataTag {
        ContentSupplier,
        ContentUser,
        ContentAttribution,
        ImageId,
        ImageUrl,
        ImageMimeType,
        EditorialTitle,
        EditorialText,
        EditorialLanguage,
        ReviewId,
        ReviewDateTime,
        ReviewTitle,
        ReviewText,
        ReviewLanguage,
        ReviewRating,
        CustomDataTag = 1000
    };

    QPlaceContent(Type type = NoType);
    ~QPlaceContent();

    QPlaceContent(const QPlaceContent &other) noexcept;
    QPlaceContent &operator=(const QPlaceContent &other) noexcept;

    QPlaceContent(QPlaceContent &&other) noexcept = default;
    QT_MOVE_ASSIGNMENT_OPERATOR_IMPL_VIA_MOVE_AND_SWAP(QPlaceContent)
    void swap(QPlaceContent &other) noexcept
    { d_ptr.swap(other.d_ptr); }
    void detach();

    bool operator==(const QPlaceContent &other) const;
    bool operator!=(const QPlaceContent &other) const;

    QPlaceContent::Type type() const;

    QList<DataTag> dataTags() const;
    QVariant value(DataTag tag) const;
    void setValue(DataTag tag, const QVariant &);

#if QT_DEPRECATED_SINCE(6, 0)
    QT_DEPRECATED_VERSION_X_6_0("Use value()") QPlaceSupplier supplier() const
    { return value(QPlaceContent::ContentSupplier).value<QPlaceSupplier>(); }
    QT_DEPRECATED_VERSION_X_6_0("Use setValue()") void setSupplier(const QPlaceSupplier &supplier)
    { setValue(QPlaceContent::ContentSupplier, QVariant::fromValue(supplier)); }

    QT_DEPRECATED_VERSION_X_6_0("Use value()") QPlaceUser user() const
    { return value(QPlaceContent::ContentUser).value<QPlaceUser>(); }
    QT_DEPRECATED_VERSION_X_6_0("Use setValue()") void setUser(const QPlaceUser &user)
    { setValue(QPlaceContent::ContentUser, QVariant::fromValue(user)); }

    QT_DEPRECATED_VERSION_X_6_0("Use value()") QString attribution() const
    { return value(QPlaceContent::ContentAttribution).value<QString>(); }
    QT_DEPRECATED_VERSION_X_6_0("Use setValue()") void setAttribution(const QString &attribution)
    { setValue(QPlaceContent::ContentAttribution, QVariant::fromValue(attribution)); }
#endif

protected:
    inline QPlaceContentPrivate *d_func();
    inline const QPlaceContentPrivate *d_func() const;

private:
    QExplicitlySharedDataPointer<QPlaceContentPrivate> d_ptr;
    friend class QPlaceContentPrivate;
};

QT_END_NAMESPACE

Q_DECLARE_METATYPE(QPlaceContent)
Q_DECLARE_METATYPE(QPlaceContent::Type)

#endif

