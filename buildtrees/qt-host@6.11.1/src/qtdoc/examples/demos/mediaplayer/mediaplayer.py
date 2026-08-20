# Copyright (C) 2023 The Qt Company Ltd.
# SPDX-License-Identifier: LicenseRef-Qt-Commercial OR BSD-3-Clause

import os
import sys
from argparse import ArgumentParser, RawTextHelpFormatter
from pathlib import Path

from PySide6.QtCore import QDir, QFileInfo, QObject, QUrl, Slot
from PySide6.QtGui import QGuiApplication
from PySide6.QtQml import QQmlApplicationEngine, QmlElement, QmlSingleton
from PySide6.QtMultimedia import QMediaFormat

_opt_verbose = False


QML_IMPORT_NAME = "io.qt.filenameprovider"
QML_IMPORT_MAJOR_VERSION = 1


@QmlElement
@QmlSingleton
class FileNameProvider(QObject):
    def __init__(self, p=None):
        super().__init__(p)

    @Slot(str, result=str)
    def getFileName(self, p):
        return QFileInfo(p).fileName()


def nameFilters():
    """Create a tuple of name filters/preferred index for populating the
       open file dialog."""
    result = []
    preferredFilter = ""
    formats = QMediaFormat().supportedFileFormats(QMediaFormat.Decode)
    for m, format in enumerate(formats):
        mediaFormat = QMediaFormat(format)
        mimeType = mediaFormat.mimeType()
        if mimeType.isValid():
            filter = QMediaFormat.fileFormatDescription(format) + " ("
            for i, suffix in enumerate(mimeType.suffixes()):
                if i:
                    filter += ' '
                filter += "*." + suffix
            filter += ')'
            result.append(filter)
            if mimeType.name() == "video/mp4":
                preferredFilter = filter
    result.sort()
    preferred = result.index(preferredFilter) if preferredFilter else 0
    return (result, preferred)


if __name__ == "__main__":
    argument_parser = ArgumentParser(description="Media Player",
                                     formatter_class=RawTextHelpFormatter)
    argument_parser.add_argument("-v", "--verbose", action="store_true",
                                 help="Generate more output")
    argument_parser.add_argument("url", help="The URL to open", nargs='?',
                                 type=str)
    options = argument_parser.parse_args()
    _opt_verbose = options.verbose

    if _opt_verbose:
        os.environ["QML_IMPORT_TRACE"] = "1"

    source = QUrl.fromUserInput(options.url, QDir.currentPath()) if options.url else QUrl()

    app = QGuiApplication(sys.argv)
    engine = QQmlApplicationEngine()
    app_dir = Path(__file__).parent
    app_dir_url = QUrl.fromLocalFile(os.fspath(app_dir))
    engine.addImportPath(os.fspath(app_dir))
    nameFilterList, selectedNameFilter = nameFilters()
    engine.setInitialProperties({
        "source": source,
        "nameFilters": nameFilterList,
        "selectedNameFilter": selectedNameFilter})
    engine.loadFromModule("MediaPlayer", "Main")

    if not engine.rootObjects():
        sys.exit(-1)
    sys.exit(app.exec())
