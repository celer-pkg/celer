// Copyright (C) 2023 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR BSD-3-Clause

#ifndef RESTSERVICE_H
#define RESTSERVICE_H

#include "abstractresource.h"

#include <QtQml/qqml.h>
#include <QtQml/qqmlparserstatus.h>
#include <QtNetwork/qrestaccessmanager.h>
#include <QtNetwork/qnetworkrequestfactory.h>
#include <QtCore/qobject.h>
#include <QtCore/qurl.h>

class RestService : public QObject, public QQmlParserStatus
{
    Q_OBJECT
    Q_PROPERTY(QUrl url READ url WRITE setUrl NOTIFY urlChanged)
    Q_PROPERTY(bool sslSupported READ sslSupported CONSTANT)
    Q_PROPERTY(QQmlListProperty<AbstractResource> resources READ resources)
    Q_CLASSINFO("DefaultProperty", "resources")
    Q_INTERFACES(QQmlParserStatus)
    QML_ELEMENT

public:
    explicit RestService(QObject *parent = nullptr);
    ~RestService() override = default;

    bool sslSupported();

    QUrl url() const;
    void setUrl(const QUrl &url);

    void classBegin() override;
    void componentComplete() override;

    QQmlListProperty<AbstractResource> resources();

signals:
    void urlChanged();

private:
    QList<AbstractResource*> m_resources;
    QNetworkAccessManager m_qnam;
    std::shared_ptr<QRestAccessManager> m_manager;
    std::shared_ptr<QNetworkRequestFactory> m_serviceApi;
};

#endif // RESTSERVICE_H
