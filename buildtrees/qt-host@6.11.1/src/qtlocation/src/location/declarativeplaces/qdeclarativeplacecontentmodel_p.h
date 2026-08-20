// Copyright (C) 2022 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR LGPL-3.0-only OR GPL-2.0-only OR GPL-3.0-only
// Qt-Security score:significant reason:default

#ifndef QDECLARATIVEPLACECONTENTMODEL_H
#define QDECLARATIVEPLACECONTENTMODEL_H

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
#include <QtCore/QAbstractListModel>
#include <QtQml/QQmlParserStatus>
#include <QtLocation/QPlaceContent>
#include <QtLocation/QPlaceContentReply>
#include <QtLocation/QPlaceContentRequest>
#include <QtLocation/QPlaceSupplier>

Q_MOC_INCLUDE(<QtLocation/private/qdeclarativeplace_p.h>)

QT_BEGIN_NAMESPACE

class QDeclarativePlace;
class QDeclarativeGeoServiceProvider;
class QGeoServiceProvider;

class Q_LOCATION_EXPORT QDeclarativePlaceContentModel : public QAbstractListModel, public QQmlParserStatus
{
    Q_OBJECT

    Q_PROPERTY(QDeclarativePlace *place READ place WRITE setPlace NOTIFY placeChanged)
    Q_PROPERTY(int batchSize READ batchSize WRITE setBatchSize NOTIFY batchSizeChanged)
    Q_PROPERTY(int totalCount READ totalCount NOTIFY totalCountChanged)

    Q_INTERFACES(QQmlParserStatus)

public:
    explicit QDeclarativePlaceContentModel(QPlaceContent::Type type, QObject *parent = nullptr);
    ~QDeclarativePlaceContentModel();

    QDeclarativePlace *place() const;
    void setPlace(QDeclarativePlace *place);

    int batchSize() const;
    void setBatchSize(int batchSize);

    int totalCount() const;

    void clearData();

    void initializeCollection(int totalCount, const QPlaceContent::Collection &collection);

    // from QAbstractListModel
    int rowCount(const QModelIndex &parent) const override;
    QVariant data(const QModelIndex &index, int role) const override;
    QHash<int, QByteArray> roleNames() const override;

    enum Roles {
        ContentSupplierRole = Qt::UserRole,
        ContentUserRole,
        ContentAttributionRole,
        EditorialTitleRole,
        EditorialTextRole,
        EditorialLanguageRole,
        ImageIdRole,
        ImageUrlRole,
        ImageMimeTypeRole,
        ReviewIdRole,
        ReviewDateTimeRole,
        ReviewTitleRole,
        ReviewTextRole,
        ReviewLanguageRole,
        ReviewRatingRole
    };

    bool canFetchMore(const QModelIndex &parent) const override;
    void fetchMore(const QModelIndex &parent) override;

    // from QQmlParserStatus
    void classBegin() override;
    void componentComplete() override;

Q_SIGNALS:
    void placeChanged();
    void batchSizeChanged();
    void totalCountChanged();

private Q_SLOTS:
    void fetchFinished();

protected:
    QPlaceContent::Collection m_content;
    QMap<QString, QPlaceSupplier> m_suppliers;
    QMap<QString, QPlaceUser>m_users;

private:
    QDeclarativePlace *m_place = nullptr;
    QPlaceContent::Type m_type;
    int m_batchSize = 1;
    int m_contentCount = -1;

    QPlaceContentReply *m_reply = nullptr;
    QPlaceContentRequest m_nextRequest;

    bool m_complete = false;
};

class QDeclarativePlaceReviewModel : public QDeclarativePlaceContentModel
{
    Q_GADGET
    QML_NAMED_ELEMENT(ReviewModel)
    QML_ADDED_IN_VERSION(5, 0)

public:
    explicit QDeclarativePlaceReviewModel(QObject *parent = nullptr)
        : QDeclarativePlaceContentModel(QPlaceContent::ReviewType, parent)
    {}
};

class QDeclarativePlaceEditorialModel : public QDeclarativePlaceContentModel
{
    Q_GADGET
    QML_NAMED_ELEMENT(EditorialModel)
    QML_ADDED_IN_VERSION(5, 0)
public:
    explicit QDeclarativePlaceEditorialModel(QObject *parent = nullptr)
        : QDeclarativePlaceContentModel(QPlaceContent::EditorialType, parent)
    {}
};

class QDeclarativePlaceImageModel : public QDeclarativePlaceContentModel
{
    Q_GADGET
    QML_NAMED_ELEMENT(ImageModel)
    QML_ADDED_IN_VERSION(5, 0)
public:
    explicit QDeclarativePlaceImageModel(QObject *parent = nullptr)
        : QDeclarativePlaceContentModel(QPlaceContent::ImageType, parent)
    {}
};

QT_END_NAMESPACE

#endif // QDECLARATIVEPLACECONTENTMODEL_H
