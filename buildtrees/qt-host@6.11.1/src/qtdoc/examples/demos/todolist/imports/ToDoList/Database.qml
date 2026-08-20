// Copyright (C) 2023 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR BSD-3-Clause

pragma Singleton

import QtQml
import QtQuick.LocalStorage

QtObject {
    id: root

    property var _db

    function _database() {
        if (_db) return _db

        try {
            let db = LocalStorage.openDatabaseSync("ToDoList", "1.0", "ToDoList app database")

            db.transaction(function (tx) {
                tx.executeSql(`CREATE TABLE IF NOT EXISTS tasks (
                              task_id INTEGER PRIMARY KEY AUTOINCREMENT,
                              task_name,
                              task_dueDate TEXT,
                              task_dueTime TEXT,
                              task_notes TEXT,
                              done INTEGER,
                              highlighted INTEGER
                              )`);
            })

            _db = db
        } catch (error) {
            console.log("Error opening databse: " + error)
        };
        return _db
    }

    function addTask(taskName, taskDueDate, taskDueTime, taskNotes, taskDone, taskHighlighted) {
        let results
        root._database().transaction(function(tx){
            tx.executeSql(`INSERT INTO tasks (task_name, task_dueDate, task_dueTime,
                          task_notes, done, highlighted) VALUES(?,?,?,?,?,?);`,
                        [taskName, taskDueDate, taskDueTime, taskNotes, taskDone, taskHighlighted])
            results = tx.executeSql("SELECT * FROM tasks ORDER BY task_id DESC LIMIT 1")
        })
        return results.rows.item(0).task_id
    }

    function updateTask(taskId, taskName, taskDueDate, taskDueTime, taskNotes) {
        root._database().transaction(function (tx) {
            tx.executeSql(`UPDATE tasks set task_name=?, task_dueDate=?, task_dueTime=?,
                          task_notes=? WHERE task_id=?`,
                          [taskName, taskDueDate, taskDueTime, taskNotes, taskId])
        })
    }

    function getTasks() {
        let tasks = []
        root._database().transaction(function(tx){
            let results =  tx.executeSql("SELECT * FROM tasks ORDER BY done, task_id DESC")
            for (let i = 0; i < results.rows.length; i++) {
                let row = results.rows.item(i)
                tasks.push({
                               "task_id" : row.task_id,
                               "name" : row.task_name,
                               "date" : row.task_dueDate,
                               "time" : row.task_dueTime ? row.task_dueTime : "",
                               "notes" : row.task_notes? row.task_notes : "",
                               "done" : row.done,
                               "highlight" : row.highlighted
                           })
            }
        })
        return tasks;
    }

    function deleteTasks() {
        root._database().transaction(function(tx){
            tx.executeSql("DELETE FROM tasks")
        })
    }

    function deleteDoneTasks() {
        root._database().transaction(function(tx){
            tx.executeSql("DELETE FROM tasks WHERE done = 1")
        })
    }

    function deleteTask(taskId) {
        root._database().transaction(function (tx) {
            tx.executeSql("DELETE FROM tasks WHERE task_id = ?", [taskId])
        })
    }

    function updateTaskState(taskId, done) {
        root._database().transaction(function (tx) {
            tx.executeSql("UPDATE tasks set done=? WHERE task_id=?", [done, taskId])
        })
    }

    function updateTaskHighlight(taskId, highlighted) {
        root._database().transaction(function (tx) {
            tx.executeSql("UPDATE tasks set highlighted=? WHERE task_id=?", [highlighted, taskId])
        })
    }
}
