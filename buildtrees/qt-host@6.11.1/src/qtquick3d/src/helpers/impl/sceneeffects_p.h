// Copyright (C) 2022 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR GPL-3.0-only
// Qt-Security score:significant reason:default


#ifndef SCENEEFFECTS_H
#define SCENEEFFECTS_H

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

#include <QtQuick3D/private/qquick3deffect_p.h>
#include <QtQuick3D/private/qquick3dsceneenvironment_p.h>

QT_BEGIN_NAMESPACE

class QQuick3DSceneEnvironment;
class SceneEffectEnvironment;

class SceneEffectBase : public QQuick3DEffect
{
    Q_OBJECT
    Q_PROPERTY(QQuick3DSceneEnvironment * environment READ environment WRITE setEnvironment NOTIFY environmentChanged)
    QML_NAMED_ELEMENT(SceneEffectBase)
    QML_UNCREATABLE("SceneEffectBase is Abstract")
public:
    explicit SceneEffectBase(QQuick3DObject *parent = nullptr);

    QQuick3DSceneEnvironment *environment() const;
    void setEnvironment(QQuick3DSceneEnvironment *newEnvironment);

signals:
    void environmentChanged();

protected:
    void scheduleEnvUpdate();

private:
    QQuick3DSceneEnvironment *m_environment = nullptr;
    virtual void registerWithEnv(SceneEffectEnvironment *newEnvironment) = 0;
    virtual void unregisterWithEnv(SceneEffectEnvironment *oldEnvironment) = 0;
};

class MainSceneEffect : public SceneEffectBase
{
    Q_OBJECT
    QML_NAMED_ELEMENT(MainSceneEffect)
public:
    explicit MainSceneEffect(QQuick3DObject *p = nullptr);

private:
    void registerWithEnv(SceneEffectEnvironment *newEnvironment) override;
    void unregisterWithEnv(SceneEffectEnvironment *oldEnvironment) override;
};

class DepthOfFieldEffect : public SceneEffectBase
{
    Q_OBJECT
    Q_PROPERTY(bool enabled READ enabled WRITE setEnabled NOTIFY enabledChanged)
    QML_NAMED_ELEMENT(DepthOfFieldEffect)
public:
    explicit DepthOfFieldEffect(QQuick3DObject *p = nullptr);

    bool enabled() const;
    void setEnabled(bool newEnabled);

signals:
    void enabledChanged();

private:
    void registerWithEnv(SceneEffectEnvironment *newEnvironment) override;
    void unregisterWithEnv(SceneEffectEnvironment *oldEnvironment) override;
    bool m_enabled = false;
};

class SsgiEnvEffect : public SceneEffectBase
{
    Q_OBJECT
    Q_PROPERTY(bool enabled READ enabled WRITE setEnabled NOTIFY enabledChanged)
    QML_NAMED_ELEMENT(SsgiEnvEffect)
public:
    explicit SsgiEnvEffect(QQuick3DObject *p = nullptr);

    bool enabled() const;
    void setEnabled(bool newEnabled);

signals:
    void enabledChanged();

private:
    void registerWithEnv(SceneEffectEnvironment *newEnvironment) override;
    void unregisterWithEnv(SceneEffectEnvironment *oldEnvironment) override;
    bool m_enabled = false;
};

class SsrEnvEffect : public SceneEffectBase
{
    Q_OBJECT
    Q_PROPERTY(bool enabled READ enabled WRITE setEnabled NOTIFY enabledChanged)
    QML_NAMED_ELEMENT(SsrEnvEffect)
public:
    explicit SsrEnvEffect(QQuick3DObject *p = nullptr);

    bool enabled() const;
    void setEnabled(bool newEnabled);

signals:
    void enabledChanged();

private:
    void registerWithEnv(SceneEffectEnvironment *newEnvironment) override;
    void unregisterWithEnv(SceneEffectEnvironment *oldEnvironment) override;
    bool m_enabled = false;
};

class SceneEffectEnvironment : public QQuick3DSceneEnvironment
{
    Q_OBJECT
    QML_NAMED_ELEMENT(SceneEffectEnvironment)

public:
    explicit SceneEffectEnvironment(QQuick3DObject *p = nullptr);
    void setMainSceneEffect(MainSceneEffect *tonemapper);
    void setDeptOfFieldEffect(DepthOfFieldEffect *dof);
    void setSsgiEffect(SsgiEnvEffect *ssgi);
    void setSsrEffect(SsrEnvEffect *ssr);

protected:
    QSSGRenderGraphObject *updateSpatialNode(QSSGRenderGraphObject *node) override;

private:
    QVector<QQuick3DEffect *> m_effects;
    MainSceneEffect *m_tonemapper = nullptr;
    DepthOfFieldEffect *m_dof = nullptr;
    SsgiEnvEffect *m_ssgi = nullptr;
    SsrEnvEffect *m_ssr = nullptr;

protected:
    const QVector<QQuick3DEffect *> &effectList() const override;
    bool useBuiltinTonemapper() const override;
};

QT_END_NAMESPACE

#endif // SCENEEFFECTS_H
