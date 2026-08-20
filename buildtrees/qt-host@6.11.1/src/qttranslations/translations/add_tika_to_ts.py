# !/usr/bin/env python3
# Copyright (C) 2024 The Qt Company Ltd.
# SPDX-License-Identifier: LicenseRef-Qt-Commercial OR GPL-3.0-only

# This script generates a translation file (tika_for_ts.ts) containing
# mimetypes descriptions extracted from tika-mimetypes.xml file.
# It organizes all descriptions under a single context named QMimeType.
# The generated .ts file is then used during "make lupdate" time to
# integrate these entries with other translation files.

import argparse
import xml.etree.ElementTree as XML_ET
from xml.dom import minidom

def parse_tika_mime_types(tika_file):
    tree = XML_ET.parse(tika_file)
    root = tree.getroot()

    for mime in root.findall(".//mime-type"):
        mime_type = mime.get("type")
        if mime_type:
            comment_element = mime.find("comment")
            if comment_element is not None and comment_element.text:
                yield comment_element.text.strip(), mime_type

def create_ts_file_with_mime_types(mime_types, output_ts_file):
    ts_root = XML_ET.Element("TS")
    ts_root.set("version", "2.1")
    context = XML_ET.SubElement(ts_root, "context")
    name = XML_ET.SubElement(context, "name")
    name.text = "QMimeType"

    for comment, mime_type in mime_types:
        message = XML_ET.SubElement(context, "message")
        source = XML_ET.SubElement(message, "source")
        source.text = comment
        extracomment = XML_ET.SubElement(message, "extracomment")
        extracomment.text = mime_type

    tika_ts = XML_ET.tostring(ts_root, encoding="utf-8", method="xml")
    final_tika = minidom.parseString(tika_ts).toprettyxml(indent="  ")
    header = '<?xml version="1.0" encoding="utf-8"?>\n<!DOCTYPE TS>\n'
    final_tika = header + final_tika.split("\n", 1)[1]

    with open(output_ts_file, "w", encoding="utf-8") as f:
        f.write(final_tika)

if __name__ == "__main__":
    parser = argparse.ArgumentParser()
    parser.add_argument('tika_file')
    parser.add_argument('output_ts_file')
    args = parser.parse_args()
    print(f"Generating {args.output_ts_file} from {args.tika_file}...")
    mime_types = list(parse_tika_mime_types(args.tika_file))
    create_ts_file_with_mime_types(mime_types, args.output_ts_file)
    print(f"File {args.output_ts_file} created successfully.")
