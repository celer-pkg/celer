// Copyright (C) 2023 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR BSD-3-Clause

import QtQuick

CalendarViewForm {

    function range(start : int, end : int) : list<int> {
        return new Array(end - start + 1).fill().map((_, idx) => start + idx)
    }

    monthSelector.displayText: monthSelectorText.substr(0,3)

    years: range(2023, 2040)
    months: [qsTr("January"), qsTr("February"), qsTr("March"),
            qsTr("April"), qsTr("May"), qsTr("June"),
            qsTr("July"), qsTr("August"), qsTr("September"),
            qsTr("October"), qsTr("November"), qsTr("December")]

    monthSelector.onCurrentIndexChanged: {
        calendar.month = currentMonth
    }

    yearSelector.onCurrentIndexChanged: {
        calendarYear = years[currentYear]
    }

    nextMonth.onTapped: {
        if (currentMonth < months.length - 1) {
           currentMonth++
        } else {
           currentMonth = 0
        }
    }

    nextYear.onTapped: {
        if (currentYear < years.length - 1) {
            currentYear++
        }
    }

    previousYear.onTapped: {
        if (currentYear > 0) {
            currentYear--
        }
    }

    previousMonth.onTapped: {
        if (currentMonth) {
            currentMonth--
        } else {
            currentMonth = months.length - 1
        }
    }

    cancel.onTapped: visible = false
    calendar.onPressed: (calendarDate) => { selectedDate = calendarDate }
}
