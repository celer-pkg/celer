// Copyright (C) 2024 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR BSD-3-Clause

#ifndef CLIENTWORKER_H
#define CLIENTWORKER_H

#include "chatmessages.qpb.h"
#include "qtgrpcchat_client.grpc.qpb.h"

#include "logmodel.h"

#include <QtQmlIntegration/QtQmlIntegration>

#include <QtCore/QDir>
#include <QtCore/QObject>
#include <QtCore/QUrl>

#include <memory>
#include <optional>
#include <qqmlintegration.h>

// Export the Backend namespace to QML
namespace Backend {
    Q_NAMESPACE
    Q_CLASSINFO("RegisterEnumClassesUnscoped", "false")
    enum class ChatState {
        Disconnected = 0,
        Connecting,
        Connected,
        Disconnecting,
    };
    Q_ENUM_NS(ChatState);
    QML_FOREIGN_NAMESPACE(Backend)
    QML_NAMED_ELEMENT(Backend)
}

// ClientWorker runs in a separate thread to offload expensive operations.
class ClientWorker : public QObject
{
    Q_OBJECT
public:
//! [client-2a]
    explicit ClientWorker(std::shared_ptr<LogModel> logger, QObject *parent = nullptr);
    ~ClientWorker() override;

//! [client-2a]
    std::optional<QDir> getValidDownloadDir() const;
    std::optional<chat::ChatMessage> createMessage() const;

signals:
    void registerFinished(const QGrpcStatus &status);
    void chatError(const QString &message);
    void chatStreamFinished(const QGrpcStatus &status);
    void chatStateChanged(Backend::ChatState state);
    void chatStreamMessageReceived(const chat::ChatMessage &message);
    void fileMessageSent(const chat::ChatMessage &fileMessage);

//! [client-2b]
public Q_SLOTS:
    void registerUser(const chat::Credentials &credentials);
    void login(const chat::Credentials &credentials);
    void logout();
    void sendFile(const QUrl &url);
    void sendFiles(const QList<QUrl> &urls);
    void sendMessage(const chat::ChatMessage &message);
//! [client-2b]
    void setHostUri(const QUrl &hostUri);

private:
    void setState(Backend::ChatState state);
    void connectStream(const QGrpcCallOptions &opts);
    bool initializeClient();
    static QString toUniqueFilePath(const QString &filePath);
    QUrl saveFileRequest(const chat::FileMessage &fileRequest) const;

private:
    QUrl m_hostUri;
    bool m_hostUriDirty = false;
    std::unique_ptr<chat::QtGrpcChat::Client> m_client;

    std::unique_ptr<QGrpcBidiStream> m_chatStream;
    Backend::ChatState m_chatState = {};
    chat::ChatMessage m_chatResponse;
    QList<QUrl> m_fileRequestBuffer;

    chat::Credentials m_userCredentials;
    std::shared_ptr<LogModel> m_logModel;

    Q_DISABLE_COPY_MOVE(ClientWorker)
};


Q_DECLARE_METATYPE(ClientWorker)

#endif // CLIENTWORKER_H
