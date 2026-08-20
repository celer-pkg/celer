// Copyright (C) 2016 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR GPL-3.0-only

#ifndef QTDATAVIS3D_QCUSTOMLABELITEM_H
#define QTDATAVIS3D_QCUSTOMLABELITEM_H

#include <QtDataVisualization/qdatavisualizationglobal.h>
#include <QtDataVisualization/QCustom3DItem>
#include <QtGui/QVector3D>
#include <QtGui/QQuaternion>
#include <QtGui/QFont>
#include <QtGui/QColor>

QT_BEGIN_NAMESPACE

class QCustom3DLabelPrivate;

class Q_DATAVISUALIZATION_EXPORT QCustom3DLabel : public QCustom3DItem
{
    Q_OBJECT
    Q_PROPERTY(QString text READ text WRITE setText NOTIFY textChanged)
    Q_PROPERTY(QFont font READ font WRITE setFont NOTIFY fontChanged)
    Q_PROPERTY(QColor textColor READ textColor WRITE setTextColor NOTIFY textColorChanged)
    Q_PROPERTY(QColor backgroundColor READ backgroundColor WRITE setBackgroundColor NOTIFY backgroundColorChanged)
    Q_PROPERTY(bool borderEnabled READ isBorderEnabled WRITE setBorderEnabled NOTIFY borderEnabledChanged)
    Q_PROPERTY(bool backgroundEnabled READ isBackgroundEnabled WRITE setBackgroundEnabled NOTIFY backgroundEnabledChanged)
    Q_PROPERTY(bool facingCamera READ isFacingCamera WRITE setFacingCamera NOTIFY facingCameraChanged)

public:
    explicit QCustom3DLabel(QObject *parent = nullptr);
    explicit QCustom3DLabel(const QString &text, const QFont &font, const QVector3D &position,
                            const QVector3D &scaling, const QQuaternion &rotation,
                            QObject *parent = nullptr);
    virtual ~QCustom3DLabel();

    void setText(const QString &text);
    QString text() const;

    void setFont(const QFont &font);
    QFont font() const;

    void setTextColor(const QColor &color);
    QColor textColor() const;

    void setBackgroundColor(const QColor &color);
    QColor backgroundColor() const;

    void setBorderEnabled(bool enabled);
    bool isBorderEnabled() const;

    void setBackgroundEnabled(bool enabled);
    bool isBackgroundEnabled() const;

    void setFacingCamera(bool enabled);
    bool isFacingCamera() const;

Q_SIGNALS:
    void textChanged(const QString &text);
    void fontChanged(const QFont &font);
    void textColorChanged(const QColor &color);
    void backgroundColorChanged(const QColor &color);
    void borderEnabledChanged(bool enabled);
    void backgroundEnabledChanged(bool enabled);
    void facingCameraChanged(bool enabled);

protected:
    QCustom3DLabelPrivate *dptr();
    const QCustom3DLabelPrivate *dptrc() const;

private:
    Q_DISABLE_COPY(QCustom3DLabel)

    friend class Abstract3DRenderer;
};

QT_END_NAMESPACE

#endif
