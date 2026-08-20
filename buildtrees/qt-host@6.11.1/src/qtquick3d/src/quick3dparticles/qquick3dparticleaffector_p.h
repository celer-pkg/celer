// Copyright (C) 2021 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR GPL-3.0-only
// Qt-Security score:significant reason:default


#ifndef QQUICK3DPARTICLEAFFECTOR_H
#define QQUICK3DPARTICLEAFFECTOR_H

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

#include <QObject>
#include <QList>
#include <QQmlListProperty>
#include <QtQuick3D/private/qquick3dnode_p.h>
#include <QtQuick3DParticles/private/qquick3dparticledata_p.h>
#include <QtQuick3DParticles/private/qquick3dparticlesystem_p.h>
#include <QtQuick3DParticles/private/qquick3dparticleemitter_p.h>

QT_BEGIN_NAMESPACE

class Q_QUICK3DPARTICLES_EXPORT QQuick3DParticleAffector : public QQuick3DNode
{
    Q_OBJECT
    Q_PROPERTY(QQuick3DParticleSystem *system READ system WRITE setSystem NOTIFY systemChanged)
    Q_PROPERTY(QQmlListProperty<QQuick3DParticle> particles READ particles)
    Q_PROPERTY(bool enabled READ enabled WRITE setEnabled NOTIFY enabledChanged)
    QML_NAMED_ELEMENT(Affector3D)
    QML_UNCREATABLE("Affector3D is abstract")
    QML_ADDED_IN_VERSION(6, 2)

public:
    QQuick3DParticleAffector(QQuick3DNode *parent = nullptr);
    ~QQuick3DParticleAffector() override;

    QQuick3DParticleSystem *system() const;
    bool enabled() const;

    // Particles list handling
    QQmlListProperty<QQuick3DParticle> particles();
    void appendParticle(QQuick3DParticle *);
    qsizetype particleCount() const;
    QQuick3DParticle *particle(qsizetype) const;
    void clearParticles();
    void replaceParticle(qsizetype, QQuick3DParticle *);
    void removeLastParticle();

public Q_SLOTS:
    void setSystem(QQuick3DParticleSystem *system);
    void setEnabled(bool enabled);

Q_SIGNALS:
    void update();
    void systemChanged();
    void enabledChanged();

protected:
    QList<QQuick3DParticle *> m_particles;
    QQuick3DNode *m_systemSharedParent = nullptr;

private:
    friend class QQuick3DParticleSystem;
    // From QQmlParserStatus
    void componentComplete() override;

    // Called once per frame for all the enabled affectors.
    virtual void prepareToAffect();
    // Called for each living particle attached to the attractor.
    virtual void affectParticle(const QQuick3DParticleData &sd, QQuick3DParticleDataCurrent *d, float time) = 0;

    static void appendParticle(QQmlListProperty<QQuick3DParticle> *, QQuick3DParticle *);
    static qsizetype particleCount(QQmlListProperty<QQuick3DParticle> *);
    static QQuick3DParticle *particle(QQmlListProperty<QQuick3DParticle> *, qsizetype);
    static void clearParticles(QQmlListProperty<QQuick3DParticle> *);
    static void replaceParticle(QQmlListProperty<QQuick3DParticle> *, qsizetype, QQuick3DParticle *);
    static void removeLastParticle(QQmlListProperty<QQuick3DParticle> *);

    QQuick3DParticleSystem *m_system = nullptr;
    bool m_enabled = true;
    QMap<QQuick3DParticle *, QMetaObject::Connection> m_connections;
};

QT_END_NAMESPACE

#endif // QQUICK3DPARTICLEAFFECTOR_H
