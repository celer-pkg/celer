// Copyright (C) 2016 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR LGPL-3.0-only OR GPL-2.0-only OR GPL-3.0-only
// Qt-Security score:significant reason:default


#include "qsvgrenderer.h"

#ifndef QT_NO_SVGRENDERER

#include "qsvgdocument_p.h"

#include "qbytearray.h"
#include "qtimer.h"
#include "qtransform.h"
#include "qdebug.h"
#include "private/qobject_p.h"


QT_BEGIN_NAMESPACE

/*!
    \class QSvgRenderer
    \inmodule QtSvg
    \ingroup painting

    \brief The QSvgRenderer class is used to draw the contents of SVG files onto paint devices.
    \since 4.1
    \reentrant

    Using QSvgRenderer, Scalable Vector Graphics (SVG) can be rendered onto any QPaintDevice
    subclass, including QWidget, QImage, and QGLWidget.

    QSvgRenderer provides an API that supports basic features of SVG rendering, such as loading
    and rendering of static drawings, and more interactive features like animation. Since the
    rendering is performed using QPainter, SVG drawings can be rendered on any subclass of
    QPaintDevice.

    SVG drawings are either loaded when an QSvgRenderer is constructed, or loaded later
    using the load() functions. Data is either supplied directly as serialized XML, or
    indirectly using a file name. If a valid file has been loaded, either when the renderer
    is constructed or at some later time, isValid() returns true; otherwise it returns false.
    QSvgRenderer provides the render() slot to render the current document, or the current
    frame of an animated document, using a given painter.

    The defaultSize() function provides information about the amount of space that is required
    to render the currently loaded SVG file. This is useful for paint devices, such as QWidget,
    that often need to supply a size hint to their parent layout.
    The default size of a drawing may differ from its visible area, found using the \l viewBox
    property.

    Animated SVG drawings are supported, and can be controlled with a simple collection of
    functions and properties:

    \list
    \li The animated() function indicates whether a drawing contains animation information.
    \omit
    \li The animationDuration() function provides the duration in milliseconds of the
       animation, without taking any looping into account.
    \li The \l currentFrame property contains the current frame of the animation.
    \endomit
    \li The \l framesPerSecond property contains the rate at which the animation plays.
    \endlist

    Finally, the QSvgRenderer class provides the repaintNeeded() signal which is emitted
    whenever the rendering of the document needs to be updated.

    \sa QSvgWidget, {Qt SVG C++ Classes}, QPicture
*/

class QSvgRendererPrivate : public QObjectPrivate
{
    Q_DECLARE_PUBLIC(QSvgRenderer)
public:
    explicit QSvgRendererPrivate()
        : QObjectPrivate(),
          timer(0),
          fps(30)
    {
        options = defaultOptions();
    }

    void startOrStopTimer()
    {
        if (animationEnabled && render && render->animated() && fps > 0) {
            ensureTimerCreated();
            timer->start(1000 / fps);
        } else if (timer) {
            timer->stop();
        }
    }

    void ensureTimerCreated()
    {
        Q_Q(QSvgRenderer);
        if (!timer) {
            timer = new QTimer(q);
            q->connect(timer, &QTimer::timeout, q, &QSvgRenderer::repaintNeeded);
        }
    }

    static void callRepaintNeeded(QSvgRenderer *const q);

    static QtSvg::Options defaultOptions()
    {
        static bool envOk = false;
        static QtSvg::Options envOpts = QtSvg::Options::fromInt(
                qEnvironmentVariableIntValue("QT_SVG_DEFAULT_OPTIONS", &envOk));
        return envOk ? envOpts : appDefaultOptions;
    }

    std::unique_ptr<QSvgDocument> render;
    QTimer *timer;
    int fps;
    QtSvg::Options options;
    static QtSvg::Options appDefaultOptions;
    bool animationEnabled = true;
};

QtSvg::Options QSvgRendererPrivate::appDefaultOptions;

/*!
    Constructs a new renderer with the given \a parent.
*/
QSvgRenderer::QSvgRenderer(QObject *parent)
    : QObject(*(new QSvgRendererPrivate), parent)
{
}

/*!
    Constructs a new renderer with the given \a parent and loads the contents of the
    SVG file with the specified \a filename.
*/
QSvgRenderer::QSvgRenderer(const QString &filename, QObject *parent)
    : QObject(*new QSvgRendererPrivate, parent)
{
    load(filename);
}

/*!
    Constructs a new renderer with the given \a parent and loads the SVG data
    from the byte array specified by \a contents.
*/
QSvgRenderer::QSvgRenderer(const QByteArray &contents, QObject *parent)
    : QObject(*new QSvgRendererPrivate, parent)
{
    load(contents);
}

/*!
    \since 4.5

    Constructs a new renderer with the given \a parent and loads the SVG data
    using the stream reader specified by \a contents.
*/
QSvgRenderer::QSvgRenderer(QXmlStreamReader *contents, QObject *parent)
    : QObject(*new QSvgRendererPrivate, parent)
{
    load(contents);
}

/*!
    Destroys the renderer.
*/
QSvgRenderer::~QSvgRenderer()
{

}

/*!
    Returns true if there is a valid current document; otherwise returns false.
*/
bool QSvgRenderer::isValid() const
{
    Q_D(const QSvgRenderer);
    return bool(d->render);
}

/*!
    Returns the default size of the document contents.
*/
QSize QSvgRenderer::defaultSize() const
{
    Q_D(const QSvgRenderer);
    if (d->render)
        return d->render->size();
    else
        return QSize();
}

/*!
    Returns viewBoxF().toRect().

    \sa viewBoxF()
*/
QRect QSvgRenderer::viewBox() const
{
    Q_D(const QSvgRenderer);
    if (d->render)
        return d->render->viewBox().toRect();
    else
        return QRect();
}

/*!
    \property QSvgRenderer::viewBox
    \brief the rectangle specifying the visible area of the document in logical coordinates
    \since 4.2
*/
void QSvgRenderer::setViewBox(const QRect &viewbox)
{
    Q_D(QSvgRenderer);
    if (d->render)
        d->render->setViewBox(viewbox);
}

/*!
    Returns true if the current document contains animated elements; otherwise
    returns false.

    \sa framesPerSecond()
*/
bool QSvgRenderer::animated() const
{
    Q_D(const QSvgRenderer);
    if (d->render)
        return d->render->animated();
    else
        return false;
}

/*!
    \property QSvgRenderer::animationEnabled
    \brief whether the animation should run, if the SVG is animated

    Setting the property to false stops the animation timer.
    Setting the property to true starts the animation timer,
    provided that the SVG contains animated elements.

    If the SVG is not animated, the property will have no effect.
    Otherwise, the property defaults to true.

    \sa animated()

    \since 6.7
*/
bool QSvgRenderer::isAnimationEnabled() const
{
    Q_D(const QSvgRenderer);
    return d->animationEnabled;
}

void QSvgRenderer::setAnimationEnabled(bool enable)
{
    Q_D(QSvgRenderer);
    d->animationEnabled = enable;
    d->startOrStopTimer();
}

/*!
    \property QSvgRenderer::framesPerSecond
    \brief the number of frames per second to be shown

    The number of frames per second is 0 if the current document is not animated.

    \sa animated()
*/
int QSvgRenderer::framesPerSecond() const
{
    Q_D(const QSvgRenderer);
    return d->fps;
}

void QSvgRenderer::setFramesPerSecond(int num)
{
    Q_D(QSvgRenderer);
    if (num < 0) {
        qWarning("QSvgRenderer::setFramesPerSecond: Cannot set negative value %d", num);
        return;
    }
    d->fps = num;
    d->startOrStopTimer();
}

/*!
    \property QSvgRenderer::aspectRatioMode

    \brief how rendering adheres to the SVG view box aspect ratio

    The accepted modes are:
    \list
    \li Qt::IgnoreAspectRatio (the default): the aspect ratio is ignored and the
        rendering is stretched to the target bounds.
    \li Qt::KeepAspectRatio: rendering is centered and scaled as large as possible
        within the target bounds while preserving aspect ratio.
    \endlist

    \since 5.15
*/

Qt::AspectRatioMode QSvgRenderer::aspectRatioMode() const
{
    Q_D(const QSvgRenderer);
    if (d->render && d->render->preserveAspectRatio())
        return Qt::KeepAspectRatio;
    return Qt::IgnoreAspectRatio;
}

void QSvgRenderer::setAspectRatioMode(Qt::AspectRatioMode mode)
{
    Q_D(QSvgRenderer);
    if (d->render) {
        if (mode == Qt::KeepAspectRatio)
            d->render->setPreserveAspectRatio(true);
        else if (mode == Qt::IgnoreAspectRatio)
            d->render->setPreserveAspectRatio(false);
    }
}

/*!
    \property QSvgRenderer::options
    \since 6.7

    This property holds a set of QtSvg::Option flags that can be used
    to enable or disable various features of the parsing and rendering of SVG files.

    In order to take effect, this property must be set \c before load() is executed. Note that the
    constructors taking an SVG source parameter will perform loading during construction.

    \sa setDefaultOptions
 */
QtSvg::Options QSvgRenderer::options() const
{
    Q_D(const QSvgRenderer);
    return d->options;
}

void QSvgRenderer::setOptions(QtSvg::Options flags)
{
    Q_D(QSvgRenderer);
    d->options = flags;
}

/*!
    Sets the option flags that renderers will be created with to \a flags.
    By default, no flags are set.

    At runtime, this can be overridden by the QT_SVG_DEFAULT_OPTIONS environment variable.

    \since 6.8
*/

void QSvgRenderer::setDefaultOptions(QtSvg::Options flags)
{
    QSvgRendererPrivate::appDefaultOptions = flags;
}

/*!
  \property QSvgRenderer::currentFrame
  \brief the current frame of the document's animation, or 0 if the document is not animated
  \internal

  \sa animationDuration(), framesPerSecond, animated()
*/

/*!
  \internal
*/
int QSvgRenderer::currentFrame() const
{
    Q_D(const QSvgRenderer);
    return d->render->currentFrame();
}

/*!
  \internal
*/
void QSvgRenderer::setCurrentFrame(int frame)
{
    Q_D(QSvgRenderer);
    d->render->setCurrentFrame(frame);
}

/*!
    \internal

    Returns the number of frames in the animation, or 0 if the current document is not
    animated.

    \sa animated(), framesPerSecond
*/
int QSvgRenderer::animationDuration() const
{
    Q_D(const QSvgRenderer);
    return d->render->animationDuration();
}

/*!
 \internal
 \since 4.5

 We can't have template functions, that's loadDocument(), as friends, for this
 code, so we let this function be a friend of QSvgRenderer instead.
 */
void QSvgRendererPrivate::callRepaintNeeded(QSvgRenderer *const q)
{
    emit q->repaintNeeded();
}

template<typename TInputType>
static bool loadDocument(QSvgRenderer *const q,
                         QSvgRendererPrivate *const d,
                         const TInputType &in)
{
    d->render = QSvgDocument::load(in, d->options);
    if (d->render && !d->render->size().isValid())
        d->render.reset();

    d->startOrStopTimer();

    if (d->render)
        d->render->restartAnimation();

    //force first update
    QSvgRendererPrivate::callRepaintNeeded(q);

    return bool(d->render);
}

/*!
    Loads the SVG file specified by \a filename, returning true if the content
    was successfully parsed; otherwise returns false.
*/
bool QSvgRenderer::load(const QString &filename)
{
    Q_D(QSvgRenderer);
    return loadDocument(this, d, filename);
}

/*!
    Loads the specified SVG format \a contents, returning true if the content
    was successfully parsed; otherwise returns false.
*/
bool QSvgRenderer::load(const QByteArray &contents)
{
    Q_D(QSvgRenderer);
    return loadDocument(this, d, contents);
}

/*!
  Loads the specified SVG in \a contents, returning true if the content
  was successfully parsed; otherwise returns false.

  The reader will be used from where it currently is positioned. If \a contents
  is \c null, behavior is undefined.

  \since 4.5
*/
bool QSvgRenderer::load(QXmlStreamReader *contents)
{
    Q_D(QSvgRenderer);
    return loadDocument(this, d, contents);
}

/*!
    Renders the current document, or the current frame of an animated
    document, using the given \a painter.
*/
void QSvgRenderer::render(QPainter *painter)
{
    Q_D(QSvgRenderer);
    if (d->render) {
        if (d->render->animator())
            d->render->animator()->advanceAnimations();
        d->render->draw(painter);
    }
}

/*!
    \fn void QSvgRenderer::repaintNeeded()

    This signal is emitted whenever the rendering of the document
    needs to be updated, usually for the purposes of animation.
*/

/*!
    Renders the given element with \a elementId using the given \a painter
    on the specified \a bounds. If the bounding rectangle is not specified
    the SVG element is mapped to the whole paint device.
*/
void QSvgRenderer::render(QPainter *painter, const QString &elementId,
                          const QRectF &bounds)
{
    Q_D(QSvgRenderer);
    if (d->render) {
        if (d->render->animator())
            d->render->animator()->advanceAnimations();
        d->render->draw(painter, elementId, bounds);
    }
}

/*!
    Renders the current document, or the current frame of an animated document,
    using the given \a painter on the specified \a bounds within the painter.
    If \a bounds is not empty, the output will be scaled to fill it, ignoring
    any aspect ratio implied by the SVG.
*/
void QSvgRenderer::render(QPainter *painter, const QRectF &bounds)
{
    Q_D(QSvgRenderer);
    if (d->render) {
        if (d->render->animator())
            d->render->animator()->advanceAnimations();
        d->render->draw(painter, bounds);
    }
}

QRectF QSvgRenderer::viewBoxF() const
{
    Q_D(const QSvgRenderer);
    if (d->render)
        return d->render->viewBox();
    else
        return QRect();
}

void QSvgRenderer::setViewBox(const QRectF &viewbox)
{
    Q_D(QSvgRenderer);
    if (d->render)
        d->render->setViewBox(viewbox);
}

/*!
    \since 4.2

    Returns bounding rectangle of the item with the given \a id.
    The transformation matrix of parent elements is not affecting
    the bounds of the element.

    \sa transformForElement()
*/
QRectF QSvgRenderer::boundsOnElement(const QString &id) const
{
    Q_D(const QSvgRenderer);
    QRectF bounds;
    if (d->render)
        bounds = d->render->boundsOnElement(id);
    return bounds;
}


/*!
    \since 4.2

    Returns true if the element with the given \a id exists
    in the currently parsed SVG file and is a renderable
    element.

    Note: this method returns true only for elements that
    can be rendered. Which implies that elements that are considered
    part of the fill/stroke style properties, e.g. radialGradients
    even tough marked with "id" attributes will not be found by this
    method.
*/
bool QSvgRenderer::elementExists(const QString &id) const
{
    Q_D(const QSvgRenderer);
    bool exists = false;
    if (d->render)
        exists = d->render->elementExists(id);
    return exists;
}

/*!
    \since 5.15

    Returns the transformation matrix for the element
    with the given \a id. The matrix is a product of
    the transformation of the element's parents. The transformation of
    the element itself is not included.

    To find the bounding rectangle of the element in logical coordinates,
    you can apply the matrix on the rectangle returned from boundsOnElement().

    \sa boundsOnElement()
*/
QTransform QSvgRenderer::transformForElement(const QString &id) const
{
    Q_D(const QSvgRenderer);
    QTransform trans;
    if (d->render)
        trans = d->render->transformForElement(id);
    return trans;
}

QT_END_NAMESPACE

#include "moc_qsvgrenderer.cpp"

#endif // QT_NO_SVGRENDERER
