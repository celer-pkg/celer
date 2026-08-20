// Copyright (C) 2015 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR LGPL-3.0-only OR GPL-2.0-only OR GPL-3.0-only

#ifndef QPLACEDETAILSREPLYIMPL_H
#define QPLACEDETAILSREPLYIMPL_H

#include <QtNetwork/QNetworkReply>
#include <QtLocation/QPlaceDetailsReply>

QT_BEGIN_NAMESPACE

class QPlaceManager;
class QPlaceManagerEngineNokiaV2;

class QPlaceDetailsReplyImpl : public QPlaceDetailsReply
{
    Q_OBJECT

public:
    QPlaceDetailsReplyImpl(QNetworkReply *reply, QPlaceManagerEngineNokiaV2 *parent);
    ~QPlaceDetailsReplyImpl();

    void setPlaceId(const QString &placeId) { m_placeId = placeId; }

private slots:
    void setError(QPlaceReply::Error error_, const QString &errorString);
    void replyFinished();
    void replyError(QNetworkReply::NetworkError error);

private:
    QPlaceManagerEngineNokiaV2 *m_engine;
    QString m_placeId;
};

QT_END_NAMESPACE

#endif // QPLACEDETAILSREPLYIMPL_H
