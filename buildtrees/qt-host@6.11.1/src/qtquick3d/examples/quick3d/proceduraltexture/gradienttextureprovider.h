// Copyright (C) 2025 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR BSD-3-Clause

#ifndef GRADIENTTEXTUREPROVIDER_H
#define GRADIENTTEXTUREPROVIDER_H

#include <QQuick3DTextureProviderExtension>
#include <QtGui/QColor>
#include <QQmlEngine>

//! [class definition]
class GradientTextureProvider : public QQuick3DTextureProviderExtension
{
    Q_OBJECT
    Q_PROPERTY(int height READ height WRITE setHeight NOTIFY heightChanged)
    Q_PROPERTY(int width READ width WRITE setWidth NOTIFY widthChanged)
    Q_PROPERTY(QColor startColor READ startColor WRITE setStartColor NOTIFY startColorChanged)
    Q_PROPERTY(QColor endColor READ endColor WRITE setEndColor NOTIFY endColorChanged)
    QML_ELEMENT
//! [class definition]
public:
    GradientTextureProvider();
    int height() const;
    void setHeight(int newHeight);
    int width() const;
    void setWidth(int newWidth);

    QColor startColor() const;
    void setStartColor(const QColor &newStartColor);

    QColor endColor() const;
    void setEndColor(const QColor &newEndColor);

Q_SIGNALS:
    void heightChanged();
    void widthChanged();
    void startColorChanged();
    void endColorChanged();

protected:
    QSSGRenderGraphObject *updateSpatialNode(QSSGRenderGraphObject *node) override;

private:
    int m_height = 256;
    int m_width = 256;
    QColor m_startColor = QColor(QStringLiteral("#d4fc79"));
    QColor m_endColor = QColor(QStringLiteral("#96e6a1"));
};

#endif // GRADIENTTEXTUREPROVIDER_H
