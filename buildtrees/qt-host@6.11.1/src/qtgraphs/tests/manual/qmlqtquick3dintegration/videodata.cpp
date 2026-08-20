// Copyright (C) 2024 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR GPL-3.0-only

#define NOMINMAX

#include "videodata.h"
#include <QImage>
#include <QSize>
#include <QtGraphs/QBar3DSeries>
#include <QtMultimedia/QVideoFrame>

VideoData::VideoData()
    : m_barDataArray(0)
    , m_started(false)
{
    qRegisterMetaType<QBar3DSeries *>();

    QVideoWidget *video = new QVideoWidget();
    m_player = new QMediaPlayer(this);
    m_player->setVideoOutput(video);
    //m_player->setPlaybackRate(0.5);
    video->setVisible(true);
    connect(m_player, &QMediaPlayer::hasVideoChanged, this, &VideoData::videoAvailableChanged);
    connect(m_player, &QMediaPlayer::mediaStatusChanged, this, &VideoData::statusChanged);

    m_barDataArray.clear();
    m_barDataArray.reserve(m_resolution.height());
    for (int i = 0; i < m_resolution.height(); i++) {
        QBarDataRow newProxyRow(m_resolution.width());
        m_barDataArray.append(newProxyRow);
    }
}

VideoData::~VideoData() {}

void VideoData::videoAvailableChanged(bool available)
{
    Q_UNUSED(available);
    // if (available)
    //     qDebug() << m_videosink->videoSize() << m_videosink->videoFrame();
}

void VideoData::statusChanged(QMediaPlayer::MediaStatus status)
{
    qDebug() << __func__ << status;
    // handle status message
    switch (status) {
    case QMediaPlayer::NoMedia:
        qDebug() << "no media";
        break;
    case QMediaPlayer::LoadedMedia:
        qDebug() << "Loaded";
        updateData();
        break;
    case QMediaPlayer::LoadingMedia:
        qDebug() << tr("Loading...");
        break;
    case QMediaPlayer::BufferingMedia:
    case QMediaPlayer::BufferedMedia:
        qDebug() << (tr("Buffering %1%").arg(qRound(m_player->bufferProgress() * 100.)));
        break;
    case QMediaPlayer::StalledMedia:
        qDebug() << (tr("Stalled %1%").arg(qRound(m_player->bufferProgress() * 100.)));
        break;
    case QMediaPlayer::EndOfMedia:
        m_player->play();
        break;
    case QMediaPlayer::InvalidMedia:
        qDebug() << m_player->errorString();
        break;
    }
}

void VideoData::setSeries(QBar3DSeries *series)
{
    m_series = series;
}

void VideoData::updateData()
{
    if (!m_started) {
        qDebug() << "We are stopped. The changes will take effect once started.";
        return;
    }
    QImage depthMap = m_videosink->videoFrame().toImage();
    depthMap = depthMap.scaled(m_resolution);
    setData(depthMap);
}

void VideoData::setData(const QImage &image)
{
    QImage heightImage = image;

    uchar *bits = heightImage.bits();

    int imageHeight = heightImage.height();
    int imageWidth = heightImage.width();
    int bitCount = imageWidth * 4 * (imageHeight - 1);
    int widthBits = imageWidth * 4;

    QBarDataArray dataArray = m_barDataArray;
    for (int i = 0; i < imageHeight; i++, bitCount -= widthBits) {
        QBarDataRow &newRow = dataArray[i];
        for (int j = 0; j < imageWidth; j++)
            newRow[j] = QBarDataItem(float(bits[bitCount + (j * 4)]));
    }

    m_series->dataProxy()->resetArray(dataArray);
}

void VideoData::start()
{
    if (!m_started) {
        // m_player->setSource(QUrl(
        //     "http://commondatastorage.googleapis.com/gtv-videos-bucket/sample/BigBuckBunny.mp4"));
        m_player->setSource(QUrl::fromLocalFile("./video/video.mp4"));
        m_videosink = m_player->videoSink();
        connect(m_videosink, &QVideoSink::videoFrameChanged, this, &VideoData::updateData);
    }
    m_started = true;
    m_player->play();
}

void VideoData::stop()
{
    m_started = false;
    m_player->stop();
}
