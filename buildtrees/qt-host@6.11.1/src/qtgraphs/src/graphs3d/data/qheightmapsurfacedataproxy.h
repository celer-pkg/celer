// Copyright (C) 2023 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR GPL-3.0-only
// Qt-Security score:significant reason:default


#ifndef QTGRAPHS_QHEIGHTMAPSURFACEDATAPROXY_H
#define QTGRAPHS_QHEIGHTMAPSURFACEDATAPROXY_H

#include <QtCore/qstring.h>
#include <QtGraphs/qsurfacedataproxy.h>
#include <QtGui/qimage.h>

QT_BEGIN_NAMESPACE

class QHeightMapSurfaceDataProxyPrivate;

class Q_GRAPHS_EXPORT QHeightMapSurfaceDataProxy : public QSurfaceDataProxy
{
    Q_OBJECT
    Q_DECLARE_PRIVATE(QHeightMapSurfaceDataProxy)
    Q_PROPERTY(QImage heightMap READ heightMap WRITE setHeightMap NOTIFY heightMapChanged FINAL)
    Q_PROPERTY(QString heightMapFile READ heightMapFile WRITE setHeightMapFile NOTIFY
                   heightMapFileChanged FINAL)
    Q_PROPERTY(float minXValue READ minXValue WRITE setMinXValue NOTIFY minXValueChanged FINAL)
    Q_PROPERTY(float maxXValue READ maxXValue WRITE setMaxXValue NOTIFY maxXValueChanged FINAL)
    Q_PROPERTY(float minZValue READ minZValue WRITE setMinZValue NOTIFY minZValueChanged FINAL)
    Q_PROPERTY(float maxZValue READ maxZValue WRITE setMaxZValue NOTIFY maxZValueChanged FINAL)
    Q_PROPERTY(float minYValue READ minYValue WRITE setMinYValue NOTIFY minYValueChanged FINAL)
    Q_PROPERTY(float maxYValue READ maxYValue WRITE setMaxYValue NOTIFY maxYValueChanged FINAL)
    Q_PROPERTY(bool autoScaleY READ autoScaleY WRITE setAutoScaleY NOTIFY autoScaleYChanged FINAL)
    QML_NAMED_ELEMENT(HeightMapSurfaceDataProxy)

public:
    explicit QHeightMapSurfaceDataProxy(QObject *parent = nullptr);
    explicit QHeightMapSurfaceDataProxy(const QImage &image, QObject *parent = nullptr);
    explicit QHeightMapSurfaceDataProxy(const QString &filename, QObject *parent = nullptr);
    ~QHeightMapSurfaceDataProxy() override;

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
    void minYValueChanged(float value);
    void maxYValueChanged(float value);
    void autoScaleYChanged(bool enabled);

protected:
    explicit QHeightMapSurfaceDataProxy(QHeightMapSurfaceDataProxyPrivate &d,
                                        QObject *parent = nullptr);
    void handlePendingResolve();

private:
    Q_DISABLE_COPY(QHeightMapSurfaceDataProxy)
};

QT_END_NAMESPACE

#endif
