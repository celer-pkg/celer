// Copyright (C) 2021 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR BSD-3-Clause

#ifndef ANNOTATEDURL_H
#define ANNOTATEDURL_H

#include <QNdefFilter>
#include <QNdefMessage>
#include <QNearFieldManager>

#include <QObject>

QT_FORWARD_DECLARE_CLASS(QNearFieldTarget)
QT_FORWARD_DECLARE_CLASS(QPixmap)
QT_FORWARD_DECLARE_CLASS(QUrl)

//! [0]
class AnnotatedUrl : public QObject
{
    Q_OBJECT

public:
    explicit AnnotatedUrl(QObject *parent = 0);
    ~AnnotatedUrl();

    void startDetection();

signals:
    void annotatedUrl(const QUrl &url, const QString &title, const QPixmap &pixmap);
    void nfcStateChanged(bool enabled);
    void tagError(const QString &error);

public slots:
    void targetDetected(QNearFieldTarget *target);
    void targetLost(QNearFieldTarget *target);
    void handleMessage(const QNdefMessage &message, QNearFieldTarget *target);
    void handlePolledNdefMessage(QNdefMessage message);
    void handleAdapterStateChange(QNearFieldManager::AdapterState state);

private:
    QNearFieldManager *manager;
    QNdefFilter messageFilter;
};
//! [0]

#endif // ANNOTATEDURL_H
