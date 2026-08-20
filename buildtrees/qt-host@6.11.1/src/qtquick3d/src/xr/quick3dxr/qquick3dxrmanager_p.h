// Copyright (C) 2024 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR GPL-3.0-only
// Qt-Security score:significant reason:default


#ifndef QQUICK3DXRMANAGER_P_H
#define QQUICK3DXRMANAGER_P_H

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

#include <QtQuick3DXr/private/qtquick3dxrglobal_p.h>

#include <QtCore/QObject>
#include <QtCore/QVersionNumber>
#include <QtQml/QQmlEngine>
#include <QtQml/QQmlComponent>

QT_BEGIN_NAMESPACE

class QQuickWindow;
class QQuickRenderControl;
class QQuick3DNode;
class QQuick3DViewport;
class QQuick3DXrEyeCamera;
class QQuick3DXrView;
class QQuick3DXrOrigin;
class QQuick3DXrInputManagerPrivate;
class QRhiTexture;
class QQuick3DXrAnimationDriver;

class QQuick3DXrManagerPrivate;
class QQuick3DXrInputManager;

class QQuick3DXrManager : public QObject
{
    Q_OBJECT
    Q_DECLARE_PRIVATE(QQuick3DXrManager)
public:
    explicit QQuick3DXrManager(QObject *parent = nullptr);
    ~QQuick3DXrManager();

    bool isReady() const;

    bool initialize();
    void teardown();

    bool isValid() const;

    [[nodiscard]] bool setPassthroughEnabled(bool enabled);
    bool isPassthroughEnabled() const;

    QtQuick3DXr::FoveationLevel getFixedFoveationLevel() const;
    void setFixedFoveationLevel(QtQuick3DXr::FoveationLevel level);

    QtQuick3DXr::ReferenceSpace getReferenceSpace() const;
    void setReferenceSpace(QtQuick3DXr::ReferenceSpace newReferenceSpace);

    bool isDepthSubmissionEnabled() const;
    void setDepthSubmissionEnabled(bool enable);

    QString errorString() const;

    void setSamples(int samples);

    void setMultiViewRenderingEnabled(bool enable);
    bool isMultiViewRenderingEnabled() const;
    bool isMultiViewRenderingSupported() const;
    static bool isMultiviewRenderingDisabled();

    void setXROrigin(QQuick3DXrOrigin *origin);

    void getDefaultClipDistances(float &nearClip, float &farClip) const;

private Q_SLOTS:
    void update();

Q_SIGNALS:
    void initialized();
    void sessionEnded();
    void xrOriginChanged();
    void frameReady();
    void referenceSpaceChanged();
    void multiViewRenderingEnabledChanged();

protected:
    bool event(QEvent *e) override;

private:
    friend class QQuick3DXrRuntimeInfo;
    friend class QQuick3DXrView;

    QQuick3DXrInputManager *getInputManager() const;

    bool setupGraphics();

    void renderFrame();

    void preSetupQuickScene();
    bool setupQuickScene();

    bool supportsPassthrough() const;

    QQuickWindow *m_quickWindow = nullptr;
    QQuickRenderControl *m_renderControl = nullptr;
    QQuick3DViewport *m_vrViewport = nullptr;
    QQuick3DXrOrigin *m_xrOrigin = nullptr;
    QQuick3DXrAnimationDriver *m_animationDriver = nullptr;
    bool m_xrOriginWarningShown = false;

    std::unique_ptr<QQuick3DXrManagerPrivate> d_ptr;
};

QT_END_NAMESPACE

#endif // QQUICK3DXRMANAGER_P_H
