# Copyright (C) 2025 The Qt Company Ltd.
# SPDX-License-Identifier: BSD-3-Clause

qt_feature("lottie-network" PRIVATE
    LABEL "Lottie network support"
    PURPOSE "Provides network transparency for loading lottie animations"
    CONDITION QT_FEATURE_qml_network
)

