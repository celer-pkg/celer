// Copyright (C) 2022 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR LGPL-3.0-only OR GPL-2.0-only OR GPL-3.0-only
// Qt-Security score:significant reason:default

#ifndef QDECLARATIVEPLACE_P_H
#define QDECLARATIVEPLACE_P_H

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
#include <QtCore/QObject>
#include <QtQml/QQmlListProperty>
#include <QtQml/QQmlParserStatus>
#include <QtQml/QQmlPropertyMap>
#include <QtLocation/QPlace>
#include <QtLocation/QPlaceRatings>
#include <QtLocation/QPlaceIcon>

#include <QtPositioningQuick/private/qdeclarativegeolocation_p.h>
#include <QtLocation/private/qdeclarativecategory_p.h>
#include <QtLocation/private/qdeclarativeplacecontentmodel_p.h>

QT_BEGIN_NAMESPACE

class QPlaceReply;
class QPlaceManager;
class QPlaceSupplier;
class QDeclarativeContactDetails;

class Q_LOCATION_EXPORT QDeclarativePlace : public QObject, public QQmlParserStatus
{
    Q_OBJECT
    QML_NAMED_ELEMENT(Place)
    QML_ADDED_IN_VERSION(5, 0)

    Q_PROPERTY(QPlace place READ place WRITE setPlace)
    Q_PROPERTY(QDeclarativeGeoServiceProvider *plugin READ plugin WRITE setPlugin NOTIFY pluginChanged)
    Q_PROPERTY(QQmlListProperty<QDeclarativeCategory> categories READ categories NOTIFY categoriesChanged)
    Q_PROPERTY(QDeclarativeGeoLocation *location READ location WRITE setLocation NOTIFY locationChanged)
    Q_PROPERTY(QPlaceRatings ratings READ ratings WRITE setRatings NOTIFY ratingsChanged)
    Q_PROPERTY(QPlaceSupplier supplier READ supplier WRITE setSupplier NOTIFY supplierChanged)
    Q_PROPERTY(QPlaceIcon icon READ icon WRITE setIcon NOTIFY iconChanged)
    Q_PROPERTY(QString name READ name WRITE setName NOTIFY nameChanged)
    Q_PROPERTY(QString placeId READ placeId WRITE setPlaceId NOTIFY placeIdChanged)
    Q_PROPERTY(QString attribution READ attribution WRITE setAttribution NOTIFY attributionChanged)

    Q_PROPERTY(QDeclarativePlaceReviewModel *reviewModel READ reviewModel NOTIFY reviewModelChanged)
    Q_PROPERTY(QDeclarativePlaceImageModel *imageModel READ imageModel NOTIFY imageModelChanged)
    Q_PROPERTY(QDeclarativePlaceEditorialModel *editorialModel READ editorialModel NOTIFY editorialModelChanged)

    Q_PROPERTY(QObject *extendedAttributes READ extendedAttributes NOTIFY extendedAttributesChanged)
    Q_PROPERTY(QDeclarativeContactDetails *contactDetails READ contactDetails NOTIFY contactDetailsChanged)
    Q_PROPERTY(bool detailsFetched READ detailsFetched NOTIFY detailsFetchedChanged)
    Q_PROPERTY(Status status READ status NOTIFY statusChanged)

    Q_PROPERTY(QString primaryPhone READ primaryPhone NOTIFY primaryPhoneChanged)
    Q_PROPERTY(QString primaryFax READ primaryFax NOTIFY primaryFaxChanged)
    Q_PROPERTY(QString primaryEmail READ primaryEmail NOTIFY primaryEmailChanged)
    Q_PROPERTY(QUrl primaryWebsite READ primaryWebsite NOTIFY primaryWebsiteChanged)

    Q_PROPERTY(Visibility visibility READ visibility WRITE setVisibility NOTIFY visibilityChanged)
    Q_PROPERTY(QDeclarativePlace *favorite READ favorite WRITE setFavorite NOTIFY favoriteChanged)

    Q_INTERFACES(QQmlParserStatus)

public:
    explicit QDeclarativePlace(QObject *parent = nullptr);
    QDeclarativePlace(const QPlace &src, QDeclarativeGeoServiceProvider *plugin, QObject *parent = nullptr);
    ~QDeclarativePlace();

    enum Status {Ready, Saving, Fetching, Removing, Error};
    Q_ENUM(Status)

    enum Visibility {
        UnspecifiedVisibility = QLocation::UnspecifiedVisibility,
        DeviceVisibility = QLocation::DeviceVisibility,
        PrivateVisibility = QLocation::PrivateVisibility,
        PublicVisibility = QLocation::PublicVisibility
    };
    Q_ENUM(Visibility)

    //From QQmlParserStatus
    void classBegin() override {}
    void componentComplete() override;

    void setPlugin(QDeclarativeGeoServiceProvider *plugin);
    QDeclarativeGeoServiceProvider *plugin() const;

    QDeclarativePlaceReviewModel *reviewModel();
    QDeclarativePlaceImageModel *imageModel();
    QDeclarativePlaceEditorialModel *editorialModel();

    QPlace place() const;
    void setPlace(const QPlace &src);

    QQmlListProperty<QDeclarativeCategory> categories();
    static void category_append(QQmlListProperty<QDeclarativeCategory> *prop,
                                  QDeclarativeCategory *value);
    static qsizetype category_count(QQmlListProperty<QDeclarativeCategory> *prop);
    static QDeclarativeCategory *category_at(QQmlListProperty<QDeclarativeCategory> *prop, qsizetype index);
    static void category_clear(QQmlListProperty<QDeclarativeCategory> *prop);

    QDeclarativeGeoLocation *location() const;
    void setLocation(QDeclarativeGeoLocation *location);
    QPlaceRatings ratings() const;
    void setRatings(const QPlaceRatings &ratings);
    QPlaceSupplier supplier() const;
    void setSupplier(const QPlaceSupplier &supplier);
    QPlaceIcon icon() const;
    void setIcon(const QPlaceIcon &icon);
    QString name() const;
    void setName(const QString &name);
    QString placeId() const;
    void setPlaceId(const QString &placeId);
    QString attribution() const;
    void setAttribution(const QString &attribution);
    bool detailsFetched() const;

    Status status() const;
    void setStatus(Status status, const QString &errorString = QString());

    Q_INVOKABLE void getDetails();
    Q_INVOKABLE void save();
    Q_INVOKABLE void remove();
    Q_INVOKABLE QString errorString() const;

    QString primaryPhone() const;
    QString primaryFax() const;
    QString primaryEmail() const;
    QUrl primaryWebsite() const;

    QQmlPropertyMap *extendedAttributes() const;

    QDeclarativeContactDetails *contactDetails() const;

    Visibility visibility() const;
    void setVisibility(Visibility visibility);

    QDeclarativePlace *favorite() const;
    void setFavorite(QDeclarativePlace *favorite);

    Q_INVOKABLE void copyFrom(QDeclarativePlace *original);
    Q_INVOKABLE void initializeFavorite(QDeclarativeGeoServiceProvider *plugin);

Q_SIGNALS:
    void pluginChanged();
    void categoriesChanged();
    void locationChanged();
    void ratingsChanged();
    void supplierChanged();
    void iconChanged();
    void nameChanged();
    void placeIdChanged();
    void attributionChanged();
    void detailsFetchedChanged();
    void reviewModelChanged();
    void imageModelChanged();
    void editorialModelChanged();

    void primaryPhoneChanged();
    void primaryFaxChanged();
    void primaryEmailChanged();
    void primaryWebsiteChanged();

    void extendedAttributesChanged();
    void contactDetailsChanged();
    void statusChanged();
    void visibilityChanged();
    void favoriteChanged();

private Q_SLOTS:
    void finished();
    void contactsModified(const QString &, const QVariant &);
    void pluginReady();
    void cleanupDeletedCategories();
private:
    void synchronizeCategories();
    void pullExtendedAttributes();
    void synchronizeContacts();
    void primarySignalsEmission(const QString &type = QString());
    QString primaryValue(const QString &contactType) const;

private:
    QPlaceManager *manager();

    QList<QDeclarativeCategory *> m_categories;
    QDeclarativeGeoLocation *m_location = nullptr;
    QDeclarativePlaceReviewModel *m_reviewModel = nullptr;
    QDeclarativePlaceImageModel *m_imageModel = nullptr;
    QDeclarativePlaceEditorialModel *m_editorialModel = nullptr;
    QQmlPropertyMap *m_extendedAttributes = nullptr;
    QDeclarativeContactDetails *m_contactDetails = nullptr;

    QPlace m_src;

    QPlaceReply *m_reply = nullptr;

    QDeclarativeGeoServiceProvider *m_plugin = nullptr;
    bool m_complete = false;

    QString m_prevPrimaryPhone;
    QString m_prevPrimaryEmail;
    QString m_prevPrimaryFax;
    QUrl m_prevPrimaryWebsite;

    QDeclarativePlace *m_favorite = nullptr;

    Status m_status = Ready;
    QString m_errorString;

    QList<QDeclarativeCategory *>m_categoriesToBeDeleted;
};

QT_END_NAMESPACE

QML_DECLARE_TYPE(QDeclarativePlace)

#endif // QDECLARATIVEPLACE_P_H
