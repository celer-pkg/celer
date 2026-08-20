// Copyright (C) 2025 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR BSD-3-Clause

#pragma once

#include "playstoreloader.h"

#include <QObject>
#include <qqmlintegration.h>
#include <QLibrary>
class MapLoader: public QObject
{
    Q_OBJECT
    QML_ELEMENT

public:
    enum ErrorResult {
        Continue,
        ExitApp
    };
    Q_ENUM(ErrorResult)
    MapLoader(QObject * parent = nullptr);
    Q_INVOKABLE void loadModuleFromStore(const QString& moduleName);
    Q_INVOKABLE QStringList getImageNames();
    Q_INVOKABLE void showMapInfo();
    Q_INVOKABLE void removeModule(const QString& moduleName);
    Q_INVOKABLE void cancelDownload();

signals:
    void showToast(const QString& message);
    void showMapInfoPopup(const QString& message);
    void showDownloadPopup();
    void hideDownloadPopup();
    void updateDownloadProgress(long bytes, long total);
    void moduleLoaded();
    void errorOccured(const QString& error, ErrorResult result);
    void installCanceled();

private slots:
    void stateChanged(PlayStoreLoader::State state);
    void downloadProgress(qsizetype bytes, qsizetype total);
    void handleError(int errorCode, const QString &errorString);
    void confirmationRequest(int errorCode, const QString &errorString);
    void finished();

private:
    QString loadMapInfo();

private:
    std::unique_ptr<PlayStoreLoaderHandler> mHandler;
    QLibrary mWintermapLibrary;
};
