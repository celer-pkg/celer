// Copyright (C) 2023 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR BSD-3-Clause

pragma ComponentBehavior: Bound

import QtQuick
import QtQuick.Controls.Fusion
import QtQuick.Dialogs
import QtQuick.Layouts
import QtCore
import MediaControls
import Config
import io.qt.filenameprovider

Rectangle {
    id: root

    implicitWidth: 380
    color: Config.mainColor
    border.color: "lightgrey"
    radius: 10

    property int currentIndex: -1
    property bool isShuffled: false
    property alias mediaCount: files.count
    signal playlistUpdated()
    signal currentFileRemoved()

    function getSource() {
        if (isShuffled && mediaCount > 1) {
            let randomIndex = Math.floor(Math.random() * mediaCount)
            while (randomIndex == currentIndex) {
                randomIndex = Math.floor(Math.random() * mediaCount)
            }
            currentIndex = randomIndex
        }
        return files.get(currentIndex).path
    }

    function addFiles(index, selectedFiles) {
        selectedFiles.forEach(function (file){
            const url = new URL(file)
            files.insert(index,
                {
                    path: url,
                    isMovie: isMovie(FileNameProvider.getFileName(url.toString()))
                })
        })
        playlistUpdated()
    }

    function addFile(index, selectedFile) {
        if (index > mediaCount || index < 0) {
            index = 0
            currentIndex = 0
        }
        files.insert(index,
            {
                path: selectedFile,
                isMovie: isMovie(FileNameProvider.getFileName(selectedFile.toString()))
            })

    }

    function isMovie(path) {
        const paths = path.split('.')
        const extension = paths[paths.length - 1]
        const musicFormats = ["mp3", "wav", "aac"]
        for (const format of musicFormats) {
            if (format === extension) {
                return false
            }
        }
        return true
    }

    MouseArea {
        anchors.fill: root
        preventStealing: true
    }

    FileDialog {
        id: folderView
        title: qsTr("Add files to playlist")
        currentFolder: StandardPaths.standardLocations(StandardPaths.MoviesLocation)[0]
        fileMode: FileDialog.OpenFiles
        onAccepted: {
            root.addFiles(files.count, folderView.selectedFiles)
            close()
        }
    }

    ListModel {
        id: files
    }

    Item {
        id: playlist
        anchors.fill: root
        anchors.margins: 30

        RowLayout {
            id: header
            width: playlist.width

            Label {
                font.bold: true
                font.pixelSize: 20
                text: qsTr("Playlist")
                color: Config.secondaryColor

                Layout.fillWidth: true
            }

            CustomButton {
                icon.source: ControlImages.iconSource("Add_file")
                onClicked: folderView.open()
            }
        }

        ListView {
            id: listView
            model: files
            anchors.fill: playlist
            anchors.topMargin: header.height + 30
            spacing: 20

            delegate: RowLayout {
                id: row
                width: listView.width
                spacing: 15

                required property string path
                required property int index
                required property bool isMovie

                Image {
                    id: mediaIcon

                    states: [
                        State {
                            name: "activeMovie"
                            when: root.currentIndex === row.index && row.isMovie
                            PropertyChanges {
                                mediaIcon.source: Images.iconSource("Movie_Active", false)
                            }
                        },
                        State {
                            name: "inactiveMovie"
                            when: root.currentIndex !== row.index && row.isMovie
                            PropertyChanges {
                                mediaIcon.source: Images.iconSource("Movie_Icon")
                            }
                        },
                        State {
                            name: "activeMusic"
                            when: root.currentIndex === row.index && !row.isMovie
                            PropertyChanges {
                                mediaIcon.source: Images.iconSource("Music_Active", false)
                            }
                        },
                        State {
                            name: "inactiveMusic"
                            when: root.currentIndex !== row.index && !row.isMovie
                            PropertyChanges {
                                mediaIcon.source: Images.iconSource("Music_Icon")
                            }
                        }
                    ]
                }

                Label {
                    Layout.fillWidth: true
                    elide: Text.ElideRight
                    font.bold: root.currentIndex === row.index
                    color: root.currentIndex === row.index ? Config.highlightColor : Config.secondaryColor
                    font.pixelSize: 18
                    text: {
                        return FileNameProvider.getFileName(row.path)
                    }
                }

                CustomButton {
                    icon.source: ControlImages.iconSource("Trash_Icon")
                    onClicked: {
                        const removedIndex = row.index
                        files.remove(row.index)
                        if (root.currentIndex === removedIndex) {
                            root.currentFileRemoved()
                        } else if (root.currentIndex > removedIndex) {
                            --root.currentIndex
                        }
                    }
                }
            }

            remove: Transition {
                NumberAnimation {
                    property: "opacity"
                    from: 1.0
                    to: 0.0
                    duration: 400
                }
            }

            add: Transition {
                NumberAnimation {
                    property: "opacity"
                    from: 0.0
                    to: 1.0
                    duration: 400
                }
                NumberAnimation {
                    property: "scale"
                    from: 0.5
                    to: 1.0
                    duration: 400
                }
            }

            displaced: Transition {
                NumberAnimation {
                    properties: "y"
                    duration: 600
                    easing.type: Easing.OutBounce
                }
            }
        }
    }
}
