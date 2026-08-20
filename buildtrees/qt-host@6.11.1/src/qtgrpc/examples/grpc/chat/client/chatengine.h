// Copyright (C) 2024 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR BSD-3-Clause

#ifndef CHATENGINE_H
#define CHATENGINE_H

#include "chatmessages.qpb.h"
#include "clientworker.h"
#include "chatmessagemodel.h"
#include "userstatusmodel.h"
#include "logmodel.h"

#include <QtQmlIntegration/QtQmlIntegration>

#include <QtCore/QObject>
#include <QtCore/QString>
#include <QtCore/QThread>
#include <QtCore/QTimer>

#include <memory>

QT_BEGIN_NAMESPACE
class QClipboard;
QT_END_NAMESPACE

class ChatEngine : public QObject
{
    Q_OBJECT
    QML_ELEMENT
    QML_SINGLETON

    Q_PROPERTY(UserStatusModel *userStatusModel READ userStatusModel NOTIFY chatStreamFinished)
    Q_PROPERTY(ChatMessageModel *chatMessageModel READ chatMessageModel NOTIFY chatStreamFinished)
    Q_PROPERTY(LogModel *logModel READ logModel CONSTANT)
    Q_PROPERTY(QString username READ username NOTIFY chatStateChanged)
    Q_PROPERTY(QUrl hostUri READ hostUri WRITE setHostUri NOTIFY hostUriChanged)
    Q_PROPERTY(Backend::ChatState chatState READ chatState NOTIFY chatStateChanged)

public:
//! [client-1]
    explicit ChatEngine(QObject *parent = nullptr);
    ~ChatEngine() override;

    // Register operations
    Q_INVOKABLE void registerUser(const chat::Credentials &credentials);
    // ChatRoom operations
    Q_INVOKABLE void login(const chat::Credentials &credentials);
    Q_INVOKABLE void logout();
    Q_INVOKABLE void sendText(const QString &message);
    Q_INVOKABLE void sendFile(const QUrl &url);
    Q_INVOKABLE void sendFiles(const QList<QUrl> &urls);
    Q_INVOKABLE bool sendFilesFromClipboard();
//! [client-1]

    Q_INVOKABLE static void openUrl(const QByteArray &url);
    Q_INVOKABLE static QList<QString> findLocalIps();
    Q_INVOKABLE static QList<QString> displayServerPorts();
    Q_INVOKABLE QString getSchemeSymbol();

    UserStatusModel *userStatusModel() noexcept { return m_userStatusModel.get(); }
    ChatMessageModel *chatMessageModel() noexcept { return m_chatMessageModel.get(); }
    LogModel *logModel() noexcept { return m_logModel.get(); }

    [[nodiscard]] const QString &username() const & { return m_userCredentials.name(); }

    const QUrl &hostUri() const & { return m_hostUri; }
    void setHostUri(const QUrl &hostUri);

    Backend::ChatState chatState() const { return m_chatState; }

signals:
    void registerFinished(const QGrpcStatus &status);

    void chatError(const QString &message);
    void chatStateChanged();
    void chatStreamFinished(const QGrpcStatus &status);
    void hostUriChanged();

protected:
    bool eventFilter(QObject *obj, QEvent *event) override;

private:
    void initialize();
    void setUserStatus(chat::UserStatus::Type status);

private:
    std::shared_ptr<LogModel> m_logModel;
    QThread m_clientThread;
    ClientWorker *m_clientWorker;
    QUrl m_hostUri;

    chat::Credentials m_userCredentials;
    chat::UserStatus::Type m_userStatus;

    std::unique_ptr<UserStatusModel> m_userStatusModel;
    std::unique_ptr<ChatMessageModel> m_chatMessageModel;

    QClipboard *m_clipboard;
    QTimer m_inactivityTimer;
    Backend::ChatState m_chatState = {};

    Q_DISABLE_COPY_MOVE(ChatEngine)
};

#endif // CHATENGINE_H
