// Copyright (C) 2020 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR BSD-3-Clause

#include <QtQuick3D/qquick3dinstancing.h>

class ImageInstanceTable : public QQuick3DInstancing
{
    Q_OBJECT
    QML_ELEMENT

    Q_PROPERTY(int gridSize READ gridSize WRITE setGridSize NOTIFY gridSizeChanged)
    Q_PROPERTY(float gridSpacing READ gridSpacing WRITE setGridSpacing NOTIFY gridSpacingChanged)
    Q_PROPERTY(QString image READ image WRITE setImage NOTIFY imageChanged)

public:
    explicit ImageInstanceTable(QQuick3DObject *parent = nullptr);
    ~ImageInstanceTable() override;

    int gridSize() const;
    float gridSpacing() const;
    QString image() const;

public slots:
    void setGridSize(int gridSize);
    void setGridSpacing(float gridSpacing);
    void setImage(QString image);

signals:
    void gridSizeChanged(int gridSize);
    void gridSpacingChanged(float gridSpacing);
    void imageChanged(QString image);

protected:
    QByteArray getInstanceBuffer(int *instanceCount) override;

private:
    int m_instanceCount = 0;
    QByteArray m_instanceData;
    bool m_dirty = true;
    int m_gridSize = 0;
    float m_gridSpacing = 100;
    QString m_imageSource;
};
