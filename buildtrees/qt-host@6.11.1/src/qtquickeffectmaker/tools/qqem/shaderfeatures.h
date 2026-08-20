// Copyright (C) 2022 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR GPL-3.0-only

#ifndef SHADERFEATURES_H
#define SHADERFEATURES_H

#include <QFlags>
#include <QString>

class ShaderFeatures
{
public:
    enum Feature {
        Time        = 1 << 0,
        Frame       = 1 << 1,
        Resolution  = 1 << 2,
        Source      = 1 << 3,
        Mouse       = 1 << 4,
        FragCoord   = 1 << 5,
        GridMesh    = 1 << 6,
        BlurSources = 1 << 7
    };
    Q_DECLARE_FLAGS(Features, Feature)

    ShaderFeatures();
    void update(const QString &vs, const QString &fs, const QString &qml);

    bool enabled(ShaderFeatures::Feature feature) const {
        return m_enabledFeatures.testFlag(feature);
    }
private:
    friend class EffectManager;
    void checkLine(const QString &line, ShaderFeatures::Features &features);
    ShaderFeatures::Features m_enabledFeatures;
    int m_gridMeshWidth = 1;
    int m_gridMeshHeight = 1;
};

Q_DECLARE_OPERATORS_FOR_FLAGS(ShaderFeatures::Features)

#endif // SHADERFEATURES_H
