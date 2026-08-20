// Copyright (C) 2025 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR BSD-3-Clause

#pragma once

#include <QObject>
#include <QUuid>

class PlayStoreLoaderHandler;

namespace PlayStoreLoader {
Q_NAMESPACE

enum class State {
    Uninitialized,
    Initialized,
    Pending,
    Downloading,
    Downloaded,
    RequiresUserConfirmation,
    Canceling,
    Canceled,
    Installing,
    Installed,
    Loading,
    Loaded,
    Error
};
Q_ENUM_NS(State)

enum class ModuleRemovalStatus {
    Canceled,
    Failure,
    Success
};

Q_ENUM_NS(ModuleRemovalStatus)

std::unique_ptr<PlayStoreLoaderHandler> getHandler();
void loadModule(const QString &callId, const QString &moduleName);
QSet<QString> getInstalledModules();
void uninstallModules(const QString &callId, const QStringList &modules);
void cancelLoad(const QString &callId);

}; // namespace PlayStoreLoader

class PlayStoreLoaderHandler : public QObject
{
    Q_OBJECT

public:
    explicit PlayStoreLoaderHandler(QObject *parent);
    ~PlayStoreLoaderHandler() override;
    const QString &callId() const & noexcept;
    void setState(PlayStoreLoader::State state);

signals:
    void stateChanged(PlayStoreLoader::State state);
    void downloadProgress(qsizetype bytes, qsizetype total);
    void errorOccured(int errorCode, const QString &errorString);
    void confirmationRequest(int errorCode, const QString &errorString);
    void finished();
    void moduleUninstalled(PlayStoreLoader::ModuleRemovalStatus status);

private:
    PlayStoreLoader::State m_state = PlayStoreLoader::State::Uninitialized;
    QString m_callId = QUuid::createUuid().toString();
};
