// Copyright (C) 2022 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR LGPL-3.0-only OR GPL-2.0-only OR GPL-3.0-only
// Qt-Security score:significant reason:default

#ifndef QDECLARATIVESEARCHRESULTMODEL_P_H
#define QDECLARATIVESEARCHRESULTMODEL_P_H

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
#include <QtLocation/private/qdeclarativesearchmodelbase_p.h>
#include <QtLocation/private/qdeclarativecategory_p.h>
#include <QtLocation/private/qdeclarativeplace_p.h>

QT_BEGIN_NAMESPACE

class QPlaceIcon;
class QDeclarativeGeoServiceProvider;

class Q_LOCATION_EXPORT QDeclarativeSearchResultModel : public QDeclarativeSearchModelBase
{
    Q_OBJECT
    QML_NAMED_ELEMENT(PlaceSearchModel)
    QML_ADDED_IN_VERSION(5, 0)

    Q_PROPERTY(QString searchTerm READ searchTerm WRITE setSearchTerm NOTIFY searchTermChanged)
    Q_PROPERTY(QQmlListProperty<QDeclarativeCategory> categories READ categories NOTIFY categoriesChanged)
    Q_PROPERTY(QString recommendationId READ recommendationId WRITE setRecommendationId NOTIFY recommendationIdChanged)
    Q_PROPERTY(RelevanceHint relevanceHint READ relevanceHint WRITE setRelevanceHint NOTIFY relevanceHintChanged)
    Q_PROPERTY(QDeclarativePlace::Visibility visibilityScope READ visibilityScope WRITE setVisibilityScope NOTIFY visibilityScopeChanged)

    Q_PROPERTY(int count READ rowCount NOTIFY rowCountChanged)
    Q_PROPERTY(QDeclarativeGeoServiceProvider *favoritesPlugin READ favoritesPlugin WRITE setFavoritesPlugin NOTIFY favoritesPluginChanged)
    Q_PROPERTY(QVariantMap favoritesMatchParameters READ favoritesMatchParameters WRITE setFavoritesMatchParameters NOTIFY favoritesMatchParametersChanged)

    Q_PROPERTY(bool incremental MEMBER m_incremental NOTIFY incrementalChanged REVISION(5, 12))

public:
    enum SearchResultType {
        UnknownSearchResult = QPlaceSearchResult::UnknownSearchResult,
        PlaceResult = QPlaceSearchResult::PlaceResult,
        ProposedSearchResult = QPlaceSearchResult::ProposedSearchResult
    };
    Q_ENUM(SearchResultType)

    enum RelevanceHint {
        UnspecifiedHint = QPlaceSearchRequest::UnspecifiedHint,
        DistanceHint = QPlaceSearchRequest::DistanceHint,
        LexicalPlaceNameHint = QPlaceSearchRequest::LexicalPlaceNameHint
    };
    Q_ENUM(RelevanceHint)

    explicit QDeclarativeSearchResultModel(QObject *parent = nullptr);
    ~QDeclarativeSearchResultModel();

    QString searchTerm() const;
    void setSearchTerm(const QString &searchTerm);

    QQmlListProperty<QDeclarativeCategory> categories();
    static void categories_append(QQmlListProperty<QDeclarativeCategory> *list,
                                  QDeclarativeCategory *category);
    static qsizetype categories_count(QQmlListProperty<QDeclarativeCategory> *list);
    static QDeclarativeCategory *category_at(QQmlListProperty<QDeclarativeCategory> *list, qsizetype index);
    static void categories_clear(QQmlListProperty<QDeclarativeCategory> *list);

    QString recommendationId() const;
    void setRecommendationId(const QString &recommendationId);

    QDeclarativeSearchResultModel::RelevanceHint relevanceHint() const;
    void setRelevanceHint(QDeclarativeSearchResultModel::RelevanceHint hint);

    QDeclarativePlace::Visibility visibilityScope() const;
    void setVisibilityScope(QDeclarativePlace::Visibility visibilityScope);

    QDeclarativeGeoServiceProvider *favoritesPlugin() const;
    void setFavoritesPlugin(QDeclarativeGeoServiceProvider *plugin);

    QVariantMap favoritesMatchParameters() const;
    void setFavoritesMatchParameters(const QVariantMap &parameters);

    int rowCount(const QModelIndex &parent = QModelIndex()) const override;

    void clearData(bool suppressSignal = false) override;
    QVariant data(const QModelIndex &index, int role = Qt::DisplayRole) const override;
    Q_INVOKABLE QVariant data(int index, const QString &roleName) const;
    QHash<int, QByteArray> roleNames() const override;

    Q_INVOKABLE void updateWith(int proposedSearchIndex);

    void updateSearchRequest();

Q_SIGNALS:
    void searchTermChanged();
    void categoriesChanged();
    void recommendationIdChanged();
    void relevanceHintChanged();
    void visibilityScopeChanged();

    void rowCountChanged();
    void favoritesPluginChanged();
    void favoritesMatchParametersChanged();
    void dataChanged();
    void incrementalChanged();

protected:
    QPlaceReply *sendQuery(QPlaceManager *manager, const QPlaceSearchRequest &request) override;
    void initializePlugin(QDeclarativeGeoServiceProvider *plugin) override;

protected Q_SLOTS:
    void queryFinished() override;
    void onContentUpdated() override;

private Q_SLOTS:
    void updateLayout(const QList<QPlace> &favoritePlaces = QList<QPlace>());

    void placeUpdated(const QString &placeId);
    void placeRemoved(const QString &placeId);

private:
    enum Roles {
        SearchResultTypeRole = Qt::UserRole,
        TitleRole,
        IconRole,
        DistanceRole,
        PlaceRole,
        SponsoredRole
    };

    int getRow(const QString &placeId) const;
    QList<QPlaceSearchResult> resultsFromPages() const;
    void removePageRow(int row);

    QList<QDeclarativeCategory *> m_categories;
    QLocation::VisibilityScope m_visibilityScope = QLocation::UnspecifiedVisibility;

    QMap<int, QList<QPlaceSearchResult>> m_pages;
    QList<QPlaceSearchResult> m_results;
    QList<QPlaceSearchResult> m_resultsBuffer;
    QList<QDeclarativePlace *> m_places;
    QList<QPlaceIcon> m_icons;

    QDeclarativeGeoServiceProvider *m_favoritesPlugin = nullptr;
    QVariantMap m_matchParameters;
    bool m_incremental = false;
};

QT_END_NAMESPACE

#endif // QDECLARATIVESEARCHRESULTMODEL_P_H
