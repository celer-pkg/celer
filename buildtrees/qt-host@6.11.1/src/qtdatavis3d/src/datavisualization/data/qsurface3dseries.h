// Copyright (C) 2016 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR GPL-3.0-only

#ifndef QTDATAVIS3D_QSURFACE3DSERIES_H
#define QTDATAVIS3D_QSURFACE3DSERIES_H

#include <QtDataVisualization/qabstract3dseries.h>
#include <QtDataVisualization/qsurfacedataproxy.h>

QT_BEGIN_NAMESPACE

class QSurface3DSeriesPrivate;

class Q_DATAVISUALIZATION_EXPORT QSurface3DSeries : public QAbstract3DSeries
{
    Q_OBJECT
    Q_PROPERTY(QSurfaceDataProxy *dataProxy READ dataProxy WRITE setDataProxy NOTIFY dataProxyChanged)
    Q_PROPERTY(QPoint selectedPoint READ selectedPoint WRITE setSelectedPoint NOTIFY selectedPointChanged)
    Q_PROPERTY(bool flatShadingEnabled READ isFlatShadingEnabled WRITE setFlatShadingEnabled NOTIFY flatShadingEnabledChanged)
    Q_PROPERTY(bool flatShadingSupported READ isFlatShadingSupported NOTIFY flatShadingSupportedChanged)
    Q_PROPERTY(DrawFlags drawMode READ drawMode WRITE setDrawMode NOTIFY drawModeChanged)
    Q_PROPERTY(QImage texture READ texture WRITE setTexture NOTIFY textureChanged)
    Q_PROPERTY(QString textureFile READ textureFile WRITE setTextureFile NOTIFY textureFileChanged)
    Q_PROPERTY(QColor wireframeColor READ wireframeColor WRITE setWireframeColor NOTIFY wireframeColorChanged REVISION(6, 3))

public:
    enum DrawFlag {
        DrawWireframe = 1,
        DrawSurface = 2,
        DrawSurfaceAndWireframe = DrawWireframe | DrawSurface
    };
    Q_ENUM(DrawFlag)
    Q_DECLARE_FLAGS(DrawFlags, DrawFlag)
    Q_FLAG(DrawFlags)

    explicit QSurface3DSeries(QObject *parent = nullptr);
    explicit QSurface3DSeries(QSurfaceDataProxy *dataProxy, QObject *parent = nullptr);
    virtual ~QSurface3DSeries();

    void setDataProxy(QSurfaceDataProxy *proxy);
    QSurfaceDataProxy *dataProxy() const;

    void setSelectedPoint(const QPoint &position);
    QPoint selectedPoint() const;
    static QPoint invalidSelectionPosition();

    void setFlatShadingEnabled(bool enabled);
    bool isFlatShadingEnabled() const;

    void setDrawMode(DrawFlags mode);
    QSurface3DSeries::DrawFlags drawMode() const;

    bool isFlatShadingSupported() const;

    void setTexture(const QImage &texture);
    QImage texture() const;
    void setTextureFile(const QString &filename);
    QString textureFile() const;

    void setWireframeColor(const QColor &color);
    QColor wireframeColor() const;

Q_SIGNALS:
    void dataProxyChanged(QSurfaceDataProxy *proxy);
    void selectedPointChanged(const QPoint &position);
    void flatShadingEnabledChanged(bool enable);
    void flatShadingSupportedChanged(bool enable);
    void drawModeChanged(QSurface3DSeries::DrawFlags mode);
    void textureChanged(const QImage &image);
    void textureFileChanged(const QString &filename);
    Q_REVISION(6, 3) void wireframeColorChanged(const QColor &color);

protected:
    explicit QSurface3DSeries(QSurface3DSeriesPrivate *d, QObject *parent = nullptr);
    QSurface3DSeriesPrivate *dptr();
    const QSurface3DSeriesPrivate *dptrc() const;

private:
    Q_DISABLE_COPY(QSurface3DSeries)

    friend class Surface3DController;
};

Q_DECLARE_OPERATORS_FOR_FLAGS(QSurface3DSeries::DrawFlags)

QT_END_NAMESPACE

#endif
