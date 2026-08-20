// Copyright (C) 2023 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR BSD-3-Clause

import QtQuick
import ToDoList

TasksListForm {

    hideTasks.onTapped: {
        if (taskList.opacity) {
            hideAnim.start()
        } else {
            showAnim.start()
        }
    }

    onTaskDataChanged: dialogPopup.open()

    dialogPopup.onAccepted: {
        Database.deleteTask(taskData.id)
        listModel.remove(taskData.idx)
    }
}
