// Copyright (C) 2018 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR GPL-3.0-only

#include "qlottieshape_p.h"

#include <QJsonArray>
#include <QJsonObject>
#include <QLoggingCategory>

#include "qlottiegroup_p.h"
#include "qlottiefill_p.h"
#include "qlottiegfill_p.h"
#include "qlottiestroke_p.h"
#include "qlottierect_p.h"
#include "qlottieellipse_p.h"
#include "qlottiepolystar_p.h"
#include "qlottieround_p.h"
#include "qlottietrimpath_p.h"
#include "qlottieshapetransform_p.h"
#include "qlottiefreeformshape_p.h"
#include "qlottierepeater_p.h"
#include "qlottieconstants_p.h"

QT_BEGIN_NAMESPACE

QLottieShape::AppliedTrimPtr::AppliedTrimPtr() = default;

QLottieShape::AppliedTrimPtr::~AppliedTrimPtr()
{
    if (m_appliedTrim.tag() == OwnsAppliedTrim::Yes)
        delete m_appliedTrim.data();
}

void QLottieShape::AppliedTrimPtr::reset(QLottieTrimPath *appliedTrim, OwnsAppliedTrim owns)
{
    if (appliedTrim == m_appliedTrim.data() && m_appliedTrim.tag() == owns)
        return;

    if (m_appliedTrim.tag() == OwnsAppliedTrim::Yes) {
        if (m_appliedTrim.data() == appliedTrim) {
            // If you set the same appliedTrim again but remove the ownership, we have to delete
            // the appliedTrim. What is left then is a nullptr.
            appliedTrim = nullptr;
        }

        delete m_appliedTrim.data();
    }

    m_appliedTrim = appliedTrim;
    m_appliedTrim.setTag(owns);
}

QLottieTrimPath *QLottieShape::AppliedTrimPtr::data() const
{
    return m_appliedTrim.data();
}

bool QLottieShape::AppliedTrimPtr::owns() const
{
    return m_appliedTrim.tag() == OwnsAppliedTrim::Yes;
}

QLottieShape::QLottieShape() = default;
QLottieShape::~QLottieShape() = default;

QLottieShape::QLottieShape(const QLottieShape &other)
    : QLottieBase(other)
{
    m_direction = other.m_direction;
    m_path = other.m_path;
    if (other.m_appliedTrim.owns()) {
        m_appliedTrim.reset(
                static_cast<QLottieTrimPath *>(other.m_appliedTrim.data()->clone()),
                OwnsAppliedTrim::Yes);
    } else {
        m_appliedTrim.reset(other.m_appliedTrim.data(), OwnsAppliedTrim::No);
    }
}

QLottieBase *QLottieShape::clone() const
{
    return new QLottieShape(*this);
}

QLottieShape *QLottieShape::construct(QJsonObject definition, QLottieBase *parent)
{
    qCDebug(lcLottieQtLottieParser) << "QLottieShape::parse()";

    std::unique_ptr<QLottieShape> shape;
    if (!definition.contains("ty"_L1)) {
        qCWarning(lcLottieQtLottieParser) << "Shape" << definition.value("nm"_L1).toString()
                                          << "is missing required key \"ty\"";
        return nullptr;
    }
    const QByteArray type = definition.value("ty"_L1).toString().toLatin1();

    if (Q_UNLIKELY(type.size() != 2)) {
        qCInfo(lcLottieQtLottieParser) << "Unsupported shape type:"
                                             << type;
        return nullptr;
    }

#define LOTTIE_SHAPE_TAG(c1, c2) int((quint32(c1)<<8) | quint32(c2))

    int typeToBuild = LOTTIE_SHAPE_TAG(type[0], type[1]);

    switch (typeToBuild) {
    case LOTTIE_SHAPE_TAG('g', 'r'):
    {
        qCDebug(lcLottieQtLottieParser) << "Parse group";
        shape.reset(new QLottieGroup(parent));
        if (shape->parse(definition) < 0)
            return nullptr;
        shape->setType(LOTTIE_SHAPE_GROUP_IX);
        break;
    }
    case LOTTIE_SHAPE_TAG('r', 'c'):
    {
        qCDebug(lcLottieQtLottieParser) << "Parse rect";
        shape.reset(new QLottieRect(parent));
        if (shape->parse(definition) < 0)
            return nullptr;
        shape->setType(LOTTIE_SHAPE_RECT_IX);
        break;
    }
    case LOTTIE_SHAPE_TAG('f', 'l'):
    {
        qCDebug(lcLottieQtLottieParser) << "Parse fill";
        shape.reset(new QLottieFill(parent));
        if (shape->parse(definition) < 0)
            return nullptr;
        shape->setType(LOTTIE_SHAPE_FILL_IX);
        break;
    }
    case LOTTIE_SHAPE_TAG('g', 'f'):
    {
        qCDebug(lcLottieQtLottieParser) << "Parse gradient fill";
        shape.reset(new QLottieGFill(parent));
        if (shape->parse(definition) < 0)
            return nullptr;
        shape->setType(LOTTIE_SHAPE_GFILL_IX);
        break;
    }
    case LOTTIE_SHAPE_TAG('s', 't'):
    {
        qCDebug(lcLottieQtLottieParser) << "Parse stroke";
        shape.reset(new QLottieStroke(parent));
        if (shape->parse(definition) < 0)
            return nullptr;
        shape->setType(LOTTIE_SHAPE_STROKE_IX);
        break;
    }
    case LOTTIE_SHAPE_TAG('t', 'r'):
    {
        qCDebug(lcLottieQtLottieParser) << "Parse shape transform";
        shape.reset(new QLottieShapeTransform(parent));
        if (shape->parse(definition) < 0)
            return nullptr;
        shape->setType(LOTTIE_SHAPE_TRANS_IX);
        break;
    }
    case LOTTIE_SHAPE_TAG('e', 'l'):
    {
        qCDebug(lcLottieQtLottieParser) << "Parse ellipse";
        shape.reset(new QLottieEllipse(parent));
        if (shape->parse(definition) < 0)
            return nullptr;
        shape->setType(LOTTIE_SHAPE_ELLIPSE_IX);
        break;
    }
    case LOTTIE_SHAPE_TAG('s', 'r'):
    {
        qCDebug(lcLottieQtLottieParser) << "Parse polystar";
        shape.reset(new QLottiePolyStar(parent));
        if (shape->parse(definition) < 0)
            return nullptr;
        shape->setType(LOTTIE_SHAPE_STAR_IX);
        break;
    }
    case LOTTIE_SHAPE_TAG('r', 'd'):
    {
        qCDebug(lcLottieQtLottieParser) << "Parse round";
        shape.reset(new QLottieRound(parent));
        if (shape->parse(definition) < 0)
            return nullptr;
        shape->setType(LOTTIE_SHAPE_ROUND_IX);
        break;
    }
    case LOTTIE_SHAPE_TAG('s', 'h'):
    {
        qCDebug(lcLottieQtLottieParser) << "Parse path";
        shape.reset(new QLottieFreeFormShape(parent));
        if (shape->parse(definition) < 0)
            return nullptr;
        shape->setType(LOTTIE_SHAPE_SHAPE_IX);
        break;
    }
    case LOTTIE_SHAPE_TAG('t', 'm'):
    {
        qCDebug(lcLottieQtLottieParser) << "Parse trim path";
        shape.reset(new QLottieTrimPath(parent));
        if (shape->parse(definition) < 0)
            return nullptr;
        shape->setType(LOTTIE_SHAPE_TRIM_IX);
        break;
    }
    case LOTTIE_SHAPE_TAG('r', 'p'):
    {
        qCDebug(lcLottieQtLottieParser) << "Parse repeater";
        shape.reset(new QLottieRepeater(definition, parent));
        shape->setType(LOTTIE_SHAPE_REPEATER_IX);
        break;
    }
    case LOTTIE_SHAPE_TAG('g', 's'): // ### LOTTIE_SHAPE_GSTROKE_IX
        // fall through
    default:
        qCInfo(lcLottieQtLottieParser) << "Unsupported shape type:"
                                             << type;
    }

#undef LOTTIE_SHAPE_TAG

    return shape.release();
}

bool QLottieShape::acceptsTrim() const
{
    return false;
}

void QLottieShape::applyTrim(const QLottieTrimPath &trimmer)
{
    if (trimmer.isParallel())
        m_path = trimmer.trim(m_path);
}

const QLottieTrimPath *QLottieShape::currentTrim() const
{
    return m_appliedTrim.data();
}

int QLottieShape::direction() const
{
    return m_direction;
}

const QPainterPath &QLottieShape::path() const
{
    return m_path;
}

QT_END_NAMESPACE
