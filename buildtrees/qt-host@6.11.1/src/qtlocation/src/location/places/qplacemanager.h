// Copyright (C) 2015 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR LGPL-3.0-only OR GPL-2.0-only OR GPL-3.0-only
// Qt-Security score:significant reason:default

#ifndef QPLACEMANAGER_H
#define QPLACEMANAGER_H

#include <QtLocation/QPlaceReply>

#include <QtCore/QList>
#include <QtCore/QString>
#include <QtCore/QObject>

QT_BEGIN_NAMESPACE

class QLocale;
class QPlace;
class QPlaceCategory;
class QPlaceContentReply;
class QPlaceContentRequest;
class QPlaceDetailsReply;
class QPlaceIcon;
class QPlaceIdReply;
class QPlaceManagerEngine;
class QPlaceMatchReply;
class QPlaceMatchRequest;
class QPlaceSearchRequest;
class QPlaceSearchReply;
class QPlaceSearchResult;
class QPlaceSearchSuggestionReply;

class Q_LOCATION_EXPORT QPlaceManager : public QObject
{
    Q_OBJECT
public:
    ~QPlaceManager();

    QString managerName() const;
    int managerVersion() const;

    QPlaceDetailsReply *getPlaceDetails(const QString &placeId) const;

    QPlaceContentReply *getPlaceContent(const QPlaceContentRequest &request) const;

    QPlaceSearchReply *search(const QPlaceSearchRequest &query) const;

    QPlaceSearchSuggestionReply *searchSuggestions(const QPlaceSearchRequest &request) const;

    QPlaceIdReply *savePlace(const QPlace &place);
    QPlaceIdReply *removePlace(const QString &placeId);

    QPlaceIdReply *saveCategory(const QPlaceCategory &category, const QString &parentId = QString());
    QPlaceIdReply *removeCategory(const QString &categoryId);

    QPlaceReply *initializeCategories();
    QString parentCategoryId(const QString &categoryId) const;
    QStringList childCategoryIds(const QString &parentId = QString()) const;

    QPlaceCategory category(const QString &categoryId) const;
    QList<QPlaceCategory> childCategories(const QString &parentId = QString()) const;

    QList<QLocale> locales() const;
    void setLocale(const QLocale &locale);
    void setLocales(const QList<QLocale> &locale);

    QPlace compatiblePlace(const QPlace &place) const;

    QPlaceMatchReply *matchingPlaces(const QPlaceMatchRequest &request) const;

Q_SIGNALS:
    void finished(QPlaceReply *reply);
    void errorOccurred(QPlaceReply *, QPlaceReply::Error error,
                       const QString &errorString = QString());

    void placeAdded(const QString &placeId);
    void placeUpdated(const QString &placeId);
    void placeRemoved(const QString &placeId);

    void categoryAdded(const QPlaceCategory &category, const QString &parentId);
    void categoryUpdated(const QPlaceCategory &category, const QString &parentId);
    void categoryRemoved(const QString &categoryId, const QString &parentId);
    void dataChanged();

private:
    explicit QPlaceManager(QPlaceManagerEngine *engine, QObject *parent = nullptr);
    Q_DISABLE_COPY(QPlaceManager)

    QPlaceManagerEngine *d;

    friend class QGeoServiceProvider;
    friend class QGeoServiceProviderPrivate;
    friend class QPlaceIcon;
};

QT_END_NAMESPACE

#endif // QPLACEMANAGER_H
