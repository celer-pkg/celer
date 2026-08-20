// Copyright (C) 2023 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR BSD-3-Clause

#include "abstractresource.h"
#include "restservice.h"

#if QT_CONFIG(ssl)
#include <QtNetwork/qsslsocket.h>
#endif
#include <QtNetwork/qrestaccessmanager.h>

using namespace Qt::StringLiterals;

RestService::RestService(QObject *parent) : QObject(parent)
{
    m_qnam.setAutoDeleteReplies(true);
    m_manager = std::make_shared<QRestAccessManager>(&m_qnam);
    m_serviceApi = std::make_shared<QNetworkRequestFactory>();
}

void RestService::setUrl(const QUrl &url)
{
    if (m_serviceApi->baseUrl() == url)
        return;
    m_serviceApi->setBaseUrl(url);
    QHttpHeaders authenticationHeaders;
    // reqres.in requires an API-key, see https://reqres.in/signup
    if (url.host().startsWith("reqres"_L1))
        authenticationHeaders.append("x-api-key", "reqres-free-v1");
    m_serviceApi->setCommonHeaders(authenticationHeaders);
    emit urlChanged();
}

QUrl RestService::url() const
{
    return m_serviceApi->baseUrl();
}

bool RestService::sslSupported()
{
#if QT_CONFIG(ssl)
    return QSslSocket::supportsSsl();
#else
    return false;
#endif
}

QQmlListProperty<AbstractResource> RestService::resources()
{
    return {this, &m_resources};
}

void RestService::classBegin()
{
}

void RestService::componentComplete()
{
    for (const auto resource : std::as_const(m_resources)) {
         resource->setAccessManager(m_manager);
         resource->setServiceApi(m_serviceApi);
    }
}
