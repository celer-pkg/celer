// Copyright (C) 2024 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR BSD-3-Clause

#include "clientworker.h"
#include "logginginterceptor.h"
#include "chatmessages.qpb.h"

#include <QtQml/QQmlFile>

#include <QtGrpc/QGrpcCallOptions>
#include <QtGrpc/QGrpcChannelOptions>
#include <QtGrpc/QGrpcHttp2Channel>
#include <QtGrpc/QGrpcInterceptorChain>

#include <QtNetwork/QSslCertificate>
#include <QtNetwork/QSslConfiguration>

#include <QtCore/QFile>
#include <QtCore/QFileInfo>
#include <QtCore/QMimeDatabase>
#include <QtCore/QStandardPaths>
#include <QtCore/QUrl>
#include <QtCore/QTimer>

#include <chrono>

using namespace std::chrono_literals;
using namespace Backend;

ClientWorker::ClientWorker(std::shared_ptr<LogModel> logger, QObject *parent)
    : QObject(parent), m_logModel(std::move(logger))
{
}

ClientWorker::~ClientWorker() = default;

std::optional<QDir> ClientWorker::getValidDownloadDir() const
{
    if (!m_client || !m_chatStream || m_userCredentials.name().isEmpty())
        return {};
    const auto path = QStandardPaths::writableLocation(QStandardPaths::DownloadLocation);
    if (path.isEmpty()) {
        qWarning("No download location found for standard path!");
        return {};
    }
    QDir downloadDir(path + "/QtGrpcChat/" + m_userCredentials.name());
    if (!downloadDir.exists())
        downloadDir.mkpath(".");
    return downloadDir;
}

std::optional<chat::ChatMessage> ClientWorker::createMessage() const
{
    if (m_userCredentials.name().isEmpty())
        return {};
    chat::ChatMessage message;
    message.setTimestamp(QDateTime::currentDateTimeUtc().toMSecsSinceEpoch());
    message.setUsername(m_userCredentials.name());
    return message;
}

void ClientWorker::setHostUri(const QUrl &hostUri)
{
    const auto scheme = hostUri.scheme();
    if (scheme != "http" && scheme != "https") {
        emit chatError(tr("Specified scheme '%1' is not supported. Use 'http' or 'https'.")
                           .arg(scheme));
        return;
    }
    if (m_hostUri == hostUri)
        return;
    m_hostUri = hostUri;
    m_hostUriDirty = true;
}

//! [client-4b]
void ClientWorker::registerUser(const chat::Credentials &credentials)
{
    if (credentials.name().isEmpty() || credentials.password().isEmpty()) {
        emit chatError(tr("Invalid credentials for registration"));
        return;
    }

    if ((!m_client || m_hostUriDirty) && !initializeClient()) {
        emit chatError(tr("Failed registration: unabled to initialize client"));
        return;
    }

    auto reply = m_client->Register(credentials, QGrpcCallOptions{}.setDeadlineTimeout(2s));
    const auto *replyPtr = reply.get();
    connect(
        replyPtr, &QGrpcCallReply::finished, this,
        [this, reply = std::move(reply)](const QGrpcStatus &status) {
            emit registerFinished(status);
        },
        Qt::SingleShotConnection);
}
//! [client-4b]

//! [client-5a]
void ClientWorker::login(const chat::Credentials &credentials)
{
    if (credentials.name().isEmpty() || credentials.password().isEmpty()) {
        emit chatError(tr("Invalid credentials for login"));
        return;
    }
//! [client-5a]
    if ((!m_client || m_hostUriDirty) && !initializeClient()) {
        emit chatError(tr("Failed login: unabled to initialize client"));
        return;
    }

    if (m_chatStream) {
        emit chatError(tr("Already logged in to an active session."));
        return;
    }
    m_userCredentials = credentials;

//! [client-5b]
    QGrpcCallOptions opts;
    opts.setMetadata({
        { "user-name",     credentials.name().toUtf8()     },
        { "user-password", credentials.password().toUtf8() },
    });
    connectStream(opts);
}
//! [client-5b]

void ClientWorker::logout()
{
    if (!m_chatStream || m_chatState == ChatState::Disconnected
        || m_chatState == ChatState::Disconnecting) {
        return;
    }
    setState(ChatState::Disconnecting);
    m_chatStream->writesDone();
}

void ClientWorker::sendFile(const QUrl &url)
{
    // Send in chunks of 128KiB
    constexpr auto MaxMessageSize = quint64(128 * 1024);

    if (m_chatState == ChatState::Connecting) {
        m_fileRequestBuffer.push_back(url);
        return;
    }

    if (!m_chatStream || m_chatState != ChatState::Connected) {
        emit chatError("Unable to send file: no active chat session");
        return;
    }

    const auto fileUrl = QQmlFile::urlToLocalFileOrQrc(url);
    const auto utf8Url = url.toString().toUtf8();
    const auto fileName = QFileInfo(fileUrl).fileName();

    if (!QFile::exists(fileUrl)) {
        emit chatError(tr("The file \"%1\" could not be found").arg(fileName));
        return;
    }

    static QMimeDatabase mimeDb;

    QFile file(fileUrl);
    if (!file.open(QIODevice::ReadOnly)) {
        emit chatError(tr("The file \"%1\" could not be opened").arg(fileName));
        return;
    }

    chat::FileMessage fileMessage;
    fileMessage.setName(fileName);
    fileMessage.setSize(uint64_t(file.size()));

    QMimeType mime = mimeDb.mimeTypeForFile(fileUrl);
    const auto mimeName = mime.name();
    if (mimeName.contains("image"))
        fileMessage.setType(chat::FileMessage::Type::IMAGE);
    else if (mimeName.contains("audio"))
        fileMessage.setType(chat::FileMessage::Type::AUDIO);
    else if (mimeName.contains("video"))
        fileMessage.setType(chat::FileMessage::Type::VIDEO);
    else if (mime.inherits("text/plain"))
        fileMessage.setType(chat::FileMessage::Type::TEXT);
    else
        fileMessage.setType(chat::FileMessage::Type::UNKNOWN);

    // Send the file as chunks when the maximum supported message size is exceeded.
    if (auto count = (quint64(file.size()) + MaxMessageSize - 1) / MaxMessageSize; count > 1) {
        chat::FileMessage::Continuation continuation;
        continuation.setIndex(0);
        continuation.setCount(count);
        continuation.setUuid(QUuid::createUuid());
        fileMessage.setContinuation(std::move(continuation));
    }

    auto message = createMessage();
    if (!message)
        return;

    while (!file.atEnd()) {
        QByteArray content = file.read(MaxMessageSize);

        message->setFile(fileMessage);
        message->file().setContent(std::move(content));
        m_chatStream->writeMessage(*message);

        // We only need the content for writing it to the disk. The visual
        // representation only needs the url.
        message->file().setContent(utf8Url);
        emit fileMessageSent(*message);

        if (fileMessage.hasContinuation()) {
            auto c = fileMessage.continuation();
            c.setIndex(c.index() + 1);
            fileMessage.setContinuation(std::move(c));
        }
    }
}

void ClientWorker::sendFiles(const QList<QUrl> &urls)
{
    for (const auto &url : urls)
        sendFile(url);
}

//! [client-7b]
void ClientWorker::sendMessage(const chat::ChatMessage &message)
{
    if (!m_chatStream || m_chatState != ChatState::Connected) {
        emit chatError(tr("Unable to send message"));
        return;
    }
    m_chatStream->writeMessage(message);
}
//! [client-7b]

void ClientWorker::setState(ChatState state)
{
    if (m_chatState == state)
        return;
    m_chatState = state;
    emit chatStateChanged(state);
}

//! [client-6a]
void ClientWorker::connectStream(const QGrpcCallOptions &opts)
{
//! [client-6a]
    auto initialMessage = createMessage();
    if (!initialMessage)
        return;

//! [client-6b]
    m_chatStream = m_client->ChatRoom(*initialMessage, opts);
//! [client-6b]

    if (!m_chatStream) {
        m_userCredentials = {};
        emit chatError(tr("initiating bidirectional stream failed"));
        return;
    }

//! [client-6c]
    connect(m_chatStream.get(), &QGrpcBidiStream::finished, this,
            [this, opts](const QGrpcStatus &status) {
                if (m_chatState == ChatState::Connected) {
                    // If we're connected retry again in 250 ms, no matter the error.
                    QTimer::singleShot(250, [this, opts]() { connectStream(opts); });
                } else {
                    setState(ChatState::Disconnected);
                    m_chatResponse = {};
                    m_userCredentials = {};
                    m_chatStream.reset();
                    emit chatStreamFinished(status);
                }
            });
//! [client-6c]

//! [client-6d]
    connect(m_chatStream.get(), &QGrpcBidiStream::messageReceived, this, [this] {
//! [client-6d]
        if (!m_chatStream->read(&m_chatResponse)) {
            qWarning("Failed to deserialize!");
            return;
        }

//! [client-6e]
        switch (m_chatResponse.contentField()) {
        case chat::ChatMessage::ContentFields::UninitializedField:
            qDebug("Received uninitialized message");
            return;
        case chat::ChatMessage::ContentFields::Text:
            if (m_chatResponse.text().content().isEmpty())
                return;
            break;
        case chat::ChatMessage::ContentFields::File:
            // Download any file messages and store the downloaded URL in the
            // content, allowing the model to reference it from there.
            m_chatResponse.file()
                .setContent(saveFileRequest(m_chatResponse.file()).toString().toUtf8());
            break;
//! [client-6e]
        case chat::ChatMessage::ContentFields::UserStatus:
            if (m_chatResponse.userStatus().type() == chat::UserStatus::Type::LOGIN
                && m_chatResponse.username() == m_userCredentials.name()) {
                setState(ChatState::Connected); // Login successful
                for (const auto &m : m_fileRequestBuffer) // Send potentially buffered messages
                    sendFile(m);
                m_fileRequestBuffer.clear();
            }
        }
//! [client-6f]
        emit chatStreamMessageReceived(m_chatResponse);
    });
    setState(Backend::ChatState::Connecting);
}
//! [client-6f]

bool ClientWorker::initializeClient()
{
    QGrpcChannelOptions opts;
#if QT_CONFIG(ssl)
    //! [client-ssl]
    if (m_hostUri.scheme() == "https") {
        if (!QSslSocket::supportsSsl()) {
            emit chatError(tr("The device doesn't support SSL. Please use the 'http' scheme."));
            return false;
        }
        QFile crtFile(":/res/root.crt");
        if (!crtFile.open(QFile::ReadOnly)) {
            qFatal("Unable to load root certificate");
            return false;
        }

        QSslConfiguration sslConfig;
        QSslCertificate crt(crtFile.readAll());
        sslConfig.addCaCertificate(crt);
        sslConfig.setProtocol(QSsl::TlsV1_2OrLater);
        sslConfig.setAllowedNextProtocols({ "h2" }); // Allow HTTP/2

        // Disable hostname verification to allow connections from any local IP.
        // Acceptable for development but avoid in production for security.
        sslConfig.setPeerVerifyMode(QSslSocket::VerifyNone);
        opts.setSslConfiguration(sslConfig);
    }
    //! [client-ssl]
#endif

    m_client = std::make_unique<chat::QtGrpcChat::Client>();

    //! [interceptor-1]
    QGrpcInterceptorChain interceptorChain;
    interceptorChain.add(std::make_unique<LoggingInterceptor>(m_logModel));
    auto channel = std::make_shared<QGrpcHttp2Channel>(m_hostUri, opts,
                                                       std::move(interceptorChain));
    //! [interceptor-1]

    if (!m_client->attachChannel(std::move(channel)))
        return false;
    m_hostUriDirty = false;
    return true;
}

QString ClientWorker::toUniqueFilePath(const QString &filePath)
{
    qsizetype count = 0;
    QFileInfo fileInfo(filePath);
    QString newFilePath = filePath;

    while (QFile::exists(newFilePath)) {
        newFilePath = QString("%1/%2_%3.%4")
                          .arg(fileInfo.absolutePath(), fileInfo.completeBaseName())
                          .arg(count)
                          .arg(fileInfo.suffix());
        ++count;
    }
    return newFilePath;
}

QUrl ClientWorker::saveFileRequest(const chat::FileMessage &fileRequest) const
{
    const auto downloadDir = getValidDownloadDir();
    if (!downloadDir)
        return {};

    QString savedFile = downloadDir->path() + QDir::separator();
    if (fileRequest.hasContinuation()) {
        savedFile += fileRequest.continuation().uuid().toString();
    } else {
        // Since this is the only transmission it must be locally unique already
        savedFile = toUniqueFilePath(savedFile += fileRequest.name());
    }

    QFile writer(savedFile);
    if (!writer.open(QIODevice::WriteOnly | QIODevice::Append)) {
        qWarning() << "cannot open" << writer.fileName();
        return {};
    }
    if (writer.write(fileRequest.content()) < 0) {
        qDebug() << "Failed to write content to file" << writer.fileName();
        return {};
    }

    if (fileRequest.hasContinuation()
        && fileRequest.continuation().index() == fileRequest.continuation().count() - 1) {
        savedFile = toUniqueFilePath(downloadDir->path() + '/' + fileRequest.name());
        if (!writer.rename(savedFile))
            return {};
    } else if (fileRequest.hasContinuation()) {
        return {}; // Don't return the path to incomplete downloads
    }

    return QUrl::fromLocalFile(savedFile);
}
