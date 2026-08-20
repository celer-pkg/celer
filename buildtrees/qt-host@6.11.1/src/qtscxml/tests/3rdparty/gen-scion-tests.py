#!/usr/bin/python
# Copyright (C) 2016 The Qt Company Ltd.
# SPDX-License-Identifier: LicenseRef-Qt-Commercial OR GPL-3.0-only WITH Qt-GPL-exception-1.0
from os.path import isfile, join, splitext

f = open("scion.qrc", "w")
f.write("<!DOCTYPE RCC><RCC version=\"1.0\">\n<qresource>\n")

g = open("scion.h","w")
g.write("const char *testBases[] = {")

first = True
mypath = "scion-tests/scxml-test-framework/test"
for root, _, filenames in walk(mypath):
    for filename in filenames:
        if filename.endswith(".scxml"):
            base = join(root,splitext(filename)[0])
            json = base+".json"
            if isfile(json):
                f.write("<file>")
                f.write(join(root,filename))
                f.write("</file>\n")
                f.write("<file>")
                f.write(json)
                f.write("</file>\n")
                if first:
                    first = False
                else:
                    g.write(",")
                g.write("\n    \"" + base + "\"")

f.write("</qresource></RCC>\n")
f.close()

g.write("\n};\n")
g.close()

