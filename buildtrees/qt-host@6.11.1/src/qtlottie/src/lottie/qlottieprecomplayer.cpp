// Copyright (C) 2025 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR GPL-3.0-only

#include "qlottieprecomplayer_p.h"

#include <QJsonArray>
#include <QJsonObject>
#include <QJsonValue>
#include <QLoggingCategory>
#include <QString>

#include "qlottiebasictransform_p.h"
#include "qlottierenderer_p.h"

QT_BEGIN_NAMESPACE

using namespace Qt::Literals::StringLiterals;

QLottiePrecompLayer::QLottiePrecompLayer(const QLottiePrecompLayer &other)
    : QLottieLayer(other)
{
}

QLottiePrecompLayer::QLottiePrecompLayer(const QMap<QString, QJsonObject> &assets)
    : m_assets(assets)
{
}

QLottieBase *QLottiePrecompLayer::clone() const
{
    return new QLottiePrecompLayer(*this);
}

void QLottiePrecompLayer::render(QLottieRenderer &renderer) const
{
    if (!m_isActive)
        return;

    renderer.saveState();

    QLottieLayer::render(renderer);

    renderer.finish(*this);
    renderer.restoreState();
}

int QLottiePrecompLayer::parse(const QJsonObject &definition)
{
    m_type = LOTTIE_LAYER_PRECOMP_IX;

    QLottieLayer::parse(definition);
    if (m_hidden)
        return 0;

    qCDebug(lcLottieQtLottieParser) << "QLottiePrecompLayer::QLottiePrecompLayer()" << m_name;

    m_startTime = definition.value("st"_L1).toDouble(); // only relevant for precomps

    if (!checkRequiredKeys(definition, "Precomp Layer"_L1, { "refId"_L1 }, m_name))
        return -1;
    QString refId = definition.value("refId"_L1).toString();

    QJsonObject asset = m_assets.value(refId);
    QJsonArray jsonLayers = asset.value("layers"_L1).toArray();
    int numLayers = QLottieLayer::constructLayers(jsonLayers, this, m_assets);

    m_size = QSize(definition.value("w"_L1).toInt(-1), definition.value("h"_L1).toInt(-1));

    qCDebug(lcLottieQtLottieParser) << "QLottiePrecompLayer created" << numLayers << "layers";
    return 0;
}

QT_END_NAMESPACE
