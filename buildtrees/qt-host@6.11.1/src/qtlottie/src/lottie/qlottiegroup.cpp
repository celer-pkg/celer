// Copyright (C) 2018 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR GPL-3.0-only

#include "qlottiegroup_p.h"

#include <QJsonObject>
#include <QJsonArray>
#include <QString>

#include "qlottiebase_p.h"
#include "qlottieshape_p.h"
#include "qlottietrimpath_p.h"
#include "qlottiebasictransform_p.h"

QT_BEGIN_NAMESPACE

using namespace Qt::Literals::StringLiterals;

QLottieGroup::QLottieGroup(QLottieBase *parent)
{
    setParent(parent);
}

QLottieBase *QLottieGroup::clone() const
{
    return new QLottieGroup(*this);
}

void QLottieGroup::updateProperties(int frame)
{
    QLottieShape::updateProperties(frame);

    for (QLottieBase *child : children()) {
        if (child->hidden())
            continue;

        QLottieShape *shape = static_cast<QLottieShape*>(child);
        if (shape->type() == LOTTIE_SHAPE_TRIM_IX) {
            QLottieTrimPath *trim = static_cast<QLottieTrimPath*>(shape);
            if (QLottieTrimPath *appliedTrim = m_appliedTrim.data())
                appliedTrim->applyTrim(*trim);
            else
                m_appliedTrim.reset(trim, OwnsAppliedTrim::No);
        } else if (QLottieTrimPath *appliedTrim = m_appliedTrim.data()) {
            if (shape->acceptsTrim())
                shape->applyTrim(*appliedTrim);
        }
    }
}

void QLottieGroup::render(QLottieRenderer &renderer) const
{
    qCDebug(lcLottieQtLottieRender) << "Group:" << name();

    renderer.saveState();

    renderer.render(*this);

    const QLottieTrimPath *appliedTrim = m_appliedTrim.data();
    if (appliedTrim && !appliedTrim->hidden()) {
        if (appliedTrim->isParallel())
            renderer.setTrimmingState(QLottieRenderer::Parallel);
        else
            renderer.setTrimmingState(QLottieRenderer::Sequential);
    } else
        renderer.setTrimmingState(QLottieRenderer::Off);

    renderChildren(renderer);

    if (appliedTrim && !appliedTrim->hidden() && !appliedTrim->isParallel())
        appliedTrim->render(renderer);

    renderer.finish(*this);

    renderer.restoreState();
}

int QLottieGroup::parse(const QJsonObject &definition)
{
    QLottieBase::parse(definition);
    if (m_hidden)
        return 0;

    qCDebug(lcLottieQtLottieParser) << "QLottieGroup::parse()"
                                       << m_name;

    QJsonArray groupItems = definition.value("it"_L1).toArray();
    QJsonArray::const_iterator itemIt = groupItems.constEnd();
    while (itemIt != groupItems.constBegin()) {
        itemIt--;
        QLottieShape *shape = QLottieShape::construct((*itemIt).toObject(), this);
        if (shape) {
            // Transform affects how group contents are drawn.
            // It must be traversed first when drawing
            if (shape->type() == LOTTIE_SHAPE_TRANS_IX)
                prependChild(shape);
            else
                appendChild(shape);
        }
    }

    return 0;
}

bool QLottieGroup::acceptsTrim() const
{
    return true;
}

void QLottieGroup::applyTrim(const QLottieTrimPath &trimmer)
{
    if (!m_appliedTrim.data()) {
        QLottieTrimPath *appliedTrim = static_cast<QLottieTrimPath *>(trimmer.clone());
        m_appliedTrim.reset(appliedTrim, OwnsAppliedTrim::Yes);
        appliedTrim->setParent(parent());
        // Setting a friendly name helps in testing
        appliedTrim->setName(QStringLiteral("Inherited from") + trimmer.name());
    }

    for (QLottieBase *child : children()) {
        QLottieShape *shape = static_cast<QLottieShape*>(child);
        if (shape->acceptsTrim())
            shape->applyTrim(*m_appliedTrim.data());
    }
}

QT_END_NAMESPACE
