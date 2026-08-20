# Copyright (C) 2024 The Qt Company Ltd.
# SPDX-License-Identifier: BSD-3-Clause

qt_feature("graphs-2d" PUBLIC
    LABEL "2D Graphs"
    PURPOSE "Support for 2D graphs"
)

qt_feature("graphs-2d-area" PUBLIC
    LABEL "Area"
    PURPOSE "Support for Area graph"
)

qt_feature("graphs-2d-bar" PUBLIC
    LABEL "Bar"
    PURPOSE "Support for Bar graph"
)

qt_feature("graphs-2d-donut-pie" PUBLIC
    LABEL "Donut and Pie"
    PURPOSE "Support for Donut and Pie graphs"
)

qt_feature("graphs-2d-line" PUBLIC
    LABEL "Line"
    PURPOSE "Support for Line graph"
)

qt_feature("graphs-2d-scatter" PUBLIC
    LABEL "Scatter"
    PURPOSE "Support for Scatter graph"
)

qt_feature("graphs-2d-spline" PUBLIC
    LABEL "Spline"
    PURPOSE "Support for Spline graph"
)

qt_feature("graphs-2d-custom" PUBLIC
    LABEL "Custom"
    PURPOSE "Support for Custom graph"
)

qt_feature("graphs-3d" PUBLIC
    LABEL "3D Graphs"
    PURPOSE "Support for 3D graphs"
    CONDITION TARGET Qt6::Quick3D
)

qt_feature("graphs-3d-bars3d" PUBLIC
    LABEL "Bars3D"
    PURPOSE "Support for Bars3D graph"
    CONDITION TARGET Qt6::Quick3D
)

qt_feature("graphs-3d-scatter3d" PUBLIC
    LABEL "Scatter3D"
    PURPOSE "Support for Scatter3D graph"
    CONDITION TARGET Qt6::Quick3D
)

qt_feature("graphs-3d-surface3d" PUBLIC
    LABEL "Surface3D"
    PURPOSE "Support for Surface3D graph"
    CONDITION TARGET Qt6::Quick3D
)
