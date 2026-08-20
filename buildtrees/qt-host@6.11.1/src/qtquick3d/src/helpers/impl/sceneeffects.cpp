// Copyright (C) 2022 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR GPL-3.0-only
// Qt-Security score:significant reason:default


#include "sceneeffects_p.h"

QT_BEGIN_NAMESPACE

SceneEffectBase::SceneEffectBase(QQuick3DObject *parent)
    : QQuick3DEffect(parent)
{

}

QQuick3DSceneEnvironment *SceneEffectBase::environment() const
{
    return m_environment;
}

void SceneEffectBase::setEnvironment(QQuick3DSceneEnvironment *newEnvironment)
{
    if (m_environment == newEnvironment)
        return;

    if (auto env = qobject_cast<SceneEffectEnvironment *>(m_environment))
        unregisterWithEnv(env);

    m_environment = newEnvironment;

    QQuick3DObjectPrivate::attachWatcher(this, &SceneEffectBase::setEnvironment, newEnvironment, m_environment);

    if (auto env = qobject_cast<SceneEffectEnvironment *>(m_environment))
        registerWithEnv(env);

    emit environmentChanged();
}

void SceneEffectBase::scheduleEnvUpdate()
{
    if (m_environment)
        m_environment->update();
}

void SceneEffectBase::registerWithEnv(SceneEffectEnvironment *env)
{
    Q_UNUSED(env);
    qWarning() << "We shouldn't be here!!!";
}
void SceneEffectBase::unregisterWithEnv(SceneEffectEnvironment *oldEnvironment)
{
    Q_UNUSED(oldEnvironment);
    qWarning() << "We shouldn't be here!!!";
}

MainSceneEffect::MainSceneEffect(QQuick3DObject *p)
    : SceneEffectBase(p)
{

}

void MainSceneEffect::registerWithEnv(SceneEffectEnvironment *newEnvironment)
{
    if (newEnvironment)
        newEnvironment->setMainSceneEffect(this);
}

void MainSceneEffect::unregisterWithEnv(SceneEffectEnvironment *oldEnvironment)
{
    if (oldEnvironment)
        oldEnvironment->setMainSceneEffect(nullptr);
}

DepthOfFieldEffect::DepthOfFieldEffect(QQuick3DObject *p)
    : SceneEffectBase(p)
{

}

void DepthOfFieldEffect::registerWithEnv(SceneEffectEnvironment *newEnvironment)
{
    if (newEnvironment)
        newEnvironment->setDeptOfFieldEffect(this);
}

void DepthOfFieldEffect::unregisterWithEnv(SceneEffectEnvironment *oldEnvironment)
{
    if (oldEnvironment)
        oldEnvironment->setDeptOfFieldEffect(nullptr);
}

bool DepthOfFieldEffect::enabled() const
{
    return m_enabled;
}

void DepthOfFieldEffect::setEnabled(bool newEnabled)
{
    if (m_enabled == newEnabled)
        return;
    m_enabled = newEnabled;
    scheduleEnvUpdate();
    emit enabledChanged();
}

SsgiEnvEffect::SsgiEnvEffect(QQuick3DObject *p)
    : SceneEffectBase(p)
{
}

void SsgiEnvEffect::registerWithEnv(SceneEffectEnvironment *newEnvironment)
{
    if (newEnvironment)
        newEnvironment->setSsgiEffect(this);
}

void SsgiEnvEffect::unregisterWithEnv(SceneEffectEnvironment *oldEnvironment)
{
    if (oldEnvironment)
        oldEnvironment->setSsgiEffect(nullptr);
}

bool SsgiEnvEffect::enabled() const
{
    return m_enabled;
}

void SsgiEnvEffect::setEnabled(bool newEnabled)
{
    if (m_enabled == newEnabled)
        return;

    m_enabled = newEnabled;
    scheduleEnvUpdate();
    emit enabledChanged();
}

SsrEnvEffect::SsrEnvEffect(QQuick3DObject *p)
    : SceneEffectBase(p)
{
}

void SsrEnvEffect::registerWithEnv(SceneEffectEnvironment *newEnvironment)
{
    if (newEnvironment)
        newEnvironment->setSsrEffect(this);
}

void SsrEnvEffect::unregisterWithEnv(SceneEffectEnvironment *oldEnvironment)
{
    if (oldEnvironment)
        oldEnvironment->setSsrEffect(nullptr);
}

bool SsrEnvEffect::enabled() const
{
    return m_enabled;
}

void SsrEnvEffect::setEnabled(bool newEnabled)
{
    if (m_enabled == newEnabled)
        return;

    m_enabled = newEnabled;
    scheduleEnvUpdate();
    emit enabledChanged();
}

SceneEffectEnvironment::SceneEffectEnvironment(QQuick3DObject *p)
    : QQuick3DSceneEnvironment(p)
{
}

void SceneEffectEnvironment::setMainSceneEffect(MainSceneEffect *tonemapper)
{
    if (m_tonemapper == tonemapper)
        return;

    QQuick3DObjectPrivate::attachWatcher(this, &SceneEffectEnvironment::setMainSceneEffect, tonemapper, m_tonemapper);

    m_tonemapper = tonemapper;
}

void SceneEffectEnvironment::setDeptOfFieldEffect(DepthOfFieldEffect *dof)
{
    if (m_dof == dof)
        return;

    QQuick3DObjectPrivate::attachWatcher(this, &SceneEffectEnvironment::setDeptOfFieldEffect, dof, m_dof);

    m_dof = dof;
}

void SceneEffectEnvironment::setSsgiEffect(SsgiEnvEffect *ssgi)
{
    if (m_ssgi == ssgi)
        return;

    QQuick3DObjectPrivate::attachWatcher(this, &SceneEffectEnvironment::setSsgiEffect, ssgi, m_ssgi);

    m_ssgi = ssgi;
}

void SceneEffectEnvironment::setSsrEffect(SsrEnvEffect *ssr)
{
    if (m_ssr == ssr)
        return;

    QQuick3DObjectPrivate::attachWatcher(this, &SceneEffectEnvironment::setSsrEffect, ssr, m_ssr);

    m_ssr = ssr;
}

QSSGRenderGraphObject *SceneEffectEnvironment::updateSpatialNode(QSSGRenderGraphObject *node)
{
    m_effects = QQuick3DSceneEnvironment::effectList();
    if (m_ssgi && m_ssgi->enabled())
        m_effects.push_back(m_ssgi);
    if (m_ssr && m_ssr->enabled()) {
        m_effects.push_back(m_ssr);
    }
    if (m_dof && m_dof->enabled())
        m_effects.push_back(m_dof);
    if (m_tonemapper)
        m_effects.push_back(m_tonemapper);
    node = QQuick3DSceneEnvironment::updateSpatialNode(node);

    return node;
}

const QVector<QQuick3DEffect *> &SceneEffectEnvironment::effectList() const
{
    return m_effects;
}

bool SceneEffectEnvironment::useBuiltinTonemapper() const
{
    return false;
}

QT_END_NAMESPACE
