// Copyright (C) 2022 The Qt Company Ltd.
// Copyright (C) 2016 Jolla Ltd, author: <gunnar.sletta@jollamobile.com>
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR LGPL-3.0-only OR GPL-2.0-only OR GPL-3.0-only
// Qt-Security score:significant reason:default

#ifndef QGFXSHADERBUILDER_P_H
#define QGFXSHADERBUILDER_P_H

#include <QtCore/QObject>
#include <QtCore/QVariantMap>
#include <QtCore/QUrl>
#include <rhi/qshaderbaker.h>

#include <QtQml/QJSValue>
#include <QtQml/qqmlregistration.h>

QT_BEGIN_NAMESPACE

class QTemporaryFile;
class QGfxShaderBuilder : public QObject
{
    Q_OBJECT

    QML_NAMED_ELEMENT(ShaderBuilder)
    QML_SINGLETON
    QML_ADDED_IN_VERSION(5, 0)
public:
    QGfxShaderBuilder();
    ~QGfxShaderBuilder() override;

    Q_INVOKABLE QVariantMap gaussianBlur(const QJSValue &parameters);
    Q_INVOKABLE QUrl buildVertexShader(const QByteArray &code);
    Q_INVOKABLE QUrl buildFragmentShader(const QByteArray &code);

private:
    QUrl buildShader(const QByteArray &code, QShader::Stage stage);

    int m_maxBlurSamples = 0;
    QShaderBaker m_shaderBaker;
};

QT_END_NAMESPACE

#endif // QGFXSHADERBUILDER_P_H
