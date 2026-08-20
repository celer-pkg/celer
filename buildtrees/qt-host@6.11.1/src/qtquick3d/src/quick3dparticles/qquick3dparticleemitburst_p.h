// Copyright (C) 2021 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR GPL-3.0-only
// Qt-Security score:significant reason:default


#ifndef QQUICK3DPARTICLEEMITBURST_H
#define QQUICK3DPARTICLEEMITBURST_H

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
#include <QQmlEngine>
#include <QtQml/qqmlparserstatus.h>
#include <QtQuick3DParticles/qtquick3dparticlesglobal.h>
#include <private/qglobal_p.h>

QT_BEGIN_NAMESPACE

class QQuick3DParticleEmitter;

class Q_QUICK3DPARTICLES_EXPORT QQuick3DParticleEmitBurst : public QObject, public QQmlParserStatus
{
    Q_OBJECT
    Q_PROPERTY(int time READ time WRITE setTime NOTIFY timeChanged)
    Q_PROPERTY(int amount READ amount WRITE setAmount NOTIFY amountChanged)
    Q_PROPERTY(int duration READ duration WRITE setDuration NOTIFY durationChanged)

    QML_NAMED_ELEMENT(EmitBurst3D)
    Q_INTERFACES(QQmlParserStatus)
    QML_ADDED_IN_VERSION(6, 2)

public:
    QQuick3DParticleEmitBurst(QObject *parent = nullptr);
    ~QQuick3DParticleEmitBurst() override;

    int time() const;
    int amount() const;
    int duration() const;

public Q_SLOTS:
    void setTime(int time);
    void setAmount(int amount);
    void setDuration(int duration);

Q_SIGNALS:
    void timeChanged();
    void amountChanged();
    void durationChanged();

protected:
    // From QQmlParserStatus
    void componentComplete() override;
    void classBegin() override {}

private:
    friend class QQuick3DParticleEmitter;

    QQuick3DParticleEmitter *m_parentEmitter = nullptr;
    int m_time = 0;
    int m_amount = 0;
    int m_duration = 0;
};

QT_END_NAMESPACE

#endif // QQUICK3DPARTICLEEMITBURST_H
