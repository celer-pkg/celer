// Copyright (C) 2018 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR GPL-3.0-only

#ifndef LOTTIEANIMATION_P_H
#define LOTTIEANIMATION_P_H

//
//  W A R N I N G
//  -------------
//
// This file is not part of the Qt API.  It exists purely as an
// implementation detail.  This header file may change from version to
// version without notice, or even be removed.
//
// We mean it.
//

#include <QtLottie/qtlottieexports.h>

#include <QtQuick/qquickpainteditem.h>

#include <QtGui/qimage.h>

#include <QtCore/qbytearray.h>
#include <QtCore/qlist.h>
#include <QtCore/qmetaobject.h>
#include <QtCore/qthread.h>

QT_BEGIN_NAMESPACE

class QBatchRenderer;

class Q_LOTTIE_EXPORT QLottieAnimation : public QQuickPaintedItem
{
    Q_OBJECT
    Q_PROPERTY(QUrl source READ source WRITE setSource NOTIFY sourceChanged)
    Q_PROPERTY(int frameRate READ frameRate WRITE setFrameRate RESET resetFrameRate NOTIFY frameRateChanged)
    Q_PROPERTY(int startFrame READ startFrame NOTIFY startFrameChanged)
    Q_PROPERTY(int endFrame READ endFrame NOTIFY endFrameChanged)
    Q_PROPERTY(Status status READ status WRITE setStatus NOTIFY statusChanged)
    Q_PROPERTY(Quality quality READ quality WRITE setQuality NOTIFY qualityChanged)
    Q_PROPERTY(bool autoPlay MEMBER m_autoPlay NOTIFY autoPlayChanged)
    Q_PROPERTY(int loops MEMBER m_loops NOTIFY loopsChanged)
    Q_PROPERTY(Direction direction READ direction WRITE setDirection NOTIFY directionChanged)

    QML_NAMED_ELEMENT(LottieAnimation)
    QML_ADDED_IN_VERSION(1, 0)

public:
    enum Status{Null, Loading, Ready, Error};
    Q_ENUM(Status)

    enum Quality{LowQuality, MediumQuality, HighQuality};
    Q_ENUM(Quality)

    enum Direction{Forward = 1, Reverse = -1};
    Q_ENUM(Direction)

    enum LoopCount{Infinite = -1};
    Q_ENUM(LoopCount)

    explicit QLottieAnimation(QQuickItem *parent = nullptr);
    ~QLottieAnimation() override;

    void paint(QPainter *painter) override;

    Status status() const;

    QUrl source() const;
    void setSource(const QUrl &source);

    int frameRate() const;
    void setFrameRate(int frameRate);
    void resetFrameRate();

    Quality quality() const;
    void setQuality(Quality quality);

    Direction direction() const;
    void setDirection(Direction direction);

    int startFrame() const;
    int endFrame() const;
    int currentFrame() const;

    QVersionNumber version() const;

    Q_INVOKABLE void start();

    Q_INVOKABLE void play();
    Q_INVOKABLE void pause();
    Q_INVOKABLE void togglePause();
    Q_INVOKABLE void stop();
    Q_INVOKABLE void gotoAndPlay(int frame);
    Q_INVOKABLE bool gotoAndPlay(const QString &frameMarker);
    Q_INVOKABLE void gotoAndStop(int frame);
    Q_INVOKABLE bool gotoAndStop(const QString &frameMarker);
    Q_INVOKABLE double getDuration(bool inFrames = false);

    QByteArray jsonSource() const;

Q_SIGNALS:
    void statusChanged();
    void qualityChanged();
    void sourceChanged();
    void finished();
    void frameRateChanged();
    void autoPlayChanged();
    void loopsChanged();
    void directionChanged();
    void startFrameChanged();
    void endFrameChanged();

protected Q_SLOTS:
    void loadFinished(const QByteArray &json);
    void renderNextFrame();

protected:
    void componentComplete() override;

    void setStatus(Status status);

    void setStartFrame(int startFrame);
    void setEndFrame(int endFrame);

    void load();

    virtual int parse(const QByteArray &jsonSource);

protected:
    QBatchRenderer *m_frameRenderThread = nullptr;
    QMetaObject::Connection m_waitForFrameConn;

    Status m_status = Null;
    QVersionNumber m_version = QVersionNumber();
    int m_startFrame = 0;
    int m_endFrame = 0;
    int m_currentFrame = 0;
    int m_frameRate = 30;
    int m_animFrameRate = 30;
    qreal m_animWidth = 0;
    qreal m_animHeight = 0;
    QHash<QString, int> m_markers;
    QUrl m_source;
    QTimer *m_frameAdvance = nullptr;

    void gotoFrame(int frame);
    void reset();

private:
    Quality m_quality = HighQuality;
    bool m_autoPlay = true;
    int m_loops = 1;
    int m_currentLoop = 0;
    int m_direction = Forward;
    QByteArray m_jsonSource;
};

QT_END_NAMESPACE

Q_DECLARE_METATYPE(QLottieAnimation*)

#endif // LOTTIEANIMATION_P_H
