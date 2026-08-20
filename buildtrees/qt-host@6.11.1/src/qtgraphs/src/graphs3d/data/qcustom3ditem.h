// Copyright (C) 2023 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR GPL-3.0-only
// Qt-Security score:significant reason:default


#ifndef QTGRAPHS_QCUSTOM3DITEM_H
#define QTGRAPHS_QCUSTOM3DITEM_H

#include <QtCore/qobject.h>
#include <QtGraphs/qgraphsglobal.h>
#include <QtGui/qimage.h>
#include <QtGui/qquaternion.h>
#include <QtGui/qvector3d.h>
#include <QtQmlIntegration/qqmlintegration.h>

QT_BEGIN_NAMESPACE

class QCustom3DItemPrivate;

class Q_GRAPHS_EXPORT QCustom3DItem : public QObject
{
    Q_OBJECT
    Q_DECLARE_PRIVATE(QCustom3DItem)
    Q_PROPERTY(QString meshFile READ meshFile WRITE setMeshFile NOTIFY meshFileChanged FINAL)
    Q_PROPERTY(
        QString textureFile READ textureFile WRITE setTextureFile NOTIFY textureFileChanged FINAL)
    Q_PROPERTY(QVector3D position READ position WRITE setPosition NOTIFY positionChanged FINAL)
    Q_PROPERTY(bool positionAbsolute READ isPositionAbsolute WRITE setPositionAbsolute NOTIFY
                   positionAbsoluteChanged FINAL)
    Q_PROPERTY(QVector3D scaling READ scaling WRITE setScaling NOTIFY scalingChanged FINAL)
    Q_PROPERTY(QQuaternion rotation READ rotation WRITE setRotation NOTIFY rotationChanged FINAL)
    Q_PROPERTY(bool visible READ isVisible WRITE setVisible NOTIFY visibleChanged FINAL)
    Q_PROPERTY(bool shadowCasting READ isShadowCasting WRITE setShadowCasting NOTIFY
                   shadowCastingChanged FINAL)
    Q_PROPERTY(bool scalingAbsolute READ isScalingAbsolute WRITE setScalingAbsolute NOTIFY
                   scalingAbsoluteChanged FINAL)
    Q_PROPERTY(bool rotationAbsolute READ isRotationAbsolute WRITE setRotationAbsolute NOTIFY
                   rotationAbsoluteChanged REVISION(6, 11))
    QML_NAMED_ELEMENT(Custom3DItem)

public:
    explicit QCustom3DItem(QObject *parent = nullptr);
    explicit QCustom3DItem(const QString &meshFile,
                           QVector3D position,
                           QVector3D scaling,
                           const QQuaternion &rotation,
                           const QImage &texture,
                           QObject *parent = nullptr);
    ~QCustom3DItem() override;

    void setMeshFile(const QString &meshFile);
    QString meshFile() const;

    void setTextureFile(const QString &textureFile);
    QString textureFile() const;

    void setPosition(QVector3D position);
    QVector3D position() const;

    void setPositionAbsolute(bool positionAbsolute);
    bool isPositionAbsolute() const;

    void setScaling(QVector3D scaling);
    QVector3D scaling() const;

    void setScalingAbsolute(bool scalingAbsolute);
    bool isScalingAbsolute() const;

    void setRotation(const QQuaternion &rotation);
    QQuaternion rotation();

    void setRotationAbsolute(bool rotationAbsolute);
    bool isRotationAbsolute() const;

    void setVisible(bool visible);
    bool isVisible() const;

    void setShadowCasting(bool enabled);
    bool isShadowCasting() const;

    Q_INVOKABLE void setRotationAxisAndAngle(QVector3D axis, float angle);

    void setTextureImage(const QImage &textureImage);

Q_SIGNALS:
    void meshFileChanged(const QString &meshFile);
    void textureFileChanged(const QString &textureFile);
    void positionChanged(QVector3D position);
    void positionAbsoluteChanged(bool positionAbsolute);
    void scalingChanged(QVector3D scaling);
    void rotationChanged(const QQuaternion &rotation);
    Q_REVISION(6, 11) void rotationAbsoluteChanged(bool newRotationAbsolute);
    void visibleChanged(bool visible);
    void shadowCastingChanged(bool shadowCasting);
    void scalingAbsoluteChanged(bool scalingAbsolute);
    void needUpdate();

protected:
    QCustom3DItem(QCustom3DItemPrivate &d, QObject *parent = nullptr);

private:
    Q_DISABLE_COPY(QCustom3DItem)

    friend class QQuickGraphsItem;
};

QT_END_NAMESPACE

#endif
