// Copyright (C) 2016 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR BSD-3-Clause

#ifndef IMAGEANALYZER_H
#define IMAGEANALYZER_H

#include <QFutureWatcher>
#include <QtWidgets>

QT_BEGIN_NAMESPACE
class QNetworkAccessManager;
class QNetworkReply;
class QNetworkDiskCache;
QT_END_NAMESPACE

//! [ ImageAnalyzer - public interface ]
class ImageAnalyzer : public QObject
{
    Q_OBJECT
public:
    ImageAnalyzer(QNetworkDiskCache * netcache, QObject * parent=0);

    QRgb lastResults();
    float lastRed();
    float lastGreen();
    float lastBlue();
    bool isBusy();
    Q_PROPERTY(bool busy READ isBusy);
    Q_PROPERTY(float red READ lastRed);
    Q_PROPERTY(float green READ lastGreen);
    Q_PROPERTY(float blue READ lastBlue);
    ~ImageAnalyzer();

public slots:
    /*! initiates analysis of all the urls in the list */
    void startAnalysis(const QStringList & urls);

signals:
    void finishedAnalysis();
    void updateProgress(int completed, int total);
    //! [ ImageAnalyzer - public interface ]

private slots:
    void handleReply(QNetworkReply*);
    void doneProcessing();
    void progressStatus(int);

private:
    QRgb processImages();
    void fetchURLs();
    void queueImage(QImage img);

    //! [ ImageAnalyzer - private members ]
private:
    QNetworkAccessManager* m_network;
    QNetworkDiskCache* m_cache;
    QStringList m_URLQueue;
    QList<QImage> m_imageQueue;
    int m_outstandingFetches;
    QFutureWatcher<QRgb> * m_watcher;
    //! [ ImageAnalyzer - private members ]
};

QRgb averageRGB(const QImage &img);

#endif
