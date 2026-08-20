// Copyright (C) 2023 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR GPL-3.0-only
// Qt-Security score:significant reason:default


#ifndef QTGRAPHS_QSURFACE3DSERIES_H
#define QTGRAPHS_QSURFACE3DSERIES_H

#include <QtGraphs/qabstract3dseries.h>
#include <QtGraphs/qsurfacedataproxy.h>
#include <QtGraphs/qvalue3daxis.h>

QT_BEGIN_NAMESPACE

class QSurface3DSeriesPrivate;

class Q_GRAPHS_EXPORT QSurface3DSeries : public QAbstract3DSeries
{
    Q_OBJECT
    Q_DECLARE_PRIVATE(QSurface3DSeries)
    Q_PROPERTY(QSurfaceDataProxy *dataProxy READ dataProxy WRITE setDataProxy NOTIFY
                   dataProxyChanged FINAL)
    Q_PROPERTY(
        QPoint selectedPoint READ selectedPoint WRITE setSelectedPoint NOTIFY selectedPointChanged)
    Q_PROPERTY(bool flatShadingSupported READ isFlatShadingSupported NOTIFY
                   flatShadingSupportedChanged FINAL)
    Q_PROPERTY(QSurface3DSeries::DrawFlags drawMode READ drawMode WRITE setDrawMode NOTIFY
                   drawModeChanged FINAL)
    Q_PROPERTY(QSurface3DSeries::Shading shading READ shading WRITE setShading NOTIFY shadingChanged)
    Q_PROPERTY(QImage texture READ texture WRITE setTexture NOTIFY textureChanged FINAL)
    Q_PROPERTY(
        QString textureFile READ textureFile WRITE setTextureFile NOTIFY textureFileChanged FINAL)
    Q_PROPERTY(QColor wireframeColor READ wireframeColor WRITE setWireframeColor NOTIFY
                   wireframeColorChanged FINAL)
    Q_PROPERTY(
        QSurfaceDataArray dataArray READ dataArray WRITE setDataArray NOTIFY dataArrayChanged FINAL)
    Q_PROPERTY(bool rowsSanitized READ rowsSanitized WRITE setRowsSanitized NOTIFY rowsSanitizedChanged REVISION(6, 11))
    Q_PROPERTY(QValue3DAxis *axisX READ axisX WRITE setAxisX RESET resetAxisX NOTIFY axisXChanged
                       REVISION(6, 11))
    Q_PROPERTY(QValue3DAxis *axisY READ axisY WRITE setAxisY RESET resetAxisY NOTIFY axisYChanged
                       REVISION(6, 11))
    Q_PROPERTY(QValue3DAxis *axisZ READ axisZ WRITE setAxisZ RESET resetAxisZ NOTIFY axisZChanged
                       REVISION(6, 11))
    QML_ELEMENT
    QML_UNCREATABLE("Trying to create uncreatable: QSurface3DSeries, use Surface3DSeries instead.")

public:
    enum DrawFlag {
        DrawWireframe = 0x1,
        DrawSurface = 0x2,
        DrawSurfaceAndWireframe = DrawWireframe | DrawSurface,
        DrawFilledSurface = 0x4,
    };
    Q_FLAG(DrawFlag)
    Q_DECLARE_FLAGS(DrawFlags, DrawFlag)

    enum class Shading { Smooth, Flat };
    Q_ENUM(Shading)

    explicit QSurface3DSeries(QObject *parent = nullptr);
    explicit QSurface3DSeries(QSurfaceDataProxy *dataProxy, QObject *parent = nullptr);
    ~QSurface3DSeries() override;

    void setDataProxy(QSurfaceDataProxy *proxy);
    QSurfaceDataProxy *dataProxy() const;

    void setSelectedPoint(QPoint position);
    QPoint selectedPoint() const;
    static QPoint invalidSelectionPosition();

    void setShading(const QSurface3DSeries::Shading shading);
    QSurface3DSeries::Shading shading() const;

    void setDrawMode(QSurface3DSeries::DrawFlags mode);
    QSurface3DSeries::DrawFlags drawMode() const;

    bool isFlatShadingSupported() const;

    void setTexture(const QImage &texture);
    QImage texture() const;
    void setTextureFile(const QString &filename);
    QString textureFile() const;

    void setWireframeColor(QColor color);
    QColor wireframeColor() const;

    void setRowsSanitized(bool sanitized);
    bool rowsSanitized() const;

    void setDataArray(const QSurfaceDataArray &newDataArray);
    void clearRow(qsizetype rowIndex);
    void clearArray();
    const QSurfaceDataArray &dataArray() const &;
    QSurfaceDataArray dataArray() &&;

    void setAxisX(QValue3DAxis *axis);
    void setAxisY(QValue3DAxis *axis);
    void setAxisZ(QValue3DAxis *axis);
    QValue3DAxis *axisX() const;
    QValue3DAxis *axisY() const;
    QValue3DAxis *axisZ() const;
    void resetAxisX();
    void resetAxisY();
    void resetAxisZ();

Q_SIGNALS:
    void dataProxyChanged(QSurfaceDataProxy *proxy);
    void selectedPointChanged(QPoint position);
    void flatShadingSupportedChanged(bool enabled);
    void drawModeChanged(QSurface3DSeries::DrawFlags mode);
    void textureChanged(const QImage &image);
    void textureFileChanged(const QString &filename);
    void wireframeColorChanged(QColor color);
    void dataArrayChanged(const QSurfaceDataArray &array);
    void shadingChanged(const Shading shading);
    Q_REVISION(6, 11) void rowsSanitizedChanged(bool newRowsSanitized);
    Q_REVISION(6, 11) void axisXChanged(QValue3DAxis *axis);
    Q_REVISION(6, 11) void axisYChanged(QValue3DAxis *axis);
    Q_REVISION(6, 11) void axisZChanged(QValue3DAxis *axis);

protected:
    explicit QSurface3DSeries(QSurface3DSeriesPrivate &d, QObject *parent = nullptr);

private:
    Q_DISABLE_COPY(QSurface3DSeries)

    friend class QQuickGraphsSurface;
};
Q_DECLARE_OPERATORS_FOR_FLAGS(QSurface3DSeries::DrawFlags)

QT_END_NAMESPACE

#endif
