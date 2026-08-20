// Copyright (C) 2023 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR GPL-3.0-only
// Qt-Security score:significant reason:default


#ifndef QTGRAPHS_QCUSTOMLABELITEM_H
#define QTGRAPHS_QCUSTOMLABELITEM_H

#include <QtGraphs/qcustom3ditem.h>
#include <QtGraphs/qgraphsglobal.h>
#include <QtGui/qcolor.h>
#include <QtGui/qfont.h>
#include <QtGui/qquaternion.h>
#include <QtGui/qvector3d.h>

QT_BEGIN_NAMESPACE

class QCustom3DLabelPrivate;

class Q_GRAPHS_EXPORT QCustom3DLabel : public QCustom3DItem
{
    Q_OBJECT
    Q_DECLARE_PRIVATE(QCustom3DLabel)
    Q_PROPERTY(QString text READ text WRITE setText NOTIFY textChanged FINAL)
    Q_PROPERTY(QFont font READ font WRITE setFont NOTIFY fontChanged FINAL)
    Q_PROPERTY(QColor textColor READ textColor WRITE setTextColor NOTIFY textColorChanged FINAL)
    Q_PROPERTY(QColor backgroundColor READ backgroundColor WRITE setBackgroundColor NOTIFY
                   backgroundColorChanged FINAL)
    Q_PROPERTY(bool borderVisible READ isBorderVisible WRITE setBorderVisible NOTIFY
                   borderVisibleChanged FINAL)
    Q_PROPERTY(bool backgroundVisible READ isBackgroundVisible WRITE setBackgroundVisible NOTIFY
                   backgroundVisibleChanged FINAL)
    Q_PROPERTY(bool facingCamera READ isFacingCamera WRITE setFacingCamera NOTIFY
                   facingCameraChanged FINAL)
    QML_NAMED_ELEMENT(Custom3DLabel)

public:
    explicit QCustom3DLabel(QObject *parent = nullptr);
    explicit QCustom3DLabel(const QString &text,
                            const QFont &font,
                            QVector3D position,
                            QVector3D scaling,
                            const QQuaternion &rotation,
                            QObject *parent = nullptr);
    ~QCustom3DLabel() override;

    void setText(const QString &text);
    QString text() const;

    void setFont(const QFont &font);
    QFont font() const;

    void setTextColor(QColor color);
    QColor textColor() const;

    void setBackgroundColor(QColor color);
    QColor backgroundColor() const;

    void setBorderVisible(bool visible);
    bool isBorderVisible() const;

    void setBackgroundVisible(bool visible);
    bool isBackgroundVisible() const;

    void setFacingCamera(bool enabled);
    bool isFacingCamera() const;

Q_SIGNALS:
    void textChanged(const QString &text);
    void fontChanged(const QFont &font);
    void textColorChanged(QColor color);
    void backgroundColorChanged(QColor color);
    void borderVisibleChanged(bool visible);
    void backgroundVisibleChanged(bool visible);
    void facingCameraChanged(bool enabled);

private:
    Q_DISABLE_COPY(QCustom3DLabel)
};

QT_END_NAMESPACE

#endif
