// Copyright (C) 2021 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR GPL-3.0-only

#include "typelibs/ieframe.h"

int main(int argc, char *argv[])
{
    SHDocVw::WebBrowser* webBrowser = new SHDocVw::WebBrowser;
    delete webBrowser;
    return 0;
}
