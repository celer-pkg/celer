// Copyright (C) 2021 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR LGPL-3.0-only OR GPL-2.0-only OR GPL-3.0-only
// Qt-Security score:significant

#include <QtJsonRpc/private/qjsonrpcprotocol_p.h>
#include <QtLanguageServer/private/qlanguageserverprotocol_p.h>
#include <QtLanguageServer/private/qlanguageservergen_p_p.h>

#include <QtCore/qjsonobject.h>

#include <memory>

QT_BEGIN_NAMESPACE

using namespace QLspSpecification;
using namespace Qt::StringLiterals;

/*!
\internal
\class QLanguageServerProtocol
\brief Implements the language server protocol

QLanguageServerProtocol objects handles the language server
protocol both to send and receive (ask the client and reply to the
client).

To ask the client you use the requestXX or notifyXX methods.
To reply to client request you have to register your handlers via
registerXXRequestHandler, notifications can be handled connecting the
receivedXXNotification signals.

The method themselves are implemented in qlanguageservergen*
files which are generated form the specification.

You have to provide a function to send the data that the protocol
generates to the constructor, and you have to feed the data you
receive to it via the receivedData method.

Limitations: for the client use case (Creator),the handling of partial
results could be improved. A clean solution should handle the progress
notification, do some extra tracking and give the partial results back
to the call that did the request.
*/

QLanguageServerProtocol::QLanguageServerProtocol(const QJsonRpcTransport::DataHandler &sender)
    : ProtocolGen(std::make_unique<ProtocolGenPrivate>())
{
    transport()->setDataHandler(sender);
    transport()->setDiagnosticHandler([this](QJsonRpcTransport::DiagnosticLevel l,
                                             const QString &msg) {
        handleResponseError(
                ResponseError { int(ErrorCodes::InternalError), msg.toUtf8(),
                                QJsonObject({ { u"errorLevel"_s,
                                                ((l == QJsonRpcTransport::DiagnosticLevel::Error)
                                                         ? u"error"_s
                                                         : u"warning"_s) } }) });
    });
}

void QLanguageServerProtocol::receiveData(const QByteArray &data)
{
    transport()->receiveData(data);
}

QT_END_NAMESPACE
