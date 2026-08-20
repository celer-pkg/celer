// Copyright (C) 2017 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR BSD-3-Clause

import QtQuick

Column {
    id: delegate

    required property string title
    required property string content
    required property string link
    required property string pubDate

    width: delegate.ListView.view.width
    spacing: 8

    // Returns a string representing how long ago an event occurred
    function timeSinceEvent(pubDate) {
        var result = pubDate;

        // We need to modify the pubDate read from the RSS feed
        // so the JavaScript Date object can interpret it
        var d = pubDate.replace(',','').split(' ');
        if (d.length != 6)
            return result;

        var date = new Date([d[0], d[2], d[1], d[3], d[4], 'GMT' + d[5]].join(' '));

        if (!isNaN(date.getDate())) {
            let age = new Date() - date;
            let minutes = Math.floor(Number(age) / 60000);
            if (minutes < 1440) {
                if (minutes < 2)
                    result = qsTr("Just now");
                else if (minutes < 60)
                    result = '' + minutes + ' ' + qsTr("minutes ago")
                else if (minutes < 120)
                    result = qsTr("1 hour ago");
                else
                    result = '' + Math.floor(minutes/60) + ' ' + qsTr("hours ago");
            }
            else {
                result = date.toDateString();
            }
        }
        return result;
    }

    Item { height: 8; width: delegate.width }

    Row {
        width: parent.width
        spacing: 8

        Column {
            Item {
                width: 4
                height: titleText.font.pixelSize / 4
            }

            Image {
                id: titleImage
                source: delegate.content
                width: Math.min(delegate.width / 2, sourceSize.width)
                fillMode: Image.PreserveAspectFit
            }
        }

        Text {
            id: titleText

            text: delegate.title.replace(/&#39;/g, "'")
            width: delegate.width - titleImage.width
            wrapMode: Text.WordWrap
            font.pixelSize: 26
            font.bold: true
        }
    }

    Text {
        width: delegate.width
        font.pixelSize: 12
        textFormat: Text.RichText
        font.italic: true
        text: delegate.timeSinceEvent(delegate.pubDate) + " (<a href=\"" + delegate.link + "\">Link</a>)"
        onLinkActivated: function(link) {
            Qt.openUrlExternally(link)
        }
    }
}
