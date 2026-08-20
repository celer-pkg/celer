// Copyright (C) 2024 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR GPL-3.0-only
// Qt-Security score:significant reason:default


#include "qquick3dxrmanager_p.h"

#include <QtCore/QCoreApplication>
#include <QtCore/QDebug>
#include <QtCore/qjsonobject.h>
#include <QtCore/qjsonarray.h>

#include <rhi/qrhi.h>

#include <QtQuick/private/qquickwindow_p.h>
#include <QtQuick/QQuickRenderControl>
#include <QtQuick/QQuickRenderTarget>
#include <QtQuick/QQuickItem>

#include <QtQuick3D/private/qquick3dnode_p.h>
#include <QtQuick3D/private/qquick3dviewport_p.h>


// #include "qquick3dxrcamera_p.h"
#include "qquick3dxranimationdriver_p.h"

#if defined(Q_OS_VISIONOS)
# include <QtQuick3DXr/private/qquick3dxrmanager_visionos_p.h>
#else
# include "openxr/qquick3dxrmanager_openxr_p.h"
#endif

#include "qquick3dxrorigin_p.h"
#include "qquick3dxrinputmanager_p.h"

QT_BEGIN_NAMESPACE

Q_DECLARE_LOGGING_CATEGORY(lcQuick3DXr);
Q_LOGGING_CATEGORY(lcQuick3DXr, "qt.quick3d.xr");

QQuick3DXrManager::QQuick3DXrManager(QObject *parent)
    : QObject(parent)
    , d_ptr(new QQuick3DXrManagerPrivate(*this))
{
}

QQuick3DXrManager::~QQuick3DXrManager()
{
    teardown();

    // maintain the correct order
    delete m_vrViewport;
    delete m_quickWindow;
    delete m_renderControl;
    delete m_animationDriver;
}

bool QQuick3DXrManager::isReady() const
{
    Q_D(const QQuick3DXrManager);
    return d->isReady();
}

bool QQuick3DXrManager::initialize()
{
    Q_D(QQuick3DXrManager);

    QString m_errorString;

    // TODO: Handle visionos being async a bit better
    if (!d->initialize()) {
        if (!d->isReady())
            m_errorString = QStringLiteral("Waiting for the renderer to start.");
        else
            m_errorString = QStringLiteral("Failed to initialize the XR manager.");

        return false;
    }

    // Setup Graphics
    return setupGraphics();
}

void QQuick3DXrManager::teardown()
{
    Q_D(QQuick3DXrManager);
    d->teardown();
}

bool QQuick3DXrManager::isValid() const
{
    Q_D(const QQuick3DXrManager);
    return d->isValid();
}

bool QQuick3DXrManager::setPassthroughEnabled(bool enabled)
{
    Q_D(QQuick3DXrManager);
    // Returns true if passthrough is supported
    return d->setPassthroughEnabled(enabled);
}

bool QQuick3DXrManager::isPassthroughEnabled() const
{
    Q_D(const QQuick3DXrManager);
    return d->isPassthroughEnabled();
}

void QQuick3DXrManager::setMultiViewRenderingEnabled(bool enable)
{
    Q_D(QQuick3DXrManager);
    d->setMultiViewRenderingEnabled(enable);
}

bool QQuick3DXrManager::isMultiViewRenderingEnabled() const
{
    Q_D(const QQuick3DXrManager);
    return d->isMultiViewRenderingEnabled();
}

bool QQuick3DXrManager::isMultiViewRenderingSupported() const
{
    QRhi *rhi = m_renderControl->rhi();
    return rhi ? rhi->isFeatureSupported(QRhi::MultiView) : false;
}

void QQuick3DXrManager::setXROrigin(QQuick3DXrOrigin *origin)
{
    m_xrOrigin = origin;
    update();
}

void QQuick3DXrManager::getDefaultClipDistances(float &nearClip, float &farClip) const
{
    Q_D(const QQuick3DXrManager);
    d->getDefaultClipDistances(nearClip, farClip);
}

QtQuick3DXr::FoveationLevel QQuick3DXrManager::getFixedFoveationLevel() const
{
#if defined(Q_OS_VISIONOS)
    // Foveation is not configurable on VisionOS
    return QtQuick3DXr::FoveationLevel::HighFoveation;
#else
    Q_D(const QQuick3DXrManager);
    return QtQuick3DXr::FoveationLevel(d->m_foveationLevel);
#endif
}

void QQuick3DXrManager::setFixedFoveationLevel(QtQuick3DXr::FoveationLevel level)
{
#if defined(Q_OS_VISIONOS)
    // Foveation is not configurable on VisionOS
    Q_UNUSED(level);
#else
    Q_D(QQuick3DXrManager);
    const XrFoveationLevelFB xrLevel = XrFoveationLevelFB(level);
    if (d->m_foveationLevel == xrLevel)
        return;

    d->m_foveationLevel = xrLevel;
    d->setupMetaQuestFoveation();
#endif
}

QtQuick3DXr::ReferenceSpace QQuick3DXrManager::getReferenceSpace() const
{
    Q_D(const QQuick3DXrManager);
    return d->getReferenceSpace();
}

void QQuick3DXrManager::setReferenceSpace(QtQuick3DXr::ReferenceSpace newReferenceSpace)
{
    Q_D(QQuick3DXrManager);

    d->setReferenceSpace(newReferenceSpace);
}

bool QQuick3DXrManager::isDepthSubmissionEnabled() const
{
    Q_D(const QQuick3DXrManager);
    return d->isDepthSubmissionEnabled();
}

void QQuick3DXrManager::setDepthSubmissionEnabled(bool enable)
{
    Q_D(QQuick3DXrManager);
    d->setDepthSubmissionEnabled(enable);
}

QString QQuick3DXrManager::errorString() const
{
    Q_D(const QQuick3DXrManager);
    return d->errorString();
}

void QQuick3DXrManager::setSamples(int samples)
{
    Q_D(QQuick3DXrManager);
    d->setSamples(samples);
}

void QQuick3DXrManager::update()
{
    Q_D(QQuick3DXrManager);
    d->update();
}

bool QQuick3DXrManager::event(QEvent *e)
{
    Q_D(QQuick3DXrManager);

    if (e->type() == QEvent::UpdateRequest) {
        d->processXrEvents();
        d->update();
        return true;
    }
    return QObject::event(e);
}

bool QQuick3DXrManager::isMultiviewRenderingDisabled()
{
    const bool disabled = qEnvironmentVariableIntValue("QT_QUICK3D_XR_DISABLE_MULTIVIEW") != 0;
    return disabled;
}

QQuick3DXrInputManager *QQuick3DXrManager::getInputManager() const
{
    Q_D(const QQuick3DXrManager);
    return d->m_inputManager.data();
}

bool QQuick3DXrManager::setupGraphics()
{
    Q_D(QQuick3DXrManager);

    // FIXME: Should probably make sure we don't accidentally get here more then once
    // or if we're re-initializing, in which case: make sure to clean up properly first.
    if (d->isGraphicsInitialized())
        return true;

    preSetupQuickScene();

    if (!d->setupGraphics(m_quickWindow))
        return false;

    if (!setupQuickScene())
        return false;

    QRhi *rhi = m_quickWindow->rhi();
    QSSG_ASSERT_X(rhi != nullptr, "No RHI handle!", return false);

    if (!d->isMultiViewRenderingEnabled())
        emit multiViewRenderingEnabledChanged();

    return d->finalizeGraphics(rhi);
}

void QQuick3DXrManager::renderFrame()
{
    Q_D(QQuick3DXrManager);

    if (!m_xrOrigin) {
        if (!m_xrOriginWarningShown) {
            qWarning() << "No XrOrigin found!";
            m_xrOriginWarningShown = true;
        }
        return;
    }

    d->doRenderFrame();
}

void QQuick3DXrManager::preSetupQuickScene()
{
    if (!m_renderControl)
        m_renderControl = new QQuickRenderControl;
    if (!m_quickWindow)
        m_quickWindow = new QQuickWindow(m_renderControl);
}

bool QQuick3DXrManager::setupQuickScene()
{
    Q_D(QQuick3DXrManager);

    d->setupWindow(m_quickWindow);

    if (!m_animationDriver) {
        m_animationDriver = new QQuick3DXrAnimationDriver;
        m_animationDriver->install();
    }

    const bool initSuccess = m_renderControl->initialize();
    if (!initSuccess) {
        qWarning("Quick 3D XR: Failed to create renderControl (failed to initialize RHI?)");
        return false;
    }

    QRhi *rhi = m_renderControl->rhi();
    if (!rhi) {
        qWarning("Quick3D XR: No QRhi from renderControl. This should not happen.");
        return false;
    }

    qCDebug(lcQuick3DXr, "Quick 3D XR: QRhi initialized with backend %s", rhi->backendName());

    return true;
}

bool QQuick3DXrManager::supportsPassthrough() const
{
    Q_D(const QQuick3DXrManager);
    return d->supportsPassthrough();
}

QT_END_NAMESPACE
