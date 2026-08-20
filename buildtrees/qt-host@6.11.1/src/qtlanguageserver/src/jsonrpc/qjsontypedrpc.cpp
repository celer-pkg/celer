// Copyright (C) 2021 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR LGPL-3.0-only OR GPL-2.0-only OR GPL-3.0-only
// Qt-Security score:significant
#include "qjsontypedrpc_p.h"
#include <QtCore/QtGlobal>

QT_BEGIN_NAMESPACE

namespace QJsonRpc {
void TypedResponse::addOnCloseAction(const OnCloseAction &act)
{
    switch (m_status) {
    case Status::Started:
        m_onCloseActions.append(act);
        break;
    case Status::Invalid:
        qCWarning(QTypedJson::jsonRpcLog)
                << "addOnCloseAction called on moved QJsonTypedResponse" << idToString(m_id);
        Q_ASSERT(false);
        Q_FALLTHROUGH();
    case Status::SentSuccess:
    case Status::SentError:
        act(m_status, m_id, *m_typedRpc);
        break;
    }
}

void TypedResponse::doOnCloseActions()
{
    m_typedRpc->doOnCloseAction(m_status, m_id);
    for (const auto &a : m_onCloseActions) {
        a(m_status, m_id, *m_typedRpc);
    }
    m_onCloseActions.clear();
}

void TypedResponse::sendErrorResponse(int code, const QByteArray &message)
{
    sendErrorResponse<std::optional<int>>(code, message, std::optional<int>());
}

void TypedRpc::installOnCloseAction(const TypedResponse::OnCloseAction &closeAction)
{
    m_onCloseAction = closeAction;
}

TypedResponse::OnCloseAction TypedRpc::onCloseAction()
{
    return m_onCloseAction;
}

void TypedRpc::doOnCloseAction(TypedResponse::Status status, const IdType &id)
{
    if (m_onCloseAction)
        m_onCloseAction(status, id, *this);
}

} // namespace QJsonRpc

QT_END_NAMESPACE
