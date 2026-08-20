// Copyright (C) 2023 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR BSD-3-Clause
#ifndef ABSTRACTRESOURCE_H
#define ABSTRACTRESOURCE_H

#include <QtQml/qqml.h>
#include <QtNetwork/qrestaccessmanager.h>
#include <QtNetwork/qnetworkrequestfactory.h>
#include <QtCore/qobject.h>

class AbstractResource : public QObject
{
    Q_OBJECT
    QML_ANONYMOUS

public:
    explicit AbstractResource(QObject *parent = nullptr) : QObject (parent)
    {}

    virtual ~AbstractResource() = default;

    void setAccessManager(std::shared_ptr<QRestAccessManager> manager)
    {
        m_manager = manager;
    }

    void setServiceApi(std::shared_ptr<QNetworkRequestFactory> serviceApi)
    {
        m_api = serviceApi;
    }

protected:
    std::shared_ptr<QRestAccessManager> m_manager;
    std::shared_ptr<QNetworkRequestFactory> m_api;
};

#endif // ABSTRACTRESOURCE_H
