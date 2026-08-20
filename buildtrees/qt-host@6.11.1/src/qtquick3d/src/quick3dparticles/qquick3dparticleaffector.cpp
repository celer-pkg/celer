// Copyright (C) 2021 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR GPL-3.0-only
// Qt-Security score:significant reason:default


#include "qquick3dparticleaffector_p.h"
#include "qquick3dparticleutils_p.h"

QT_BEGIN_NAMESPACE

/*!
    \qmltype Affector3D
    \inherits Node
    \inqmlmodule QtQuick3D.Particles3D
    \brief Affectors modify the attributes of particles during their lifetime.
    \since 6.2

    The Affector3D is an abstract base class of affectors like \l Gravity3D, \l Wander3D, and \l PointRotator3D.

    By default affectors affect all particles in the system, but this can be limited by defining
    the \l particles list. If the system has multiple affectors, the order of affectors may
    result in different outcome, as affectors are applied one after another.
*/

QQuick3DParticleAffector::QQuick3DParticleAffector(QQuick3DNode *parent)
    : QQuick3DNode(parent)
{
}

QQuick3DParticleAffector::~QQuick3DParticleAffector()
{
    for (const auto &connection : std::as_const(m_connections))
        QObject::disconnect(connection);
    if (m_system)
        m_system->unRegisterParticleAffector(this);
}

/*!
    \qmlproperty ParticleSystem3D Affector3D::system

    This property defines the \l ParticleSystem3D for the affector. If system is direct parent of the affector,
    this property does not need to be defined.
*/
QQuick3DParticleSystem *QQuick3DParticleAffector::system() const
{
    return m_system;
}

void QQuick3DParticleAffector::setSystem(QQuick3DParticleSystem *system)
{
    if (m_system == system)
        return;

    if (m_system)
        m_system->unRegisterParticleAffector(this);

    m_system = system;
    if (m_system)
        m_system->registerParticleAffector(this);

    m_systemSharedParent = getSharedParentNode(this, m_system);

    Q_EMIT systemChanged();
    Q_EMIT update();
}

/*!
    \qmlproperty bool Affector3D::enabled

    If enabled is set to \c false, this affector will not alter any particles.
    Usually this is used to conditionally turn an affector on or off.

    The default value is \c true.
*/
bool QQuick3DParticleAffector::enabled() const
{
    return m_enabled;
}

void QQuick3DParticleAffector::setEnabled(bool enabled)
{
    if (m_enabled == enabled)
        return;

    m_enabled = enabled;
    Q_EMIT enabledChanged();
    Q_EMIT update();
}

void QQuick3DParticleAffector::componentComplete()
{
    if (!m_system && qobject_cast<QQuick3DParticleSystem*>(parent()))
        setSystem(qobject_cast<QQuick3DParticleSystem*>(parent()));
}

void QQuick3DParticleAffector::prepareToAffect()
{
}

// Particles

/*!
    \qmlproperty List<Particle3D> Affector3D::particles

    This list controls which logical particles will be affected.
    When empty, all particles in the system are affected.
*/
QQmlListProperty<QQuick3DParticle> QQuick3DParticleAffector::particles()
{
    return {this, this,
             &QQuick3DParticleAffector::appendParticle,
             &QQuick3DParticleAffector::particleCount,
             &QQuick3DParticleAffector::particle,
             &QQuick3DParticleAffector::clearParticles,
             &QQuick3DParticleAffector::replaceParticle,
             &QQuick3DParticleAffector::removeLastParticle};
}

void QQuick3DParticleAffector::appendParticle(QQuick3DParticle *n) {
    m_particles.append(n);
    m_connections.insert(n, QObject::connect(n, &QObject::destroyed, this, [this](QObject *obj) {
        QQuick3DParticle *particle = qobject_cast<QQuick3DParticle *>(obj);
        m_particles.removeAll(particle);
        QObject::disconnect(m_connections[particle]);
        m_connections.remove(particle);
    }));
}

qsizetype QQuick3DParticleAffector::particleCount() const
{
    return m_particles.size();
}

QQuick3DParticle *QQuick3DParticleAffector::particle(qsizetype index) const
{
    return m_particles.at(index);
}

void QQuick3DParticleAffector::clearParticles() {
    m_particles.clear();
}

void QQuick3DParticleAffector::replaceParticle(qsizetype index, QQuick3DParticle *n)
{
    QQuick3DParticle *remove = m_particles[index];
    QObject::disconnect(m_connections[remove]);
    m_connections.remove(remove);
    m_particles[index] = n;
    m_connections.insert(n, QObject::connect(n, &QObject::destroyed, this, [this](QObject *obj) {
        QQuick3DParticle *particle = qobject_cast<QQuick3DParticle *>(obj);
        m_particles.removeAll(particle);
        QObject::disconnect(m_connections[particle]);
        m_connections.remove(particle);
    }));
}

void QQuick3DParticleAffector::removeLastParticle()
{
    QQuick3DParticle *last = m_particles.last();
    QObject::disconnect(m_connections[last]);
    m_connections.remove(last);
    m_particles.removeLast();
}

// Particles - static
void QQuick3DParticleAffector::appendParticle(QQmlListProperty<QQuick3DParticle> *list, QQuick3DParticle *p) {
    reinterpret_cast<QQuick3DParticleAffector *>(list->data)->appendParticle(p);
}

void QQuick3DParticleAffector::clearParticles(QQmlListProperty<QQuick3DParticle> *list) {
    reinterpret_cast<QQuick3DParticleAffector *>(list->data)->clearParticles();
}

void QQuick3DParticleAffector::replaceParticle(QQmlListProperty<QQuick3DParticle> *list, qsizetype i, QQuick3DParticle *p)
{
    reinterpret_cast<QQuick3DParticleAffector *>(list->data)->replaceParticle(i, p);
}

void QQuick3DParticleAffector::removeLastParticle(QQmlListProperty<QQuick3DParticle> *list)
{
    reinterpret_cast<QQuick3DParticleAffector *>(list->data)->removeLastParticle();
}

QQuick3DParticle *QQuick3DParticleAffector::particle(QQmlListProperty<QQuick3DParticle> *list, qsizetype i) {
    return reinterpret_cast<QQuick3DParticleAffector *>(list->data)->particle(i);
}

qsizetype QQuick3DParticleAffector::particleCount(QQmlListProperty<QQuick3DParticle> *list) {
    return reinterpret_cast<QQuick3DParticleAffector *>(list->data)->particleCount();
}

QT_END_NAMESPACE
