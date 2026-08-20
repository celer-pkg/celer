' Copyright (C) 2016 The Qt Company Ltd.
' SPDX-License-Identifier: LicenseRef-Qt-Commercial OR GPL-3.0-only WITH Qt-GPL-exception-1.0

' This script can be loaded in Qt TestCon, and used to script
' the hierarchyax project.
'
' Instructions: Open testcon, insert the QParentWidget class,
'               load this script, run "Main()" macro.

Sub Main
    ' Create new widget object
    QParentWidget.createSubWidget("ABC")

    ' Retrieve widget
    Set widget = QParentWidget.subWidget("ABC")

    ' Read label property
    label = widget.label
    MainWindow.logMacro 0, "Old widget label: "&label, 0, ""

    ' Write label property
    widget.label = "renamed "&label
    label = widget.label
    MainWindow.logMacro 0, "New widget label: "&label, 0, ""
End Sub
