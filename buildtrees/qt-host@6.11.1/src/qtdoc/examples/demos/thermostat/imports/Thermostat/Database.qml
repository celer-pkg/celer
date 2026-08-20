// Copyright (C) 2023 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR BSD-3-Clause

pragma Singleton

import QtQml
import QtQuick
import QtQuick.LocalStorage

QtObject {
    id: root

    property var _db

    function _database() {
        if (_db) return _db

        try {
            let db = LocalStorage.openDatabaseSync("Thermostat", "1.0", "Thermostat app database")

            db.transaction(function (tx) {
                tx.executeSql(`CREATE TABLE IF NOT EXISTS scheduleThermostat (
                    id INTEGER PRIMARY KEY AUTOINCREMENT,
                    room,
                    date TEXT,
                    hours TEXT,
                    days TEXT,
                    temp INTEGER,
                    mode INTEGER
                )`);
            })

            _db = db
        } catch (error) {
            console.log("Error opening databse: " + error)
        };
        return _db
    }

    function saveSchedule(room, date, hours, days, temp, mode) {
        root._database().transaction(function(tx){
            tx.executeSql(`INSERT INTO scheduleThermostat (room, date, hours,
                        days, temp, mode) VALUES(?,?,?,?,?,?);`,
                        [room, date, hours, days, temp, mode])
        })
    }

    function updateSchedule(room, date, hours, days, temp, mode) {
        root._database().transaction(function (tx) {
            tx.executeSql(`UPDATE scheduleThermostat set hours=?, days=?,
                          temp=?, mode=? WHERE room=? AND date=?`,
                          [hours, days, temp, mode, room, date])
        })
    }

    function getSchedule(room, date) {
        let options
        root._database().transaction(function(tx){
            let result = tx.executeSql("SELECT * FROM scheduleThermostat WHERE room=? AND date=?",
                                         [room, date])
            options = result.rows.item(0)
        })
        return options
    }

    function deleteAll() {
        root._database().transaction(function(tx){
            tx.executeSql("DELETE FROM scheduleThermostat")
        })
    }
}
