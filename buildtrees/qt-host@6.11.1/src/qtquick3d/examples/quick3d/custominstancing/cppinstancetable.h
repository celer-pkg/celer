// Copyright (C) 2020 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR BSD-3-Clause

#include <QtQuick3D/qquick3dinstancing.h>
QT_FORWARD_DECLARE_CLASS(QRandomGenerator)
//! [properties]
class CppInstanceTable : public QQuick3DInstancing
{
    Q_OBJECT
    QML_ELEMENT

    Q_PROPERTY(int gridSize READ gridSize WRITE setGridSize NOTIFY gridSizeChanged)
    Q_PROPERTY(float gridSpacing READ gridSpacing WRITE setGridSpacing NOTIFY gridSpacingChanged)
    Q_PROPERTY(int randomSeed READ randomSeed WRITE setRandomSeed NOTIFY randomSeedChanged)
//! [properties]
public:
    explicit CppInstanceTable(QQuick3DObject *parent = nullptr);
    ~CppInstanceTable() override;

    int gridSize() const;
    float gridSpacing() const;
    int randomSeed() const;

public slots:
    void setGridSize(int gridSize);
    void setGridSpacing(float gridSpacing);
    void setRandomSeed(int randomSeed);

signals:
    void gridSizeChanged();
    void gridSpacingChanged();
    void randomSeedChanged();

protected:
    QByteArray getInstanceBuffer(int *instanceCount) override;

private:
    int m_instanceCount = 0;
    QByteArray m_instanceData;
    bool m_dirty = true;
    int m_gridSize = 0;
    float m_gridSpacing = 100;
    int m_randomSeed;
};
