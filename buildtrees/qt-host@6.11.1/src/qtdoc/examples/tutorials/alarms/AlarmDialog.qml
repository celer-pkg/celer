// Copyright (C) 2018 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR BSD-3-Clause

pragma ComponentBehavior: Bound

import QtQuick
import QtQuick.Controls
import QtQuick.Layouts
import QtQuick.Window

Dialog {
    id: alarmDialog
    title: "Add new alarm"
    modal: true
    standardButtons: DialogButtonBox.Ok | DialogButtonBox.Cancel

    property AlarmModel alarmModel

    function formatNumber(number) {
        return number < 10 && number >= 0 ? "0" + number : number.toString()
    }

    onAccepted: {
        alarmModel.append({
            "hour": hoursTumbler.currentIndex,
            "minute": minutesTumbler.currentIndex,
            "day": dayTumbler.currentIndex + 1,
            "month": monthTumbler.currentIndex + 1,
            "year": yearTumbler.years[yearTumbler.currentIndex],
            "activated": true,
            "label": "",
            "repeat": false,
            "daysToRepeat": [
                { "dayOfWeek": 0, "repeat": false },
                { "dayOfWeek": 1, "repeat": false },
                { "dayOfWeek": 2, "repeat": false },
                { "dayOfWeek": 3, "repeat": false },
                { "dayOfWeek": 4, "repeat": false },
                { "dayOfWeek": 5, "repeat": false },
                { "dayOfWeek": 6, "repeat": false }
            ],
        })
    }
    onRejected: alarmDialog.close()

    contentItem: RowLayout {
        RowLayout {
            id: rowTumbler

            Tumbler {
                id: hoursTumbler
                model: 24
                delegate: TumblerDelegate {
                    text: alarmDialog.formatNumber(modelData)
                }
            }
            Tumbler {
                id: minutesTumbler
                model: 60
                delegate: TumblerDelegate {
                    text: alarmDialog.formatNumber(modelData)
                }
            }
        }

        RowLayout {
            id: datePicker

            Layout.leftMargin: 20

            readonly property var days: [31, 29, 31, 30, 31, 30, 31, 31, 30, 31, 30, 31]

            Tumbler {
                id: dayTumbler

                function updateModel() {
                    // Populate the model with days of the month. For example: [0, ..., 30]
                    var previousIndex = dayTumbler.currentIndex
                    var array = []
                    var newDays = datePicker.days[monthTumbler.currentIndex]
                    for (let i = 1; i <= newDays; ++i)
                        array.push(i)
                    dayTumbler.model = array
                    dayTumbler.currentIndex = Math.min(newDays - 1, previousIndex)
                }

                Component.onCompleted: updateModel()

                delegate: TumblerDelegate {
                    text: alarmDialog.formatNumber(modelData)
                }
            }
            Tumbler {
                id: monthTumbler

                onCurrentIndexChanged: dayTumbler.updateModel()

                model: 12
                delegate: TumblerDelegate {
                    text: alarmDialog.locale.standaloneMonthName(modelData, Locale.ShortFormat)
                }
            }
            Tumbler {
                id: yearTumbler

                // This array is populated with the next three years. For example: [2018, 2019, 2020]
                readonly property var years: (function() {
                    var currentYear = new Date().getFullYear()
                    return [0, 1, 2].map(function(value) { return value + currentYear; })
                })()

                model: years
                delegate: TumblerDelegate {
                    text: alarmDialog.formatNumber(modelData)
                }
            }
        }
    }
}
