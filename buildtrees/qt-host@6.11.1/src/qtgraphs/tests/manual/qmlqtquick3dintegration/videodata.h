// Copyright (C) 2024 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR GPL-3.0-only

#ifndef VIDEODATA_H
#define VIDEODATA_H

#include <QtGraphs/QBarDataProxy>
#include <QtMultimedia/QMediaPlayer>
#include <QtMultimedia/QVideoSink>
#include <QtMultimediaWidgets/QVideoWidget>

class VideoData : public QObject
{
    Q_OBJECT

public:
    explicit VideoData();
    ~VideoData();

    Q_INVOKABLE void start();
    Q_INVOKABLE void stop();

    void updateData();
    void setData(const QImage &image);

    Q_INVOKABLE void setSeries(QBar3DSeries *series);

public Q_SLOTS:
    void videoAvailableChanged(bool available);
    void statusChanged(QMediaPlayer::MediaStatus status);

private:
    // QSize m_resolution = QSize(65, 30); // The live data resolution
    QSize m_resolution = QSize(130, 60); // The live data resolution
    // QSize m_resolution = QSize(325, 175); // The live data resolution
    QBarDataArray m_barDataArray;
    bool m_started;
    QMediaPlayer *m_player;
    QVideoSink *m_videosink;
    QBar3DSeries *m_series;
};

#endif
