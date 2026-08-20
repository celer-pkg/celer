// Copyright (C) 2015 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR LGPL-3.0-only OR GPL-2.0-only OR GPL-3.0-only
// Qt-Security score:significant reason:default

#ifndef QDECLARATIVEGEOCODEMODEL_H
#define QDECLARATIVEGEOCODEMODEL_H

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

#include <QtLocation/qgeocodereply.h>
#include <QtPositioningQuick/private/qdeclarativegeoaddress_p.h>
#include <QtPositioningQuick/private/qdeclarativegeolocation_p.h>

#include <QtQml/qqml.h>
#include <QtQml/QQmlParserStatus>
#include <QAbstractListModel>
#include <QPointer>


QT_BEGIN_NAMESPACE

class QGeoServiceProvider;
class QGeoCodingManager;
class QDeclarativeGeoLocation;

class Q_LOCATION_EXPORT QDeclarativeGeocodeModel : public QAbstractListModel, public QQmlParserStatus
{
    Q_OBJECT
    QML_NAMED_ELEMENT(GeocodeModel)

    Q_PROPERTY(QDeclarativeGeoServiceProvider *plugin READ plugin WRITE setPlugin NOTIFY pluginChanged)
    Q_PROPERTY(bool autoUpdate READ autoUpdate WRITE setAutoUpdate NOTIFY autoUpdateChanged)
    Q_PROPERTY(Status status READ status NOTIFY statusChanged)
    Q_PROPERTY(QString errorString READ errorString NOTIFY errorChanged)
    Q_PROPERTY(int count READ count NOTIFY countChanged)
    Q_PROPERTY(int limit READ limit WRITE setLimit NOTIFY limitChanged)
    Q_PROPERTY(int offset READ offset WRITE setOffset NOTIFY offsetChanged)
    Q_PROPERTY(QVariant query READ query WRITE setQuery NOTIFY queryChanged)
    Q_PROPERTY(QVariant bounds READ bounds WRITE setBounds NOTIFY boundsChanged)
    Q_PROPERTY(GeocodeError error READ error NOTIFY errorChanged)
    Q_INTERFACES(QQmlParserStatus)

public:
    enum Status {
        Null,
        Ready,
        Loading,
        Error
    };
    Q_ENUM(Status)

    enum GeocodeError {
        NoError = QGeoCodeReply::NoError,
        EngineNotSetError = QGeoCodeReply::EngineNotSetError, //TODO Qt6 consider merge with NotSupportedError
        CommunicationError = QGeoCodeReply::CommunicationError, //TODO Qt6 merge with Map's ConnectionError
        ParseError = QGeoCodeReply::ParseError,
        UnsupportedOptionError = QGeoCodeReply::UnsupportedOptionError, //TODO Qt6 consider rename UnsupportedOperationError
        CombinationError = QGeoCodeReply::CombinationError,
        UnknownError = QGeoCodeReply::UnknownError,
        //we leave gap for future QGeoCodeReply errors

        //QGeoServiceProvider related errors start here
        UnknownParameterError = 100,
        MissingRequiredParameterError
    };
    Q_ENUM(GeocodeError)

    enum Roles {
        LocationRole = Qt::UserRole + 1
    };

    explicit QDeclarativeGeocodeModel(QObject *parent = nullptr);
    virtual ~QDeclarativeGeocodeModel();

    // From QQmlParserStatus
    void classBegin() override {}
    void componentComplete() override;

    // From QAbstractListModel
    int rowCount(const QModelIndex &parent) const override;
    QVariant data(const QModelIndex &index, int role) const override;
    QHash<int,QByteArray> roleNames() const override;

    void setPlugin(QDeclarativeGeoServiceProvider *plugin);
    QDeclarativeGeoServiceProvider *plugin() const;

    void setBounds(const QVariant &boundingArea);
    QVariant bounds() const;

    Status status() const;
    QString errorString() const;
    GeocodeError error() const;

    bool autoUpdate() const;
    void setAutoUpdate(bool update);

    int count() const;
    Q_INVOKABLE QDeclarativeGeoLocation *get(int index);

    int limit() const;
    void setLimit(int limit);
    int offset() const;
    void setOffset(int offset);

    QVariant query() const;
    void setQuery(const QVariant &query);
    Q_INVOKABLE void reset();
    Q_INVOKABLE void cancel();

Q_SIGNALS:
    void countChanged();
    void pluginChanged();
    void statusChanged();
    void errorChanged(); //emitted also for errorString notification
    void locationsChanged();
    void autoUpdateChanged();
    void boundsChanged();
    void queryChanged();
    void limitChanged();
    void offsetChanged();

public Q_SLOTS:
    void update();

protected Q_SLOTS:
    void queryContentChanged();
    void geocodeFinished(QGeoCodeReply *reply);
    void geocodeError(QGeoCodeReply *reply,
                     QGeoCodeReply::Error error,
                     const QString &errorString);
    void pluginReady();

protected:
    QGeoCodingManager *searchManager();
    void setStatus(Status status);
    void setError(GeocodeError error, const QString &errorString);
    bool autoUpdate_ = false;
    bool complete_ = false;

private:
    void setLocations(const QList<QGeoLocation> &locations);
    void abortRequest();
    QGeoCodeReply *reply_ = nullptr;

    QDeclarativeGeoServiceProvider *plugin_ = nullptr;
    QGeoShape boundingArea_;

    QList<QDeclarativeGeoLocation *> declarativeLocations_;

    Status status_ = QDeclarativeGeocodeModel::Null;
    QString errorString_;
    GeocodeError error_ = QDeclarativeGeocodeModel::NoError;
    QVariant queryVariant_;
    QGeoCoordinate coordinate_;
    QDeclarativeGeoAddress *address_ = nullptr;
    QString searchString_;

    int limit_ = -1;
    int offset_ = 0;
};

QT_END_NAMESPACE

#endif
