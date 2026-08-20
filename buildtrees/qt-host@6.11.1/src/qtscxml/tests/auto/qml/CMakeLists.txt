# Copyright (C) 2022 The Qt Company Ltd.
# SPDX-License-Identifier: BSD-3-Clause

if(TARGET Qt::StateMachineQml)
    add_subdirectory(qqmlstatemachine)
    add_subdirectory(qqmlstatemachinemetatype)
endif()
if(TARGET Qt::ScxmlQml)
    add_subdirectory(scxmlqmlcpp)
endif()
