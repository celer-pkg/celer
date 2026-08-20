// Copyright (C) 2022 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR GPL-3.0-only
// Qt-Security score:significant reason:default


#ifndef QQUICK3DPARTICLEDYNAMICBURST_H
#define QQUICK3DPARTICLEDYNAMICBURST_H

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

#include <QtQuick3DParticles/private/qquick3dparticleemitburst_p.h>

QT_BEGIN_NAMESPACE

class Q_QUICK3DPARTICLES_EXPORT QQuick3DParticleDynamicBurst : public QQuick3DParticleEmitBurst
{
    Q_OBJECT
    Q_PROPERTY(bool enabled READ enabled WRITE setEnabled NOTIFY enabledChanged)
    Q_PROPERTY(int amountVariation READ amountVariation WRITE setAmountVariation NOTIFY amountVariationChanged)
    Q_PROPERTY(TriggerMode triggerMode READ triggerMode WRITE setTriggerMode NOTIFY triggerModeChanged)
    QML_NAMED_ELEMENT(DynamicBurst3D)
    QML_ADDED_IN_VERSION(6, 3)

public:
    enum TriggerMode
    {
        TriggerTime = 0,
        TriggerStart,
        TriggerEnd
    };
    Q_ENUM(TriggerMode)

    explicit QQuick3DParticleDynamicBurst(QObject *parent = nullptr);
    bool enabled() const;
    int amountVariation() const;
    TriggerMode triggerMode() const;

public Q_SLOTS:
    void setEnabled(bool enabled);
    void setAmountVariation(int value);
    void setTriggerMode(TriggerMode mode);

Q_SIGNALS:
    void enabledChanged();
    void amountVariationChanged();
    void triggerModeChanged();

private:
    friend class QQuick3DParticleEmitter;

    bool m_enabled = true;
    int m_amountVariation = 0;
    TriggerMode m_triggerMode = TriggerMode::TriggerTime;
};

QT_END_NAMESPACE

#endif // QQUICK3DPARTICLEDYNAMICBURST_H
