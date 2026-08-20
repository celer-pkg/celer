// Copyright (C) 2025 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR GPL-3.0-only
// Qt-Security score:significant reason:default


#ifndef QSSGRENDERROOT_P_H
#define QSSGRENDERROOT_P_H

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

#include <QtQuick3DRuntimeRender/private/qssgrendernode_p.h>

#include <QtGui/qmatrix4x4.h>

#include <memory>

QT_BEGIN_NAMESPACE

class QSSGGlobalRenderNodeData;

class Q_QUICK3DRUNTIMERENDER_EXPORT QSSGRenderRoot final : public QSSGRenderNode
{
public:
    QSSGRenderRoot();
    ~QSSGRenderRoot();

    using VersionType = QSSGRenderNodeVersionType;

    enum class DirtyFlag : quint8
    {
        TreeDirty = 0x1
    };
    using FlagT = std::underlying_type_t<DirtyFlag>;

    static constexpr DirtyFlag DirtyMask { std::numeric_limits<FlagT>::max() };

    [[nodiscard]] bool isDirty(DirtyFlag dirtyFlag = DirtyMask) const
    {
        return ((m_rootDirtyFlags & FlagT(dirtyFlag)) != 0)
        || ((dirtyFlag == DirtyMask) && QSSGRenderNode::isDirty());
    }
    void markDirty(DirtyFlag dirtyFlag);
    void clearDirty(DirtyFlag dirtyFlag);

    [[nodiscard]] const std::shared_ptr<QSSGGlobalRenderNodeData> &globalNodeData() const
    {
        return m_gnd;
    }

    // This is the starting point when indexing the nodes.
    // By default it is 0, which means the first version when
    // indexing will be 1. The start version can be set to a
    // different value when needed. For example we'll set this
    // to the last version of the layer node when the window
    // changes, as that avoids accidentally re-using the same
    // version when re-indexing the layer node, as that can cause
    // issues.
    void setStartVersion(VersionType startVersion)
    {
        m_startVersion = startVersion;
    }

    [[nodiscard]] VersionType startVersion() const
    {
        return m_startVersion;
    }

    [[nodiscard]] static QSSGRenderRoot *get(QSSGRenderRoot **ref)
    {
        return (ref && *ref) ? *ref : nullptr;
    }

    void reindex();

private:
    QSSGRenderRoot *self = this;
    VersionType m_startVersion = 0;
    std::shared_ptr<QSSGGlobalRenderNodeData> m_gnd;
    FlagT m_rootDirtyFlags = FlagT(DirtyFlag::TreeDirty);
};

QT_END_NAMESPACE

#endif // QSSGRENDERROOT_P_H
