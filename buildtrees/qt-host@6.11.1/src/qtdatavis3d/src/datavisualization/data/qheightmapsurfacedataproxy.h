// Copyright (C) 2016 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR GPL-3.0-only

#ifndef QTDATAVIS3D_QHEIGHTMAPSURFACEDATAPROXY_H
#define QTDATAVIS3D_QHEIGHTMAPSURFACEDATAPROXY_H

#include <QtDataVisualization/qsurfacedataproxy.h>
#include <QtGui/QImage>
#include <QtCore/QString>

QT_BEGIN_NAMESPACE

class QHeightMapSurfaceDataProxyPrivate;

class Q_DATAVISUALIZATION_EXPORT QHeightMapSurfaceDataProxy : public QSurfaceDataProxy
{
    Q_OBJECT

    Q_PROPERTY(QImage heightMap READ heightMap WRITE setHeightMap NOTIFY heightMapChanged)
    Q_PROPERTY(QString heightMapFile READ heightMapFile WRITE setHeightMapFile NOTIFY heightMapFileChanged)
    Q_PROPERTY(float minXValue READ minXValue WRITE setMinXValue NOTIFY minXValueChanged)
    Q_PROPERTY(float maxXValue READ maxXValue WRITE setMaxXValue NOTIFY maxXValueChanged)
    Q_PROPERTY(float minZValue READ minZValue WRITE setMinZValue NOTIFY minZValueChanged)
    Q_PROPERTY(float maxZValue READ maxZValue WRITE setMaxZValue NOTIFY maxZValueChanged)
    Q_PROPERTY(float minYValue READ minYValue WRITE setMinYValue NOTIFY minYValueChanged REVISION(6, 3))
    Q_PROPERTY(float maxYValue READ maxYValue WRITE setMaxYValue NOTIFY maxYValueChanged REVISION(6, 3))
    Q_PROPERTY(bool autoScaleY READ autoScaleY WRITE setAutoScaleY NOTIFY autoScaleYChanged REVISION(6, 3))

public:
    explicit QHeightMapSurfaceDataProxy(QObject *parent = nullptr);
    explicit QHeightMapSurfaceDataProxy(const QImage &image, QObject *parent = nullptr);
    explicit QHeightMapSurfaceDataProxy(const QString &filename, QObject *parent = nullptr);
    virtual ~QHeightMapSurfaceDataProxy();

    void setHeightMap(const QImage &image);
    QImage heightMap() const;
    void setHeightMapFile(const QString &filename);
    QString heightMapFile() const;

    void setValueRanges(float minX, float maxX, float minZ, float maxZ);
    void setMinXValue(float min);
    float minXValue() const;
    void setMaxXValue(float max);
    float maxXValue() const;
    void setMinZValue(float min);
    float minZValue() const;
    void setMaxZValue(float max);
    float maxZValue() const;
    void setMinYValue(float min);
    float minYValue() const;
    void setMaxYValue(float max);
    float maxYValue() const;
    void setAutoScaleY(bool enabled);
    bool autoScaleY() const;

Q_SIGNALS:
    void heightMapChanged(const QImage &image);
    void heightMapFileChanged(const QString &filename);
    void minXValueChanged(float value);
    void maxXValueChanged(float value);
    void minZValueChanged(float value);
    void maxZValueChanged(float value);
    Q_REVISION(6, 3) void minYValueChanged(float value);
    Q_REVISION(6, 3) void maxYValueChanged(float value);
    Q_REVISION(6, 3) void autoScaleYChanged(bool enabled);

protected:
    explicit QHeightMapSurfaceDataProxy(QHeightMapSurfaceDataProxyPrivate *d, QObject *parent = nullptr);
    QHeightMapSurfaceDataProxyPrivate *dptr();
    const QHeightMapSurfaceDataProxyPrivate *dptrc() const;

private:
    Q_DISABLE_COPY(QHeightMapSurfaceDataProxy)

    friend class Surface3DController;
};

QT_END_NAMESPACE

#endif
