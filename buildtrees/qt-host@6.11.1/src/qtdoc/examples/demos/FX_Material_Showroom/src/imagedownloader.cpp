// Copyright (C) 2023 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR BSD-3-Clause

#include "imagedownloader.h"

#include <QCoreApplication>
#include <QDir>
#include <QFile>
#include <QNetworkReply>
#include <QNetworkRequest>

ImageDownloader::ImageDownloader(QObject *parent)
    : QObject(parent)
    , m_baseLocal(QCoreApplication::applicationDirPath() + "/content/")
{}

void ImageDownloader::downloadImages()
{
    m_downloadedCount = 0;

    m_downloadPaths = getDownloadList();
    if (m_downloadPaths.isEmpty())
        emit finished();
    else
        downloadNextAsset();
}

void ImageDownloader::downloadNextAsset()
{
    if (m_downloadedCount < m_downloadPaths.size()) {
        emit downloadStart(m_downloadedCount + 1);

        const QString &assetUrl = m_downloadPaths[m_downloadedCount];
        QUrl url(BASE_REMOTE + assetUrl);
        QNetworkRequest request(url);

        QNetworkReply *reply = m_manager.get(request);

        connect(reply, &QNetworkReply::finished, this, &ImageDownloader::onDownloadFinished);
        connect(reply, &QNetworkReply::downloadProgress,
                this, &ImageDownloader::onDownloadProgress);
    }
}

QStringList ImageDownloader::getDownloadList() const
{
    QStringList downloadList;
    std::copy_if(assetsList.begin(), assetsList.end(), std::back_inserter(downloadList),
                 [&](const QString &imgPath) {
        QString localPath = m_baseLocal + imgPath;
        return !QFile(localPath).exists();
    });

    return downloadList;
}

int ImageDownloader::downloadCount() const
{
    return m_downloadPaths.size();
}

void ImageDownloader::onDownloadProgress(qint64 bytesReceived, qint64 bytesTotal)
{
    emit downloadProgress(double(bytesReceived)/double(bytesTotal));
}

void ImageDownloader::onDownloadFinished()
{
    QNetworkReply *reply = qobject_cast<QNetworkReply *>(sender());
    reply->deleteLater();

    if (reply->error() == QNetworkReply::NoError) {
        QByteArray imageData = reply->readAll();

        QFileInfo fi(m_baseLocal + m_downloadPaths[m_downloadedCount]);
        QDir dir (fi.path());
        if (!dir.exists()) {
            if (!dir.mkpath("."))
                qWarning() << "Failed to create save path:" << fi.path();
        }
        QFile file(fi.filePath());
        if (file.open(QIODevice::WriteOnly)) {
            file.write(imageData);
            file.close();
        } else {
            qWarning() << "Failed to save image:" << file.errorString();
        }
    } else {
        qWarning() << "Download failed:" << reply->errorString();
    }

    ++m_downloadedCount;

    if (m_downloadedCount == m_downloadPaths.size())
        emit finished();
    else
        downloadNextAsset();
}
