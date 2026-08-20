// Copyright (C) 2023 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR GPL-3.0-only
// Qt-Security score:significant reason:default


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

#ifndef QQUICK3DTEXTUREDATAFRONTEND_H
#define QQUICK3DTEXTUREDATAFRONTEND_H

#include <QtQuick3DHelpers/qtquick3dhelpersexports.h>
#include <QtQuick3D/QQuick3DTextureData>
#include <QtQml/QQmlEngine>

#include <QtCore/QByteArray>

QT_BEGIN_NAMESPACE

class Q_QUICK3DHELPERS_EXPORT QQuick3DTextureDataFrontend : public QQuick3DTextureData
{
    Q_OBJECT
    Q_PROPERTY(QQuick3DTextureData::Format format READ format WRITE setFormat NOTIFY formatChanged)
    Q_PROPERTY(int width READ width WRITE setWidth NOTIFY widthChanged)
    Q_PROPERTY(int height READ height WRITE setHeight NOTIFY heightChanged)
    Q_PROPERTY(int depth READ depth WRITE setDepth NOTIFY depthChanged)
    Q_PROPERTY(bool hasTransparency READ hasTransparency WRITE setHasTransparency NOTIFY hasTransparencyChanged)
    Q_PROPERTY(QByteArray textureData READ textureData WRITE setTextureData NOTIFY textureDataChanged)

    QML_NAMED_ELEMENT(ProceduralTextureData)
    QML_ADDED_IN_VERSION(6, 6)
public:
    QQuick3DTextureDataFrontend();
    QQuick3DTextureData::Format format() const;
    void setFormat(const QQuick3DTextureData::Format &newFormat);

    int depth() const;
    void setDepth(int newDepth);

    bool hasTransparency() const;
    void setHasTransparency(bool newHasTransparency);

    QByteArray textureData() const;
    void setTextureData(const QByteArray &newTextureData);

    int width() const;
    void setWidth(int newWidth);

    int height() const;
    void setHeight(int newHeight);

Q_SIGNALS:
    void formatChanged();
    void depthChanged();
    void hasTransparencyChanged();
    void textureDataChanged();
    void widthChanged();
    void heightChanged();
};

QT_END_NAMESPACE

#endif // QQUICK3DTEXTUREDATAFRONTEND_H
