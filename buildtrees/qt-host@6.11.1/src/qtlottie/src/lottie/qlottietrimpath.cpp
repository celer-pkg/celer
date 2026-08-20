// Copyright (C) 2018 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR GPL-3.0-only

#include "qlottietrimpath_p.h"

#include <QtGlobal>
#include <private/qpainterpath_p.h>
#include <private/qbezier_p.h>

#include "qlottieconstants_p.h"

QLottieTrimPath::QLottieTrimPath(QLottieBase *parent)
{
    m_appliedTrim.reset(this, OwnsAppliedTrim::No);
    setParent(parent);
}

QLottieTrimPath::QLottieTrimPath(const QLottieTrimPath &other)
    : QLottieShape(other)
{
    m_start = other.m_start;
    m_end = other.m_end;
    m_offset = other.m_offset;
    m_isParallel = other.m_isParallel;
}

QLottieBase *QLottieTrimPath::clone() const
{
    return new QLottieTrimPath(*this);
}

int QLottieTrimPath::parse(const QJsonObject &definition)
{
    QLottieBase::parse(definition);
    if (m_hidden)
        return 0;

    qCDebug(lcLottieQtLottieParser) << "QLottieTrimPath::parse():" << m_name;

    if (!checkRequiredKeys(definition, "Trim path"_L1, { "o"_L1, "s"_L1, "e"_L1 }, m_name))
        return -1;

    QJsonObject start = definition.value("s"_L1).toObject();
    start = resolveExpression(start);
    m_start.construct(start);

    QJsonObject end = definition.value("e"_L1).toObject();
    end = resolveExpression(end);
    m_end.construct(end);

    // Simplify: a constant 100% trim is a noop, just skip it
    if (!m_start.isAnimated() && !m_end.isAnimated()
        && m_start.value() == 0 && m_end.value() == 100) {
        return -2;
    }

    QJsonObject offset = definition.value("o"_L1).toObject();
    offset = resolveExpression(offset);
    m_offset.construct(offset);

    const int multiMode = definition.value("m"_L1).toInt();
    m_isParallel = (multiMode != 2);

    if (strcmp(qgetenv("QLOTTIE_FORCE_TRIM_MODE"), "sequential") == 0) {
        qCDebug(lcLottieQtLottieRender) << "Forcing trim mode to Sequential";
        m_isParallel = true;
    } else if (strcmp(qgetenv("QLOTTIE_FORCE_TRIM_MODE"), "parallel") == 0) {
        qCDebug(lcLottieQtLottieRender) << "Forcing trim mode to Parallel";
        m_isParallel = false;
    }

    return 0;
}

void QLottieTrimPath::updateProperties(int frame)
{
    m_start.update(frame);
    m_end.update(frame);
    m_offset.update(frame);

    qCDebug(lcLottieQtLottieUpdate) << name() << frame << m_start.value()
                                       << m_end.value() << m_offset.value();

    QLottieShape::updateProperties(frame);
}

void QLottieTrimPath::render(QLottieRenderer &renderer) const
{
    if (QLottieTrimPath *appliedTrim = m_appliedTrim.data()) {
        if (appliedTrim->isParallel())
            renderer.setTrimmingState(QLottieRenderer::Parallel);
        else
            renderer.setTrimmingState(QLottieRenderer::Sequential);
    } else
        renderer.setTrimmingState(QLottieRenderer::Off);

    renderer.render(*this);
}

bool QLottieTrimPath::acceptsTrim() const
{
    return true;
}

void QLottieTrimPath::applyTrim(const QLottieTrimPath &other)
{
     qCDebug(lcLottieQtLottieUpdate) << "Join trim paths:"
                                        << other.name() << "into:" << name();

    m_name = m_name + QStringLiteral(" & ") + other.name();
    qreal newStart = other.start() + (m_start.value() / 100.0) *
            (other.end() - other.start());
    qreal newEnd = other.start() + (m_end.value() / 100.0) *
            (other.end() - other.start());

    m_start.setValue(newStart);
    m_end.setValue(newEnd);
    m_offset.setValue(m_offset.value() + other.offset());
}

qreal QLottieTrimPath::start() const
{
    return m_start.value();
}

qreal QLottieTrimPath::end() const
{
    return m_end.value();
}

qreal QLottieTrimPath::offset() const
{
    return m_offset.value();
}

bool QLottieTrimPath::isParallel() const
{
    return m_isParallel;
}

QPainterPath QLottieTrimPath::trim(const QPainterPath &path) const
{
    if (isStructureDumping())
        return path;
    qreal offset = m_offset.value() / 360.0;
    qreal start = m_start.value() / 100.0;
    qreal end = m_end.value() / 100.0;
    return path.trimmed(start, end, offset);
}
