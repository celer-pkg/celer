// Copyright (C) 2025 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR LGPL-3.0-only OR GPL-2.0-only OR GPL-3.0-only

#include "qquicktransformgroup_p.h"
#include <QtQuick/private/qquickitem_p.h>

QT_BEGIN_NAMESPACE

/*!
    \qmltype TransformGroup
    \inqmlmodule QtQuick.VectorImage.Helpers
    \brief Provides specialization for sequences of transforms.

    This type allows specific specializations for sequences of transform operations required
    for VectorImage.
*/

namespace {

class DummyQuickItem : public QQuickItem
{
public:
    DummyQuickItem(QQuickTransformGroup *group) : m_group(group) {}

    virtual void itemChange(ItemChange ic, const ItemChangeData &) override
    {
        if (ic == ItemTransformHasChanged)
            m_group->triggerUpdate();
    }

private:
    QQuickTransformGroup *m_group = nullptr;
};

}

QQuickTransformGroup::QQuickTransformGroup(QObject *parent)
    : QQuickTransform(parent)
{
    m_dummyItem = new DummyQuickItem(this);
}

QQuickTransformGroup::~QQuickTransformGroup()
{
    delete m_dummyItem;
}

void QQuickTransformGroup::activateOverride(QQuickTransform *xform)
{
    m_transformFlags[xform] |= Override;
    update();
}

void QQuickTransformGroup::triggerUpdate()
{
    update();
}

void QQuickTransformGroup::deactivateOverride(QQuickTransform *xform)
{
    m_transformFlags[xform] &= ~Override;
    update();
}

void QQuickTransformGroup::deactivate(QQuickTransform *xform)
{
    m_transformFlags[xform] |= Disabled;
    update();
}

void QQuickTransformGroup::applyTo(QMatrix4x4 *matrix) const
{
    QVarLengthArray<QQuickTransform *, 8> activeTransforms;
    for (QQuickTransform *xform : m_transforms) {
        int flags = m_transformFlags.value(xform);
        if (!(flags & Disabled)) {
            activeTransforms.append(xform);
            if (flags & Override)
                break;
        }
    }

    // Apply in reverse order
    for (auto it = activeTransforms.crbegin(); it != activeTransforms.crend(); ++it)
        (*it)->applyTo(matrix);
}

QQmlListProperty<QQuickTransform> QQuickTransformGroup::transformSequence()
{
    return QQmlListProperty<QQuickTransform>(this,
                                             nullptr,
                                             transformSequence_append,
                                             transformSequence_count,
                                             transformSequence_at,
                                             transformSequence_clear);
}

void QQuickTransformGroup::transformSequence_append(QQmlListProperty<QQuickTransform> *prop,
                                                    QQuickTransform *transform)
{
    if (!transform)
        return;

    QQuickTransformGroup *that = static_cast<QQuickTransformGroup *>(prop->object);
    that->m_transforms.append(transform);
    that->update();

    transform->appendToItem(that->m_dummyItem);
}

QQuickTransform *QQuickTransformGroup::transformSequence_at(QQmlListProperty<QQuickTransform> *prop,
                                                            qsizetype idx)
{
    QQuickTransformGroup *that = static_cast<QQuickTransformGroup *>(prop->object);
    if (idx < 0 || idx >= that->m_transforms.size())
        return nullptr;
    else
        return that->m_transforms.at(idx);
}

void QQuickTransformGroup::transformSequence_clear(QQmlListProperty<QQuickTransform> *prop)
{
    QQuickTransformGroup *that = static_cast<QQuickTransformGroup *>(prop->object);
    that->m_transforms.clear();
    that->update();
}

qsizetype QQuickTransformGroup::transformSequence_count(QQmlListProperty<QQuickTransform> *prop)
{
    QQuickTransformGroup *that = static_cast<QQuickTransformGroup *>(prop->object);
    return that->m_transforms.size();
}

QT_END_NAMESPACE
