// Copyright (C) 2025 The Qt Company Ltd.
// Copyright (C) 2018 QUIt Coding <info@quitcoding.com>
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR GPL-3.0-only
// Qt-Security score:significant reason:default


#include "qcanvaspainterfactory_p.h"
#include "qcanvasrhipaintdriver_p.h"
#include "qcanvaspainter_p.h"
#include "engine/qcpainterengine_p.h"

#include <QMutex>
#include <QMutexLocker>

QT_BEGIN_NAMESPACE

/*!
    \class QCanvasPainterFactory
    \since 6.11
    \brief QCanvasPainterFactory manages instances of QCanvasPainter and the associated rendering engine.
    \inmodule QtCanvasPainter

    Applications rendering via QCanvasPainter do not create instances of the painter
    class themselves, but rather are expected to use a QCanvasPainterFactory.

    \note This class is relevant only when working with QCanvasPainter \b without a
    convenience class such as QCanvasPainterWidget or QCanvasPainterItem, because
    those provide a QCanvasPainter instance to the application.

    The following is an example code snippet of getting a QCanvasPainter for use with
    an existing, successfully initialized QRhi:

    \code
        std::unique_ptr<QCanvasPainterFactory> factory(new QCanvasPainterFactory);
        QCanvasPainter *painter = factory->create(rhi);
        QCanvasRhiPaintDriver *paintDriver = factory->paintDriver();
    \endcode

    All drawing code that operates on the same thread with the same QRhi is
    recommended to share and reuse the same QCanvasPainter, instead of having a
    dedicated painter (and so factory) in each component. This can be achieved
    by calling sharedInstance(), instead of constructing a new QCanvasPainterFactory:

    \code
        QCanvasPainterFactory *factory = QCanvasPainterFactory::sharedInstance(rhi);
        QCanvasPainter *painter = factory->painter();
        QCanvasRhiPaintDriver *paintDriver = factory->paintDriver();
    \endcode
 */

struct QCanvasPainterFactoryGlobal
{
    QHash<QRhi *, QCanvasPainterFactory *> h;
};

Q_GLOBAL_STATIC(QCanvasPainterFactoryGlobal, qcpd_g);

/*!
    \return a QCanvasPainterFactory instance associated with the given \a rhi.

    Calling this function with the same \a rhi will always return the same
    QCanvasPainterFactory.

    There is no need to call create() on the returned factory, and the \l
    painter() can be queried right away, because the result is always already
    initialized, meaning isValid() returns true.

    This function is thread safe. However, the QRhi threading rules apply as
    usual: a QRhi object, and by extension, an instance of QCanvasPainterFactory,
    QCanvasRhiPaintDriver, and QCanvasPainter associated with that QRhi are all expected
    to be used on the same one single thread.

    \note The returned QCanvasPainterFactory is not owned by the caller, and it
    becomes invalid when \a rhi is destroyed.

    \sa painter()
 */
QCanvasPainterFactory *QCanvasPainterFactory::sharedInstance(QRhi *rhi)
{
    static QMutex m;
    QMutexLocker lck(&m);

    auto &h(qcpd_g()->h);
    auto it = h.constFind(rhi);
    if (it != h.constEnd())
        return it.value();

    QCanvasPainterFactory *pd = new QCanvasPainterFactory;
    pd->create(rhi);
    h.insert(rhi, pd);

    static auto cleanupFunc = [](QRhi *rhiAboutToDie) {
        auto &h(qcpd_g()->h);
        auto it = h.find(rhiAboutToDie);
        if (it != h.end()) {
            delete it.value();
            h.erase(it);
        }
    };
    rhi->addCleanupCallback(cleanupFunc);

    return pd;
}

/*!
    Constructor.

    \sa sharedInstance()
 */
QCanvasPainterFactory::QCanvasPainterFactory()
    : d(new QCanvasPainterFactoryPrivate)
{
}

/*
    Destructor.
 */
QCanvasPainterFactory::~QCanvasPainterFactory()
{
    destroy();
    delete d;
}

/*!
    \return true if the create() has ben called successfully.
 */
bool QCanvasPainterFactory::isValid() const
{
    return d->painter && d->renderer.isValid();
}

/*!
    \return the painter, or null if isValid() is false.

    \note The returned object is not owned by the caller.
 */
QCanvasPainter *QCanvasPainterFactory::painter()
{
    return d->painter.get();
}

/*!
    \return the paint driver object, or null if isValid() is false.

    \note The returned object is not owned by the caller.
 */
QCanvasRhiPaintDriver *QCanvasPainterFactory::paintDriver()
{
    return d->paintDriver.get();
}

/*!
    Initializes the QCanvasPainter rendering infrastructure, if it has not been done
    already for this factory.

    \return the painter, or null if the renderer could not be initialized.

    The factory will be associated with \a rhi, and it cannot be used with any
    other QRhi, unless destroy(), and then create() are called again.

    Repeated calls to this function have no effect and are harmless. Call
    destroy() first if a reinitialization of the painter infrastructure is
    desired, possibly with a different \a rhi.

    \note There is no need to call this function when sharedInstance() was used
    to retrieve a factory instance.

    \sa destroy(), isValid(), painter()
 */
QCanvasPainter *QCanvasPainterFactory::create(QRhi *rhi)
{
    if (!d->paintDriver)
        d->paintDriver.reset(new QCanvasRhiPaintDriver);

    if (!d->painter)
        d->painter.reset(new QCanvasPainter);

    if (!d->renderer.isValid()) {
        // Each painter owns an engine, and each painter+engine combo is associated with one renderer at a time.
        // The renderer we have to manage (create, destroy), and it is per-QRhi.
        d->renderer.create(rhi, d->painter.get());
    }

    if (!d->renderer.isValid()) {
        qWarning("QCanvasPainterFactory::create() failed to initialize renderer");
        return nullptr;
    }

    QCanvasRhiPaintDriverPrivate *pd = QCanvasRhiPaintDriverPrivate::get(d->paintDriver.get());
    pd->painter = d->painter.get();
    pd->renderer = &d->renderer;
    pd->currentCb = nullptr;
    pd->currentRt = nullptr;

    return d->painter.get();
}

/*!
    Tears down the rendering infrastructure. Normally there is no need to call
    this function. Rather, it is used in situations where it will be followed by
    a create().

    \sa create()
 */
void QCanvasPainterFactory::destroy()
{
    if (d->painter)
        QCanvasPainterPrivate::get(d->painter.get())->clearTextureCache();

    if (d->renderer.isValid())
        d->renderer.destroy();

    // Can be followed by a new call to create(). d->painter and its engine stay intact.
}

QT_END_NAMESPACE
