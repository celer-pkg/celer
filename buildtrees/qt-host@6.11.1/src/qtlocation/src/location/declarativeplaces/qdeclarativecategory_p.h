// Copyright (C) 2022 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR LGPL-3.0-only OR GPL-2.0-only OR GPL-3.0-only
// Qt-Security score:significant reason:default

#ifndef QDECLARATIVECATEGORY_P_H
#define QDECLARATIVECATEGORY_P_H

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
#include <QtQml/qqml.h>
#include <QtQml/QQmlParserStatus>
#include <QObject>

#include <QtLocation/qplacecategory.h>
#include <QtLocation/qplaceicon.h>

#include <QtLocation/private/qdeclarativegeoserviceprovider_p.h>

QT_BEGIN_NAMESPACE

class QPlaceReply;
class QPlaceManager;

class Q_LOCATION_EXPORT QDeclarativeCategory : public QObject, public QQmlParserStatus
{
    Q_OBJECT
    QML_NAMED_ELEMENT(Category)
    QML_ADDED_IN_VERSION(5, 0)

    Q_PROPERTY(QPlaceCategory category READ category WRITE setCategory)
    Q_PROPERTY(QDeclarativeGeoServiceProvider *plugin READ plugin WRITE setPlugin NOTIFY pluginChanged)
    Q_PROPERTY(QString categoryId READ categoryId WRITE setCategoryId NOTIFY categoryIdChanged)
    Q_PROPERTY(QString name READ name WRITE setName NOTIFY nameChanged)
    Q_PROPERTY(Visibility visibility READ visibility WRITE setVisibility NOTIFY visibilityChanged)
    Q_PROPERTY(QPlaceIcon icon READ icon WRITE setIcon NOTIFY iconChanged)
    Q_PROPERTY(Status status READ status NOTIFY statusChanged)

    Q_INTERFACES(QQmlParserStatus)

public:
    explicit QDeclarativeCategory(QObject *parent = nullptr);
    QDeclarativeCategory(const QPlaceCategory &category, QDeclarativeGeoServiceProvider *plugin, QObject *parent = nullptr);
    ~QDeclarativeCategory();

    enum Visibility {
        UnspecifiedVisibility = QLocation::UnspecifiedVisibility,
        DeviceVisibility = QLocation::DeviceVisibility,
        PrivateVisibility = QLocation::PrivateVisibility,
        PublicVisibility = QLocation::PublicVisibility
    };
    Q_ENUM(Visibility)

    enum Status {Ready, Saving, Removing, Error};
    Q_ENUM(Status)

    //From QQmlParserStatus
    void classBegin() override {}
    void componentComplete() override;

    void setPlugin(QDeclarativeGeoServiceProvider *plugin);
    QDeclarativeGeoServiceProvider *plugin() const;

    QPlaceCategory category();
    void setCategory(const QPlaceCategory &category);

    QString categoryId() const;
    void setCategoryId(const QString &catID);
    QString name() const;
    void setName(const QString &name);

    Visibility visibility() const;
    void setVisibility(Visibility visibility);

    QPlaceIcon icon() const;
    void setIcon(const QPlaceIcon &icon);

    Q_INVOKABLE QString errorString() const;

    Status status() const;
    void setStatus(Status status, const QString &errorString = QString());

    Q_INVOKABLE void save(const QString &parentId = QString());
    Q_INVOKABLE void remove();

Q_SIGNALS:
    void pluginChanged();
    void categoryIdChanged();
    void nameChanged();
    void visibilityChanged();
    void iconChanged();
    void statusChanged();

private Q_SLOTS:
    void replyFinished();
    void pluginReady();

private:
    QPlaceManager *manager();

    QPlaceCategory m_category;
    QDeclarativeGeoServiceProvider *m_plugin = nullptr;
    QPlaceReply *m_reply = nullptr;
    bool m_complete = false;
    Status m_status = Ready;
    QString m_errorString;
};

QT_END_NAMESPACE

QML_DECLARE_TYPE(QDeclarativeCategory)

#endif // QDECLARATIVECATEGORY_P_H
