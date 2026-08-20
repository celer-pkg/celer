// Copyright (C) 2024 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR BSD-3-Clause

#include "chatengine.h"
#include "chatmessages.qpb.h"
#include "clientworker.h"

#include <QtGui/QClipboard>
#include <QtGui/QDesktopServices>
#include <QtGui/QGuiApplication>
#include <QtGui/QImage>

#include <QtNetwork/QHostAddress>
#include <QtNetwork/QNetworkInterface>

#include <QtCore/QDateTime>
#include <QtCore/QFile>
#include <QtCore/QList>
#include <QtCore/QUrl>

ChatEngine::ChatEngine(QObject *parent)
    : QObject(parent), m_logModel(std::make_shared<LogModel>()),
      m_clientWorker(new ClientWorker(m_logModel)), m_clipboard(QGuiApplication::clipboard())
{
    initialize();
#if QT_CONFIG(ssl)
    setHostUri(QUrl("https://localhost:65002"));
#else
    setHostUri(QUrl("http://localhost:65003"));
#endif
    //! [client-3]
    m_clientWorker->moveToThread(&m_clientThread);
    m_clientThread.start();
    connect(&m_clientThread, &QThread::finished, m_clientWorker, &QObject::deleteLater);
    connect(m_clientWorker, &ClientWorker::registerFinished, this, &ChatEngine::registerFinished);
    connect(m_clientWorker, &ClientWorker::chatError, this, &ChatEngine::chatError);
    //! [client-3]
    connect(m_clientWorker, &ClientWorker::chatStreamFinished, this,
            [this](const QGrpcStatus &status) {
                initialize();
                emit chatStreamFinished(status);
            });
    connect(m_clientWorker, &ClientWorker::chatStateChanged, this,
            [this](Backend::ChatState state) {
                if (state == Backend::ChatState::Connected) {
                    m_inactivityTimer.start();
                    m_userStatus = chat::UserStatus::Type::ACTIVE;
                }
                m_chatState = state;
                emit chatStateChanged();
            });

    connect(m_clientWorker, &ClientWorker::chatStreamMessageReceived, this,
            [this](const chat::ChatMessage &message) {
                // The client worker already does some message pre-processing so that
                // we just have to care about updating the models correctly.
                bool newUser = m_userStatusModel->updateUserStatus(message);
                if (message.hasUserStatus()
                    && message.userStatus().type() == chat::UserStatus::Type::LOGIN) {
                    if (!newUser)
                        return; // don't append reconnects
                    if (message.username() == username())
                        return; // don't append our own login
                }
                m_chatMessageModel->appendMessage(message);
            });

    // Simple mechanism to detect user absence from the application
    m_inactivityTimer.setInterval(120'000); // Inactive after 2min
    connect(&m_inactivityTimer, &QTimer::timeout, this, [this]() {
        setUserStatus(chat::UserStatus::Type::INACTIVE);
    });
}

ChatEngine::~ChatEngine()
{
    m_clientThread.quit();
    m_clientThread.wait();
}

//! [client-4a]
void ChatEngine::registerUser(const chat::Credentials &credentials)
{
    QMetaObject::invokeMethod(m_clientWorker, &ClientWorker::registerUser, credentials);
}
//! [client-4a]

void ChatEngine::login(const chat::Credentials &credentials)
{
    m_userCredentials = credentials;
    QMetaObject::invokeMethod(m_clientWorker, &ClientWorker::login, credentials);
}

void ChatEngine::logout()
{
    QMetaObject::invokeMethod(m_clientWorker, &ClientWorker::logout);
}

//! [client-7a]
void ChatEngine::sendText(const QString &message)
{
    if (message.trimmed().isEmpty())
        return;

    if (auto request = m_clientWorker->createMessage()) {
        chat::TextMessage tmsg;
        tmsg.setContent(message.toUtf8());
        request->setText(std::move(tmsg));
        QMetaObject::invokeMethod(m_clientWorker, &ClientWorker::sendMessage, *request);
        m_chatMessageModel->appendMessage(*request);
    }
}
//! [client-7a]

void ChatEngine::sendFile(const QUrl &url)
{
    QMetaObject::invokeMethod(m_clientWorker, &ClientWorker::sendFile, url);
}

void ChatEngine::sendFiles(const QList<QUrl> &urls)
{
    QMetaObject::invokeMethod(m_clientWorker, &ClientWorker::sendFiles, urls);
}

bool ChatEngine::sendFilesFromClipboard()
{
    if (!m_clipboard)
        return false;

    const QMimeData *mime = m_clipboard->mimeData();
    if (!mime)
        return false;

    if (mime->hasImage()) {
        auto img = mime->imageData().value<QImage>();
        if (img.isNull()) {
            qDebug("Received empty clipbaord image");
            return false;
        }

        const auto downloadDir = m_clientWorker->getValidDownloadDir();
        if (!downloadDir)
            return false;
        const auto path = downloadDir->path() + QDir::separator() + "clipboard_"
            + QUuid::createUuid().toString() + ".png";

        if (!img.save(path)) {
            qWarning() << "Couldn't write clipboard image: " << path;
            return false;
        }

        QMetaObject::invokeMethod(m_clientWorker, &ClientWorker::sendFile,
                                  QUrl::fromLocalFile(path));
        return true;
    } else if (mime->hasUrls()) {
        QMetaObject::invokeMethod(m_clientWorker, &ClientWorker::sendFiles, mime->urls());
        return true;
    }

    return false;
}

void ChatEngine::openUrl(const QByteArray &url)
{
    QDesktopServices::openUrl(QUrl(url));
}

QList<QString> ChatEngine::findLocalIps()
{
    QList<QString> out;
    const QList<QHostAddress> addresses = QNetworkInterface::allAddresses();
    for (const QHostAddress &address : addresses) {
        if (address.protocol() == QAbstractSocket::IPv4Protocol && !address.isLoopback())
            out.append(address.toString());
    }
    return out;
}

QList<QString> ChatEngine::displayServerPorts()
{
    return {
#if QT_CONFIG(ssl)
        "65002 (https)",
#endif
        "65003 (http)",
    };
}

QString ChatEngine::getSchemeSymbol()
{
#ifdef USE_EMOJI_FONT
    const auto s = m_hostUri.scheme();
    if (s == "https")
        return "🛡️";
    return "🔓";
#else
    return "";
#endif
}

void ChatEngine::setHostUri(const QUrl &hostUri)
{
    if (m_hostUri == hostUri)
        return;
    m_hostUri = hostUri;
    QMetaObject::invokeMethod(m_clientWorker, &ClientWorker::setHostUri, hostUri);
    emit hostUriChanged();
}

bool ChatEngine::eventFilter(QObject *obj, QEvent *event)
{
    if (event->type() == QEvent::KeyPress || event->type() == QEvent::MouseMove
        || event->type() == QEvent::TouchBegin || event->type() == QEvent::Enter) {

        if (m_userStatus == chat::UserStatus::Type::INACTIVE)
            setUserStatus(chat::UserStatus::Type::ACTIVE);

        // Restart timer only if user is active
        if (m_userStatus == chat::UserStatus::Type::ACTIVE)
            m_inactivityTimer.start();
    }

    if (event->type() == QEvent::Close) {
        // Log out active session and delay quitting to ensure graceful shutdown.
        logout();
        QTimer::singleShot(125, QCoreApplication::quit);
        return true;
    }

    return QObject::eventFilter(obj, event);
}

void ChatEngine::initialize()
{
    m_userCredentials = {};
    m_userStatus = {};
    m_userStatusModel = std::make_unique<UserStatusModel>();
    m_chatMessageModel = std::make_unique<ChatMessageModel>();
    connect(m_clientWorker, &ClientWorker::fileMessageSent, m_chatMessageModel.get(),
            &ChatMessageModel::appendMessage);
}

void ChatEngine::setUserStatus(chat::UserStatus::Type status)
{
    if (m_userStatus == status || m_userStatus == chat::UserStatus::Type::NONE)
        return;

    switch (status) {
        case chat::UserStatus::Type::ACTIVE:
            m_inactivityTimer.start();
            break;
        case chat::UserStatus::Type::INACTIVE:
            m_inactivityTimer.stop();
            break;
        default:
            return;
    }

    m_userStatus = status;

    auto request = m_clientWorker->createMessage();
    if (!request)
        return;

    chat::UserStatus userStatus;
    userStatus.setType(m_userStatus);
    request->setUserStatus(std::move(userStatus));

    // Send status update to the server and update all models.
    m_userStatusModel->updateUserStatus(*request);
    QMetaObject::invokeMethod(m_clientWorker, &ClientWorker::sendMessage, *request);
    m_chatMessageModel->appendMessage(*request);
}
