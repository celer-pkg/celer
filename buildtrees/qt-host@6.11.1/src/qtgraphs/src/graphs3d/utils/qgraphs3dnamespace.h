// Copyright (C) 2024 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR GPL-3.0-only
// Qt-Security score:significant reason:default

#ifndef QTGRAPHS_QGRAPHS3DNAMESPACE_H
#define QTGRAPHS_QGRAPHS3DNAMESPACE_H
#include <QtCore/qobjectdefs.h>
#include <QtGraphs/qtgraphsexports.h>
#include <QtQmlIntegration/qqmlintegration.h>

QT_BEGIN_NAMESPACE

namespace QtGraphs3D {

Q_NAMESPACE_EXPORT(Q_GRAPHS_EXPORT)
QML_NAMED_ELEMENT(Graphs3D)

enum class SelectionFlag {
    None = 0x00,
    Item = 0x01,
    Row = 0x02,
    ItemAndRow = Item | Row,
    Column = 0x04,
    ItemAndColumn = Item | Column,
    RowAndColumn = Row | Column,
    ItemRowAndColumn = Item | Row | Column,
    Slice = 0x08,
    MultiSeries = 0x10,
};
Q_FLAG_NS(SelectionFlag)
Q_DECLARE_FLAGS(SelectionFlags, SelectionFlag)

enum class SliceCaptureType {
    NoImage,
    RowImage,
    ColumnImage,
};
Q_ENUM_NS(SliceCaptureType)

enum class ShadowQuality {
    None,
    Low,
    Medium,
    High,
    SoftLow,
    SoftMedium,
    SoftHigh,
};
Q_ENUM_NS(ShadowQuality)

enum class ElementType {
    None,
    Series,
    AxisXLabel,
    AxisYLabel,
    AxisZLabel,
    CustomItem,
};
Q_ENUM_NS(ElementType)

enum class OptimizationHint {
    Default,
    Legacy,
};
Q_ENUM_NS(OptimizationHint)

enum class RenderingMode {
    DirectToBackground,
    Indirect,
};
Q_ENUM_NS(RenderingMode)

enum class CameraPreset {
    NoPreset,
    FrontLow,
    Front,
    FrontHigh,
    LeftLow,
    Left,
    LeftHigh,
    RightLow,
    Right,
    RightHigh,
    BehindLow,
    Behind,
    BehindHigh,
    IsometricLeft,
    IsometricLeftHigh,
    IsometricRight,
    IsometricRightHigh,
    DirectlyAbove,
    DirectlyAboveCW45,
    DirectlyAboveCCW45,
    FrontBelow,
    LeftBelow,
    RightBelow,
    BehindBelow,
    DirectlyBelow,
};
Q_ENUM_NS(CameraPreset)

enum class GridLineType { Shader, Geometry };
Q_ENUM_NS(GridLineType)

enum class TransparencyTechnique {
    Default,
    Approximate,
    Accurate,
};
Q_ENUM_NS(TransparencyTechnique)

} // namespace QtGraphs3D
Q_DECLARE_OPERATORS_FOR_FLAGS(QtGraphs3D::SelectionFlags)

QT_END_NAMESPACE

#endif // QTGRAPHS_QGRAPHS3DNAMESPACE_H
