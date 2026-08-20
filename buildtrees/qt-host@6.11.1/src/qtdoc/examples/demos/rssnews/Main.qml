// Copyright (C) 2017 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR BSD-3-Clause

pragma ComponentBehavior: Bound

import QtQuick
import QtQuick.Window
import QtQml.XmlListModel

Rectangle {
    id: window

    width: 800
    height: 480

    property string currentFeed: rssFeeds.get(0).feed
    property bool loading: feedModel.status === XmlListModel.Loading
    property bool isPortrait: Screen.primaryOrientation === Qt.PortraitOrientation

    onLoadingChanged: {
        if (feedModel.status == XmlListModel.Ready)
            list.positionViewAtBeginning()
    }

    RssFeeds { id: rssFeeds }

    XmlListModel {
        id: feedModel

        source: "https://" + window.currentFeed
        query: "/rss/channel/item"

        XmlListModelRole { name: "title"; elementName: "title"; attributeName: ""}
        XmlListModelRole { name: "content"; elementName: "content"; attributeName: "url" }
        XmlListModelRole { name: "link"; elementName: "link"; attributeName: "" }
        XmlListModelRole { name: "pubDate"; elementName: "pubDate"; attributeName: "" }
    }

    ListView {
        id: categories
        property int itemWidth: 190

        width: window.isPortrait ? parent.width : itemWidth
        height: window.isPortrait ? itemWidth : parent.height
        orientation: window.isPortrait ? ListView.Horizontal : ListView.Vertical
        anchors.top: parent.top
        model: rssFeeds
        delegate: CategoryDelegate {
            itemSize: categories.itemWidth
            isLoading: window.loading
            onClicked: function () {
                if (window.currentFeed == feed)
                    feedModel.reload()
                else
                    window.currentFeed = feed

            }
        }
        spacing: 3
    }

    ScrollBar {
        id: listScrollBar

        orientation: window.isPortrait ? Qt.Horizontal : Qt.Vertical
        height: window.isPortrait ? 8 : categories.height;
        width: window.isPortrait ? categories.width : 8
        scrollArea: categories;
        anchors.right: categories.right
    }

    ListView {
        id: list

        anchors.left: window.isPortrait ? window.left : categories.right
        anchors.right: closeButton.left
        anchors.top: window.isPortrait ? categories.bottom : window.top
        anchors.bottom: window.bottom
        anchors.leftMargin: 30
        anchors.rightMargin: 4
        clip: window.isPortrait
        model: feedModel
        footer: footerText
        delegate: NewsDelegate {}
    }

    ScrollBar {
        scrollArea: list
        width: 8
        anchors.right: window.right
        anchors.top: window.isPortrait ? categories.bottom : window.top
        anchors.bottom: window.bottom
    }

    Component {
        id: footerText

        Rectangle {
            width: parent.width
            height: closeButton.height
            color: "lightgray"

            Text {
                text: "RSS Feed from Yahoo News"
                anchors.centerIn: parent
                font.pixelSize: 14
            }
        }
    }


    Image {
        id: closeButton
        source: "content/images/btn_close.png"
        scale: 0.8
        anchors.top: parent.top
        anchors.right: parent.right
        anchors.margins: 4
        opacity: (window.isPortrait && categories.moving) ? 0.2 : 1.0
        Behavior on opacity {
            NumberAnimation { duration: 300; easing.type: Easing.OutSine }
        }

        MouseArea {
            anchors.fill: parent
            onClicked: {
                Qt.quit()
            }
        }
    }
}
