// Copyright (C) 2023 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR GPL-3.0-only
// Qt-Security score:significant reason:default

#include "qblendtreenode_p.h"

QT_BEGIN_NAMESPACE

/*!
    \qmltype BlendTreeNode
    \nativetype QBlendTreeNode
    \inqmlmodule QtQuick.Timeline.BlendTrees
    \ingroup qtqmltypes

    \brief Base class for all blend tree nodes.

    BlendTreeNode is the base class for all blend tree nodes. It is not
    intended to be used directly, but rather to be subclassed to create
    custom blend tree nodes.
*/

/*!
    \qmlproperty bool BlendTreeNode::outputEnabled

    This property determines whether the blend tree node should commit
    the property changes. The default value is /c false.

    Any node can be an output node which commits the property changes,
    but it should usually be the last node in the blend tree.  If multiple
    nodes are outputting data and there is a conflict, then the last
    property change will be used. So ideally if there are multiple output
    nodes in a tree, the targets and properties effected should be disjoint.
*/

QBlendTreeNode::QBlendTreeNode(QObject *parent)
    : QObject{parent}
{
    connect(this, &QBlendTreeNode::frameDataChanged, this, &QBlendTreeNode::handleFrameDataChanged);
    connect(this, &QBlendTreeNode::outputEnabledChanged, this, &QBlendTreeNode::handleFrameDataChanged);
}

const QHash<QQmlProperty, QVariant> &QBlendTreeNode::frameData()
{
    return m_frameData;
}

bool QBlendTreeNode::outputEnabled() const
{
    return m_outputEnabled;
}

void QBlendTreeNode::setOutputEnabled(bool isOutputEnabled)
{
    if (m_outputEnabled == isOutputEnabled)
        return;
    m_outputEnabled = isOutputEnabled;
    Q_EMIT outputEnabledChanged();
}

void QBlendTreeNode::handleFrameDataChanged()
{
    // If we are not outputting data, then there is nothing to do
    if (!m_outputEnabled)
        return;

    // Commit the property
    for (auto it = m_frameData.cbegin(), end = m_frameData.cend(); it != end; ++it) {
        const auto &property = it.key();
        property.write(it.value());
    }
}

QT_END_NAMESPACE
