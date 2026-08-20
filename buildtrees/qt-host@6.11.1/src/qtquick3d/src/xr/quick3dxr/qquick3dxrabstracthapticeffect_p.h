// Copyright (C) 2024 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR GPL-3.0-only
// Qt-Security score:significant reason:default


#ifndef QQUICK3DXRABSTRACTHAPTICACTION_H
#define QQUICK3DXRABSTRACTHAPTICACTION_H

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

QT_BEGIN_NAMESPACE
class QQuick3DXrSimpleHapticEffect;

class QQuick3DXrAbstractHapticEffect : public QObject
{
    Q_OBJECT
    QML_NAMED_ELEMENT(XrHapticEffect)
    QML_UNCREATABLE("XrHapticEffect is an abstract base class.")
    QML_ADDED_IN_VERSION(6, 9)
};

class QQuick3DXrSimpleHapticEffect : public QQuick3DXrAbstractHapticEffect
{
    Q_OBJECT
    QML_NAMED_ELEMENT(XrSimpleHapticEffect)
    QML_ADDED_IN_VERSION(6, 9)

    Q_PROPERTY(float amplitude READ amplitude WRITE setAmplitude NOTIFY amplitudeChanged FINAL)
    Q_PROPERTY(float duration READ duration WRITE setDuration NOTIFY durationChanged FINAL)
    Q_PROPERTY(float frequency READ frequency WRITE setFrequency NOTIFY frequencyChanged FINAL)

public:
    float amplitude();
    void setAmplitude(float newAmplitude);
    float duration();
    void setDuration(float newDuration);
    float frequency();
    void setFrequency(float newFrequency);

signals:
    void amplitudeChanged();
    void durationChanged();
    void frequencyChanged();

private:
    float m_amplitude = 0.5;
    float m_duration = 30;
    float m_frequency = 3000;
};

QT_END_NAMESPACE

#endif
