// Copyright (C) 2025 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR GPL-3.0-only

#include "qlottieflatlayers_p.h"

#include <QJsonObject>
#include <QJsonArray>
#include <QString>

#include "qlottiebase_p.h"
#include "qlottieimage_p.h"
#include "qlottieshape_p.h"
#include "qlottietrimpath_p.h"
#include "qlottiebasictransform_p.h"
#include "qlottierenderer_p.h"

QT_BEGIN_NAMESPACE

using namespace Qt::Literals::StringLiterals;

QLottieNullLayer::QLottieNullLayer(const QLottieNullLayer &other)
    : QLottieLayer(other)
{
}

QLottieBase *QLottieNullLayer::clone() const
{
    return new QLottieNullLayer(*this);
}

void QLottieNullLayer::render(QLottieRenderer &renderer) const
{
    if (!m_isActive)
        return;

    renderer.saveState();

    applyLayerTransform(renderer);

    renderer.render(*this);

    renderer.finish(*this);
    renderer.restoreState();
}

int QLottieNullLayer::parse(const QJsonObject &definition)
{
    m_type = LOTTIE_LAYER_NULL_IX;

    int ret = QLottieLayer::parse(definition);

    if (m_hidden)
        return 0;

    qCDebug(lcLottieQtLottieParser) << "QLottieNullLayer::QLottieNullLayer()" << m_name;
    return ret;
}

QLottieSolidLayer::QLottieSolidLayer(const QLottieSolidLayer &other)
    : QLottieLayer(other)
{
    m_color = other.m_color;
}

QLottieBase *QLottieSolidLayer::clone() const
{
    return new QLottieSolidLayer(*this);
}

void QLottieSolidLayer::render(QLottieRenderer &renderer) const
{
    if (!m_isActive)
        return;

    renderer.saveState();

    applyLayerTransform(renderer);

    renderer.render(*this);

    renderer.finish(*this);
    renderer.restoreState();
}

int QLottieSolidLayer::parse(const QJsonObject &definition)
{
    m_type = LOTTIE_LAYER_SOLID_IX;

    int ret = QLottieLayer::parse(definition);

    if (m_hidden)
        return 0;

    if (!checkRequiredKeys(definition, "Layer"_L1, { "sw"_L1, "sh"_L1, "sc"_L1 }, m_name))
        return -1;

    m_size = QSize(definition.value("sw"_L1).toInt(-1), definition.value("sh"_L1).toInt(-1));
    m_color = QColor(definition.value("sc"_L1).toString());

    qCDebug(lcLottieQtLottieParser) << "QLottieSolidLayer::QLottieSolidLayer()" << m_name;
    return ret;
}

QColor QLottieSolidLayer::color() const
{
    return m_color;
}

QLottieImageLayer::QLottieImageLayer(const QLottieImageLayer &other)
    : QLottieLayer(other)
{
}

QLottieBase *QLottieImageLayer::clone() const
{
    return new QLottieImageLayer(*this);
}

void QLottieImageLayer::render(QLottieRenderer &renderer) const
{
    if (!m_isActive)
        return;

    renderer.saveState();

    QLottieLayer::render(renderer);

    renderer.finish(*this);
    renderer.restoreState();
}

int QLottieImageLayer::parse(const QJsonObject &definition)
{
    m_type = LOTTIE_LAYER_IMAGE_IX;

    int ret = QLottieLayer::parse(definition);

    if (ret >= 0) {
        QLottieImage *image = new QLottieImage(this);
        ret = image->construct(definition);
        if (ret >= 0)
            appendChild(image);
    }

    if (m_hidden)
        return 0;

    qCDebug(lcLottieQtLottieParser) << "QLottieImageLayer::QLottieImageLayer()" << m_name;
    return ret;
}

QT_END_NAMESPACE
