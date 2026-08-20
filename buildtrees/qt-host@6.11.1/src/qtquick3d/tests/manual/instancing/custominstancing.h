// Copyright (C) 2020 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR GPL-3.0-only

#ifndef CUSTOMINSTANCING_H
#define CUSTOMINSTANCING_H

#include <QtQuick3D/qquick3dinstancing.h>

class CustomInstancing : public QQuick3DInstancing
{
    Q_OBJECT
    QML_ELEMENT

    Q_PROPERTY(int instanceCount READ instanceCount WRITE setInstanceCount NOTIFY instanceCountChanged)

public:
    explicit CustomInstancing(QQuick3DObject *parent = nullptr) : QQuick3DInstancing(parent) {}
    ~CustomInstancing() override {}

    QByteArray getInstanceBuffer(int *instanceCount) override;

    int instanceCount() const
    {
        return m_instanceCount;
    }

public slots:
    void setInstanceCount(int instanceCount);

signals:
    void instanceCountChanged(int instanceCount);

private:
    int m_instanceCount = 0;
};

#endif // CUSTOMINSTANCING_H
