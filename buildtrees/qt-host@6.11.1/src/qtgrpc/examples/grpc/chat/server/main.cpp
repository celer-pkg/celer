// Copyright (C) 2019 Alexey Edelev <semlanik@gmail.com>
// Copyright (C) 2024 Dennis Oberst <dennis.ob@protonmail.com>
// Copyright (C) 2024 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR BSD-3-Clause

#include "chatmessages.pb.h"
#include "qtgrpcchat.grpc.pb.h"
#include "credentials/certificates.h"

#include <grpc++/grpc++.h>
#include <grpc++/security/server_credentials.h>
#include <grpc++/ext/proto_server_reflection_plugin.h>

#include <chrono>
#include <cstdlib>
#include <fstream>
#include <iostream>
#include <memory>
#include <mutex>
#include <queue>
#include <string>
#include <vector>

namespace {

std::ostream &operator<<(std::ostream &os, const chat::Credentials &u)
{
    return os << u.name() << ' ' << u.password() << '\n';
}

std::istream &operator>>(std::istream &is, chat::Credentials &u)
{
    std::string name, password;
    if (is >> name >> password) {
        u.set_name(std::move(name));
        u.set_password(std::move(password));
    }
    return is;
}

} // namespace

class QtGrpcChatService;

// A ChatRoomReactor instance represent the bidirectional streaming RPC, i.e. a connected client.
class ChatRoomReactor : public grpc::ServerBidiReactor<chat::ChatMessage, chat::ChatMessage>
{
public:
    explicit ChatRoomReactor(QtGrpcChatService *parent, grpc::CallbackServerContext *context);

    const std::string &name() const noexcept { return m_user.name(); }
    chat::UserStatus::Type userStatusType() const noexcept { return m_userStatusType; }

//! [server-4]
    // Share \a response. It will be kept alive until the last write operation finishes.
    void startSharedWrite(std::shared_ptr<chat::ChatMessage> response)
    {
        std::scoped_lock lock(m_writeMtx);
        if (m_response) {
            m_responseQueue.emplace(std::move(response));
        } else {
            m_response = std::move(response);
            StartWrite(m_response.get());
        }
    }
//! [server-4]

    std::shared_ptr<chat::ChatMessage> createMessage() const
    {
        auto message = std::make_shared<chat::ChatMessage>();
        message->set_username(name());
        message->set_timestamp(std::chrono::system_clock::now().time_since_epoch().count());
        return message;
    }

private:
    void OnDone() override;
    void OnCancel() override;
    void OnReadDone(bool ok) override;
    void OnWriteDone(bool ok) override;

private:
    static const std::string &userNameHeader()
    {
        static std::string userNameHeader = "user-name";
        return userNameHeader;
    }
    static const std::string &userPasswordHeader()
    {
        static std::string userPasswordHeader = "user-password";
        return userPasswordHeader;
    }

    bool m_hasLogin = false;
    grpc::CallbackServerContext *m_context;
    QtGrpcChatService *m_service;
    chat::Credentials m_user;
    chat::UserStatus::Type m_userStatusType;

    std::shared_ptr<chat::ChatMessage> m_request;
    std::shared_ptr<chat::ChatMessage> m_response;
    std::queue<std::shared_ptr<chat::ChatMessage>> m_responseQueue;
    std::mutex m_writeMtx;
};

//! [server-1]
class QtGrpcChatService final : public chat::QtGrpcChat::CallbackService
//! [server-1]
{
public:
    static const std::string &httpsAddress()
    {
        static std::string address = "0.0.0.0:65002";
        return address;
    }
    static const std::string &httpAddress()
    {
        static std::string address = "0.0.0.0:65003";
        return address;
    }

    const std::string &dbPath() const noexcept { return m_dbPath; }
    const std::vector<ChatRoomReactor *> &activeClients() & noexcept { return m_activeClients; }

    bool login(ChatRoomReactor *client)
    {
        std::unique_lock lock(m_activeClientsMtx);
        if (std::find(m_activeClients.begin(), m_activeClients.end(), client) != m_activeClients.end())
            return false;

        std::cout << "login:  " << client << ", " << client->name() << std::endl;
        m_activeClients.push_back(client);

        // Notify all clients including the connecting one about the new login
        auto msg = client->createMessage();
        msg->mutable_user_status()->set_type(chat::UserStatus::LOGIN);
        broadcast(msg, nullptr);

        // Send all active clients of the session to the connecting client
        for (auto *activeClient : m_activeClients) {
            if (activeClient == client)
                continue;
            msg = activeClient->createMessage();
            msg->mutable_user_status()->set_type(activeClient->userStatusType());
            msg->mutable_user_status()->set_fetched(true);
            client->startSharedWrite(msg);
        }

        msg.reset(); // detach
        return true;
    }

    bool logout(ChatRoomReactor *client)
    {
        std::unique_lock lock(m_activeClientsMtx);
        if (const auto it = std::find(m_activeClients.begin(), m_activeClients.end(), client);
            it != m_activeClients.end()) {
            std::cout << "logout: " << client << ", " << client->name() << std::endl;
            m_activeClients.erase(it);
            return true;
        }
        return false;
    }

//! [server-3]
    // Broadcast \a message to all connected clients. Optionally \a skip a client
    void broadcast(const std::shared_ptr<chat::ChatMessage> &message, const ChatRoomReactor *skip)
    {
        for (auto *client : activeClients()) {
            assert(client);
            if (skip && client == skip)
                continue;
            client->startSharedWrite(message);
        }
    }
//! [server-3]

private:
//! [server-2]
    grpc::ServerBidiReactor<chat::ChatMessage, chat::ChatMessage> *
    ChatRoom(grpc::CallbackServerContext *context) override
    {
        return new ChatRoomReactor(this, context);
    }

    grpc::ServerUnaryReactor *Register(grpc::CallbackServerContext *context,
                                       const chat::Credentials *request,
                                       chat::None * /*response*/) override
//! [server-2]
    {
        auto *reactor = context->DefaultReactor();
        if (request->name().empty() || request->password().empty()) {
            reactor->Finish({ grpc::StatusCode::INVALID_ARGUMENT, "Invalid user request" });
            return reactor;
        }

        std::fstream file(m_dbPath, std::ios::in | std::ios::out | std::ios::app);
        if (!file) {
            reactor->Finish({ grpc::StatusCode::UNAVAILABLE, "Database unavailable" });
            return reactor;
        }

        chat::Credentials credSearch;
        file.seekg(0);
        while (file >> credSearch) {
            if (credSearch.name() == request->name()) {
                reactor->Finish({ grpc::StatusCode::ALREADY_EXISTS,
                                  request->name() + " already exists" });
                return reactor;
            }
        }

        file.clear();
        if (!(file << *request)) {
            reactor->Finish({ grpc::StatusCode::DATA_LOSS, "Failed to register user" });
            return reactor;
        }

        reactor->Finish(grpc::Status::OK);
        return reactor;
    }

private:
    std::string m_dbPath = "users.db";
    std::vector<ChatRoomReactor *> m_activeClients;
    std::mutex m_activeClientsMtx;
};

ChatRoomReactor::ChatRoomReactor(QtGrpcChatService *parent, grpc::CallbackServerContext *context)
    : m_context(context), m_service(parent)
{
    const auto &metadata = m_context->client_metadata();
    const auto nameIt = metadata.find(userNameHeader());
    const auto passIt = metadata.find(userPasswordHeader());

    if (nameIt == metadata.end() || passIt == metadata.end()) {
        Finish({ grpc::StatusCode::NOT_FOUND, "Metadata not found" });
        return;
    }

    if (nameIt->second.empty() || passIt->second.empty()) {
        Finish({ grpc::StatusCode::INVALID_ARGUMENT, "Invalid metadata" });
        return;
    }

    for (const auto *client : m_service->activeClients()) {
        if (client->name() == nameIt->second) {
            Finish({ grpc::StatusCode::ALREADY_EXISTS, "User already active" });
            return;
        }
    }

    std::fstream file(m_service->dbPath(), std::ios::in);
    if (!file) {
        Finish({ grpc::StatusCode::UNAVAILABLE, "Database unavailable" });
        return;
    }

    chat::Credentials credentials;
    bool found = false;
    while (file >> credentials) {
        if (credentials.name() == nameIt->second) {
            found = true;
            break;
        }
    }

    if (!found) {
        Finish({ grpc::StatusCode::UNAUTHENTICATED, "User not found" });
        return;
    }

    if (credentials.password() != passIt->second) {
        Finish({ grpc::StatusCode::INVALID_ARGUMENT, "Invalid password" });
        return;
    }

    m_user = credentials;
    if (m_hasLogin = m_service->login(this); !m_hasLogin) {
        Finish({ grpc::StatusCode::INTERNAL, "Login failed" });
        return;
    }

    m_userStatusType = chat::UserStatus::ACTIVE;
    m_request = std::make_shared<chat::ChatMessage>();
    StartRead(m_request.get());
}

void ChatRoomReactor::OnDone()
{
    if (m_hasLogin && m_service->logout(this)) {
        // If the stream is not cancelled we expect a graceful logout
        // attempt and inform all other users. Otherwise we expect
        // a reconnect attempt from the client.
        if (!m_context->IsCancelled()) {
            auto msg = createMessage();
            msg->mutable_user_status()->set_type(chat::UserStatus::LOGOUT);
            m_service->broadcast(msg, this);
        }
    } else {
        std::cerr << "Failed to logout client" << m_user.name() << '\n';
    }

    delete this;
}

void ChatRoomReactor::OnCancel()
{
    Finish(grpc::Status::CANCELLED);
}

void ChatRoomReactor::OnReadDone(bool ok)
{
    if (!ok) {
        if (m_context->IsCancelled())
            return;
        Finish(grpc::Status::OK);
        return;
    }

    if (m_request->has_user_status())
        m_userStatusType = m_request->user_status().type();

//! [server-5]
    // Distribute the incoming message to all other clients.
    m_service->broadcast(m_request, this);
    m_request = std::make_shared<chat::ChatMessage>(); // detach
    StartRead(m_request.get());
//! [server-5]
}

void ChatRoomReactor::OnWriteDone(bool ok)
{
    if (!ok) {
        if (m_context->IsCancelled())
            return;
        Finish(grpc::Status::OK);
        return;
    }

//! [server-6]
    std::scoped_lock lock(m_writeMtx);

    if (!m_responseQueue.empty()) {
        m_response = std::move(m_responseQueue.front());
        m_responseQueue.pop();
        StartWrite(m_response.get());
        return;
    }

    m_response.reset();
//! [server-6]
}

int main(int /* argc */, char * /* argv */[])
{
    grpc::reflection::InitProtoReflectionServerBuilderPlugin();
    std::unique_ptr<grpc::Server> server;
    QtGrpcChatService service;
    {
        grpc::ServerBuilder builder;
//! [server-ssl]
        grpc::SslServerCredentialsOptions sslOpts;
        sslOpts.pem_key_cert_pairs.emplace_back(grpc::SslServerCredentialsOptions::PemKeyCertPair{
            LocalhostKey,
            LocalhostCert,
        });
        builder.AddListeningPort(QtGrpcChatService::httpsAddress(), grpc::SslServerCredentials(sslOpts));
        builder.AddListeningPort(QtGrpcChatService::httpAddress(), grpc::InsecureServerCredentials());
//! [server-ssl]
        builder.RegisterService(&service);
        server = builder.BuildAndStart();
        std::cout << "Server listening on https://" << QtGrpcChatService::httpsAddress() << std::endl;
        std::cout << "Server listening on http://" << QtGrpcChatService::httpAddress() << std::endl;
    }
    server->Wait();
}
