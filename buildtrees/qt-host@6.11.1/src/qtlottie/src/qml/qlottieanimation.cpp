// Copyright (C) 2018 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR GPL-3.0-only
// Qt-Security score:critical reason:data-parser

#include "qlottieanimation_p.h"

#include <private/qbatchrenderer_p.h>
#include <private/qlottiebase_p.h>
#include <private/qlottieconstants_p.h>
#include <private/qlottielayer_p.h>
#include <private/qlottierasterrenderer_p.h>
#include <private/qtlottie-config_p.h>

#include <QtQuick/qquickpainteditem.h>

#include <QtQml/qqmlcontext.h>
#include <QtQml/qqmlengine.h>
#include <QtQml/qqmlfile.h>

#include <QtGui/qimage.h>
#include <QtGui/qpainter.h>

#if QT_CONFIG(lottie_network)
#include <QtNetwork/qnetworkaccessmanager.h>
#include <QtNetwork/qnetworkreply.h>
#endif

#include <QtCore/qfile.h>
#include <QtCore/qjsonarray.h>
#include <QtCore/qjsondocument.h>
#include <QtCore/qjsonobject.h>
#include <QtCore/qloggingcategory.h>
#include <QtCore/qmetaobject.h>
#include <QtCore/qpoint.h>
#include <QtCore/qthread.h>
#include <QtCore/qtimer.h>

#include <math.h>

using namespace Qt::StringLiterals;

QT_BEGIN_NAMESPACE

/*!
    \qmltype LottieAnimation
    \inqmlmodule Qt.labs.lottieqt
    \since 5.13
    \inherits Item
    \brief A Lottie player for Qt.

    The LottieAnimation type shows Lottie format files.

    LottieAnimation is used to load and render Lottie files exported
    from Adobe After Effects. Currently, only subset of the full Lottie
    specification is supported. Most notable deviations are:

    \list
    \li Only Shape layer supported
    \li Only integer frame-mode of a timeline supported
        (real frame numbers and time are rounded to the nearest integer)
    \li Expressions are not supported
    \endlist

    For the full list of devations, please see see the \l{Limitations}
    section.

    \section1 Example Usage

    The following example shows a simple usage of the LottieAnimation type

    \qml
    LottieAnimation {
        loops: 2
        quality: LottieAnimation.MediumQuality
        source: "animation.json"
        autoPlay: false
        onStatusChanged: {
            if (status === LottieAnimation.Ready) {
                // any acvities needed before
                // playing starts go here
                gotoAndPlay(startFrame);
            }
        }
        onFinished: {
            console.log("Finished playing")
        }
    }
    \endqml

    \note Changing width or height of the element does not change the size
    of the animation within. Also, it is not possible to align the the content
    inside of a \c LottieAnimation element. To achieve this, position the
    animation inside e.g. an \c Item.

    \section1 Rendering Performance

    Internally, the rendered frame data is cached to improve performance. You
    can control the memory usage by setting the QLOTTIE_RENDER_CACHE_SIZE
    environment variable (default value is 2).

    You can monitor the rendering performance by turning on two logging categories:

    \list
    \li \c qt.lottieqt.lottie.render - Provides information how the animation
        is rendered
    \li \c qt.lottieqt.lottie.render.thread - Provides information how the
        rendering process proceeds.
    \endlist

    Specifically, you can monitor does the frame cache gets constantly full, or
    does the rendering process have to wait for frames to become ready. The
    first case implies that the animation is too complex, and the rendering
    cannot keep up the pace. Try making the animation simpler, or optimize
    the QML scene.
*/

/*!
    \qmlproperty bool LottieAnimation::autoPlay

    Defines whether the player will start playing animation automatically after
    the animation file has been loaded.

    The default value is \c true.
*/

/*!
    \qmlproperty int LottieAnimation::loops

    This property holds the number of loops the player will repeat.
    The value \c LottieAnimation.Infinite means that the the player repeats
    the animation continuously.

    The default value is \c 1.
*/

/*!
    \qmlsignal LottieAnimation::finished()

    This signal is emitted when the player has finished playing. In case of
    looping, the signal is emitted when the last loop has been finished.
*/

QLottieAnimation::QLottieAnimation(QQuickItem *parent)
    : QQuickPaintedItem(parent)
{
    m_frameAdvance = new QTimer(this);
    m_frameAdvance->setInterval(1000 / m_frameRate);
    m_frameAdvance->setSingleShot(false);
    connect (m_frameAdvance, &QTimer::timeout, this, &QLottieAnimation::renderNextFrame);

    m_frameRenderThread = QBatchRenderer::instance();

    qRegisterMetaType<QLottieAnimation*>();

    setAntialiasing(m_quality == HighQuality);
}

QLottieAnimation::~QLottieAnimation()
{
    QMetaObject::invokeMethod(m_frameRenderThread, "deregisterAnimator", Q_ARG(QLottieAnimation*, this));
}

void QLottieAnimation::componentComplete()
{
    QQuickPaintedItem::componentComplete();

    if (m_source.isValid())
        load();
}

void QLottieAnimation::paint(QPainter *painter)
{
    QLottieBase* lottieTree = m_frameRenderThread->getFrame(this, m_currentFrame);

    if (!lottieTree) {
        qCDebug(lcLottieQtLottieRender) << "QLottieAnimation::paint: Got empty element tree."
                                              "Cannot draw (Animator:" << static_cast<void*>(this) << ")";
        return;
    }

    QLottieRasterRenderer renderer(painter);

    qCDebug(lcLottieQtLottieRender) << static_cast<void*>(this) << "Start to paint frame"  << m_currentFrame;

    for (QLottieBase *elem : lottieTree->children()) {
        if (elem->active(m_currentFrame))
            elem->render(renderer);
        else
            qCDebug(lcLottieQtLottieRender) << "Element '" << elem->name() << "' inactive. No need to paint";
    }

    if (m_frameAdvance->isActive()) {
        m_frameRenderThread->frameRendered(this, m_currentFrame);
        m_currentFrame += m_direction;

        if (m_currentFrame < m_startFrame || m_currentFrame > m_endFrame) {
            m_currentLoop += (m_loops > 0 ? 1 : 0);
        }

        if ((m_loops - m_currentLoop) != 0) {
            m_currentFrame = m_currentFrame < m_startFrame ? m_endFrame :
                    m_currentFrame > m_endFrame ? m_startFrame : m_currentFrame;
        }
    }
}

/*!
    \qmlproperty enumeration LottieAnimation::status

    This property holds the current status of the LottieAnimation element.

    \value LottieAnimation.Null
           An initial value that is used when the source is not defined
           (Default)

    \value LottieAnimation.Loading
           The player is loading a Lottie file

    \value LottieAnimation.Ready
           Loading has finished successfully and the player is ready to play
           the animation

    \value LottieAnimation.Error
           An error occurred while loading the animation

    For example, you could implement \c onStatusChanged signal
    handler to monitor progress of loading an animation as follows:

    \qml
    LottieAnimation {
        source: "animation.json"
        autoPlay: false
        onStatusChanged: {
            if (status === LottieAnimation.Ready)
                start();
        }
    \endqml
*/
QLottieAnimation::Status QLottieAnimation::status() const
{
    return m_status;
}

void QLottieAnimation::setStatus(QLottieAnimation::Status status)
{
    if (Q_UNLIKELY(m_status == status))
        return;

    m_status = status;
    emit statusChanged();
}

/*!
    \qmlproperty url LottieAnimation::source

    The source of the Lottie asset that LottieAnimation plays.

    LottieAnimation can handle any URL scheme supported by Qt.
    The URL may be absolute, or relative to the URL of the component.

    Setting the source property starts loading the animation asynchronously.
    To monitor progress of loading, connect to the \l status change signal.
*/
QUrl QLottieAnimation::source() const
{
    return m_source;
}

void QLottieAnimation::setSource(const QUrl &source)
{
    if (m_source != source) {
        m_source = source;
        emit sourceChanged();

        if (isComponentComplete())
            load();
    }
}

/*!
    \qmlproperty int LottieAnimation::startFrame
    \readonly

    Frame number of the start of the animation. The value
    is available after the animation has been loaded and
    ready to play.
*/
int QLottieAnimation::startFrame() const
{
    return m_startFrame;
}

void QLottieAnimation::setStartFrame(int startFrame)
{
    if (Q_UNLIKELY(m_startFrame == startFrame))
        return;

    m_startFrame = startFrame;
    emit startFrameChanged();
}

/*!
    \qmlproperty int LottieAnimation::endFrame
    \readonly

    Frame number of the end of the animation. The value
    is available after the animation has been loaded and
    ready to play.
*/
int QLottieAnimation::endFrame() const
{
    return m_endFrame;
}

void QLottieAnimation::setEndFrame(int endFrame)
{
    if (Q_UNLIKELY(m_endFrame == endFrame))
        return;

    m_endFrame = endFrame;
    emit endFrameChanged();
}

int QLottieAnimation::currentFrame() const
{
    return m_currentFrame;
}

QVersionNumber QLottieAnimation::version() const
{
    return m_version;
}

/*!
    \qmlproperty int LottieAnimation::frameRate

    This property holds the frame rate value of the Lottie animation.

    \c frameRate changes after the asset has been loaded. Changing the
    frame rate does not have effect before that, as the value defined in the
    asset overrides the value. To change the frame rate, you can write:

    \qml
    LottieAnimation {
        source: "animation.json"
        onStatusChanged: {
            if (status === LottieAnimation.Ready)
                frameRate = 60;
        }
    \endqml
*/
int QLottieAnimation::frameRate() const
{
    return m_frameRate;
}

void QLottieAnimation::setFrameRate(int frameRate)
{
    if (Q_UNLIKELY(m_frameRate == frameRate || frameRate <= 0))
        return;

    m_frameRate = frameRate;
    emit frameRateChanged();

    m_frameAdvance->setInterval(1000 / m_frameRate);
}

void QLottieAnimation::resetFrameRate()
{
    setFrameRate(m_animFrameRate);
}

/*!
    \qmlproperty enumeration LottieAnimation::quality

    Speficies the rendering quality of the lottie player.
    If \c LowQuality is selected the rendering will happen into a frame
    buffer object, whereas with other options, the rendering will be done
    onto \c QImage (which in turn will be rendered on the screen).

    \value LottieAnimation.LowQuality
           Antialiasing or a smooth pixmap transformation algorithm are not
           used

    \value LottieAnimation.MediumQuality
           Smooth pixmap transformation algorithm is used but no antialiasing
           (Default)

    \value LottieAnimation.HighQuality
           Antialiasing and a smooth pixmap tranformation algorithm are both
           used
*/
QLottieAnimation::Quality QLottieAnimation::quality() const
{
    return m_quality;
}

void QLottieAnimation::setQuality(QLottieAnimation::Quality quality)
{
    if (m_quality != quality) {
        m_quality = quality;
        if (quality == LowQuality)
            setRenderTarget(QQuickPaintedItem::FramebufferObject);
        else
            setRenderTarget(QQuickPaintedItem::Image);
        setSmooth(quality != LowQuality);
        setAntialiasing(quality == HighQuality);
        emit qualityChanged();
    }
}

void QLottieAnimation::reset()
{
    m_currentFrame = m_direction > 0 ? m_startFrame : m_endFrame;
    m_currentLoop = 0;
    QMetaObject::invokeMethod(m_frameRenderThread, "gotoFrame",
                              Q_ARG(QLottieAnimation*, this),
                              Q_ARG(int, m_currentFrame));
}

/*!
    \qmlmethod void LottieAnimation::start()

    Starts playing the animation from the beginning.
*/
void QLottieAnimation::start()
{
    reset();
    m_frameAdvance->start();
}

/*!
    \qmlmethod void LottieAnimation::play()

    Starts or continues playing from the current position.
*/
void QLottieAnimation::play()
{
    QMetaObject::invokeMethod(m_frameRenderThread, "gotoFrame",
                              Q_ARG(QLottieAnimation*, this),
                              Q_ARG(int, m_currentFrame));
    m_frameAdvance->start();
}

/*!
    \qmlmethod void LottieAnimation::pause()

    Pauses the playback.
*/
void QLottieAnimation::pause()
{
    m_frameAdvance->stop();
    QMetaObject::invokeMethod(m_frameRenderThread, "gotoFrame",
                              Q_ARG(QLottieAnimation*, this),
                              Q_ARG(int, m_currentFrame));
}

/*!
    \qmlmethod void LottieAnimation::togglePause()

    Toggles the status of player between playing and paused states.
*/
void QLottieAnimation::togglePause()
{
    if (m_frameAdvance->isActive()) {
        pause();
    } else {
        play();
    }
}

/*!
    \qmlmethod void LottieAnimation::stop()

    Stops the playback and returns to startFrame.
*/
void QLottieAnimation::stop()
{
    m_frameAdvance->stop();
    reset();
    renderNextFrame();
}

/*!
    \qmlmethod void LottieAnimation::gotoAndPlay(int frame)

    Plays the asset from the given \a frame.
*/
void QLottieAnimation::gotoAndPlay(int frame)
{
    gotoFrame(frame);
    m_currentLoop = 0;
    m_frameAdvance->start();
}

/*!
    \qmlmethod bool LottieAnimation::gotoAndPlay(string frameMarker)

    Plays the asset from the frame that has a marker with the given \a frameMarker.
    Returns \c true if the frameMarker was found, \c false otherwise.
*/
bool QLottieAnimation::gotoAndPlay(const QString &frameMarker)
{
    if (m_markers.contains(frameMarker)) {
        gotoAndPlay(m_markers.value(frameMarker));
        return true;
    } else
        return false;
}

/*!
    \qmlmethod void LottieAnimation::gotoAndStop(int frame)

    Moves the playhead to the given \a frame and stops.
*/
void QLottieAnimation::gotoAndStop(int frame)
{
    m_frameAdvance->stop();
    gotoFrame(frame);
    renderNextFrame();
}

/*!
    \qmlmethod bool LottieAnimation::gotoAndStop(string frameMarker)

    Moves the playhead to the given marker and stops.
    Returns \c true if \a frameMarker was found, \c false otherwise.
*/
bool QLottieAnimation::gotoAndStop(const QString &frameMarker)
{
    if (m_markers.contains(frameMarker)) {
        gotoAndStop(m_markers.value(frameMarker));
        return true;
    } else
        return false;
}

void QLottieAnimation::gotoFrame(int frame)
{
    m_currentFrame = qMax(m_startFrame, qMin(frame, m_endFrame));
    QMetaObject::invokeMethod(m_frameRenderThread, "gotoFrame",
                              Q_ARG(QLottieAnimation*, this),
                              Q_ARG(int, m_currentFrame));
}

/*!
    \qmlmethod double LottieAnimation::getDuration(bool inFrames)

    Returns the duration of the currently playing asset.

    If a given \a inFrames is \c true, the return value is the duration in
    number of frames. Otherwise, returns the duration in seconds.
*/
double QLottieAnimation::getDuration(bool inFrames)
{
    return (m_endFrame - m_startFrame) /
            static_cast<double>(inFrames ? 1 : m_frameRate);
}

/*!
    \qmlproperty enumeration LottieAnimation::direction

    This property holds the direction of rendering.

    \value LottieAnimation.Forward
           Forward direction (Default)

    \value LottieAnimation.Reverse
           Reverse direction
*/
QLottieAnimation::Direction QLottieAnimation::direction() const
{
    return static_cast<Direction>(m_direction);
}

void QLottieAnimation::setDirection(QLottieAnimation::Direction direction)
{
    if (Q_UNLIKELY(static_cast<Direction>(m_direction) == direction))
        return;

    m_direction = direction;
    m_currentLoop = 0;
    emit directionChanged();

    m_frameRenderThread->gotoFrame(this, m_currentFrame);
}

void QLottieAnimation::load()
{
    const QQmlContext *context = qmlContext(this);

    // resolvedUrl() performs URL interception if necessary.
    const QUrl loadUrl = context ? context->resolvedUrl(m_source) : m_source;

    if (loadUrl.isEmpty()) {
        setStatus(Null);
        return;
    }

    setStatus(Loading);

    if (QQmlFile::isLocalFile(loadUrl)) {
        QFile file(QQmlFile::urlToLocalFileOrQrc(loadUrl));
        if (!file.open(QIODevice::ReadOnly))
            setStatus(Error);
        else
            loadFinished(file.readAll());
        return;
    }

#if QT_CONFIG(lottie_network)
    QNetworkAccessManager *networkAccessManager = context
            ? context->engine()->networkAccessManager()
            : nullptr;

    if (!networkAccessManager) {
        setStatus(Error);
        return;
    }

    QNetworkRequest request(loadUrl);

    // Don't force a new connection for this request
    request.setAttribute(QNetworkRequest::HttpPipeliningAllowedAttribute, true);

    QNetworkReply *reply = networkAccessManager->get(request);

    // In case we get torn down before the reply finishes
    reply->setParent(this);

    QObject::connect(reply, &QNetworkReply::finished, this, [reply, this]() {
        if (reply->error() != QNetworkReply::NoError)
            setStatus(Error);
        else
            loadFinished(reply->readAll());
        reply->deleteLater();
    });
#else
    setStatus(Error);
#endif
}

void QLottieAnimation::loadFinished(const QByteArray &json)
{
    if (Q_UNLIKELY(parse(json) == -1)) {
        setStatus(Error);
        return;
    }

    QMetaObject::invokeMethod(m_frameRenderThread, "registerAnimator", Q_ARG(QLottieAnimation*, this));

    if (m_autoPlay)
        start();

    m_frameRenderThread->start();

    setStatus(Ready);
}

QByteArray QLottieAnimation::jsonSource() const
{
    return m_jsonSource;
}

void QLottieAnimation::renderNextFrame()
{
    if (m_currentFrame >= m_startFrame && m_currentFrame <= m_endFrame) {
        if (m_frameRenderThread->getFrame(this, m_currentFrame)) {
            update();
        } else if (!m_waitForFrameConn) {
            qCDebug(lcLottieQtLottieRender) << static_cast<void*>(this)
                                               << "Frame cache was empty for frame" << m_currentFrame;
            m_waitForFrameConn = connect(m_frameRenderThread, &QBatchRenderer::frameReady,
                                         this, [this](QLottieAnimation *target, int frameNumber) {
                if (target != this)
                    return;
                qCDebug(lcLottieQtLottieRender) << static_cast<void*>(this)
                                                   << "Frame ready" << frameNumber;
                disconnect(m_waitForFrameConn);
                update();
            });
        }
    } else if (m_loops == m_currentLoop) {
        if ( m_loops != Infinite)
            m_frameAdvance->stop();
        emit finished();
    }
}

int QLottieAnimation::parse(const QByteArray &jsonSource)
{
    m_jsonSource = jsonSource;

    QJsonParseError error;
    QJsonDocument doc = QJsonDocument::fromJson(m_jsonSource, &error);
    if (Q_UNLIKELY(error.error != QJsonParseError::NoError)) {
        qCWarning(lcLottieQtLottieParser)
            << "JSON parse error:" << error.errorString();
        return -1;
    }

    QJsonObject rootObj = doc.object();
    if (Q_UNLIKELY(rootObj.empty()))
        return -1;

    m_version = QVersionNumber::fromString(rootObj.value("v"_L1).toString());

    int startFrame = 0;
    if (!rootObj.contains("ip"_L1)) {
        qCWarning(lcLottieQtLottieParser) << "Required key \"ip\" for in point is missing";
        return -1;
    } else {
        startFrame = rootObj.value("ip"_L1).toVariant().toInt();
    }

    int endFrame = 0;
    if (!rootObj.contains("op"_L1)) {
        qCWarning(lcLottieQtLottieParser) << "Required key \"op\" for out point is missing";
        return -1;
    } else {
        endFrame = rootObj.value("op"_L1).toVariant().toInt();
    }

    if (!rootObj.contains("fr"_L1)) {
        qCWarning(lcLottieQtLottieParser) << "Required key \"fr\" for framerate is missing";
        return -1;
    }
    m_animFrameRate = rootObj.value("fr"_L1).toVariant().toInt();
    if (m_animFrameRate <= 0) {
        qCWarning(lcLottieQtLottieParser) << "Framerate \"fr\" value shold be greater than 0";
        return -1;
    }

    if (!rootObj.contains("w"_L1)) {
        qCWarning(lcLottieQtLottieParser) << "Required key \"w\" for width is missing";
        return -1;
    }
    m_animWidth = rootObj.value("w"_L1).toVariant().toReal();
    if (m_animWidth < 0) {
        qCWarning(lcLottieQtLottieParser) << "Width \"w\" value cannot be negative";
        return -1;
    }

    if (!rootObj.contains("h"_L1)) {
        qCWarning(lcLottieQtLottieParser) << "Required key \"h\" for height is missing";
        return -1;
    }
    m_animHeight = rootObj.value("h"_L1).toVariant().toReal();
    if (m_animHeight < 0) {
        qCWarning(lcLottieQtLottieParser) << "Height \"h\" value cannot be negative";
        return -1;
    }

    QJsonArray markerArr = rootObj.value("markers"_L1).toArray();
    QJsonArray::const_iterator markerIt = markerArr.constBegin();
    while (markerIt != markerArr.constEnd()) {
        QString marker = (*markerIt).toObject().value("cm"_L1).toString();
        int frame = (*markerIt).toObject().value("tm"_L1).toInt();
        m_markers.insert(marker, frame);

        if ((*markerIt).toObject().value("dr"_L1).toInt())
            qCInfo(lcLottieQtLottieParser)
                    << "property 'dr' not support in a marker";
        ++markerIt;
    }

    if (rootObj.value("chars"_L1).toArray().count())
        qCInfo(lcLottieQtLottieParser) << "chars not supported";

    setWidth(m_animWidth);
    setHeight(m_animHeight);
    setStartFrame(startFrame);
    setEndFrame(endFrame);
    setFrameRate(m_animFrameRate);

    return 0;
}

QT_END_NAMESPACE
