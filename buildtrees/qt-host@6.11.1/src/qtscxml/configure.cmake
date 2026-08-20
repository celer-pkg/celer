# Copyright (C) 2022 The Qt Company Ltd.
# SPDX-License-Identifier: BSD-3-Clause



#### Inputs



#### Libraries



#### Tests



#### Features

qt_feature("scxml" PRIVATE
    LABEL "Qt SCXML"
    PURPOSE "Allows embedding of state machines created from State Chart XML (SCXML) files."
)
qt_feature("scxml-qml" PRIVATE
    LABEL "SCXML QML Types"
    PURPOSE "Provides QML Types for Qt SCXML."
    CONDITION QT_FEATURE_scxml AND TARGET Qt::Qml
)
qt_feature("scxml-ecmascriptdatamodel" PRIVATE
    LABEL "ECMAScript data model for QtScxml"
    PURPOSE "Enables the usage of ecmascript data models in SCXML state machines."
    CONDITION QT_FEATURE_scxml AND TARGET Qt::Qml
)

qt_feature("statemachine" PRIVATE
    LABEL "Qt State Machine"
    PURPOSE "Provides hierarchical finite state machines."
)
qt_feature("statemachine-qml" PRIVATE
    LABEL "StateMachine QML Type"
    PURPOSE "Provides QML Type for Qt State Machine."
    CONDITION QT_FEATURE_statemachine AND TARGET Qt::Qml
)
qt_feature("qeventtransition" PRIVATE
    LABEL "Q(Mouse)EventTransition class"
    PURPOSE "Provides QObject-specific transitions for Qt events."
    CONDITION QT_FEATURE_statemachine AND TARGET Qt::Gui
)

qt_configure_add_summary_section(NAME "Qt SCXML")
qt_configure_add_summary_entry(ARGS "scxml")
qt_configure_add_summary_entry(ARGS "scxml-qml")
qt_configure_add_summary_entry(ARGS "scxml-ecmascriptdatamodel")
qt_configure_end_summary_section() # end of "Qt SCXML" section

qt_configure_add_summary_section(NAME "Qt State Machine")
qt_configure_add_summary_entry(ARGS "statemachine")
qt_configure_add_summary_entry(ARGS "statemachine-qml")
qt_configure_add_summary_entry(ARGS "qeventtransition")
qt_configure_end_summary_section() # end of "Qt State Machine" section

qt_extra_definition("QT_VERSION_STR" "\"${PROJECT_VERSION}\"" PUBLIC)
qt_extra_definition("QT_VERSION_MAJOR" ${PROJECT_VERSION_MAJOR} PUBLIC)
qt_extra_definition("QT_VERSION_MINOR" ${PROJECT_VERSION_MINOR} PUBLIC)
qt_extra_definition("QT_VERSION_PATCH" ${PROJECT_VERSION_PATCH} PUBLIC)
