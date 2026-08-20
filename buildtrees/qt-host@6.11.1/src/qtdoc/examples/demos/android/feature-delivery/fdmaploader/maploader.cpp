// Copyright (C) 2025 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR BSD-3-Clause

#include "maploader.h"

#include <QFile>
#include <QDir>

MapLoader::MapLoader(QObject* parent):
    QObject(parent)
{
    QSet<QString> modules = PlayStoreLoader::getInstalledModules();
    for (QString module : modules)
        qDebug() <<Q_FUNC_INFO <<"Installed module: " <<module;
}

//! [Module Loading]
Q_INVOKABLE void MapLoader::loadModuleFromStore(const QString& moduleName)
{
    if (!mHandler.get())
        mHandler = PlayStoreLoader::getHandler();
    PlayStoreLoader::loadModule(mHandler->callId(), moduleName);
    connect(mHandler.get(), &PlayStoreLoaderHandler::stateChanged, this, &MapLoader::stateChanged);
    connect(mHandler.get(), &PlayStoreLoaderHandler::downloadProgress, this,
            &MapLoader::downloadProgress);
    connect(mHandler.get(), &PlayStoreLoaderHandler::errorOccured, this, &MapLoader::handleError);
    connect(mHandler.get(), &PlayStoreLoaderHandler::confirmationRequest, this,
            &MapLoader::confirmationRequest);
    connect(mHandler.get(), &PlayStoreLoaderHandler::finished, this, &MapLoader::finished);
    connect(mHandler.get(), &PlayStoreLoaderHandler::moduleUninstalled, this,
        [this](PlayStoreLoader::ModuleRemovalStatus status){
            switch (status) {
                case PlayStoreLoader::ModuleRemovalStatus::Canceled:
                    showToast("Map Removal Canceled");
                    break;
                case PlayStoreLoader::ModuleRemovalStatus::Failure:
                    showToast("Map Removal Failed");
                    break;
                case PlayStoreLoader::ModuleRemovalStatus::Success:
                    showToast("Map Removed");
                    break;
            }
            mHandler.release();
    });
}
//! [Module Loading]

Q_INVOKABLE void MapLoader::removeModule(const QString& moduleName)
{
    if (!mHandler.get())
        mHandler = PlayStoreLoader::getHandler();
    PlayStoreLoader::uninstallModules(mHandler->callId(), QList<QString>{moduleName});
}

Q_INVOKABLE void MapLoader::cancelDownload()
{
    if (mHandler)
        PlayStoreLoader::cancelLoad(mHandler->callId());
    else
        qWarning() <<Q_FUNC_INFO <<"No valid handler";
}

Q_INVOKABLE QStringList MapLoader::getImageNames()
{
    QScopedPointer<QStringList> resultStrList;
    QDir resourceDir(":/images");
    resultStrList.reset(new QStringList(resourceDir.entryList(QDir::Files)));
    resultStrList->append(QStringList(QDir(":/").entryList(QDir::Files)));
    return *resultStrList.data();
}

Q_INVOKABLE void MapLoader::showMapInfo()
{
    QString info = loadMapInfo();
    if (info.isEmpty())
        emit showMapInfoPopup("Information not loaded");
    else
        emit showMapInfoPopup(info);
}

//! [Load Map Info]
QString MapLoader::loadMapInfo()
{
    QScopedPointer<QString> resultStr;
    typedef void* (*LoadMapInfoFunc)();
    //Find if wintermap library exists
    mWintermapLibrary.setFileName("fdwintermapmodule");
    if (!mWintermapLibrary.load()) {
        qWarning() << Q_FUNC_INFO << "Failed to load library";
        return QString();
    }
    LoadMapInfoFunc loadMapInfo = (LoadMapInfoFunc) mWintermapLibrary.resolve("loadMapInfo");
    if (loadMapInfo) {
        void* result = loadMapInfo();
        resultStr.reset(static_cast<QString*>(result));
    } else
        qWarning() << Q_FUNC_INFO << "Function loadMapInfo not loaded";

    return *resultStr.data();
}
//! [Load Map Info]

void MapLoader::stateChanged(PlayStoreLoader::State state)
{
    switch (state) {
        case PlayStoreLoader::State::Downloading:
            //Show downloading popup with cancel option
            emit showDownloadPopup();
            break;
        case PlayStoreLoader::State::Downloaded:
            //This signal seems to be unreliable, not always received after a download.
            break;
        case PlayStoreLoader::State::RequiresUserConfirmation:
            emit showToast("Requires user confirmation");
            break;
        case PlayStoreLoader::State::Canceled:
            emit installCanceled();
            mHandler.release();
            break;
        case PlayStoreLoader::State::Installing:
            emit showToast("Installing");
            break;
        case PlayStoreLoader::State::Installed:
            //Remove downloading popup
            emit hideDownloadPopup();
            emit showToast("Installed");
            break;
        case PlayStoreLoader::State::Loaded:
            mHandler.release();
            emit moduleLoaded();
            break;
        default:
            break;
    }
}
void MapLoader::downloadProgress(qsizetype bytes, qsizetype total)
{
    emit updateDownloadProgress(bytes, total);
}
void MapLoader::handleError(int errorCode, const QString &errorString)
{
    //TODO: Handle fatal errors.
    qWarning() << Q_FUNC_INFO << "Error: " << errorCode << " " << errorString;
    emit errorOccured(errorString, ErrorResult::Continue);
}

void MapLoader::confirmationRequest(int errorCode, const QString &errorString)
{
    qWarning() << Q_FUNC_INFO << "Confirmations not handled: " << errorCode << " " << errorString;
}

void MapLoader::finished()
{
    //Print installed modules.
    QSet<QString> modules = PlayStoreLoader::getInstalledModules();
    for (QString module : modules)
        qDebug() <<Q_FUNC_INFO <<"Installed module: " <<module;
}
