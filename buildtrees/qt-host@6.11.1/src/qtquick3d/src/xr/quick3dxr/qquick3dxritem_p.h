// Copyright (C) 2024 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR GPL-3.0-only
// Qt-Security score:significant reason:default


#ifndef QQUICK3DXRITEM_P_H
#define QQUICK3DXRITEM_P_H

#include <QQuickItem>
#include <QtQuick3D/private/qquick3dnode_p.h>
#include <QtQuick3DXr/private/qquick3dxrview_p.h>

#include <QtQuick/private/qquicktranslate_p.h>

//
//  W A R N I N G
//  -------------
//
// This file is not part of the Qt API.  It exists purely as an
// implementation detail.  This header file may change from version to
// version without notice, or even be removed.
//
// We mean it.
//

QT_BEGIN_NAMESPACE

class QQuick3DXrItemPrivate;
class QQuick3DXrView;

class QQuick3DXrItem : public QQuick3DNode
{
    Q_OBJECT
    Q_DECLARE_PRIVATE(QQuick3DXrItem)

    Q_PROPERTY(QQuickItem *contentItem READ contentItem WRITE setContentItem NOTIFY contentItemChanged FINAL)
    Q_PROPERTY(qreal pixelsPerUnit READ pixelsPerUnit WRITE setPixelsPerUnit NOTIFY pixelsPerUnitChanged FINAL)
    Q_PROPERTY(bool manualPixelsPerUnit READ manualPixelsPerUnit WRITE setManualPixelsPerUnit NOTIFY manualPixelsPerUnitChanged FINAL)
    Q_PROPERTY(bool automaticHeight READ automaticHeight WRITE setAutomaticHeight NOTIFY automaticHeightChanged FINAL)
    Q_PROPERTY(bool automaticWidth READ automaticWidth WRITE setAutomaticWidth NOTIFY automaticWidthChanged FINAL)
    Q_PROPERTY(qreal width READ width WRITE setWidth NOTIFY widthChanged FINAL)
    Q_PROPERTY(qreal height READ height WRITE setHeight NOTIFY heightChanged FINAL)
    Q_PROPERTY(QColor color READ color WRITE setColor NOTIFY colorChanged FINAL)

    QML_NAMED_ELEMENT(XrItem)
    QML_ADDED_IN_VERSION(6, 8)
public:
    explicit QQuick3DXrItem(QQuick3DNode *parent = nullptr);
    ~QQuick3DXrItem() override;

    QQuickItem *contentItem() const;
    void setContentItem(QQuickItem *newContentItem);

    qreal pixelsPerUnit() const;
    void setPixelsPerUnit(qreal newPixelsPerUnit);

    bool manualPixelsPerUnit() const;
    void setManualPixelsPerUnit(bool newManualPixelsPerUnit);

    qreal width() const;
    void setWidth(qreal newWidth);

    qreal height() const;
    void setHeight(qreal newHeight);

    QColor color() const;
    void setColor(const QColor &newColor);

    bool automaticHeight() const;
    void setAutomaticHeight(bool newAutomaticHeight);

    bool automaticWidth() const;
    void setAutomaticWidth(bool newAutomaticHeight);

    void componentComplete() override;

signals:
    void contentItemChanged();
    void pixelsPerUnitChanged();
    void flagsChanged();
    void manualPixelsPerUnitChanged();
    void widthChanged();
    void heightChanged();
    void colorChanged();
    void automaticHeightChanged();
    void automaticWidthChanged();
};

QT_END_NAMESPACE

#endif // QQUICK3DXRITEM_P_H
