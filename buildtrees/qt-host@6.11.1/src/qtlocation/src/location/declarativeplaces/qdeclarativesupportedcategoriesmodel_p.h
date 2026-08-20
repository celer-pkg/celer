// Copyright (C) 2022 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR LGPL-3.0-only OR GPL-2.0-only OR GPL-3.0-only
// Qt-Security score:significant reason:default

#ifndef QDECLARATIVESUPPORTEDCATEGORIESMODEL_H
#define QDECLARATIVESUPPORTEDCATEGORIESMODEL_H

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
#include <QtLocation/private/qdeclarativegeoserviceprovider_p.h>

#include <QObject>
#include <QtCore/QStringList>
#include <QtCore/QSharedPointer>
#include <QAbstractListModel>
#include <QQmlListProperty>
#include <QtQml/QQmlParserStatus>

#include <QtLocation/QPlaceCategory>

#include <QtLocation/private/qdeclarativecategory_p.h>

QT_BEGIN_NAMESPACE

class QGeoServiceProvider;
class QPlaceManager;
class QPlaceReply;

class Q_LOCATION_EXPORT QDeclarativeSupportedCategoriesModel : public QAbstractItemModel, public QQmlParserStatus
{
    Q_OBJECT
    QML_NAMED_ELEMENT(CategoryModel)
    QML_ADDED_IN_VERSION(5, 0)

    Q_PROPERTY(QDeclarativeGeoServiceProvider *plugin READ plugin WRITE setPlugin NOTIFY pluginChanged)
    Q_PROPERTY(bool hierarchical READ hierarchical WRITE setHierarchical NOTIFY hierarchicalChanged)
    Q_PROPERTY(Status status READ status NOTIFY statusChanged)

    Q_INTERFACES(QQmlParserStatus)

public:
    explicit QDeclarativeSupportedCategoriesModel(QObject *parent = nullptr);
    virtual ~QDeclarativeSupportedCategoriesModel();

    // From QQmlParserStatus
    void classBegin() override {}
    void componentComplete() override;

    // From QAbstractItemModel
    int rowCount(const QModelIndex &parent) const override;
    int columnCount(const QModelIndex &parent) const override;

    QModelIndex index(int row, int column, const QModelIndex &parent) const override;
    QModelIndex parent(const QModelIndex &child) const override;

    Q_INVOKABLE QVariant data(const QModelIndex &index, int role) const override;
    QHash<int, QByteArray> roleNames() const override;

    enum Roles {
        CategoryRole = Qt::UserRole,
        ParentCategoryRole
    };  //for internal usage only
    Q_ENUM(Roles)

    enum Status {Null, Ready, Loading, Error};
    Q_ENUM(Status)

    void setPlugin(QDeclarativeGeoServiceProvider *plugin);
    QDeclarativeGeoServiceProvider *plugin() const;

    void setHierarchical(bool hierarchical);
    bool hierarchical() const;

    Q_INVOKABLE QString errorString() const;

    Status status() const;
    void setStatus(Status status, const QString &errorString = QString());

    using QAbstractItemModel::dataChanged;
Q_SIGNALS:
    void pluginChanged();
    void hierarchicalChanged();
    void statusChanged();
    void dataChanged();

public Q_SLOTS:
    void update();

private Q_SLOTS:
    void replyFinished();
    void addedCategory(const QPlaceCategory &category, const QString &parentId);
    void updatedCategory(const QPlaceCategory &category, const QString &parentId);
    void removedCategory(const QString &categoryId, const QString &parentId);
    void connectNotificationSignals();

private:
    struct PlaceCategoryNode
    {
        QString parentId;
        QStringList childIds;
        QSharedPointer<QDeclarativeCategory> declCategory;
    };

    QStringList populateCategories(QPlaceManager *, const QPlaceCategory &parent);
    QModelIndex index(const QString &categoryId) const;
    int rowToAddChild(PlaceCategoryNode *, const QPlaceCategory &category);
    void updateLayout();
    void emitDataChanged() { Q_EMIT dataChanged(); }

    QPlaceReply *m_response = nullptr;

    QDeclarativeGeoServiceProvider *m_plugin = nullptr;
    bool m_hierarchical = true;
    bool m_complete = false;
    Status m_status = Null;
    QString m_errorString;

    QHash<QString, PlaceCategoryNode *> m_categoriesTree;
};

QT_END_NAMESPACE

QML_DECLARE_TYPE(QDeclarativeSupportedCategoriesModel)

#endif // QDECLARATIVESUPPORTEDCATEGORIESMODEL_H
