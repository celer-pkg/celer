// Copyright (C) 2023 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR BSD-3-Clause

#include "basiclogin.h"

#include <QtNetwork/qhttpheaders.h>
#include <QtNetwork/qrestaccessmanager.h>
#include <QtNetwork/qrestreply.h>

#include <QtCore/qjsondocument.h>
#include <QtCore/qjsonobject.h>

using namespace Qt::StringLiterals;

static constexpr auto tokenField = "token"_L1;
static constexpr auto emailField = "email"_L1;
static constexpr auto idField = "id"_L1;

BasicLogin::BasicLogin(QObject *parent)
    : AbstractResource(parent)
{
}

QString BasicLogin::user() const
{
    return m_user ? m_user->email : QString{};
}

bool BasicLogin::loggedIn() const
{
    return m_user.has_value();
}

void BasicLogin::login(const QVariantMap &data)
{
    m_manager->post(m_api->createRequest(m_loginPath), data, this, [this,  data] (QRestReply &reply) {
        m_user.reset();
        if (const auto json = reply.readJson();
            json && json->isObject() && json->object().contains(tokenField)) {
            m_user = User{data.value(emailField).toString(),
                          (*json)[tokenField].toVariant().toByteArray(),
                          data.value(idField).toInt()};
        }
        QHttpHeaders headers = m_api->commonHeaders();
        headers.replaceOrAppend("token", m_user ? m_user->token : ""_ba);
        m_api->setCommonHeaders(headers);
        emit userChanged();
    });
}

void BasicLogin::logout()
{
    m_manager->post(m_api->createRequest(m_logoutPath), ""_ba, this, [this](QRestReply &reply) {
        if (reply.isSuccess()) {
            m_user.reset();
            QHttpHeaders headers = m_api->commonHeaders();
            headers.removeAll("token");
            m_api->setCommonHeaders(headers);
            emit userChanged();
        }
    });
}
