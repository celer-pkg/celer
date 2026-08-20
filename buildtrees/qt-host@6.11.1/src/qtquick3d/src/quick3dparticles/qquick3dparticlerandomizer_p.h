// Copyright (C) 2021 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR GPL-3.0-only
// Qt-Security score:significant reason:default


#ifndef QQUICK3DPARTICLERANDOMIZER_H
#define QQUICK3DPARTICLERANDOMIZER_H

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

#include <QRandomGenerator>
#include <QList>
#include <private/qglobal_p.h>

QT_BEGIN_NAMESPACE

/*

Simple helper to get pseudo-random numbers which remain the same per particle & user.
Particles don't need strong randomization and the ability to seed can be useful.

Based on brief testing on my laptop, getting 1.000.000 random numbers:

1) Using QRandomGenerator::global()->generateDouble()
-> ~120ms

2) Using QPRand::get(), with m_size 4096
-> ~8ms

3) Using QPRand::get(i, j), with m_size 4096
-> ~10ms

4) Using QPRand::get(i, j), with m_size 100000
-> ~10ms

So QPRand seems fast and increasing keys amount doesn't notably affect the performance,
just the memory usage. With more particles m_size should be relatively big to make sure
particles appear random enough.

Bounded usage tips:
- qrg->bounded(4.0) == QPRand::get() * 4.0
- qrg->bounded(4) == int(QPRand::get() * 4)

*/

class QPRand
{
public:

    // Add here new enums for diferent "users".
    // This way e.g. particle can vary little on colors but more on sizes.
    enum UserType {
        Default = 0,
        WanderXPS, // PaceStart
        WanderYPS,
        WanderZPS,
        WanderXPV, // PaceVariation
        WanderYPV,
        WanderZPV,
        WanderXAV, // AmountVariation
        WanderYAV,
        WanderZAV,
        AttractorDurationV,
        AttractorPosVX,
        AttractorPosVY,
        AttractorPosVZ,
        Shape1, // Shape
        Shape2,
        Shape3,
        Shape4,
        SpriteAnimationI,
        DeterministicSeparator, // Note: Enums before this must always be
                                // deterministic based on the particle index
        LifeSpanV, // Emitter
        ScaleV,
        ScaleEV,
        RotXV,
        RotYV,
        RotZV,
        RotXVV,
        RotYVV,
        RotZVV,
        ColorRV,
        ColorGV,
        ColorBV,
        ColorAV,
        TDirPosXV, // TargetDirection
        TDirPosYV,
        TDirPosZV,
        TDirMagV,
        VDirXV, // VectorDirection
        VDirYV,
        VDirZV,
        SpriteAnimationV
    };

    void init(quint32 seed, int size = 65536) {
        m_size = size;
        m_generator.seed(seed);
        m_randomList.clear();
        m_randomList.reserve(m_size);
        for (int i = 0; i < m_size; i++)
            m_randomList << float(m_generator.generateDouble());
    }

    void setDeterministic(bool deterministic) {
        m_deterministic = deterministic;
    }

    // Return float 0.0 - 1.0
    // With the same input values, returns always the same output.
    inline float get(int particleIndex, UserType user = Default) {
        if (!m_deterministic && user > DeterministicSeparator)
            return get();
        int i = (particleIndex + user) % m_size;
        return m_randomList.at(i);
    }

    // Return float 0.0 - 1.0 from random list
    // Note: Not deterministic between runs
    inline float get() {
        m_index = (m_index < m_size - 1) ? m_index + 1 : 0;
        return m_randomList.at(m_index);
    }
    QRandomGenerator generator() const
    {
        return m_generator;
    }
private:
    QRandomGenerator m_generator;
    int m_size = 0;
    int m_index = 0;
    bool m_deterministic = false;
    QList<float> m_randomList;

};

QT_END_NAMESPACE

#endif // QQUICK3DPARTICLERANDOMIZER_H
