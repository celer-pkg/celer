# Copyright (C) 2022 The Qt Company Ltd.
# SPDX-License-Identifier: LicenseRef-Qt-Commercial OR BSD-3-Clause

#! [import_plugins]
qt_import_plugins(myapp INCLUDE Qt::QLibInputPlugin)
#! [import_plugins]

#! [import_minimal_plugin]
qt_import_plugins(myapp
    INCLUDE_BY_TYPE platforms Qt::MinimalIntegrationPlugin
)
#! [import_minimal_plugin]

#! [some_imageformat_plugins]
qt_import_plugins(myapp
    INCLUDE_BY_TYPE imageformats Qt::QJpegPlugin Qt::QGifPlugin
)
#! [some_imageformat_plugins]

#! [no_imageformats_plugins]
qt_import_plugins(myapp
    EXCLUDE_BY_TYPE imageformats
)
#! [no_imageformats_plugins]
