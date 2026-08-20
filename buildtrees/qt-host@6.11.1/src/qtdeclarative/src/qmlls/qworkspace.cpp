// Copyright (C) 2021 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR LGPL-3.0-only OR GPL-2.0-only OR GPL-3.0-only
// Qt-Security score:significant reason:default

#include "qworkspace_p.h"
#include "qqmllanguageserver_p.h"
#include "qqmllsutils_p.h"

#include <QtLanguageServer/private/qlanguageserverspectypes_p.h>
#include <QtLanguageServer/private/qlspnotifysignals_p.h>

#include <QtCore/qfile.h>
#include <variant>

QT_BEGIN_NAMESPACE
using namespace Qt::StringLiterals;
using namespace QLspSpecification;

void WorkspaceHandlers::registerHandlers(QLanguageServer *server, QLanguageServerProtocol *)
{
    QObject::connect(server->notifySignals(),
                     &QLspNotifySignals::receivedDidChangeWorkspaceFoldersNotification, this,
                     [this](const DidChangeWorkspaceFoldersParams &params) {
                         const WorkspaceFoldersChangeEvent &event = params.event;

                         const QList<WorkspaceFolder> &removed = event.removed;
                         QList<QByteArray> toRemove;
                         for (const WorkspaceFolder &folder : removed) {
                             toRemove.append(QQmlLSUtils::lspUriToQmlUrl(folder.uri));
                             m_codeModelManager->removeDirectory(
                                     QQmlLSUtils::lspUriToQmlUrl(folder.uri));
                         }
                         m_codeModelManager->removeRootUrls(toRemove);
                         const QList<WorkspaceFolder> &added = event.added;
                         QList<QByteArray> toAdd;
                         for (const WorkspaceFolder &folder : added) {
                             toAdd.append(QQmlLSUtils::lspUriToQmlUrl(folder.uri));
                         }
                         m_codeModelManager->addRootUrls(toAdd);
                     });

    QObject::connect(server, &QLanguageServer::clientInitialized, this,
                     &WorkspaceHandlers::clientInitialized);
}

QString WorkspaceHandlers::name() const
{
    return u"Workspace"_s;
}

void WorkspaceHandlers::setupCapabilities(const QLspSpecification::InitializeParams &clientInfo,
                                          QLspSpecification::InitializeResult &serverInfo)
{
    if (!clientInfo.capabilities.workspace
        || !clientInfo.capabilities.workspace->value(u"workspaceFolders"_s).toBool(false))
        return;
    WorkspaceFoldersServerCapabilities folders;
    folders.supported = true;
    folders.changeNotifications = true;
    if (!serverInfo.capabilities.workspace)
        serverInfo.capabilities.workspace = QJsonObject();
    serverInfo.capabilities.workspace->insert(u"workspaceFolders"_s,
                                              QTypedJson::toJsonValue(folders));

    openInitialWorkspace(clientInfo);
}

void WorkspaceHandlers::openInitialWorkspace(const InitializeParams &clientInfo)
{
    if (clientInfo.workspaceFolders) {
        const auto *folders = std::get_if<QList<WorkspaceFolder>>(&*clientInfo.workspaceFolders);

        // note: if *clientInfo.workspaceFolders contains a nullptr_t than it means that no WS was
        // opened yet.
        if (!folders)
            return;

        QList<QByteArray> rootPaths;
        for (const auto &folder : std::as_const(*folders)) {
            rootPaths.append(QQmlLSUtils::lspUriToQmlUrl(folder.uri));
        }
        m_codeModelManager->addRootUrls(rootPaths);
        return;
    }

    // note: rootUri is deprecated in the LSP protocol
    if (const auto *rootUri = std::get_if<QByteArray>(&clientInfo.rootUri)) {
        m_codeModelManager->addRootUrls({ QQmlLSUtils::lspUriToQmlUrl(*rootUri) });
        return;
    }
    // note: rootPath is also deprecated in the LSP protocol. It was deprecated even before rootUri
    // was deprecated.
    if (clientInfo.rootPath) {
        if (const auto *rootPath = std::get_if<QByteArray>(&*clientInfo.rootPath)) {
            m_codeModelManager->addRootUrls({
                    QUrl::fromLocalFile(QString::fromUtf8(*rootPath)).toEncoded(),
            });
            return;
        }
    }
}

void WorkspaceHandlers::clientInitialized(QLanguageServer *server)
{
    QLanguageServerProtocol *protocol = server->protocol();
    const auto clientInfo = server->clientInfo();
    QList<Registration> registrations;
    if (clientInfo.capabilities.workspace
        && clientInfo.capabilities.workspace
                   ->value(u"didChangeWatchedFiles"_s)[u"dynamicRegistration"_s]
                   .toBool(false)) {
        const int watchAll =
                int(WatchKind::Create) | int(WatchKind::Change) | int(WatchKind::Delete);
        DidChangeWatchedFilesRegistrationOptions watchedFilesParams;
        FileSystemWatcher qmlWatcher;
        qmlWatcher.globPattern = QByteArray("*.{qml,js,mjs}");
        qmlWatcher.kind = watchAll;
        FileSystemWatcher qmldirWatcher;
        qmldirWatcher.globPattern = "qmldir";
        qmldirWatcher.kind = watchAll;
        FileSystemWatcher qmltypesWatcher;
        qmltypesWatcher.globPattern = QByteArray("*.qmltypes");
        qmltypesWatcher.kind = watchAll;
        watchedFilesParams.watchers = QList<FileSystemWatcher>({
            std::move(qmlWatcher),
            std::move(qmldirWatcher),
            std::move(qmltypesWatcher)
        });
        registrations.append(Registration {
                // use ClientCapabilitiesInfo::WorkspaceDidChangeWatchedFiles as id too
                ClientCapabilitiesInfo::WorkspaceDidChangeWatchedFiles,
                ClientCapabilitiesInfo::WorkspaceDidChangeWatchedFiles,
                QTypedJson::toJsonValue(watchedFilesParams) });
    }

    if (!registrations.isEmpty()) {
        RegistrationParams params;
        params.registrations = registrations;
        protocol->requestRegistration(
                params,
                []() {
                    // successful registration
                },
                [protocol](const ResponseError &err) {
                    LogMessageParams msg;
                    msg.message = QByteArray("Registration of file updates failed, will miss file "
                                             "changes from outside the editor.");
                    msg.message.append(QString::number(err.code).toUtf8());
                    if (!err.message.isEmpty())
                        msg.message.append(" ");
                    msg.message.append(err.message);
                    msg.type = MessageType::Warning;
                    qCWarning(lspServerLog) << QString::fromUtf8(msg.message);
                    protocol->notifyLogMessage(msg);
                });
    }
}

QT_END_NAMESPACE
